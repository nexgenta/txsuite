/*
 * MHEGColour.h
 */

#ifndef __MHEGCOLOUR_H__
#define __MHEGCOLOUR_H__

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char t;	/* transparency, 0 => opaque */
} MHEGColour;

/* transparency values */
#define MHEGCOLOUR_OPAQUE	0
#define MHEGCOLOUR_TRANSPARENT	255

void MHEGColour_fromColour(MHEGColour *, Colour *);
void MHEGColour_fromNewColour(MHEGColour *, NewColour *, OctetString *);
void MHEGColour_fromRaw(MHEGColour *, unsigned char *);
void MHEGColour_fromHex(MHEGColour *, unsigned char *);

void MHEGColour_black(MHEGColour *);
void MHEGColour_white(MHEGColour *);
void MHEGColour_transparent(MHEGColour *);

#endif	/* __MHEGCOLOUR_H__ */

