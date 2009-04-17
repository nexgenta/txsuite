/*
 * MHEGStreamPlayer.h
 */

#ifndef __MHEGSTREAMPLAYER_H__
#define __MHEGSTREAMPLAYER_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/avcodec.h>

#include "ISO13522-MHEG-5.h"
#include "MHEGBackend.h"

/* seconds of video to buffer before we start playing it */
#define INIT_VIDEO_BUFFER_WAIT	1.0

/* seconds of audio to buffer before we start playing it (only used if we have no video) */
#define INIT_AUDIO_BUFFER_WAIT	1.0

/* list of decoded video frames to be displayed */
typedef struct
{
	double pts;			/* presentation time stamp */
	enum PixelFormat pix_fmt;
	unsigned int width;
	unsigned int height;
	AVPicture frame;
	unsigned char *frame_data;
	size_t nalloced;		/* number of bytes malloc'ed to frame_data */
} VideoFrame;

DEFINE_LIST_OF(VideoFrame);

LIST_TYPE(VideoFrame) *new_VideoFrameListItem(double, enum PixelFormat, unsigned int, unsigned int, AVFrame *);
void free_VideoFrameListItem(LIST_TYPE(VideoFrame) *);

/* list of decoded audio samples to play */
typedef struct
{
	double pts;			/* presentation time stamp */
	unsigned int size;		/* size of data in bytes (not uint16_t's) */
	uint16_t data[AVCODEC_MAX_AUDIO_FRAME_SIZE];
} AudioFrame;

DEFINE_LIST_OF(AudioFrame);

LIST_TYPE(AudioFrame) *new_AudioFrameListItem(void);
void free_AudioFrameListItem(LIST_TYPE(AudioFrame) *);

/* player state */
typedef struct
{
	bool playing;			/* true when our threads are active */
	bool stop;			/* true => stop playback */
	bool have_video;		/* false if we have no video stream */
	bool have_audio;		/* false if we have no audio stream */
	VideoClass *video;		/* output size/position, maybe NULL if audio only */
	AudioClass *audio;		/* output volume, maybe NULL if video only */
	int service_id;			/* service containing the audio/video components (-1 => what we are currently tuned to) */
	int video_tag;			/* video stream component tag (-1 => default for current service ID) */
	int video_pid;			/* PID in MPEG Transport Stream (-1 => not yet known) */
	int video_type;			/* video stream type (-1 => not yet known) */
	int audio_tag;			/* audio stream component tag (-1 => default for current service ID) */
	int audio_pid;			/* PID in MPEG Transport Stream (-1 => not yet known) */
	int audio_type;			/* audio stream type (-1 => not yet known) */
	AVCodecContext *audio_codec;	/* audio ouput params */
	MHEGStream *ts;			/* MPEG Transport Stream */
	pthread_t decode_tid;		/* thread decoding the MPEG stream into audio/video frames */
	pthread_t video_tid;		/* thread displaying video frames on the screen */
	pthread_t audio_tid;		/* thread feeding audio frames into the sound card */
	pthread_mutex_t base_lock;	/* used to sync audio and video */
	pthread_cond_t base_cond;	/* the video thread tells the audio thread: */
	double base_pts;		/* - the PTS of the first video frame */
	int64_t base_time;		/* - the time the first video frame was displayed */
	pthread_mutex_t videoq_lock;	/* list of decoded video frames */
	LIST_OF(VideoFrame) *videoq;	/* head of list is next to be displayed */
	pthread_mutex_t audioq_lock;	/* list of decoded audio frames */
	LIST_OF(AudioFrame) *audioq;	/* head of list is next to be played */
} MHEGStreamPlayer;

void MHEGStreamPlayer_init(MHEGStreamPlayer *);
void MHEGStreamPlayer_fini(MHEGStreamPlayer *);

void MHEGStreamPlayer_setServiceID(MHEGStreamPlayer *, int);
void MHEGStreamPlayer_setVideoStream(MHEGStreamPlayer *, VideoClass *);
void MHEGStreamPlayer_setAudioStream(MHEGStreamPlayer *, AudioClass *);

void MHEGStreamPlayer_play(MHEGStreamPlayer *);
void MHEGStreamPlayer_stop(MHEGStreamPlayer *);

#endif	/* __MHEGSTREAMPLAYER_H__ */
