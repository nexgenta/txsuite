/*
 * MHEGColour.c
 */

#include "ISO13522-MHEG-5.h"
#include "MHEGColour.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "utils.h"

void
MHEGColour_fromColour(MHEGColour *out, Colour *in)
{
	switch(in->choice)
	{
	case Colour_colour_index:
		/* index into the palette */
		error("MHEGColour_fromColour: PaletteClass not supported (index=%d)", in->u.colour_index);
		/* UK MHEG Profile says we don't support Palettes, so choose a default colour */
		MHEGColour_black(out);
		break;

	case Colour_absolute_colour:
		/* 4 byte OctetString: RGBA */
		if(in->u.absolute_colour.size == 4)
		{
			MHEGColour_fromRaw(out, in->u.absolute_colour.data);
		}
		/* 9 byte hex digit string: #RRGGBBAA */
		else if(in->u.absolute_colour.size == 9)
		{
			MHEGColour_fromHex(out, in->u.absolute_colour.data);
		}
		else
		{
			error("Invalid AbsoluteColour (size=%u)", in->u.absolute_colour.size);
			/* choose a default */
			MHEGColour_black(out);
		}
		break;

	default:
		error("Unknown Colour type: %d", in->choice);
		/* choose a default */
		MHEGColour_black(out);
		break;
	}

	return;
}

void
MHEGColour_fromNewColour(MHEGColour *out, NewColour *in, OctetString *caller_gid)
{
	int index;
	OctetString *abs;

	switch(in->choice)
	{
	case NewColour_new_colour_index:
		/* index into the palette */
		index = GenericInteger_getInteger(&in->u.new_colour_index, caller_gid);
		error("MHEGColour_fromNewColour: PaletteClass not supported (index=%d)", index);
		/* UK MHEG Profile says we don't support Palettes, so choose a default colour */
		MHEGColour_black(out);
		break;

	case NewColour_new_absolute_colour:
		abs = GenericOctetString_getOctetString(&in->u.new_absolute_colour, caller_gid);
		/* 4 byte OctetString: RGBA */
		if(abs->size == 4)
		{
			MHEGColour_fromRaw(out, abs->data);
		}
		/* 9 byte hex digit string: #RRGGBBAA */
		else if(abs->size == 9)
		{
			MHEGColour_fromHex(out, abs->data);
		}
		else
		{
			error("Invalid AbsoluteColour (size=%u)", abs->size);
			/* choose a default */
			MHEGColour_black(out);
		}
		break;

	default:
		error("Unknown Colour type: %d", in->choice);
		/* choose a default */
		MHEGColour_black(out);
		break;
	}

	return;
}

/*
 * colour from 4 bytes: RGBA
 */

void
MHEGColour_fromRaw(MHEGColour *out, unsigned char *data)
{
	out->r = data[0];
	out->g = data[1];
	out->b = data[2];
	out->t = data[3];

	return;
}

/*
 * colour from 9 hex digits: #RRGGBBAA
 */

void
MHEGColour_fromHex(MHEGColour *out, unsigned char *data)
{
	/* skip the # */
	out->r = (char2hex(data[1]) << 4) + char2hex(data[2]);
	out->g = (char2hex(data[3]) << 4) + char2hex(data[4]);
	out->b = (char2hex(data[5]) << 4) + char2hex(data[6]);
	out->t = (char2hex(data[7]) << 4) + char2hex(data[8]);

	return;
}

void
MHEGColour_black(MHEGColour *c)
{
	c->r = c->g = c->b = 0;
	c->t = MHEGCOLOUR_OPAQUE;

	return;
}

void
MHEGColour_white(MHEGColour *c)
{
	c->r = c->g = c->b = 255;
	c->t = MHEGCOLOUR_OPAQUE;

	return;
}

void
MHEGColour_transparent(MHEGColour *c)
{
	c->r = c->g = c->b = 0;
	c->t = MHEGCOLOUR_TRANSPARENT;

	return;
}

