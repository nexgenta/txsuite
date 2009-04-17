/*
 * MHEGDisplay.c
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <png.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <ffmpeg/avformat.h>

#include "MHEGEngine.h"
#include "MHEGDisplay.h"
#include "readpng.h"
#include "utils.h"

/* internal utils */
static MHEGKeyMapEntry *load_keymap(char *);

static void display_colour(XRenderColor *, MHEGColour *);

/* from GDK MwmUtils.h */
#define MWM_HINTS_DECORATIONS	(1L << 1)
typedef struct
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long input_mode;
	unsigned long status;
} MotifWmHints;

/* default keyboard mapping */
static MHEGKeyMapEntry default_keymap[] =
{
	{ XK_Up, MHEGKey_Up },
	{ XK_Down, MHEGKey_Down },
	{ XK_Left, MHEGKey_Left },
	{ XK_Right, MHEGKey_Right },
	{ XK_0, MHEGKey_0 },
	{ XK_1, MHEGKey_1 },
	{ XK_2, MHEGKey_2 },
	{ XK_3, MHEGKey_3 },
	{ XK_4, MHEGKey_4 },
	{ XK_5, MHEGKey_5 },
	{ XK_6, MHEGKey_6 },
	{ XK_7, MHEGKey_7 },
	{ XK_8, MHEGKey_8 },
	{ XK_9, MHEGKey_9 },
	{ XK_Return, MHEGKey_Select },
	{ XK_Escape, MHEGKey_Cancel },
	{ XK_r, MHEGKey_Red },
	{ XK_g, MHEGKey_Green },
	{ XK_y, MHEGKey_Yellow },
	{ XK_b, MHEGKey_Blue },
	{ XK_t, MHEGKey_Text },
	{ XK_e, MHEGKey_EPG },
	{ 0, 0 }			/* terminator */
};

void
MHEGDisplay_init(MHEGDisplay *d, bool fullscreen, char *keymap)
{
	unsigned int xrender_major;
	unsigned int xrender_minor;
	int x, y;
	XVisualInfo visinfo;
	unsigned long mask;
	XSetWindowAttributes attr;
	Atom wm_delete_window;
	XSizeHints hint;
	unsigned long gcmask;
	XGCValues gcvals;
	XRenderPictFormat *pic_format;
	XRenderPictureAttributes pa;
	Pixmap textfg;
	/* fake argc, argv for XtDisplayInitialize */
	int argc = 0;
	char *argv[1] = { NULL };

	/* remember if we are using fullscreen mode */
	d->fullscreen = fullscreen;

	/* keyboard mapping */
	if(keymap != NULL)
		d->keymap = load_keymap(keymap);
	else
		d->keymap = default_keymap;

	/* so X requests/replies in different threads don't get interleaved */
	XInitThreads();

	if((d->dpy = XOpenDisplay(NULL)) == NULL)
		fatal("Unable to open display");

	/* check the X server supports at least XRender 0.6 (needed for Bilinear filter) */
	xrender_major = 0;
	xrender_minor = 10;
	if(!XRenderQueryVersion(d->dpy, &xrender_major, &xrender_minor)
	|| xrender_minor < 6)
		fatal("X Server does not support XRender 0.6 or above");

	/* size of the Window */
	if(fullscreen)
	{
		d->xres = WidthOfScreen(DefaultScreenOfDisplay(d->dpy));
		d->yres = HeightOfScreen(DefaultScreenOfDisplay(d->dpy));
	}
	else
	{
		/* resolution defined in UK MHEG Profile */
		d->xres = MHEG_XRES;
		d->yres = MHEG_YRES;
	}

	/* create the window */
	x = (WidthOfScreen(DefaultScreenOfDisplay(d->dpy)) - d->xres) / 2;
	y = (HeightOfScreen(DefaultScreenOfDisplay(d->dpy)) - d->yres) / 2;

	/* remember the colour depth and Visual used by the Window */
	d->depth = DefaultDepth(d->dpy, DefaultScreen(d->dpy));
	if(!XMatchVisualInfo(d->dpy, DefaultScreen(d->dpy), d->depth, TrueColor, &visinfo))
		fatal("Unable to find a TrueColour Visual");
	d->vis = visinfo.visual;

	/*
	 * if the default Visual is not TrueColor we need to create a Colormap for our Visual
	 * this is probably only gonna happen for 8 bit displays
	 * you *really* want a 16 or 24 bit colour display
	 */
	if(DefaultVisual(d->dpy, DefaultScreen(d->dpy))->class != TrueColor)
	{
		d->cmap = XCreateColormap(d->dpy, DefaultRootWindow(d->dpy), d->vis, AllocNone);
		XInstallColormap(d->dpy, d->cmap);
		mask = CWColormap;
		attr.colormap = d->cmap;
	}
	else
	{
		d->cmap = None;
		mask = 0;
	}

	/* don't need any special toolkits or widgets, just a canvas to draw on */
	d->win = XCreateWindow(d->dpy,
			       DefaultRootWindow(d->dpy),
			       x, y, d->xres, d->yres, 0,
			       d->depth, InputOutput, d->vis,
			       mask, &attr);

	/* in case the WM ignored where we want it placed */
	hint.flags = USSize | USPosition | PPosition | PSize;
	hint.x = x;
	hint.y = y;
	hint.width = d->xres;
	hint.height = d->yres;
	XSetWMNormalHints(d->dpy, d->win, &hint);

	if(fullscreen)
	{
		GC gc;
		Pixmap no_pixmap;
		XColor no_colour;
		Cursor no_cursor;
		/* this is how GDK makes Windows fullscreen */
		XEvent xev;
		Atom motif_wm_hints;
		MotifWmHints mwmhints;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.window = d->win;
		xev.xclient.message_type = XInternAtom(d->dpy, "_NET_WM_STATE", False);
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = XInternAtom(d->dpy, "_NET_WM_STATE_ADD", False);
		xev.xclient.data.l[1] = XInternAtom(d->dpy, "_NET_WM_STATE_FULLSCREEN", False);
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;
		XSendEvent(d->dpy, DefaultRootWindow(d->dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
		/* get rid of the Window decorations */
		motif_wm_hints = XInternAtom(d->dpy, "_MOTIF_WM_HINTS", False);
		mwmhints.flags = MWM_HINTS_DECORATIONS;
		mwmhints.decorations = 0;
		XChangeProperty(d->dpy, d->win,
				motif_wm_hints, motif_wm_hints, 32, PropModeReplace,
				(unsigned char *) &mwmhints, sizeof(MotifWmHints) / sizeof(long));
		/*
		 * get rid of the cursor when the mouse is over our Window
		 * make the cursor a 1x1 Pixmap with no pixels displayed by the mask
		 */
		no_pixmap = XCreatePixmap(d->dpy, d->win, 1, 1, 1);
		gcmask = GCForeground;
		gcvals.foreground = 0;
		gc = XCreateGC(d->dpy, no_pixmap, gcmask, &gcvals);
		XDrawPoint(d->dpy, no_pixmap, gc, 0, 0);
		XFreeGC(d->dpy, gc);
		no_cursor = XCreatePixmapCursor(d->dpy, no_pixmap, no_pixmap, &no_colour, &no_colour, 0, 0);
		XFreePixmap(d->dpy, no_pixmap);
		XDefineCursor(d->dpy, d->win, no_cursor);
	}

	/* want to get Expose and KeyPress events */
	mask = CWEventMask;
	attr.event_mask = ExposureMask | KeyPressMask;
	XChangeWindowAttributes(d->dpy, d->win, mask, &attr);

	/* get a ClientMessage event when the window's close button is clicked */
	wm_delete_window = XInternAtom(d->dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(d->dpy, d->win, &wm_delete_window, 1);

	/*
	 * create an XRender Picture for the Window contents
	 * we composite the video frame and the MHEG overlay onto this, then copy it onto the Window
	 */
	pic_format = XRenderFindVisualFormat(d->dpy, d->vis);
	d->contents = XCreatePixmap(d->dpy, d->win, d->xres, d->yres, d->depth);
	d->contents_pic = XRenderCreatePicture(d->dpy, d->contents, pic_format, 0, NULL);

	/* create a 32-bit XRender Picture to draw the MHEG objects on */
	pic_format = XRenderFindStandardFormat(d->dpy, PictStandardARGB32);
	d->next_overlay = XCreatePixmap(d->dpy, d->win, d->xres, d->yres, 32);
	d->next_overlay_pic = XRenderCreatePicture(d->dpy, d->next_overlay, pic_format, 0, NULL);
	d->used_overlay = XCreatePixmap(d->dpy, d->win, d->xres, d->yres, 32);
	d->used_overlay_pic = XRenderCreatePicture(d->dpy, d->used_overlay, pic_format, 0, NULL);

	/* a 1x1 Picture to hold the text foreground colour */
	textfg = XCreatePixmap(d->dpy, d->win, 1, 1, 32);
	pa.repeat = True;
	d->textfg_pic = XRenderCreatePicture(d->dpy, textfg, pic_format, CPRepeat, &pa);

	/* a GC to draw on the Window */
	d->win_gc = XCreateGC(d->dpy, d->win, 0, &gcvals);

	/* a GC to XCopyArea next_overlay to used_overlay (need to avoid any XRender clip mask on next_overlay) */
	d->overlay_gc = XCreateGC(d->dpy, d->next_overlay, 0, &gcvals);

	/* get the window on the screen */
	XMapWindow(d->dpy, d->win);

	/* needs to be realised before we can set the title */
	XStoreName(d->dpy, d->win, WINDOW_TITLE);

	/* rather than having to implement our own timers we use Xt, so initialise it */
	XtToolkitInitialize();
	d->app = XtCreateApplicationContext();
	XtDisplayInitialize(d->app, d->dpy, APP_NAME, APP_CLASS, NULL, 0, &argc, argv);

	/* init ffmpeg */
	av_register_all();

	return;
}

void
MHEGDisplay_fini(MHEGDisplay *d)
{
	/* calls XCloseDisplay for us which free's all our Windows, Pixmaps, etc */
	XtDestroyApplicationContext(d->app);

	return;
}

/*
 * process the next GUI event
 * if block is false and no events are pending, return immediately
 * if block is true and no events are pending, wait for the next event
 * returns true if the GUI wants us to quit
 */

bool
MHEGDisplay_processEvents(MHEGDisplay *d, bool block)
{
	bool quit = false;
	XEvent event;
	XAnyEvent *any;
	XKeyEvent *key;
	KeySym sym;
	MHEGKeyMapEntry *map;
	XExposeEvent *exp;
	XClientMessageEvent *cm;
	static Atom wm_protocols = 0;
	static Atom wm_delete_window = 0;

	/* dont block if only a Timer is pending */
	if(!block
	&& (XtAppPending(d->app) & ~XtIMTimer) == 0)
		return false;

	/* this will block if no events are pending */
	XtAppNextEvent(d->app, &event);

	/* is it our window */
	any = &event.xany;
	if(any->display != d->dpy || any->window != d->win)
	{
		/* pass it on to Xt */
		XtDispatchEvent(&event);
		return false;
	}

	switch(event.type)
	{
	case KeyPress:
		key = &event.xkey;
		sym = XKeycodeToKeysym(d->dpy, key->keycode, 0);
		/* find the KeySym in the keyboard map */
		map = d->keymap;
		while(map->mheg_key != 0 && map->x_key != sym)
			map ++;
		if(map->mheg_key != 0)
		{
			verbose("KeyPress: %s (%u)", XKeysymToString(sym), map->mheg_key);
			MHEGEngine_keyPressed(map->mheg_key);
		}
		break;

	case Expose:
		exp = &event.xexpose;
		XCopyArea(d->dpy, d->contents, d->win, d->win_gc, exp->x, exp->y, exp->width, exp->height, exp->x, exp->y);
		break;

	case NoExpose:
		/* ignore it */
		break;

	case ClientMessage:
		cm = &event.xclient;
		/* cache these Atoms */
		if(wm_protocols == 0)
		{
			wm_protocols = XInternAtom(d->dpy, "WM_PROTOCOLS", False);
			wm_delete_window = XInternAtom(d->dpy, "WM_DELETE_WINDOW", False);
		}
		if(cm->message_type == wm_protocols
		&& cm->format == 32
		&& cm->data.l[0] == wm_delete_window)
			quit = true;
		else
			verbose("Ignoring ClientMessage type %s", XGetAtomName(d->dpy, cm->message_type));
		break;

	default:
		/* pass it on to Xt */
		XtDispatchEvent(&event);
		break;
	}

	return quit;
}

/*
 * gets the given area of the Window refreshed
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGDisplay_refresh(MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	int x, y;
	unsigned int w, h;

	/* scale if fullscreen */
	x = MHEGDisplay_scaleX(d, pos->x_position);
	y = MHEGDisplay_scaleY(d, pos->y_position);
	w = MHEGDisplay_scaleX(d, box->x_length);
	h = MHEGDisplay_scaleY(d, box->y_length);

	/*
	 * if video is being displayed, the current frame will already be in d->contents
	 * (drawn by the video thread)
	 * overlay the MHEG objects onto the video in d->contents
	 */
	XRenderComposite(d->dpy, PictOpOver, d->used_overlay_pic, None, d->contents_pic, x, y, x, y, x, y, w, h);

	/* copy the Window contents onto the screen */
	XCopyArea(d->dpy, d->contents, d->win, d->win_gc, x, y, w, h, x, y);

	return;
}

void
MHEGDisplay_clearScreen(MHEGDisplay *d)
{
	XYPosition pos = {0, 0};
	OriginalBoxSize box = {MHEG_XRES, MHEG_YRES};
	MHEGColour black;

	MHEGColour_black(&black);
	MHEGDisplay_fillRectangle(d, &pos, &box, &black);

	/* use the new object we have just drawn */
	MHEGDisplay_useOverlay(d);

	/* refresh the screen */
	MHEGDisplay_refresh(d, &pos, &box);

	XFlush(d->dpy);

	return;
}

/*
 * all these drawing routines draw onto next_overlay_pic
 * all coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 * the drawing routines themselves will scale the coords to full screen if needed
 * you have to call MHEGDisplay_useOverlay() when you have finished drawing
 * this copies next_overlay onto used_overlay
 * used_overlay_pic is composited onto any video and put on the screen by MHEGDisplay_refresh()
 */

/*
 * set the clip rectangle for all subsequent drawing on the overlay
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGDisplay_setClipRectangle(MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XRectangle clip;

	/* scale if fullscreen */
	clip.x = MHEGDisplay_scaleX(d, pos->x_position);
	clip.y = MHEGDisplay_scaleY(d, pos->y_position);
	clip.width = MHEGDisplay_scaleX(d, box->x_length);
	clip.height = MHEGDisplay_scaleY(d, box->y_length);

	XRenderSetPictureClipRectangles(d->dpy, d->next_overlay_pic, 0, 0, &clip, 1);

	return;
}

/*
 * remove the clip rectangle from the overlay
 */

void
MHEGDisplay_unsetClipRectangle(MHEGDisplay *d)
{
	/*
	 * this doesn't work...
	 * XRenderSetPictureClipRectangles(d->dpy, d->next_overlay_pic, 0, 0, NULL, 0);
	 */

	XRenderPictureAttributes attr;

	attr.clip_mask = None;

	XRenderChangePicture(d->dpy, d->next_overlay_pic, CPClipMask, &attr);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 * width is the line width in pixels
 * style should be LineStyle_solid/dashed/dotted
 */

void
MHEGDisplay_drawHoriLine(MHEGDisplay *d, XYPosition *pos, unsigned int len, int width, int style, MHEGColour *col)
{
	XRenderColor rcol;
	int x, y;
	unsigned int w, h;

	/* if it is transparent or line width is <=0, just bail out */
	if(col->t == MHEGCOLOUR_TRANSPARENT || width <= 0)
		return;

	/* convert to internal colour format */
	display_colour(&rcol, col);

	/* scale if fullscreen */
	x = MHEGDisplay_scaleX(d, pos->x_position);
	y = MHEGDisplay_scaleY(d, pos->y_position);
	w = MHEGDisplay_scaleX(d, len);
	/* aspect ratio */
	h = MHEGDisplay_scaleY(d, width);

/* TODO */
if(style != LineStyle_solid)
printf("TODO: LineStyle %d\n", style);

	/* draw a rectangle */
	XRenderFillRectangle(d->dpy, PictOpOver, d->next_overlay_pic, &rcol, x, y, w, h);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 * width is the line width in pixels
 * style should be LineStyle_solid/dashed/dotted
 */

void
MHEGDisplay_drawVertLine(MHEGDisplay *d, XYPosition *pos, unsigned int len, int width, int style, MHEGColour *col)
{
	XRenderColor rcol;
	int x, y;
	unsigned int w, h;

	/* if it is transparent or line width is <=0, just bail out */
	if(col->t == MHEGCOLOUR_TRANSPARENT || width <= 0)
		return;

	/* convert to internal colour format */
	display_colour(&rcol, col);

	/* scale if fullscreen */
	x = MHEGDisplay_scaleX(d, pos->x_position);
	y = MHEGDisplay_scaleY(d, pos->y_position);
	h = MHEGDisplay_scaleY(d, len);
	/* aspect ratio */
	w = MHEGDisplay_scaleX(d, width);

/* TODO */
if(style != LineStyle_solid)
printf("TODO: LineStyle %d\n", style);

	/* draw a rectangle */
	XRenderFillRectangle(d->dpy, PictOpOver, d->next_overlay_pic, &rcol, x, y, w, h);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGDisplay_fillRectangle(MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box, MHEGColour *col)
{
	XRenderColor rcol;
	int x, y;
	unsigned int w, h;

	/* if it is transparent, just bail out */
	if(col->t == MHEGCOLOUR_TRANSPARENT)
		return;

	/* convert to internal colour format */
	display_colour(&rcol, col);

	/* scale if fullscreen */
	x = MHEGDisplay_scaleX(d, pos->x_position);
	y = MHEGDisplay_scaleY(d, pos->y_position);
	w = MHEGDisplay_scaleX(d, box->x_length);
	h = MHEGDisplay_scaleY(d, box->y_length);

	XRenderFillRectangle(d->dpy, PictOpOver, d->next_overlay_pic, &rcol, x, y, w, h);

	return;
}

/*
 * explicitly make a transparent rectangle in the MHEG overlay
 * MHEGDisplay_fillRectangle() uses PictOpOver => it can't create a transparent box in the output
 * this uses PictOpSrc
 */

void
MHEGDisplay_fillTransparentRectangle(MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XRenderColor rcol = {0, 0, 0, 0};
	int x, y;
	unsigned int w, h;

	/* scale if fullscreen */
	x = MHEGDisplay_scaleX(d, pos->x_position);
	y = MHEGDisplay_scaleY(d, pos->y_position);
	w = MHEGDisplay_scaleX(d, box->x_length);
	h = MHEGDisplay_scaleY(d, box->y_length);

	XRenderFillRectangle(d->dpy, PictOpSrc, d->next_overlay_pic, &rcol, x, y, w, h);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGDisplay_drawBitmap(MHEGDisplay *d, XYPosition *src, OriginalBoxSize *box, MHEGBitmap *bitmap, XYPosition *dst)
{
	int src_x, src_y;
	int dst_x, dst_y;
	unsigned int w, h;

	/* in case we don't have any content yet, UK MHEG Profile says make it transparent */
	if(bitmap == NULL)
		return;

	/*
	 * scale up if fullscreen
	 * the bitmap itself is scaled when it is created in MHEGDisplay_newBitmap()
	 */
	src_x = MHEGDisplay_scaleX(d, src->x_position);
	src_y = MHEGDisplay_scaleY(d, src->y_position);
	w = MHEGDisplay_scaleX(d, box->x_length);
	h = MHEGDisplay_scaleY(d, box->y_length);
	dst_x = MHEGDisplay_scaleX(d, dst->x_position);
	dst_y = MHEGDisplay_scaleY(d, dst->y_position);

	XRenderComposite(d->dpy, PictOpOver, bitmap->image_pic, None, d->next_overlay_pic,
			 src_x, src_y, src_x, src_y, dst_x, dst_y, w, h);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGDisplay_drawCanvas(MHEGDisplay *d, XYPosition *src, OriginalBoxSize *box, MHEGCanvas *canvas, XYPosition *dst)
{
	int src_x, src_y;
	int dst_x, dst_y;
	unsigned int w, h;

	/*
	 * scale up if fullscreen
	 * the canvas image itself is scaled when it is created
	 */
	src_x = MHEGDisplay_scaleX(d, src->x_position);
	src_y = MHEGDisplay_scaleY(d, src->y_position);
	w = MHEGDisplay_scaleX(d, box->x_length);
	h = MHEGDisplay_scaleY(d, box->y_length);
	dst_x = MHEGDisplay_scaleX(d, dst->x_position);
	dst_y = MHEGDisplay_scaleY(d, dst->y_position);

	XRenderComposite(d->dpy, PictOpOver, canvas->contents_pic, None, d->next_overlay_pic,
			 src_x, src_y, src_x, src_y, dst_x, dst_y, w, h);

	return;
}

/*
 * coords should be in the range 0-MHEG_XRES, 0-MHEG_YRES
 * text can include multibyte (UTF8) chars as described in the UK MHEG Profile
 * text can also include tab characters (0x09)
 * if tabs is false, tab characters are just treated as spaces
 * text should *not* include ESC sequences to change colour or \r for new lines
 */

void
MHEGDisplay_drawTextElement(MHEGDisplay *d, XYPosition *pos, MHEGFont *font, MHEGTextElement *text, bool tabs)
{
	XRenderColor rcol;
	int orig_x;
	int x, y;
	int scrn_x;
	FT_Face face;
	FT_GlyphSlot slot;
	FT_UInt glyph;
	FT_UInt previous;
	FT_Vector kern;
	FT_Error err;
	unsigned char *data;
	unsigned int size;
	int utf8;
	int len;
	int ntabs;

	/* is there any text */
	if(text->size == 0)
		return;

	/* convert to internal colour format */
	display_colour(&rcol, &text->col);

	/* scale the x origin if fullscreen */
	orig_x = MHEGDisplay_scaleX(d, pos->x_position);
	/* y coord does not change */
	y = MHEGDisplay_scaleY(d, pos->y_position + text->y);

	/* set the text foreground colour */
	XRenderFillRectangle(d->dpy, PictOpSrc, d->textfg_pic, &rcol, 0, 0, 1, 1);

	/*
	 * can't just use XftTextRenderUtf8() because:
	 * - it doesn't do kerning
	 * - text may include tabs
	 */
	/* we do all layout calculations with the unscaled font metrics */
	face = XftLockFace(font->font);
	slot = face->glyph;

	/* no previous glyph yet */
	previous = 0;

	/* x in font units */
	x = text->x * face->units_per_EM;

	data = text->data;
	size = text->size;
	while(size > 0)
	{
		/* get the next UTF8 char */
		utf8 = next_utf8(data, size, &len);
		data += len;
		size -= len;
		/* if it's a tab, just advance to the next tab stop */
		if(utf8 == 0x09 && tabs)
		{
			/* min amount a tab should advance the text pos */
			x += font->xOffsetLeft * face->units_per_EM;
			/* move to the next tab stop */
			ntabs = x / (MHEG_TAB_WIDTH * face->units_per_EM);
			x = ((ntabs + 1) * MHEG_TAB_WIDTH) * face->units_per_EM;
			continue;
		}
		/* we are treating tabs as spaces */
		if(utf8 == 0x09)
			utf8 = 0x20;
		/* get the glyph index for the UTF8 char */
		glyph = FT_Get_Char_Index(face, utf8);
		/* do any kerning if necessary */
		if(previous != 0 && FT_HAS_KERNING(face))
		{
			FT_Get_Kerning(face, previous, glyph, FT_KERNING_UNSCALED, &kern);
			x += (kern.x * font->size * 45) / 56;
		}
		/* remember the glyph for kerning next time */
		previous = glyph;
		/* render it */
		XftUnlockFace(font->font);
		/* round up/down the X coord */
		scrn_x = MHEGDisplay_scaleX(d, x);
		scrn_x = (scrn_x + (face->units_per_EM / 2)) / face->units_per_EM;
		XftGlyphRender(d->dpy, PictOpOver, d->textfg_pic, font->font, d->next_overlay_pic,
			       0, 0, orig_x + scrn_x, y,
			       &glyph, 1);
		face = XftLockFace(font->font);
		slot = face->glyph;
		/* advance x */
		err = FT_Load_Glyph(face, glyph, FT_LOAD_NO_SCALE);
		if(err)
			continue;
		x += (slot->advance.x * font->size * 45) / 56;
		/* add on (letter spacing / 256) * units_per_EM */
		x += (face->units_per_EM * font->letter_spc * 45) / (256 * 56);
	}

	XftUnlockFace(font->font);

	return;
}

/*
 * copy the contents of next_overlay onto used_overlay
 * ie all drawing done since the last call to this will appear on the screen at the next refresh()
 */

void
MHEGDisplay_useOverlay(MHEGDisplay *d)
{
	/* avoid any XRender clip mask */
	XCopyArea(d->dpy, d->next_overlay, d->used_overlay, d->overlay_gc, 0, 0, d->xres, d->yres, 0, 0);

	return;
}

/*
 * convert the given PNG data to an internal format
 * returns NULL on error
 */

MHEGBitmap *
MHEGDisplay_newPNGBitmap(MHEGDisplay *d, OctetString *png)
{
	MHEGBitmap *b;
	png_uint_32 width, height;
	unsigned char *rgba;
	unsigned int i;

	/* nothing to do */
	if(png == NULL || png->size == 0)
		return NULL;

	/* convert the PNG into a standard format we can use as an XImage */
	if((rgba = readpng_get_image(png->data, png->size, &width, &height)) == NULL)
	{
		error("Unable to decode PNG file");
		return NULL;
	}

	/*
	 * we now have an array of 32-bit RGBA pixels in network byte order
	 * ie if pix is a char *: pix[0] = R, pix[1] = G, pix[2] = B, pix[3] = A
	 * we need to convert it to ffmpeg's PIX_FMT_RGBA32 format
	 * ffmpeg always stores PIX_FMT_RGBA32 as
	 *  (A << 24) | (R << 16) | (G << 8) | B
	 * no matter what byte order our CPU uses. ie,
	 * it is stored as BGRA on little endian CPU architectures and ARGB on big endian CPUs
	 */
	for(i=0; i<width*height; i++)
	{
		uint8_t a, r, g, b;
		uint32_t pix;
		/*
		 * if the pixel is transparent, set the RGB components to 0
		 * otherwise, if we scale up the bitmap in fullscreen mode,
		 * we may end up with a border around the image
		 * this happens, for example, with the BBC's "Press Red" image
		 * it has a transparent box around it, but the RGB values are not 0 in the transparent area
		 * when we scale it up we get a pink border around it
		 */
		a = rgba[(i * 4) + 3];
		if(a == 0)
		{
			pix = 0;
		}
		else
		{
			r = rgba[(i * 4) + 0];
			g = rgba[(i * 4) + 1];
			b = rgba[(i * 4) + 2];
			pix = (a << 24) | (r << 16) | (g << 8) | b;
		}
		*((uint32_t *) &rgba[i * 4]) = pix;
	}

	/* convert the PIX_FMT_RGBA32 data to a MHEGBitmap */
	b = MHEGBitmap_fromRGBA(d, rgba, width, height);

	/* clean up */
	readpng_free_image(rgba);

	return b;
}

/*
 * convert the given MPEG I-frame data to an internal format
 * returns NULL on error
 */

MHEGBitmap *
MHEGDisplay_newMPEGBitmap(MHEGDisplay *d, OctetString *mpeg)
{
	MHEGBitmap *b;
	AVCodecContext *codec_ctx;
	AVCodec *codec;
	AVFrame *yuv_frame;
	AVFrame *rgb_frame;
	unsigned char *padded;
	unsigned char *data;
	unsigned int size;
	int used;
	int got_picture;
	unsigned int width;
	unsigned int height;
	int nbytes;
	unsigned char *rgba = NULL;

	/* nothing to do */
	if(mpeg == NULL || mpeg->size == 0)
		return NULL;

	/* use ffmpeg to convert the data into a standard format we can use as an XImage */
	if((codec_ctx = avcodec_alloc_context()) == NULL)
		fatal("Out of memory");

	if((codec = avcodec_find_decoder(CODEC_ID_MPEG2VIDEO)) == NULL)
		fatal("Unable to initialise MPEG decoder");

	if(avcodec_open(codec_ctx, codec) < 0)
		fatal("Unable to open video codec");

	if((yuv_frame = avcodec_alloc_frame()) == NULL)
		fatal("Out of memory");
	if((rgb_frame = avcodec_alloc_frame()) == NULL)
		fatal("Out of memory");

	/* ffmpeg may read passed the end of the buffer, so pad it out */
	padded = safe_malloc(mpeg->size + FF_INPUT_BUFFER_PADDING_SIZE);
	memcpy(padded, mpeg->data, mpeg->size);
	memset(padded + mpeg->size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

	/* decode the YUV frame */
	data = padded;
	size = mpeg->size;
	do
	{
		used = avcodec_decode_video(codec_ctx, yuv_frame, &got_picture, data, size);
		data += used;
		size -= used;
	}
	while(!got_picture && size > 0);
	/* need to call it one final time with size=0, to actually get the frame */
	if(!got_picture)
		(void) avcodec_decode_video(codec_ctx, yuv_frame, &got_picture, data, size);

	if(!got_picture)
	{
		error("Unable to decode MPEG image");
		b = NULL;
	}
	else
	{
		/* convert to RGBA */
		width = codec_ctx->width;
		height = codec_ctx->height;
		if((nbytes = avpicture_get_size(PIX_FMT_RGBA32, width, height)) < 0)
			fatal("Invalid MPEG image");
		rgba = safe_malloc(nbytes);
		avpicture_fill((AVPicture *) rgb_frame, rgba, PIX_FMT_RGBA32, width, height);
		img_convert((AVPicture *) rgb_frame, PIX_FMT_RGBA32, (AVPicture*) yuv_frame, codec_ctx->pix_fmt, width, height);
		/* convert the PIX_FMT_RGBA32 data to a MHEGBitmap */
		b = MHEGBitmap_fromRGBA(d, rgba, width, height);
	}

	/* clean up */
	safe_free(padded);
	safe_free(rgba);
	av_free(yuv_frame);
	av_free(rgb_frame);
	avcodec_close(codec_ctx);

	return b;
}

void
MHEGDisplay_freeBitmap(MHEGDisplay *d, MHEGBitmap *b)
{
	if(b != NULL)
	{
		XRenderFreePicture(d->dpy, b->image_pic);
		XFreePixmap(d->dpy, b->image);
		safe_free(b);
	}

	return;
}

/*
 * construct a MHEGBitmap from an array of ffmpeg's PIX_FMT_RGBA32 pixels
 * ffmpeg always stores PIX_FMT_RGBA32 as
 *  (A << 24) | (R << 16) | (G << 8) | B
 * no matter what byte order our CPU uses. ie,
 * it is stored as BGRA on little endian CPU architectures and ARGB on big endian CPUs
 */

MHEGBitmap *
MHEGBitmap_fromRGBA(MHEGDisplay *d, unsigned char *rgba, unsigned int width, unsigned int height)
{
	MHEGBitmap *bitmap;
	unsigned char *xdata;
	uint32_t rgba_pix;
	uint32_t xpix;
	uint8_t r, g, b, a;
	unsigned int i, npixs;
	XImage *ximg;
	XRenderPictFormat *pic_format;
	enum PixelFormat av_format;
	GC gc;

	bitmap = safe_malloc(sizeof(MHEGBitmap));
	bzero(bitmap, sizeof(MHEGBitmap));

	/* find a matching XRender pixel format */
	pic_format = XRenderFindStandardFormat(d->dpy, PictStandardARGB32);
	av_format = find_av_pix_fmt(32,
				    pic_format->direct.redMask << pic_format->direct.red,
				    pic_format->direct.greenMask << pic_format->direct.green,
				    pic_format->direct.blueMask << pic_format->direct.blue);

	/* copy the RGBA values into a block we can use as XImage data */
	npixs = width * height;
	/* 4 bytes per pixel */
	xdata = safe_malloc(npixs * 4);
	/* are the pixel layouts exactly the same */
	if(av_format == PIX_FMT_RGBA32)
	{
		memcpy(xdata, rgba, npixs * 4);
	}
	else
	{
		/* swap the RGBA components as needed */
		for(i=0; i<npixs; i++)
		{
			rgba_pix = *((uint32_t *) &rgba[i * 4]);
			a = (rgba_pix >> 24) & 0xff;
			r = (rgba_pix >> 16) & 0xff;
			g = (rgba_pix >> 8) & 0xff;
			b = rgba_pix & 0xff;
			/* is it transparent */
			if(a == 0)
			{
				xpix = 0;
			}
			else
			{
				xpix = a << pic_format->direct.alpha;
				xpix |= r << pic_format->direct.red;
				xpix |= g << pic_format->direct.green;
				xpix |= b << pic_format->direct.blue;
			}
			*((uint32_t *) &xdata[i * 4]) = xpix;
		}
	}

	/* get X to draw the XImage onto a Pixmap */
	if((ximg = XCreateImage(d->dpy, NULL, 32, ZPixmap, 0, xdata, width, height, 32, 0)) == NULL)
		fatal("XCreateImage failed");
	/* passed NULL Visual to XCreateImage, so set the rgb masks now */
	ximg->red_mask = pic_format->direct.redMask;
	ximg->green_mask = pic_format->direct.greenMask;
	ximg->blue_mask = pic_format->direct.blueMask;
	/* create the Pixmap */
	bitmap->image = XCreatePixmap(d->dpy, d->win, width, height, 32);
	gc = XCreateGC(d->dpy, bitmap->image, 0, NULL);
	XPutImage(d->dpy, bitmap->image, gc, ximg, 0, 0, 0, 0, width, height);
	XFreeGC(d->dpy, gc);

	/* associate a Picture with it */
	bitmap->image_pic = XRenderCreatePicture(d->dpy, bitmap->image, pic_format, 0, NULL);

	/* if we are using fullscreen mode, scale the image */
	if(d->fullscreen)
	{
printf("TODO: MHEGBitmap_fromRGBA: take aspect ratio into account\n");
		/* set up the matrix to scale it */
		XTransform xform;
		/* X */
		xform.matrix[0][0] = (MHEG_XRES << 16) / d->xres;
		xform.matrix[0][1] = 0;
		xform.matrix[0][2] = 0;
		/* Y */
		xform.matrix[1][0] = 0;
		xform.matrix[1][1] = (MHEG_YRES << 16) / d->yres;
		xform.matrix[1][2] = 0;
		/* Z */
		xform.matrix[2][0] = 0;
		xform.matrix[2][1] = 0;
		xform.matrix[2][2] = 1 << 16;
		/* scale it */
		XRenderSetPictureTransform(d->dpy, bitmap->image_pic, &xform);
		/* set a filter to smooth the edges */
		XRenderSetPictureFilter(d->dpy, bitmap->image_pic, FilterBilinear, 0, 0);
	}

	/* we alloc'ed the XImage data, make sure XDestroyImage doesn't try to free it */
	safe_free(xdata);
	ximg->data = NULL;
	XDestroyImage(ximg);

	return bitmap;
}

/*
 * returns true if the two boxes intersect
 * sets out_pos and out_box to the intersection
 */

bool
intersects(XYPosition *p1, OriginalBoxSize *b1, XYPosition *p2, OriginalBoxSize *b2, XYPosition *out_pos, OriginalBoxSize *out_box)
{
	int x1 = p1->x_position;
	int y1 = p1->y_position;
	int w1 = b1->x_length;
	int h1 = b1->y_length;
	int x2 = p2->x_position;
	int y2 = p2->y_position;
	int w2 = b2->x_length;
	int h2 = b2->y_length;
	bool hmatch;
	bool vmatch;

	/* intersection */
	out_pos->x_position = MAX(x1, x2);
	out_pos->y_position = MAX(y1, y2);
	out_box->x_length = MIN(x1 + w1, x2 + w2) - out_pos->x_position;
	out_box->y_length = MIN(y1 + h1, y2 + h2) - out_pos->y_position;

	/* does it intersect */
	hmatch = (x1 < (x2 + w2)) && ((x1 + w1) > x2);
	vmatch = (y1 < (y2 + h2)) && ((y1 + h1) > y2);

	return hmatch && vmatch;
}

/*
 * load the given key map config file
 */

static MHEGKeyMapEntry *
load_keymap(char *filename)
{
	FILE *conf;
	MHEGKeyMapEntry *map;
	char symname[64];
	size_t len;

	if((conf = fopen(filename, "r")) == NULL)
		fatal("Unable to open keymap config '%s': %s", filename, strerror(errno));

	/* overwrite default_keymap */
	map = default_keymap;
	while(map->mheg_key != 0)
	{
		if(fgets(symname, sizeof(symname), conf) == NULL)
			fatal("Keymap config file '%s' ended unexpectedly", filename);
		/* chop off any trailing \n */
		len = strlen(symname);
		if(symname[len-1] == '\n')
			symname[len-1] = '\0';
		if((map->x_key = XStringToKeysym(symname)) == NoSymbol)
			fatal("Key '%s' does not exist", symname);
		map ++;
	}

	fclose(conf);

	return default_keymap;
}

/*
 * convert MHEGColour to internal format
 */

static void
display_colour(XRenderColor *out, MHEGColour *in)
{
	/* expand to 16 bits per channel */
	out->red = (in->r << 8) | in->r;
	out->green = (in->g << 8) | in->g;
	out->blue = (in->b << 8) | in->b;

	/* XRender has 0 as transparent and 65535 as opaque */
	out->alpha = ((255 - in->t) << 8) | (255 - in->t);

	return;
}

