/*
 * mpegts.h
 */

#ifndef __MPEGTS_H__
#define __MPEGTS_H__

/* stream types we care about */
#define STREAM_TYPE_VIDEO_MPEG1		0x01
#define STREAM_TYPE_VIDEO_MPEG2		0x02
#define STREAM_TYPE_AUDIO_MPEG1		0x03
#define STREAM_TYPE_AUDIO_MPEG2		0x04
#define STREAM_TYPE_PRIVATE_DATA	0x06
#define STREAM_TYPE_AUDIO_AAC		0x0f
#define STREAM_TYPE_VIDEO_MPEG4		0x10
#define STREAM_TYPE_VIDEO_H264		0x1b
#define STREAM_TYPE_AUDIO_AC3		0x81
#define STREAM_TYPE_AUDIO_DTS		0x8a

typedef struct MpegTSContext MpegTSContext;

MpegTSContext *mpegts_open(FILE *, int, int);
int mpegts_demux_frame(MpegTSContext *, AVPacket *);
int mpegts_demux_packet(MpegTSContext *, AVPacket *);
void mpegts_close(MpegTSContext *);

#endif	/* __MPEGTS_H__ */
