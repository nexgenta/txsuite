/*
 * MHEGFont.h
 */

#ifndef __MHEGFONT_H__
#define __MHEGFONT_H__

#include <X11/Xft/Xft.h>

#include "der_decode.h"

/* font */
typedef enum
{
	MHEGFontStyle_invalid = 0,
	MHEGFontStyle_plain
} MHEGFontStyle;

typedef struct
{
	/* FontBody */
	char *name;
	/* FontAttributes */
	MHEGFontStyle style;
	int size;
	int line_spc;
	int letter_spc;
	/* internal stuff */
	XftFont *font;		/* scaled up if fullscreen mode */
	int xOffsetLeft;	/* minimum amount tab should advance (pixels) */
} MHEGFont;

/*
 * to layout the text we create a list of MHEGTextElement's
 */
typedef struct
{
	MHEGColour col;
	int x;			/* 0-MHEG_XRES, need to add on the origin of the VisibleClass */
	int y;			/* 0-MHEG_YRES, need to add on the origin of the VisibleClass */
	int width;		/* only used in layout calculations */
	unsigned int size;	/* number of characters */
	unsigned char *data;	/* the characters */
} MHEGTextElement;

DEFINE_LIST_OF(MHEGTextElement);

/* functions */
void free_MHEGFont(MHEGFont *);

void MHEGFont_init(MHEGFont *);

void MHEGFont_setName(MHEGFont *, FontBody *);
void MHEGFont_defaultName(MHEGFont *);

void MHEGFont_setAttributes(MHEGFont *, OctetString *);
void MHEGFont_defaultAttributes(MHEGFont *);

LIST_OF(MHEGTextElement) *MHEGFont_layoutText(MHEGFont *, MHEGColour *, OctetString *, OriginalBoxSize *,
					      Justification, Justification, LineOrientation, StartCorner, bool);

#endif	/* __MHEGFONT_H__ */

