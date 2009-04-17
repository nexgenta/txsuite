/*
 * MHEGBitmap.h
 */

#ifndef __MHEGBITMAP_H__
#define __MHEGBITMAP_H__

#include <X11/X.h>
#include <X11/extensions/Xrender.h>

typedef struct
{
	Pixmap image;		/* the Bitmap image */
	Picture image_pic;	/* XRender wrapper for the image */
} MHEGBitmap;

#endif	/* __MHEGBITMAP_H__ */

