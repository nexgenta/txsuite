/*
 * LineArtClass.c
 */

#include "MHEGEngine.h"
#include "MHEGColour.h"
#include "ISO13522-MHEG-5.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "clone.h"

void
default_LineArtClassInstanceVars(LineArtClass *t, LineArtClassInstanceVars *v)
{
	bzero(v, sizeof(LineArtClassInstanceVars));

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

	/* LineArtClass */
	v->LineWidth = t->original_line_width;
	v->LineStyle = t->original_line_style;

	/* default colour is black */
	if(t->have_original_ref_line_colour)
		MHEGColour_fromColour(&v->OriginalRefLineColour, &t->original_ref_line_colour);
	else
		MHEGColour_black(&v->OriginalRefLineColour);
	/* initial line colour */
	memcpy(&v->RefLineColour, &v->OriginalRefLineColour, sizeof(MHEGColour));

	/* default is transparent */
	if(t->have_original_ref_fill_colour)
		MHEGColour_fromColour(&v->OriginalRefFillColour, &t->original_ref_fill_colour);
	else
		MHEGColour_transparent(&v->OriginalRefFillColour);
	/* initial fill colour */
	memcpy(&v->RefFillColour, &v->OriginalRefFillColour, sizeof(MHEGColour));

	/* derived classes must init this if they want to use it */
	v->canvas = NULL;

	return;
}

void
free_LineArtClassInstanceVars(LineArtClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	if(v->canvas != NULL)
		free_MHEGCanvas(v->canvas);

	return;
}

void
LineArtClass_Preparation(LineArtClass *t)
{
	error("LineArtClass: %s; LineArt is an abstract class", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
LineArtClass_Activation(LineArtClass *t)
{
	error("LineArtClass: %s; LineArt is an abstract class", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
LineArtClass_Deactivation(LineArtClass *t)
{
	error("LineArtClass: %s; LineArt is an abstract class", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
LineArtClass_Destruction(LineArtClass *t)
{
	error("LineArtClass: %s; LineArt is an abstract class", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

