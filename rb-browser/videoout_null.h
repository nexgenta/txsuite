/*
 * videoout_null.h
 */

#ifndef __VIDEOOUT_NULL_H__
#define __VIDEOOUT_NULL_H__

#include <X11/Xlib.h>

typedef struct
{
	GC gc;				/* GC to draw on the content Pixmap */
	unsigned int width;		/* size to output blank frame */
	unsigned int height;
} vo_null_ctx;

extern MHEGVideoOutputMethod vo_null_fns;

#endif	/* __VIDEOOUT_NULL_H__ */
