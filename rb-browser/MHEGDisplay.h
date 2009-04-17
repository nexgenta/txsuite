/*
 * MHEGDisplay.h
 */

#ifndef __MHEGDISPLAY_H__
#define __MHEGDISPLAY_H__

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/Xrender.h>

#include "MHEGColour.h"
#include "MHEGBitmap.h"

/* resolution defined in UK MHEG Profile */
#define MHEG_XRES	720
#define MHEG_YRES	576

/* tab stops are this number of pixels apart */
#define MHEG_TAB_WIDTH	45

#define WINDOW_TITLE	"RedButton"

/* Xt names */
#define APP_NAME	"RedButton"
#define APP_CLASS	"redButton"

/* keyboard mapping */
typedef struct
{
	KeySym x_key;			/* X Keysym value */
	unsigned int mheg_key;		/* MHEGKey_xxx value */
} MHEGKeyMapEntry;

typedef struct
{
	bool fullscreen;			/* -f cmd line flag */
	XtAppContext app;			/* Xt application context */
	Display *dpy;				/* X Display */
	Window win;				/* Window to display our Picture */
	unsigned int xres;			/* Window resolution in pixels */
	unsigned int yres;
	GC win_gc;				/* GC to draw on the Window */
	int depth;				/* colour depth of the Window */
	Visual *vis;				/* Visual (ie pixel format) used by the Window */
	Colormap cmap;				/* None, unless we needed to create a Colormap for our Visual */
	Pixmap contents;			/* current contents of the Window */
	Picture contents_pic;			/* XRender wrapper for the contents, this is what we composite on */
	Pixmap next_overlay;			/* all MHEG objects are drawn on next_overlay */
	Picture next_overlay_pic;		/* via this XRender wrapper */
	Pixmap used_overlay;			/* when MHEGDisplay_useOverlay() is called, next_overlay is copied here */
	Picture used_overlay_pic;		/* used_overlay_pic is composited onto the video */
	GC overlay_gc;				/* GC to XCopyArea next_overlay to used_overlay */
	Picture textfg_pic;			/* 1x1 solid foreground colour for text */
	MHEGKeyMapEntry *keymap;		/* keyboard mapping */
} MHEGDisplay;

void MHEGDisplay_init(MHEGDisplay *, bool, char *);
void MHEGDisplay_fini(MHEGDisplay *);

bool MHEGDisplay_processEvents(MHEGDisplay *, bool);

void MHEGDisplay_refresh(MHEGDisplay *, XYPosition *, OriginalBoxSize *);

void MHEGDisplay_clearScreen(MHEGDisplay *);

/* drawing routines */
void MHEGDisplay_setClipRectangle(MHEGDisplay *, XYPosition *, OriginalBoxSize *);
void MHEGDisplay_unsetClipRectangle(MHEGDisplay *);

void MHEGDisplay_drawHoriLine(MHEGDisplay *, XYPosition *, unsigned int, int, int, MHEGColour *);
void MHEGDisplay_drawVertLine(MHEGDisplay *, XYPosition *, unsigned int, int, int, MHEGColour *);
void MHEGDisplay_fillTransparentRectangle(MHEGDisplay *, XYPosition *, OriginalBoxSize *);
void MHEGDisplay_fillRectangle(MHEGDisplay *, XYPosition *, OriginalBoxSize *, MHEGColour *);
void MHEGDisplay_drawBitmap(MHEGDisplay *, XYPosition *, OriginalBoxSize *, MHEGBitmap *, XYPosition *);
void MHEGDisplay_drawCanvas(MHEGDisplay *, XYPosition *, OriginalBoxSize *, MHEGCanvas *, XYPosition *);
void MHEGDisplay_drawTextElement(MHEGDisplay *, XYPosition *, MHEGFont *, MHEGTextElement *, bool);

void MHEGDisplay_useOverlay(MHEGDisplay *);

/* convert PNG and MPEG I-frames to internal format */
MHEGBitmap *MHEGDisplay_newPNGBitmap(MHEGDisplay *, OctetString *);
MHEGBitmap *MHEGDisplay_newMPEGBitmap(MHEGDisplay *, OctetString *);
void MHEGDisplay_freeBitmap(MHEGDisplay *, MHEGBitmap *);

MHEGBitmap *MHEGBitmap_fromRGBA(MHEGDisplay *, unsigned char *, unsigned int, unsigned int);

/* utils */
bool intersects(XYPosition *, OriginalBoxSize *, XYPosition *, OriginalBoxSize *, XYPosition *, OriginalBoxSize *);

/* convert X/Y coords from MHEG resolution (0->720, 0->576) to output resolution */
#define MHEGDisplay_scaleX(DPY, X)	(((X) * (DPY)->xres) / MHEG_XRES)
#define MHEGDisplay_scaleY(DPY, Y)	(((Y) * (DPY)->yres) / MHEG_YRES)

#endif	/* __MHEGDISPLAY_H__ */

