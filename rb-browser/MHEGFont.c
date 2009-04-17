/*
 * MHEGFont.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "MHEGEngine.h"
#include "MHEGFont.h"
#include "utils.h"

/* internal functions */
typedef struct
{
	int width;
	int xOff;
} GlyphExtents;

static void close_font(MHEGFont *);

static bool get_font_attr(char **, unsigned int *, char *, unsigned int);

static LIST_OF(MHEGTextElement) *split_text(MHEGFont *, MHEGColour *, OctetString *, bool, int, Justification);
static int next_token(unsigned char *, int, int *);
static GlyphExtents *char_extents(MHEGFont *, int, Justification, int, int);
static bool is_breakable(int);

/* here we go */
void
free_MHEGFont(MHEGFont *f)
{
	close_font(f);

	return;
}

/*
 * free any internal data
 * values set by setName and setAttributes remain as they are
 */

static void
close_font(MHEGFont *f)
{
	if(f->font != NULL)
	{
		XftFontClose(MHEGEngine_getDisplay()->dpy, f->font);
		f->font = NULL;
	}

	return;
}

void
MHEGFont_init(MHEGFont *f)
{
	bzero(f, sizeof(MHEGFont));

	return;
}

/*
 * only font allowed by UK MHEG Profile is "rec://font/uk1"
 * FontBody is either an OctetString or a reference to a Font object
 * UK MHEG Profile does not support Font objects
 */

void
MHEGFont_setName(MHEGFont *font, FontBody *body)
{
	OctetString default_font = { 14, "rec://font/uk1" };
	OctetString *str;

	/* free any existing internal data */
	close_font(font);

	if(body->choice != FontBody_direct_font)
	{
		error("Unexpected FontBody type: %d", body->choice);
		MHEGFont_defaultName(font);
		return;
	}

	str = &body->u.direct_font;
	if(OctetString_cmp(str, &default_font) != 0)
	{
		error("Unknown font type: '%.*s'", str->size, str->data);
		MHEGFont_defaultName(font);
		return;
	}

	/* default is the only font */
	MHEGFont_defaultName(font);

	return;
}

/*
 * default font is "rec://font/uk1"
 * this is Tiresias Screenfont v7.51
 * this cost $1000 to licence from Bit Stream
 * alternatively, you can download Nebula DigiTV iTuner from http://www.nebula-electronics.com/
 * install it on a Windows box and copy C:\Windows\Fonts\tt7268m_802.ttf to your Linux box
 * if you don't have Tiresias Screenfont, we try FreeSans instead
 * if FreeSANS is not available either, we use whatever Xft returns for 'sans'
 */

static char *_default_font_name = NULL;
static char *_font_name_tiresias = "TiresiasScreenfont";
static char *_font_name_freesans = "FreeSans";
static char *_font_name_sans = "sans";

void
MHEGFont_defaultName(MHEGFont *font)
{
	Display *dpy = MHEGEngine_getDisplay()->dpy;
	char *xlfd_tiresias = "-*-TiresiasScreenfont-medium-r-normal-*";
	char *xlfd_freesans = "-*-freesans-medium-r-normal-*";
	char **names = NULL;
	int count;

	/* first time */
	if(_default_font_name == NULL)
	{
		/* do we have Tiresias */
		if((names = XListFonts(dpy, xlfd_tiresias, 1, &count)) != NULL)
		{
			_default_font_name = _font_name_tiresias;
		}
		else if((names = XListFonts(dpy, xlfd_freesans, 1, &count)) != NULL)
		{
			_default_font_name = _font_name_freesans;
			error("Font '%s' not available; using '%s' for 'rec://font/uk1'", _font_name_tiresias, _font_name_freesans);
		}
		else
		{
			_default_font_name = _font_name_sans;
			error("Font '%s' not available; using '%s' for 'rec://font/uk1'", _font_name_tiresias, _font_name_sans);
		}
		/* clean up */
		if(names != NULL)
			XFreeFontNames(names);
	}

	font->name = _default_font_name;

	return;
}

/*
 * FontAttributes are either short form (5 byte binary string)
 * or long form (eg "plain.24.28.0")
 */

void
MHEGFont_setAttributes(MHEGFont *font, OctetString *str)
{
	int style;
	unsigned int left;
	char *p;
	char out[32];

	/* free any existing internal data */
	close_font(font);

	/* doesn't seem to be any way of determining if it is the long or short form */
	if(str->size == 5)
	{
		/* short form */
		style = str->data[0] & 0x0f;
		/* only plain is supported by the UK MHEG Profile at the moment */
		if(style == 0)		/* plain */
			font->style = MHEGFontStyle_plain;
		else if(style == 1)	/* italic */
			font->style = MHEGFontStyle_plain;
		else if(style == 2)	/* bold */
			font->style = MHEGFontStyle_plain;
		else if(style == 3)	/* bold-italic */
			font->style = MHEGFontStyle_plain;
		else			/* default */
			font->style = MHEGFontStyle_plain;
		/* size */
		font->size = str->data[1];
		/* line spacing */
		font->line_spc = str->data[2];
		/* letter spacing (signed, MSB first) */
		font->letter_spc = (int16_t) ((str->data[3] << 8) | str->data[4]);
	}
	else
	{
		/* long form */
		left = str->size;
		p = str->data;
		/* style, only one possibility at the moment */
		if(get_font_attr(&p, &left, out, sizeof(out)))
		{
			if(strncmp(out, "plain", 5) == 0)
				font->style = MHEGFontStyle_plain;
			else
				font->style = MHEGFontStyle_plain;
		}
		else
		{
			/* error - use default */
			font->style = MHEGFontStyle_plain;
		}
		/* size */
		if(get_font_attr(&p, &left, out, sizeof(out)))
			font->size = atoi(out);
		else
			font->size = 24;
		/* line spacing */
		if(get_font_attr(&p, &left, out, sizeof(out)))
			font->line_spc = atoi(out);
		else
			font->line_spc = 28;
		/* letter spacing */
		if(get_font_attr(&p, &left, out, sizeof(out)))
			font->letter_spc = atoi(out);
		else
			font->letter_spc = 0;
	}

	return;
}

/*
 * copies *str upto the next '.' (or the end of *str) into out and \0 terminates it
 * *left should be the number of chars left in *str
 * out_max should be the amount of space in out
 * updates *str and *left
 * returns false if *left is 0 on entry
 */

static bool
get_font_attr(char **str, unsigned int *left, char *out, unsigned int out_max)
{
	if(*left == 0)
		return false;

	while(*left > 0 && **str != '.' && out_max > 1)
	{
		*out = **str;
		out ++;
		out_max --;
		(*str) ++;
		(*left) --;
	}

	/* terminate the output */
	*out = '\0';

	/* skip the . if we found one */
	if(*left > 0 && **str == '.')
	{
		(*str) ++;
		(*left) --;
	}

	return true;
}

/*
 * default FontAttributes for UK MHEG Profile are "plain.24.28.0"
 */

void
MHEGFont_defaultAttributes(MHEGFont *font)
{
	font->style = MHEGFontStyle_plain;
	font->size = 24;
	font->line_spc = 28;
	font->letter_spc = 0;

	return;
}

/*
 * layout the given text in the given box
 */

LIST_OF(MHEGTextElement) *
MHEGFont_layoutText(MHEGFont *f, MHEGColour *col, OctetString *text, OriginalBoxSize *box,
		    Justification hori, Justification vert, LineOrientation orient, StartCorner corner, bool wrap)
{
	LIST_OF(MHEGTextElement) *elem_list = NULL;
	LIST_TYPE(MHEGTextElement) *elem;
	FT_Face face;
	int yOffsetTop, yOffsetBottom, xOffsetLeft;
	int num_lines;
	int available_width;
	int xmax, ymax;
	LIST_TYPE(MHEGTextElement) *line_start, *next_line;

	/* is there any text */
	if(text->size == 0)
		return NULL;

	/* do we have the font metrics yet */
	if(f->font == NULL)
	{
		/*
		 * the font is opened at the size it will need to be output on the screen
		 * but all the layout calculations are done using the unscaled font metrics
		 */
		MHEGDisplay *d = MHEGEngine_getDisplay();
		/* UK MHEG Profile says 1 point = 1 pixel vertically */
		double pixel_size = (double) (f->size * d->yres) / (double) MHEG_YRES;
		/* UK MHEG Profile says use a fixed aspect ratio of 45/56 */
		double aspect = (45.0 * d->xres / MHEG_XRES) / (56.0 * d->yres / MHEG_YRES);
		f->font = XftFontOpen(d->dpy, DefaultScreen(d->dpy),
				      FC_FAMILY, FcTypeString, f->name,
				      FC_PIXEL_SIZE, FcTypeDouble, pixel_size,
				      FC_ASPECT, FcTypeDouble, aspect,
				      /* may not give us a scalable font */
				      FC_SCALABLE, FcTypeBool, FcTrue,
				      0);
		if(f->font == NULL)
			fatal("Font '%s' does not exist", f->name);
	}

	face = XftLockFace(f->font);

	/*
	 * make sure we got a scalable font
	 * if the font is not scalable the aspect ratio won't work
	 * and we can't use its outline metrics to do layout calculations
	 */
	if(!FT_IS_SCALABLE(face))
		fatal("Unable to find a scalable font for '%s'", f->name);

	/* UK MHEG Profile tells us to do layout like this (need some constants first): */
	yOffsetTop = face->bbox.yMax <= 0 ? 0 : ceil((double) (face->bbox.yMax * f->size) / (double) face->units_per_EM);
	yOffsetBottom = face->bbox.yMin >= 0 ? 0 : ceil((double) (- face->bbox.yMin * f->size) / (double) face->units_per_EM);
	/* take the 45/56 aspect ratio into account */
	xOffsetLeft = face->bbox.xMin >= 0 ? 0 : ceil((- face->bbox.xMin * f->size * 45.0) / (face->units_per_EM * 56.0));

	/* remember xOffsetLeft as this is the min amount a tab should advance the x pos */
	f->xOffsetLeft = xOffsetLeft;

	XftUnlockFace(f->font);

	/* 1a - find the max number of lines that can be rendered in the given area */
	if(box->y_length < (yOffsetBottom + yOffsetTop))
		num_lines = 1;
	else
		num_lines = ((box->y_length - (yOffsetBottom + yOffsetTop)) / f->line_spc) + 1;

	/* 1b - find the width available for each line */
	available_width = box->x_length - xOffsetLeft;

	/* 2 - work out where the line breaks should be */
	elem_list = split_text(f, col, text, wrap, available_width, hori);

	/* UK MHEG Profile says we can just do LineOrientation_horizontal and StartCorner_upper_left */

	/* 3 - work out where each line should be vertically */
	switch(vert)
	{
	case Justification_end:
		if(elem_list)
		{
			/* tail element is the lowest */
			ymax = elem_list->prev->item.y + yOffsetTop + yOffsetBottom;
			for(elem = elem_list; elem; elem = elem->next)
				elem->item.y += (int) box->y_length - ymax;
		}
		break;

	case Justification_centre:
		if(elem_list)
		{
			/* tail element is the lowest */
			ymax = elem_list->prev->item.y + yOffsetTop + yOffsetBottom;
			for(elem = elem_list; elem; elem = elem->next)
				elem->item.y += yOffsetTop + (((int) box->y_length - ymax) / 2);
		}
		break;

	case Justification_start:
	/* UK MHEG Profile says we can treat justified as start if we want */
	case Justification_justified:
	default:
		for(elem = elem_list; elem; elem = elem->next)
			elem->item.y += yOffsetTop;
		break;
	}

	/* 4 - work out where each character should be within each line */
	switch(hori)
	{
	case Justification_end:
		line_start = elem_list;
		while(line_start)
		{
			/* find the last element on this line */
			elem = line_start;
			while(elem->next && elem->next->item.y == elem->item.y)
				elem = elem->next;
			xmax = elem->item.x + elem->item.width;
			next_line = elem->next;
			for(elem = line_start; elem != next_line; elem = elem->next)
				elem->item.x += (int) box->x_length - xmax;
			line_start = next_line;
		}
		break;

	case Justification_centre:
		line_start = elem_list;
		while(line_start)
		{
			/* find the last element on this line */
			elem = line_start;
			while(elem->next && elem->next->item.y == elem->item.y)
				elem = elem->next;
			xmax = elem->item.x + elem->item.width;
			next_line = elem->next;
			for(elem = line_start; elem != next_line; elem = elem->next)
				elem->item.x += ((int) box->x_length - xmax) / 2;
			line_start = next_line;
		}
		break;

	case Justification_start:
	/* UK MHEG Profile says we can treat justified as start if we want */
	case Justification_justified:
	default:
		for(elem = elem_list; elem; elem = elem->next)
			elem->item.x += xOffsetLeft;
		break;
	}

/* TODO */
/* truncation: ie remove chars outside box */
/* setting the clip rectangle in TextClass_render solves the problem */
/* but UK profile says we shouldn't draw any partial characters */

	return elem_list;
}

/*
 * LIST_OF(MHEGTextElement) *
 * split_text(MHEGFont *f, MHEGColour *col, OctetString *text, bool wrap, int available_width, Justification hori)
 *
 * split the text into MHEGTextElement's at carriage return chars (\r)
 * also splits at colour change ESC sequences (0x1b, 0x43, 0x4, r, g, b, t)
 * if wrap is true, also word wrap the text at available_width
 * if hori is not Justification_start, tabs (0x09) are treated as spaces
 */

/* minimum nesting for colour changes is 16 in the UK MHEG Profile */
#define COLOUR_STACK_MAX	16

#define INIT_COLOUR_STACK(col)					\
do								\
{								\
	colour_stack_depth = 0;					\
	memcpy(&colour_stack[0], col, sizeof(MHEGColour));	\
	current_colour = &colour_stack[0];			\
}								\
while(0)

#define PUSH_COLOUR(raw)							\
do										\
{										\
	colour_stack_depth ++;							\
	if(colour_stack_depth < COLOUR_STACK_MAX)				\
	{									\
		MHEGColour_fromRaw(&colour_stack[colour_stack_depth], raw);	\
		current_colour = &colour_stack[colour_stack_depth];		\
	}									\
}										\
while(0)

#define POP_COLOUR								\
do										\
{										\
	if(colour_stack_depth > 0)						\
	{									\
		colour_stack_depth --;						\
		if(colour_stack_depth < COLOUR_STACK_MAX)			\
			current_colour = &colour_stack[colour_stack_depth];	\
	}									\
}										\
while(0)

/* tokens returned by next_token(), >=0 is a normal character */
#define TOK_NEWLINE		-1
#define TOK_COLOUR_START	-2
#define TOK_COLOUR_END		-3
#define TOK_IGNORE		-4

static LIST_OF(MHEGTextElement) *
split_text(MHEGFont *f, MHEGColour *col, OctetString *text, bool wrap, int available_width, Justification hori)
{
	LIST_OF(MHEGTextElement) *elem_list = NULL;	/* an empty list */
	LIST_TYPE(MHEGTextElement) *elem;
	MHEGColour colour_stack[COLOUR_STACK_MAX];
	int colour_stack_depth;
	MHEGColour *current_colour;
	FT_Face face;
	FT_UShort units_per_EM;
	int xpos, ypos;
	char *data;
	int size;
	int tok;
	int len;
	bool new_elem;
	GlyphExtents *ext;
	unsigned char *break_start, *break_end;
	int break_width;
	int break_size;
	int break_colour_stack;
	int previous;

	face = XftLockFace(f->font);
	units_per_EM = face->units_per_EM;
	XftUnlockFace(f->font);

	/* remember the current colour */
	INIT_COLOUR_STACK(col);

	/* current x,y position */
	xpos = 0;
	ypos = 0;

	/* current text position and bytes left */
	size = text->size;
	data = text->data;

	/* havent found anywhere to break text yet */
	break_start = NULL;
	break_width = 0;
	break_colour_stack = 0;
	break_end = NULL;
	break_size = 0;

	/* no previous character */
	previous = -1;

	/* no current element yet */
	elem = NULL;

	/* create the first element */
	new_elem = true;
	/* add a new MHEGTextElement for each line and text colour */
	while(size > 0)
	{
		tok = next_token(data, size, &len);
		/* check for new line or colour change */
		if(tok == TOK_NEWLINE)
		{
			/* move down a line */
			ypos += f->line_spc;
			xpos = 0;
			previous = -1;
			break_start = NULL;
			/* start a new element */
			new_elem = true;
		}
		else if(tok == TOK_COLOUR_START)
		{
			/* new colour is: 0x1b, 0x43, 0x4, r, g, b, t */
			PUSH_COLOUR(data + 3);
			/* start a new element */
			new_elem = true;
		}
		else if(tok == TOK_COLOUR_END)
		{
			POP_COLOUR;
			/* start a new element */
			new_elem = true;
		}
		else if(tok == TOK_IGNORE)
		{
			/*
			 * start a new element
			 * => we ignore unknown tags themselves but still use any text inside them
			 */
			new_elem = true;
		}
		else	/* (possibly multibyte) character */
		{
			/* do we need to start a new element */
			if(new_elem)
			{
				/* if the current element is empty, reuse it */
				if(elem == NULL || elem->item.size > 0)
				{
					elem = safe_malloc(sizeof(LIST_TYPE(MHEGTextElement)));
					LIST_APPEND(&elem_list, elem);
				}
				/* current colour */
				memcpy(&elem->item.col, current_colour, sizeof(MHEGColour));
				/* current point in the text */
				elem->item.data = data;
				elem->item.size = 0;
				/* round up/down x position */
				elem->item.x = (xpos + (units_per_EM / 2)) / units_per_EM;
				elem->item.y = ypos;
				elem->item.width = 0;
				/* created it now */
				new_elem = false;
			}
			/*
			 * have we found somewhere we can break text
			 * dont bother if this is the last character in the text
			 */
			if(wrap && size > len && is_breakable(tok))
			{
				/* remember where it starts and what colour we are using */
				break_start = data;
				break_width = elem->item.width;
				break_colour_stack = colour_stack_depth;
				/* find the end of this sequence of breakable chars */
				do
				{
					data += len;
					size -= len;
					/* add the breakable char to the current element */
					elem->item.size += len;
					ext = char_extents(f, xpos, hori, previous, tok);
					/* round up x pos to find the width in pixels */
					elem->item.width = (xpos + ext->width + (units_per_EM - 1)) / units_per_EM;
					elem->item.width -= elem->item.x;
					/* advance x pos */
					xpos += ext->xOff;
					/* remember the previous character */
					previous = tok;
					/* see what the next token is */
					tok = next_token(data, size, &len);
				}
				while(size > 0 && is_breakable(tok));
				/* remember where it ends (the character after the last breakable one) */
				break_end = data;
				break_size = size;
				/* dont move any further forward this time round the loop */
				len = 0;
			}
			else
			{
				/*
				 * do we need to word wrap
				 * have we found a breaking char yet
				 * and will tok take us over the available_width
				 */
				ext = char_extents(f, xpos, hori, previous, tok);
				if(wrap
				&& break_start != NULL
				&& xpos + ext->width > (available_width * units_per_EM))
				{
					/*
					 * if the current elem does not contain break_start
					 * lose elements until we find the one that does
					 * this will happen if we changed colour since we found break_start
					 */
					while(elem->item.data > break_start)
					{
						LIST_REMOVE(&elem_list, elem);
						safe_free(elem);
						/* current element is the tail */
						elem = elem_list->prev;
					}
					/* truncate the current element to break_start */
					elem->item.size = break_start - elem->item.data;
					elem->item.width = break_width;
					/* go back to the colour we were using when we found the break */
					colour_stack_depth = break_colour_stack;
					/* move down a line */
					ypos += f->line_spc;
					xpos = 0;
					previous = -1;
					break_start = NULL;
					/* restart from the end of the break */
					data = break_end;
					size = break_size;
					/* create a new element */
					new_elem = true;
					/* dont move any further forward this time round the loop */
					len = 0;
				}
				else
				{
					/* add the next character to the current element */
					elem->item.size += len;
					/* round up x pos to find the width in pixels */
					elem->item.width = (xpos + ext->width + (units_per_EM - 1)) / units_per_EM;
					elem->item.width -= elem->item.x;
					/* advance x pos */
					xpos += ext->xOff;
					/* remember the previous character */
					previous = tok;
				}
			}
		}
		/* move to the next token */
		data += len;
		size -= len;
	}

	return elem_list;
}

/*
 * returns the next character in the string or a TOK_xxx token
 * characters are encoded using UTF8, ie they may be 1 byte or multibyte
 * sets *len to the number of chars used
 * size should be the number of chars available in data
 */

static int
next_token(unsigned char *data, int size, int *len)
{
	int tok;
	int tag_len;

	/* is it a new line */
	if(size >= 1 && *data == '\r')
	{
		tok = TOK_NEWLINE;
		*len = 1;
		/* \r\n counts as \r */
		if(size >= 2 && *(data + 1) == '\n')
			*len = 2;
	}
	/* is it someone using \n instead of \r for a newline */
	else if(size >= 1 && *data == '\n')
	{
		tok = TOK_NEWLINE;
		*len = 1;
	}
	/* is it a marker */
	else if(size >= 1 && *data >= 0x1c && *data <= 0x1f)
	{
		tok = TOK_IGNORE;
		*len = 1;
	}
	/* is it an end tag */
	else if(size >= 2 && *data == 0x1b && *(data + 1) >= 0x60)
	{
		/* end of the colour tag - 'c' */
		if(*(data + 1) == 0x63)
			tok = TOK_COLOUR_END;
		else
			tok = TOK_IGNORE;
		*len = 2;
	}
	/* is it a start tag */
	else if(size >= 3 && *data == 0x1b)
	{
		/* data is 0x1b, tag_val, tag_data_len, tag_data */
		tag_len = 3 + (*(data + 2));
		/*
		 * work around a bug where Channel 4 broadcast colour change tags with
		 * the length byte set to 0, even though its followed by the 4 colour bytes
		 */
		if(*(data + 1) == 0x43)
			tag_len = 3 + 4;
		if(size >= tag_len)
		{
			/* is it a new colour - 'C' */
			if(*(data + 1) == 0x43)
				tok = TOK_COLOUR_START;
			else
				tok = TOK_IGNORE;
			*len = tag_len;
		}
		else
		{
			/* not enough data, ignore the 0x1b char */
			tok = TOK_IGNORE;
			*len = 1;
		}
	}
	/* a UTF8 character */
	else
	{
		tok = next_utf8(data, size, len);
	}

	return tok;
}

/*
 * returns the extents of the character 'measure'
 * if previous != -1, takes into account any kerning between the two
 * in the returned struct:
 * ext->width is the width of the char
 * ext->xOff is where the next char should start and includes f->letter_spc
 * if 'measure' is a tab (0x09) width will be 0 and xOff will give the distance to the next tab stop
 * (thats why we need to know the current xpos)
 * if hori is not Justification_start, tabs are just treated as spaces
 * all values are in font units, ie you need to divide by units_per_EM to get values in pixels
 * returns a ptr to a static struct that will be overwritten by the next call to this routine
 */

static GlyphExtents _ext;

static GlyphExtents *
char_extents(MHEGFont *f, int xpos, Justification hori, int previous, int measure)
{
	FT_Face face;
	FT_GlyphSlot slot;
	FT_UInt previous_glyph;
	FT_UInt measure_glyph;
	FT_Error err;
	FT_Vector kern;
	int ntabs;

	/* easy case, just advance to next tab stop */
	if(measure == 0x09 && hori == Justification_start)
	{
		face = XftLockFace(f->font);
		_ext.width = 0;
		/* min amount a tab should advance the text pos */
		ntabs = xpos + (f->xOffsetLeft * face->units_per_EM);
		/* move to the next tab stop */
		ntabs /= (MHEG_TAB_WIDTH * face->units_per_EM);
		_ext.xOff = ((ntabs + 1) * MHEG_TAB_WIDTH * face->units_per_EM) - xpos;
		XftUnlockFace(f->font);
		return &_ext;
	}

	/* if not Justification_start, tabs are treated as spaces */
	if(hori != Justification_start)
	{
		measure = (measure == 0x09) ? 0x20 : measure;
		previous = (previous == 0x09) ? 0x20 : previous;
	}

	/* get the metrics for measure */
	face = XftLockFace(f->font);
	slot = face->glyph;

	measure_glyph = FT_Get_Char_Index(face, measure);
	err = FT_Load_Glyph(face, measure_glyph, FT_LOAD_NO_SCALE);
	if(err)
	{
		_ext.width = 0;
		_ext.xOff = 0;
	}
	else
	{
		_ext.width = (slot->metrics.horiBearingX + slot->metrics.width) * f->size;
		_ext.xOff = slot->advance.x * f->size;
	}

	/* take any kerning into account */
	if(previous != -1 && previous != 0x09 && FT_HAS_KERNING(face))
	{
		previous_glyph = FT_Get_Char_Index(face, previous);
		FT_Get_Kerning(face, previous_glyph, measure_glyph, FT_KERNING_UNSCALED, &kern);
		_ext.width += kern.x * f->size;
		_ext.xOff += kern.x * f->size;
	}

	/* add on (letter spacing / 256) * units_per_EM */
	_ext.xOff += (face->units_per_EM * f->letter_spc) / 256;

	/* take aspect ratio into account */
	_ext.width = (_ext.width * 45) / 56;
	_ext.xOff = (_ext.xOff * 45) / 56;

	XftUnlockFace(f->font);

	return &_ext;
}

/*
 * returns true if we can word wrap text at the given character
 */

bool
is_breakable(int letter)
{
	return (letter == 0x09 || letter == 0x20);
}

