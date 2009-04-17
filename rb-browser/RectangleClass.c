/*
 * RectangleClass.c
 */

#include "MHEGEngine.h"
#include "RectangleClass.h"
#include "LineArtClass.h"
#include "RootClass.h"
#include "ExternalReference.h"
#include "GenericInteger.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "ObjectRefVariableClass.h"
#include "GenericObjectReference.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
RectangleClass_Preparation(RectangleClass *t)
{
	verbose("RectangleClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_LineArtClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
RectangleClass_Activation(RectangleClass *t)
{
	verbose("RectangleClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		RectangleClass_Preparation(t);

	/* has it already been activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

	/* set its RunningStatus */
	t->rootClass.inst.RunningStatus = true;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	/* now its RunningStatus is true, get it drawn at its position in the application's DisplayStack */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_Deactivation(RectangleClass *t)
{
	verbose("RectangleClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_Destruction(RectangleClass *t)
{
	verbose("RectangleClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already destroyed */
	if(!t->rootClass.inst.AvailabilityStatus)
		return;

	/* remove it from the DisplayStack */
	MHEGEngine_removeVisibleObject(&t->rootClass);

	/* RootClass Destruction */
	/* Deactivate it if it is running */
	if(t->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		RectangleClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_LineArtClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
RectangleClass_Clone(RectangleClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("RectangleClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, RectangleClass, rectangle);

	return;
}

void
RectangleClass_SetPosition(RectangleClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("RectangleClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_position = t->inst.Position.x_position;
	old.y_position = t->inst.Position.y_position;

	t->inst.Position.x_position = GenericInteger_getInteger(&params->new_x_position, caller_gid);
	t->inst.Position.y_position = GenericInteger_getInteger(&params->new_y_position, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&old, &t->inst.BoxSize);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
RectangleClass_GetPosition(RectangleClass *t, GetPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("RectangleClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* X position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("RectangleClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.x_position);

	/* Y position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("RectangleClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.y_position);

	return;
}

void
RectangleClass_SetBoxSize(RectangleClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	OriginalBoxSize old;

	verbose("RectangleClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_length = t->inst.BoxSize.x_length;
	old.y_length = t->inst.BoxSize.y_length;

	t->inst.BoxSize.x_length = GenericInteger_getInteger(&params->x_new_box_size, caller_gid);
	t->inst.BoxSize.y_length = GenericInteger_getInteger(&params->y_new_box_size, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&t->inst.Position, &old);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
RectangleClass_GetBoxSize(RectangleClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("RectangleClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	/* width */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("RectangleClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.x_length);

	/* height */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("RectangleClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.y_length);

	return;
}

void
RectangleClass_BringToFront(RectangleClass *t)
{
	verbose("RectangleClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_SendToBack(RectangleClass *t)
{
	verbose("RectangleClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_PutBefore(RectangleClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("RectangleClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

	if(((ref = GenericObjectReference_getObjectReference(&params->reference_visible, caller_gid)) != NULL)
	&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
	{
		MHEGEngine_putBefore(&t->rootClass, obj);
		/* if it is active, redraw it */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
RectangleClass_PutBehind(RectangleClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("RectangleClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

	if(((ref = GenericObjectReference_getObjectReference(&params->reference_visible, caller_gid)) != NULL)
	&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
	{
		MHEGEngine_putBehind(&t->rootClass, obj);
		/* if it is active, redraw it */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
RectangleClass_SetPaletteRef(RectangleClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("RectangleClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RectangleClass_SetLineWidth(RectangleClass *t, SetLineWidth *params, OctetString *caller_gid)
{
	verbose("RectangleClass: %s; SetLineWidth", ExternalReference_name(&t->rootClass.inst.ref));

	t->inst.LineWidth = GenericInteger_getInteger(&params->new_line_width, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_SetLineStyle(RectangleClass *t, SetLineStyle *params, OctetString *caller_gid)
{
	verbose("RectangleClass: %s; SetLineStyle", ExternalReference_name(&t->rootClass.inst.ref));

	t->inst.LineStyle = GenericInteger_getInteger(&params->new_line_style, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_SetLineColour(RectangleClass *t, SetLineColour *params, OctetString *caller_gid)
{
	verbose("RectangleClass: %s; SetLineColour", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGColour_fromNewColour(&t->inst.RefLineColour, &params->new_line_colour, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_SetFillColour(RectangleClass *t, SetFillColour *params, OctetString *caller_gid)
{
	verbose("RectangleClass: %s; SetFillColour", ExternalReference_name(&t->rootClass.inst.ref));

	/* if no colour is given, use transparent */
	if(params->have_new_fill_colour)
		MHEGColour_fromNewColour(&t->inst.RefFillColour, &params->new_fill_colour, caller_gid);
	else
		MHEGColour_transparent(&t->inst.RefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
RectangleClass_render(RectangleClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;
	XYPosition p;
	unsigned int len;

	verbose("RectangleClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

	/* fill it */
	MHEGDisplay_fillRectangle(d, &ins_pos, &ins_box, &t->inst.RefFillColour);

	/* draw the outline */
	p.x_position = t->inst.Position.x_position;
	p.y_position = t->inst.Position.y_position;
	len = t->inst.BoxSize.y_length - t->inst.LineWidth;
	MHEGDisplay_drawVertLine(d, &p, len, t->inst.LineWidth, t->inst.LineStyle, &t->inst.RefLineColour);

	p.x_position = t->inst.Position.x_position + t->inst.BoxSize.x_length - t->inst.LineWidth;
	p.y_position = t->inst.Position.y_position;
	len = t->inst.BoxSize.y_length - t->inst.LineWidth;
	MHEGDisplay_drawVertLine(d, &p, len, t->inst.LineWidth, t->inst.LineStyle, &t->inst.RefLineColour);

	p.x_position = t->inst.Position.x_position;
	p.y_position = t->inst.Position.y_position;
	len = t->inst.BoxSize.x_length - t->inst.LineWidth;
	MHEGDisplay_drawHoriLine(d, &p, len, t->inst.LineWidth, t->inst.LineStyle, &t->inst.RefLineColour);

	p.x_position = t->inst.Position.x_position;
	p.y_position = t->inst.Position.y_position + t->inst.BoxSize.y_length - t->inst.LineWidth;
	len = t->inst.BoxSize.x_length - t->inst.LineWidth;
	MHEGDisplay_drawHoriLine(d, &p, len, t->inst.LineWidth, t->inst.LineStyle, &t->inst.RefLineColour);

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

