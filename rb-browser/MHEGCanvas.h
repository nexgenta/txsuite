/*
 * MHEGCanvas.h
 */

#ifndef __MHEGCANVAS_H__
#define __MHEGCANVAS_H__

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

typedef struct
{
	unsigned int width;		/* in pixels, will be the scaled up value in fullscreen mode */
	unsigned int height;		/* in pixels, will be the scaled up value in fullscreen mode */
	unsigned int border;		/* border width in pixels (the scaled value in fullscreen mode) */
	Pixmap contents;		/* current image */
	Picture contents_pic;		/* XRender wrapper */
	XRenderPictFormat *pic_format;	/* pixel format */
	GC gc;				/* contains the clip mask for the border */
} MHEGCanvas;

MHEGCanvas *new_MHEGCanvas(unsigned int, unsigned int);
void free_MHEGCanvas(MHEGCanvas *);

void MHEGCanvas_setBorder(MHEGCanvas *, int, int, MHEGColour *);

void MHEGCanvas_clear(MHEGCanvas *, MHEGColour *);
void MHEGCanvas_drawArc(MHEGCanvas *, XYPosition *, OriginalBoxSize *, int, int, int, int, MHEGColour *);
void MHEGCanvas_drawSector(MHEGCanvas *, XYPosition *, OriginalBoxSize *, int, int, int, int, MHEGColour *, MHEGColour *);
void MHEGCanvas_drawLine(MHEGCanvas *, XYPosition *, XYPosition *, int, int, MHEGColour *);
void MHEGCanvas_drawOval(MHEGCanvas *, XYPosition *, OriginalBoxSize *, int, int, MHEGColour *, MHEGColour *);
void MHEGCanvas_drawPolygon(MHEGCanvas *, LIST_OF(XYPosition) *, int, int, MHEGColour *, MHEGColour *);
void MHEGCanvas_drawPolyline(MHEGCanvas *, LIST_OF(XYPosition) *, int, int, MHEGColour *);
void MHEGCanvas_drawRectangle(MHEGCanvas *, XYPosition *, OriginalBoxSize *, int, int, MHEGColour *, MHEGColour *);

#endif	/* __MHEGCANVAS_H__ */

