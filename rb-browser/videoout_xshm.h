/*
 * videoout_xshm.h
 */

#ifndef __VIDEOOUT_XSHM_H__
#define __VIDEOOUT_XSHM_H__

#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <ffmpeg/avcodec.h>

typedef struct
{
	unsigned int width;
	unsigned int height;
} FrameSize;

typedef struct
{
	XImage *current_frame;			/* frame we are currently displaying */
	XShmSegmentInfo shm;			/* shared memory used by current_frame */
	AVPicture rgb_frame;			/* ffmpeg wrapper for current_frame SHM data */
	enum PixelFormat out_format;		/* rgb_frame ffmpeg pixel format */
	ImgReSampleContext *resize_ctx;		/* NULL if we do not need to resize the frame */
	FrameSize resize_in;			/* resize_ctx input dimensions */
	FrameSize resize_out;			/* resize_ctx output dimensions */
	AVPicture resized_frame;		/* resized output frame */
	uint8_t *resized_data;			/* resized_frame data buffer */
} vo_xshm_ctx;

extern MHEGVideoOutputMethod vo_xshm_fns;

#endif	/* __VIDEOOUT_XSHM_H__ */
