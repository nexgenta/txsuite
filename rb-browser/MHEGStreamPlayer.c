/*
 * MHEGStreamPlayer.c
 */

#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "MHEGEngine.h"
#include "MHEGStreamPlayer.h"
#include "MHEGVideoOutput.h"
#include "MHEGAudioOutput.h"
#include "mpegts.h"
#include "utils.h"

/* internal routines */
static void *decode_thread(void *);
static void *video_thread(void *);
static void *audio_thread(void *);

static void set_avsync_base(MHEGStreamPlayer *, double, int64_t);

static void thread_usleep(unsigned long);
static enum CodecID find_av_codec_id(int);

/* global pool of spare VideoFrame's */
LIST_OF(VideoFrame) *free_vframes = NULL;
pthread_mutex_t free_vframes_lock = PTHREAD_MUTEX_INITIALIZER;

LIST_TYPE(VideoFrame) *
new_VideoFrameListItem(double pts, enum PixelFormat pix_fmt, unsigned int width, unsigned int height, AVFrame *frame)
{
	LIST_TYPE(VideoFrame) *vf;
	int frame_size;

	/* do we have a spare frame we can use */
	pthread_mutex_lock(&free_vframes_lock);
	if(free_vframes != NULL)
	{
		vf = free_vframes;
		LIST_REMOVE(&free_vframes, vf);
	}
	else
	{
		vf = safe_malloc(sizeof(LIST_TYPE(VideoFrame)));
		vf->item.frame_data = NULL;
		vf->item.nalloced = 0;
	}
	pthread_mutex_unlock(&free_vframes_lock);

	/* frame info */
	vf->item.pts = pts;
	vf->item.pix_fmt = pix_fmt;
	vf->item.width = width;
	vf->item.height = height;

	/*
	 * take a copy of the frame,
	 * the actual data is inside the video codec somewhere and will be overwritten by the next frame we decode
	 */
	if((frame_size = avpicture_get_size(pix_fmt, width, height)) < 0)
		fatal("Invalid frame_size");
	vf->item.frame_data = safe_fast_realloc(vf->item.frame_data, &vf->item.nalloced, frame_size);
	avpicture_fill(&vf->item.frame, vf->item.frame_data, pix_fmt, width, height);
	img_copy(&vf->item.frame, (AVPicture*) frame, pix_fmt, width, height);

	return vf;
}

void
free_VideoFrameListItem(LIST_TYPE(VideoFrame) *vf)
{
	/* add it to the free list */
	pthread_mutex_lock(&free_vframes_lock);
	LIST_APPEND(&free_vframes, vf);
	pthread_mutex_unlock(&free_vframes_lock);

	return;
}

/* global pool of spare AudioFrame's */
LIST_OF(AudioFrame) *free_aframes = NULL;
pthread_mutex_t free_aframes_lock = PTHREAD_MUTEX_INITIALIZER;

LIST_TYPE(AudioFrame) *
new_AudioFrameListItem(void)
{
	LIST_TYPE(AudioFrame) *af;

	/* do we have a spare frame we can use */
	pthread_mutex_lock(&free_aframes_lock);
	if(free_aframes != NULL)
	{
		af = free_aframes;
		LIST_REMOVE(&free_aframes, af);
	}
	else
	{
		af = safe_malloc(sizeof(LIST_TYPE(AudioFrame)));
	}
	pthread_mutex_unlock(&free_aframes_lock);

	af->item.pts = AV_NOPTS_VALUE;

	af->item.size = 0;

	return af;
}

void
free_AudioFrameListItem(LIST_TYPE(AudioFrame) *af)
{
	/* add it to the free list */
	pthread_mutex_lock(&free_aframes_lock);
	LIST_APPEND(&free_aframes, af);
	pthread_mutex_unlock(&free_aframes_lock);

	return;
}

void
MHEGStreamPlayer_init(MHEGStreamPlayer *p)
{
	bzero(p, sizeof(MHEGStreamPlayer));

	p->playing = false;
	p->stop = false;

	p->have_video = false;
	p->have_audio = false;

	p->video = NULL;
	p->audio = NULL;

	/* stream a/v components from the service we are currently tuned to */
	p->service_id = -1;

	p->audio_codec = NULL;

	pthread_mutex_init(&p->base_lock, NULL);
	pthread_cond_init(&p->base_cond, NULL);

	pthread_mutex_init(&p->videoq_lock, NULL);
	p->videoq = NULL;

	pthread_mutex_init(&p->audioq_lock, NULL);
	p->audioq = NULL;

	return;
}

void
MHEGStreamPlayer_fini(MHEGStreamPlayer *p)
{
	MHEGStreamPlayer_stop(p);

	pthread_mutex_destroy(&p->base_lock);
	pthread_cond_destroy(&p->base_cond);

	pthread_mutex_destroy(&p->videoq_lock);
	pthread_mutex_destroy(&p->audioq_lock);

	return;
}

/*
 * service ID is used to resolve the stream component tags
 * -1 => use the service we are currently tuned to,
 * ie the service we are downloading the carousel from
 */

void
MHEGStreamPlayer_setServiceID(MHEGStreamPlayer *p, int id)
{
	p->service_id = id;

	return;
}

void
MHEGStreamPlayer_setVideoStream(MHEGStreamPlayer *p, VideoClass *video)
{
	/* assert */
	if(p->playing)
		fatal("MHEGStreamPlayer_setVideoStream: trying to set stream while playing");

	if(video)
		verbose("MHEGStreamPlayer_setVideoStream: tag=%d", video->component_tag);
	else
		verbose("MHEGStreamPlayer_setVideoStream: NULL");

	/* video==NULL => forget any existing stream */
	if(video == NULL)
	{
		p->have_video = false;
		p->video = NULL;
		return;
	}

	if(p->have_video)
		error("MHEGStreamPlayer: more than one video stream; only using the last one (%d)", video->component_tag);

	p->have_video = true;

	/* output size/position */
	p->video = video;

	/* the backend will tell us the PID and stream type when we start streaming it */
	p->video_tag = video->component_tag;
	p->video_pid = -1;
	p->video_type = -1;

	return;
}

void
MHEGStreamPlayer_setAudioStream(MHEGStreamPlayer *p, AudioClass *audio)
{
	/* assert */
	if(p->playing)
		fatal("MHEGStreamPlayer_setAudioStream: trying to set stream while playing");

	if(audio)
		verbose("MHEGStreamPlayer_setAudioStream: tag=%d", audio->component_tag);
	else
		verbose("MHEGStreamPlayer_setAudioStream: NULL");

	/* audio==NULL => forget any existing stream */
	if(audio == NULL)
	{
		p->have_audio = false;
		p->audio = NULL;
		p->audio_codec = NULL;
		return;
	}

	if(p->have_audio)
		error("MHEGStreamPlayer: more than one audio stream; only using the last one (%d)", audio->component_tag);

	p->have_audio = true;

	/* volume */
	p->audio = audio;

	/* the backend will tell us the PID and stream type when we start streaming it */
	p->audio_tag = audio->component_tag;
	p->audio_pid = -1;
	p->audio_type = -1;

	p->audio_codec = NULL;

	return;
}

void
MHEGStreamPlayer_play(MHEGStreamPlayer *p)
{
	verbose("MHEGStreamPlayer_play: service_id=%d audio_tag=%d video_tag=%d", p->service_id, p->audio_tag, p->video_tag);

	/* make sure the VideoClass doesn't try to draw anything yet */
	if(p->video != NULL)
		p->video->inst.no_video = true;

	/* is audio/video output totally disabled */
	if(MHEGEngine_avDisabled()
	|| (!p->have_video && !p->have_audio))
		return;

	p->audio_pid = p->audio_tag;
	p->video_pid = p->video_tag;
	if((p->ts = MHEGEngine_openStream(p->service_id,
					  p->have_audio, &p->audio_pid, &p->audio_type,
					  p->have_video, &p->video_pid, &p->video_type)) == NULL)
	{
		error("Unable to open MPEG stream (%d, %d, %d)", p->service_id, p->audio_tag, p->video_tag);
		return;
	}

	/* let the VideoClass know we now have a video stream */
	if(p->video != NULL)
		p->video->inst.no_video = false;

	p->playing = true;
	p->stop = false;

	/*
	 * the MPEG type for some streams is set to 6 (STREAM_TYPE_PRIVATE_DATA)
	 * eg the streams for BBC News Multiscreen
	 * presumably, this is stop them getting accidentally picked up when you scan for channels
	 * luckily we know what type MPEG audio and video streams should be
	 */
	if(p->have_video && p->video_type == STREAM_TYPE_PRIVATE_DATA)
		p->video_type = STREAM_TYPE_VIDEO_MPEG2;
	if(p->have_audio && p->audio_type == STREAM_TYPE_PRIVATE_DATA)
		p->audio_type = STREAM_TYPE_AUDIO_MPEG2;

	/*
	 * we have three threads:
	 * decode_thread reads MPEG data from the TS and decodes it into YUV video frames and audio samples
	 * video_thread takes YUV frames off the videoq list, converts them to RGB and displays them on the screen
	 * audio_thread takes audio samples off the audioq list and feeds them into the sound card
	 */
	if(pthread_create(&p->decode_tid, NULL, decode_thread, p) != 0)
		fatal("Unable to create MPEG decoder thread");

	if(pthread_create(&p->video_tid, NULL, video_thread, p) != 0)
		fatal("Unable to create video output thread");

	if(pthread_create(&p->audio_tid, NULL, audio_thread, p) != 0)
		fatal("Unable to create audio output thread");

//{
// whole machine locks up if you do this
//	struct sched_param sp = {1};
//	if(pthread_setschedparam(p->video_tid, SCHED_RR, &sp) != 0)
//		error("MHEGStreamPlayer: unable to give video thread realtime priority");
//}

	return;
}

void
MHEGStreamPlayer_stop(MHEGStreamPlayer *p)
{
	verbose("MHEGStreamPlayer_stop");

	/* are we playing */
	if(!p->playing)
		return;

	/* signal the threads to stop */
	p->stop = true;

	/* wait for them to finish */
	pthread_join(p->decode_tid, NULL);
	pthread_join(p->video_tid, NULL);
	pthread_join(p->audio_tid, NULL);

	/* clean up */
	LIST_FREE(&p->videoq, VideoFrame, free_VideoFrameListItem);
	LIST_FREE(&p->audioq, AudioFrame, free_AudioFrameListItem);

	if(p->ts != NULL)
	{
		MHEGEngine_closeStream(p->ts);
		p->ts = NULL;
	}

	p->playing = false;

	return;
}

/*
 * decode_thread
 * reads the MPEG TS file
 * decodes the data into YUV video frames and audio samples
 * adds them to the tail of the videoq and audioq lists
 */

static void *
decode_thread(void *arg)
{
	MHEGStreamPlayer *p = (MHEGStreamPlayer *) arg;
	int demux_apid;
	int demux_vpid;
	MpegTSContext *tsdemux;
	AVPacket pkt;
	AVCodecContext *audio_codec_ctx = NULL;
	AVCodecContext *video_codec_ctx = NULL;
	enum CodecID codec_id;
	AVCodec *codec = NULL;
	double video_time_base = 90000.0;
	double audio_time_base = 90000.0;
	double pts;
	AVFrame *frame;
	LIST_TYPE(VideoFrame) *video_frame;
	int got_picture;
	LIST_TYPE(AudioFrame) *audio_frame;
	AudioFrame *af;
	int used;
	unsigned char *data;
	int size;

	verbose("MHEGStreamPlayer: decode thread started");

	if(p->have_video && p->video_pid != -1)
	{
		if((video_codec_ctx = avcodec_alloc_context()) == NULL)
			fatal("Out of memory");
		if((codec_id = find_av_codec_id(p->video_type)) == CODEC_ID_NONE
		|| (codec = avcodec_find_decoder(codec_id)) == NULL)
			fatal("Unsupported video codec");
		if(avcodec_open(video_codec_ctx, codec) < 0)
			fatal("Unable to open video codec");
		verbose("MHEGStreamPlayer: Video: stream type=%d codec=%s", p->video_type, codec->name);
	}

	if(p->have_audio && p->audio_pid != -1)
	{
		if((audio_codec_ctx = avcodec_alloc_context()) == NULL)
			fatal("Out of memory");
		if((codec_id = find_av_codec_id(p->audio_type)) == CODEC_ID_NONE
		|| (codec = avcodec_find_decoder(codec_id)) == NULL)
			fatal("Unsupported audio codec");
		if(avcodec_open(audio_codec_ctx, codec) < 0)
			fatal("Unable to open audio codec");
		verbose("MHEGStreamPlayer: Audio: stream type=%d codec=%s", p->audio_type, codec->name);
		/* let the audio ouput thread know what the sample rate, etc are */
		p->audio_codec = audio_codec_ctx;
	}

	if((frame = avcodec_alloc_frame()) == NULL)
		fatal("Out of memory");

	demux_apid = p->have_audio ? p->audio_pid : -1;
	demux_vpid = p->have_video ? p->video_pid : -1;
	if((tsdemux = mpegts_open(p->ts->ts, demux_apid, demux_vpid)) == NULL)
		fatal("Out of memory");

	while(!p->stop && !feof(p->ts->ts))
	{
		/* get the next complete packet for one of the streams */
		if(mpegts_demux_frame(tsdemux, &pkt) < 0)
			continue;
		/* see what stream we got a packet for */
		if(p->have_audio && pkt.stream_index == p->audio_pid && pkt.pts != AV_NOPTS_VALUE)
		{
			pts = pkt.pts / audio_time_base;
			data = pkt.data;
			size = pkt.size;
			while(size > 0)
			{
				audio_frame = new_AudioFrameListItem();
				af = &audio_frame->item;
				used = avcodec_decode_audio(audio_codec_ctx, af->data, &af->size, data, size);
				data += used;
				size -= used;
				if(af->size > 0)
				{
					af->pts = pts;
					/* 16 or 32-bit samples, but af->size is in bytes */
					if(audio_codec_ctx->sample_fmt == SAMPLE_FMT_S16)
						pts += (af->size / 2.0) / (audio_codec_ctx->channels * audio_codec_ctx->sample_rate);
					else if(audio_codec_ctx->sample_fmt == SAMPLE_FMT_S32)
						pts += (af->size / 4.0) / (audio_codec_ctx->channels * audio_codec_ctx->sample_rate);
					else
						fatal("Unsupported audio sample format (%d)", audio_codec_ctx->sample_fmt);
					pthread_mutex_lock(&p->audioq_lock);
					LIST_APPEND(&p->audioq, audio_frame);
					pthread_mutex_unlock(&p->audioq_lock);
				}
				else
				{
					free_AudioFrameListItem(audio_frame);
				}
			}
			/* don't want one thread hogging the CPU time */
			pthread_yield();
		}
		else if(p->have_video && pkt.stream_index == p->video_pid && pkt.dts != AV_NOPTS_VALUE)
		{
			(void) avcodec_decode_video(video_codec_ctx, frame, &got_picture, pkt.data, pkt.size);
			if(got_picture)
			{
				pts = pkt.dts / video_time_base;
				video_frame = new_VideoFrameListItem(pts, video_codec_ctx->pix_fmt, video_codec_ctx->width, video_codec_ctx->height, frame);
				pthread_mutex_lock(&p->videoq_lock);
				LIST_APPEND(&p->videoq, video_frame);
				pthread_mutex_unlock(&p->videoq_lock);
				/* don't want one thread hogging the CPU time */
				pthread_yield();
			}
		}
		else
		{
			verbose("MHEGStreamPlayer: decoder got unexpected/untimed packet");
		}
		av_free_packet(&pkt);
	}

	/* clean up */
	mpegts_close(tsdemux);

	av_free(frame);

	if(video_codec_ctx != NULL)
		avcodec_close(video_codec_ctx);
	if(audio_codec_ctx != NULL)
		avcodec_close(audio_codec_ctx);

	verbose("MHEGStreamPlayer: decode thread stopped");

	return NULL;
}

/*
 * video_thread
 * takes YUV frames off the videoq list
 * scales them (if necessary) to fit the output size
 * converts them to RGB
 * waits for the correct time, then displays them on the screen
 */

static void *
video_thread(void *arg)
{
	MHEGStreamPlayer *p = (MHEGStreamPlayer *) arg;
	MHEGDisplay *d = MHEGEngine_getDisplay();
	MHEGVideoOutput vo;
	int out_x;
	int out_y;
	int off_x;
	int off_y;
	unsigned int out_width;
	unsigned int out_height;
	unsigned int vid_width;
	unsigned int vid_height;
	VideoFrame *vf;
	double buffered;
	double last_buffered;
	double last_pts;
	int64_t last_time, this_time, now;
	int usecs;
	bool drop_frame;
	unsigned int nframes = 0;

	if(!p->have_video)
		return NULL;

	verbose("MHEGStreamPlayer: video thread started");

	/* assert */
	if(p->video == NULL)
		fatal("video_thread: VideoClass is NULL");

	/* wait until we have some frames buffered up */
	last_buffered = -1.0;
	do
	{
		pthread_mutex_lock(&p->videoq_lock);
		if(p->videoq != NULL)
			buffered = p->videoq->prev->item.pts - p->videoq->item.pts;
		else
			buffered = 0.0;
		pthread_mutex_unlock(&p->videoq_lock);
		if(buffered != last_buffered)
			verbose("MHEGStreamPlayer: buffered %f seconds of video", buffered);
		last_buffered = buffered;
		/* let the decoder have a go */
		if(buffered < INIT_VIDEO_BUFFER_WAIT)
			pthread_yield();
	}
	while(!p->stop && buffered < INIT_VIDEO_BUFFER_WAIT);

	/* do we need to bomb out early */
	if(p->stop)
	{
		/* wake up the audio thread */
		if(p->have_audio)
			set_avsync_base(p, 0.0, 0);
		verbose("MHEGStreamPlayer: video thread stopped before any output");
		return NULL;
	}

	/* assert */
	if(p->videoq == NULL)
		fatal("video_thread: no frames!");

	/* initialise the video output method */
	MHEGVideoOutput_init(&vo, MHEGEngine_getVideoOutputMethod());

	/* the time that we displayed the previous frame */
	last_time = 0;
	last_pts = 0;

	/* until we are told to stop... */
	while(!p->stop)
	{
		/* get the next frame */
		pthread_mutex_lock(&p->videoq_lock);
		vf = (p->videoq != NULL) ? &p->videoq->item : NULL;
		/* only we delete items from the videoq, so vf will stay valid */
		pthread_mutex_unlock(&p->videoq_lock);
		if(vf == NULL)
		{
			verbose("MHEGStreamPlayer: videoq is empty");
			/* give the decoder a bit of time to catch up */
			pthread_yield();
			continue;
		}
		/*
		 * keep track of how many frames we've played
		 * just so the dropped frame verbose message below is more useful
		 * don't care if it wraps back to 0 again
		 */
		nframes ++;
		/* see if we should drop this frame or not */
		now = av_gettime();
		/* work out when this frame should be displayed based on when the last one was */
		if(last_time != 0)
			this_time = last_time + ((vf->pts - last_pts) * 1000000.0);
		else
			this_time = now;
		/* how many usecs do we need to wait */
		usecs = this_time - now;
		/*
		 * we've still got to convert it to RGB and maybe scale it too
		 * so don't bother allowing any error here
		 */
		drop_frame = (usecs < 0);
		if(drop_frame)
		{
			verbose("MHEGStreamPlayer: dropped video frame %u (usecs=%d)", nframes, usecs);
		}
		else
		{
			/* scale the next frame if necessary */
			pthread_mutex_lock(&p->video->inst.scaled_lock);
			/* use scaled values if ScaleVideo has been called */
			if(p->video->inst.scaled)
			{
				out_width = p->video->inst.scaled_width;
				out_height = p->video->inst.scaled_height;
			}
			else
			{
				out_width = vf->width;
				out_height = vf->height;
			}
			pthread_mutex_unlock(&p->video->inst.scaled_lock);
			/* scale up if fullscreen */
			out_width = MHEGDisplay_scaleX(d, out_width);
			out_height = MHEGDisplay_scaleY(d, out_height);
			MHEGVideoOutput_prepareFrame(&vo, vf, out_width, out_height);
			/* remember the PTS for this frame */
			last_pts = vf->pts;
			/* wait until it's time to display the frame */
			now = av_gettime();
			/* don't wait if this is the first frame */
			if(last_time != 0)
			{
				/* how many usecs do we need to wait */
				usecs = this_time - now;
				if(usecs > 0)
					thread_usleep(usecs);
				/* remember when we should have displayed this frame */
				last_time = this_time;
			}
			else	/* first frame */
			{
				/* remember when we displayed this frame */
				last_time = now;
				/* tell the audio thread what the PTS and real time are for the first video frame */
				if(p->have_audio)
					set_avsync_base(p, last_pts, last_time);
			}
			/* origin and size of VideoClass */
			pthread_mutex_lock(&p->video->inst.bbox_lock);
			out_x = p->video->inst.Position.x_position;
			out_y = p->video->inst.Position.y_position;
			vid_width = p->video->inst.BoxSize.x_length;
			vid_height = p->video->inst.BoxSize.y_length;
			/* VideoDecodeOffset position */
			off_x = p->video->inst.VideoDecodeOffset.x_position;
			off_y = p->video->inst.VideoDecodeOffset.y_position;
			pthread_mutex_unlock(&p->video->inst.bbox_lock);
			/* scale if fullscreen */
			out_x = MHEGDisplay_scaleX(d, out_x);
			out_y = MHEGDisplay_scaleY(d, out_y);
			vid_width = MHEGDisplay_scaleX(d, vid_width);
			vid_height = MHEGDisplay_scaleY(d, vid_height);
			off_x = MHEGDisplay_scaleX(d, off_x);
			off_y = MHEGDisplay_scaleY(d, off_y);
			/* if the frame is smaller or larger than the VideoClass, centre it */
			out_x += (vid_width - out_width) / 2;
			out_y += (vid_height - out_height) / 2;
			/* draw the current frame */
			MHEGVideoOutput_drawFrame(&vo, out_x + off_x, out_y + off_y);
			/* redraw objects above the video */
			pthread_mutex_lock(&p->video->inst.bbox_lock);
			MHEGDisplay_refresh(d, &p->video->inst.Position, &p->video->inst.BoxSize);
			pthread_mutex_unlock(&p->video->inst.bbox_lock);
			/* get it drawn straight away */
			XFlush(d->dpy);
		}
		/* we can delete the frame from the queue now */
		pthread_mutex_lock(&p->videoq_lock);
		LIST_FREE_HEAD(&p->videoq, VideoFrame, free_VideoFrameListItem);
		pthread_mutex_unlock(&p->videoq_lock);
		/* don't want one thread hogging the CPU time */
		pthread_yield();
	}

	MHEGVideoOutput_fini(&vo);

	verbose("MHEGStreamPlayer: video thread stopped");

	return NULL;
}

/*
 * audio thread
 * takes audio samples off the audioq and feeds them into the sound card as fast as possible
 * MHEGAudioOuput_addSamples() will block while the sound card buffer is full
 */

static void *
audio_thread(void *arg)
{
	MHEGStreamPlayer *p = (MHEGStreamPlayer *) arg;
	MHEGAudioOutput ao;
	AudioFrame *af;
	double buffered;
	double base_pts;
	int64_t base_time;
	snd_pcm_format_t format = 0;	/* keep the compiler happy */
	unsigned int rate;
	unsigned int channels;
	bool done;
	int64_t now_time, next_time;
	double now_pts, next_pts;
	int usecs;
	double vpts = 0.0;

	if(!p->have_audio)
		return NULL;

	verbose("MHEGStreamPlayer: audio thread started");

	/* assert */
	if(p->audio == NULL)
		fatal("audio_thread: AudioClass is NULL");

	/* do we need to sync the audio with the video */
	if(p->have_video)
	{
		/* wait until the video thread tells us it has some frames buffered up */
		pthread_mutex_lock(&p->base_lock);
		pthread_cond_wait(&p->base_cond, &p->base_lock);
		pthread_mutex_unlock(&p->base_lock);
		/* video thread sets base_pts and base_time from the values for the first frame it displays */
		base_time = p->base_time;
		base_pts = p->base_pts;
		/* get rid of audio frames that we should have played already */
		done = false;
		while(!done)
		{
			/* do we need to bomb out early */
			if(p->stop)
			{
				verbose("MHEGStreamPlayer: audio thread stopped before any output");
				return NULL;
			}
			/* what PTS are we looking for */
			now_time = av_gettime();
			now_pts = base_pts + ((now_time - base_time) / 1000000.0);
			/* remove frames we should have played already */
			pthread_mutex_lock(&p->audioq_lock);
			while(p->audioq && p->audioq->item.pts < now_pts)
				LIST_FREE_HEAD(&p->audioq, AudioFrame, free_AudioFrameListItem);
			/* have we got the first audio sample to play yet */
			done = (p->audioq != NULL);
			pthread_mutex_unlock(&p->audioq_lock);
			if(!done)
				pthread_yield();
		}
		/* wait until it's time to play the first sample */
		pthread_mutex_lock(&p->audioq_lock);
		next_pts = p->audioq->item.pts;
		pthread_mutex_unlock(&p->audioq_lock);
		next_time = base_time + ((next_pts - base_pts) * 1000000.0);
		now_time = av_gettime();
		usecs = next_time - now_time;
		if(usecs > 0)
			thread_usleep(usecs);
	}
	else
	{
		/* wait until we have some audio frames buffered up */
		do
		{
			/* do we need to bomb out early */
			if(p->stop)
			{
				verbose("MHEGStreamPlayer: audio thread stopped before any output");
				return NULL;
			}
			/* see how many frames we have buffered so far */
			pthread_mutex_lock(&p->audioq_lock);
			if(p->audioq != NULL)
				buffered = p->audioq->prev->item.pts - p->audioq->item.pts;
			else
				buffered = 0.0;
			pthread_mutex_unlock(&p->audioq_lock);
			verbose("MHEGStreamPlayer: buffered %f seconds of audio", buffered);
			/* let the decoder have a go */
			if(buffered < INIT_AUDIO_BUFFER_WAIT)
				pthread_yield();
		}
		while(buffered < INIT_AUDIO_BUFFER_WAIT);
		/* the time that we played the first frame */
		base_time = av_gettime();
		pthread_mutex_lock(&p->audioq_lock);
		base_pts = p->audioq->item.pts;
		pthread_mutex_unlock(&p->audioq_lock);
	}

	/* in case the flag got set since we last checked */
	if(p->stop)
	{
		verbose("MHEGStreamPlayer: audio thread stopped before any output");
		return NULL;
	}

	/* even if this fails, we still need to consume the audioq */
	(void) MHEGAudioOutput_init(&ao);

	/* assert - if audioq is not empty then the codec cannot be NULL */
	if(p->audio_codec == NULL)
		fatal("audio_codec is NULL");

	/* TODO will these be big endian on a big endian machine? */
	if(p->audio_codec->sample_fmt == SAMPLE_FMT_S16)
		format = SND_PCM_FORMAT_S16_LE;
	else if(p->audio_codec->sample_fmt == SAMPLE_FMT_S32)
		format = SND_PCM_FORMAT_S32_LE;
	else
		fatal("Unsupported audio sample format (%d)", p->audio_codec->sample_fmt);

	rate = p->audio_codec->sample_rate;
	channels = p->audio_codec->channels;

	verbose("MHEGStreamPlayer: audio params: format=%d rate=%d channels=%d", format, rate, channels);

	(void) MHEGAudioOutput_setParams(&ao, format, rate, channels);

	/* until we are told to stop */
	while(!p->stop)
	{
		/* get the next audio frame */
		pthread_mutex_lock(&p->audioq_lock);
		af = (p->audioq != NULL) ? &p->audioq->item : NULL;
		/* only we delete items from the audioq, so af will stay valid */
		pthread_mutex_unlock(&p->audioq_lock);
		if(af == NULL)
		{
			/* if audio init failed, we will get this a lot */
//			verbose("MHEGStreamPlayer: audioq is empty");
			/* give the decoder a bit of time to catch up */
			pthread_yield();
			continue;
		}
/* TODO */
/* need to make sure pts is what we expect */
/* if we missed decoding a sample, play silence */
		/* if the audio is getting ahead, don't play this sample */
		pthread_mutex_lock(&p->videoq_lock);
		vpts = p->videoq ? p->videoq->item.pts : vpts;
		pthread_mutex_unlock(&p->videoq_lock);
		if(vpts <= af->pts)
		{
			/* this will block until the sound card can take the data */
			MHEGAudioOutput_addSamples(&ao, af->data, af->size);
		}
		else
		{
			verbose("MHEGStreamPlayer: dropped audio frame (%f ahead)", vpts - af->pts);
		}
/* TODO handle case when video is ahead */
		/* we can delete the frame from the queue now */
		pthread_mutex_lock(&p->audioq_lock);
		LIST_FREE_HEAD(&p->audioq, AudioFrame, free_AudioFrameListItem);
		pthread_mutex_unlock(&p->audioq_lock);
	}

	MHEGAudioOutput_fini(&ao);

	verbose("MHEGStreamPlayer: audio thread stopped");

	return NULL;
}

/*
 * set the base_pts and base_time values for the first video frame
 * signal the values have been set via the base_cond variable
 */

static void
set_avsync_base(MHEGStreamPlayer *p, double pts, int64_t realtime)
{
	pthread_mutex_lock(&p->base_lock);

	p->base_pts = pts;
	p->base_time = realtime;

	/* tell the audio thread we have set the values */
	pthread_cond_signal(&p->base_cond);

	pthread_mutex_unlock(&p->base_lock);

	return;
}

/*
 * usleep(usecs)
 * need to make sure the other threads get a go while we are sleeping
 */

static void
thread_usleep(unsigned long usecs)
{
	struct timespec ts;

	ts.tv_sec = (usecs / 1000000);
	ts.tv_nsec = (usecs % 1000000) * 1000;

	nanosleep(&ts, NULL);

	return;
}

/*
 * from libavformat/mpegts.c
 */

static enum CodecID
find_av_codec_id(int stream_type)
{
	enum CodecID codec_id;

	codec_id = CODEC_ID_NONE;
	switch(stream_type)
	{
	case STREAM_TYPE_AUDIO_MPEG1:
	case STREAM_TYPE_AUDIO_MPEG2:
		codec_id = CODEC_ID_MP3;
		break;

	case STREAM_TYPE_VIDEO_MPEG1:
	case STREAM_TYPE_VIDEO_MPEG2:
		codec_id = CODEC_ID_MPEG2VIDEO;
		break;

	case STREAM_TYPE_VIDEO_MPEG4:
		codec_id = CODEC_ID_MPEG4;
		break;

	case STREAM_TYPE_VIDEO_H264:
		codec_id = CODEC_ID_H264;
		break;

	case STREAM_TYPE_AUDIO_AAC:
		codec_id = CODEC_ID_AAC;
		break;

	case STREAM_TYPE_AUDIO_AC3:
		codec_id = CODEC_ID_AC3;
		break;

	case STREAM_TYPE_AUDIO_DTS:
		codec_id = CODEC_ID_DTS;
		break;

	default:
		break;
	}

	if(codec_id == CODEC_ID_NONE)
		error("Unsupported audio/video codec (MPEG stream type=%d)", stream_type);

	return codec_id;
}

