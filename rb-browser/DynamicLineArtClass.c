/*
 * DynamicLineArtClass.c
 */

#include "MHEGEngine.h"
#include "DynamicLineArtClass.h"
#include "LineArtClass.h"
#include "RootClass.h"
#include "ExternalReference.h"
#include "GenericObjectReference.h"
#include "GenericInteger.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "rtti.h"

void
DynamicLineArtClass_Preparation(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_LineArtClassInstanceVars(t, &t->inst);

	/* offscreen canvas to draw on */
	t->inst.canvas = new_MHEGCanvas(t->inst.BoxSize.x_length, t->inst.BoxSize.y_length);

	/*
	 * default value for BorderedBoundingBox is true
	 * the border uses OriginalLineWidth/Style/Colour
	 * ie it can never change
	 */
	if(!t->have_bordered_bounding_box || t->bordered_bounding_box)
		MHEGCanvas_setBorder(t->inst.canvas, t->original_line_width, t->original_line_style, &t->inst.OriginalRefLineColour);
	/* now we have set the border, clear the drawing area */
	MHEGCanvas_clear(t->inst.canvas, &t->inst.OriginalRefFillColour);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
DynamicLineArtClass_Activation(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		DynamicLineArtClass_Preparation(t);

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
DynamicLineArtClass_Deactivation(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_Destruction(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		DynamicLineArtClass_Deactivation(t);
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
DynamicLineArtClass_Clone(DynamicLineArtClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: DynamicLineArtClass_Clone not yet implemented\n");
	return;
}

void
DynamicLineArtClass_SetPosition(DynamicLineArtClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("DynamicLineArtClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* corrigendum says we don't need to clear to OriginalRefFillColour */

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
DynamicLineArtClass_GetPosition(DynamicLineArtClass *t, GetPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("DynamicLineArtClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* X position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.x_position);

	/* Y position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.y_position);

	return;
}

void
DynamicLineArtClass_SetBoxSize(DynamicLineArtClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	OriginalBoxSize old;

	verbose("DynamicLineArtClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_length = t->inst.BoxSize.x_length;
	old.y_length = t->inst.BoxSize.y_length;

	t->inst.BoxSize.x_length = GenericInteger_getInteger(&params->x_new_box_size, caller_gid);
	t->inst.BoxSize.y_length = GenericInteger_getInteger(&params->y_new_box_size, caller_gid);

	/* spec says we should fill the drawing area with OriginalRefFillColour */

	/* delete the old drawing area and create a new one at the new size */
	free_MHEGCanvas(t->inst.canvas);
	t->inst.canvas = new_MHEGCanvas(t->inst.BoxSize.x_length, t->inst.BoxSize.y_length);

	/* default value for BorderedBoundingBox is true */
	if(!t->have_bordered_bounding_box || t->bordered_bounding_box)
		MHEGCanvas_setBorder(t->inst.canvas, t->original_line_width, t->original_line_style, &t->inst.OriginalRefLineColour);

	/* now we have set the border, clear the drawing area */
	MHEGCanvas_clear(t->inst.canvas, &t->inst.OriginalRefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&t->inst.Position, &old);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
DynamicLineArtClass_GetBoxSize(DynamicLineArtClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("DynamicLineArtClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	/* width */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.x_length);

	/* height */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.y_length);

	return;
}

void
DynamicLineArtClass_BringToFront(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* corrigendum says we don't need to clear to OriginalRefFillColour */

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_SendToBack(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* corrigendum says we don't need to clear to OriginalRefFillColour */

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_PutBefore(DynamicLineArtClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("DynamicLineArtClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

	if(((ref = GenericObjectReference_getObjectReference(&params->reference_visible, caller_gid)) != NULL)
	&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
	{
		MHEGEngine_putBefore(&t->rootClass, obj);
		/* corrigendum says we don't need to clear to OriginalRefFillColour */
		/* if it is active, redraw it */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
DynamicLineArtClass_PutBehind(DynamicLineArtClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("DynamicLineArtClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

	if(((ref = GenericObjectReference_getObjectReference(&params->reference_visible, caller_gid)) != NULL)
	&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
	{
		MHEGEngine_putBehind(&t->rootClass, obj);
		/* corrigendum says we don't need to clear to OriginalRefFillColour */
		/* if it is active, redraw it */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
DynamicLineArtClass_SetPaletteRef(DynamicLineArtClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("DynamicLineArtClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
DynamicLineArtClass_SetLineWidth(DynamicLineArtClass *t, SetLineWidth *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; SetLineWidth", ExternalReference_name(&t->rootClass.inst.ref));

	t->inst.LineWidth = GenericInteger_getInteger(&params->new_line_width, caller_gid);

	return;
}

void
DynamicLineArtClass_SetLineStyle(DynamicLineArtClass *t, SetLineStyle *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; SetLineStyle", ExternalReference_name(&t->rootClass.inst.ref));

	t->inst.LineStyle = GenericInteger_getInteger(&params->new_line_style, caller_gid);

	return;
}

void
DynamicLineArtClass_SetLineColour(DynamicLineArtClass *t, SetLineColour *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; SetLineColour", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGColour_fromNewColour(&t->inst.RefLineColour, &params->new_line_colour, caller_gid);

	return;
}

void
DynamicLineArtClass_SetFillColour(DynamicLineArtClass *t, SetFillColour *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; SetFillColour", ExternalReference_name(&t->rootClass.inst.ref));

	/* if no colour is given, use transparent */
	if(params->have_new_fill_colour)
		MHEGColour_fromNewColour(&t->inst.RefFillColour, &params->new_fill_colour, caller_gid);
	else
		MHEGColour_transparent(&t->inst.RefFillColour);

	return;
}

void
DynamicLineArtClass_GetLineWidth(DynamicLineArtClass *t, GetLineWidth *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("DynamicLineArtClass: %s; GetLineWidth", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->line_width_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetLineWidth: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.LineWidth);

	return;
}

void
DynamicLineArtClass_GetLineStyle(DynamicLineArtClass *t, GetLineStyle *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("DynamicLineArtClass: %s; GetLineStyle", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->line_style_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("DynamicLineArtClass: GetLineStyle: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.LineStyle);

	return;
}

void
DynamicLineArtClass_GetLineColour(DynamicLineArtClass *t, GetLineColour *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; GetLineColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: DynamicLineArtClass_GetLineColour not yet implemented\n");
	return;
}

void
DynamicLineArtClass_GetFillColour(DynamicLineArtClass *t, GetFillColour *params, OctetString *caller_gid)
{
	verbose("DynamicLineArtClass: %s; GetFillColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: DynamicLineArtClass_GetFillColour not yet implemented\n");
	return;
}

void
DynamicLineArtClass_DrawArc(DynamicLineArtClass *t, DrawArc *params, OctetString *caller_gid)
{
	XYPosition pos;
	OriginalBoxSize box;
	int start;
	int arc;

	verbose("DynamicLineArtClass: %s; DrawArc", ExternalReference_name(&t->rootClass.inst.ref));

	pos.x_position = GenericInteger_getInteger(&params->x, caller_gid);
	pos.y_position = GenericInteger_getInteger(&params->y, caller_gid);
	box.x_length = GenericInteger_getInteger(&params->ellipse_width, caller_gid);
	box.y_length = GenericInteger_getInteger(&params->ellipse_height, caller_gid);
	start = GenericInteger_getInteger(&params->start_angle, caller_gid);
	arc = GenericInteger_getInteger(&params->arc_angle, caller_gid);

	/* ISO spec says ArcAngle should not be 0 */
	if(arc == 0)
	{
		error("DynamicLineArtClass_DrawArc: invalid ArcAngle (%d)", arc);
		return;
	}
	/* corrigendum puts these limits on the ellipse size */
	if(box.x_length < 0 || box.y_length < 0)
	{
		error("DynamicLineArtClass_DrawArc: invalid ellipse size (%d,%d)", box.x_length, box.y_length);
		return;
	}

	MHEGCanvas_drawArc(t->inst.canvas, &pos, &box, start, arc,
			   t->inst.LineWidth, t->inst.LineStyle,
			   &t->inst.RefLineColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_DrawSector(DynamicLineArtClass *t, DrawSector *params, OctetString *caller_gid)
{
	XYPosition pos;
	OriginalBoxSize box;
	int start;
	int arc;

	verbose("DynamicLineArtClass: %s; DrawSector", ExternalReference_name(&t->rootClass.inst.ref));

	pos.x_position = GenericInteger_getInteger(&params->x, caller_gid);
	pos.y_position = GenericInteger_getInteger(&params->y, caller_gid);
	box.x_length = GenericInteger_getInteger(&params->ellipse_width, caller_gid);
	box.y_length = GenericInteger_getInteger(&params->ellipse_height, caller_gid);
	start = GenericInteger_getInteger(&params->start_angle, caller_gid);
	arc = GenericInteger_getInteger(&params->arc_angle, caller_gid);

	/* ISO spec says ArcAngle should not be 0 */
	if(arc == 0)
	{
		error("DynamicLineArtClass_DrawSector: invalid ArcAngle (%d)", arc);
		return;
	}
	/* corrigendum puts these limits on the ellipse size */
	if(box.x_length < 0 || box.y_length < 0)
	{
		error("DynamicLineArtClass_DrawSector: invalid ellipse size (%d,%d)", box.x_length, box.y_length);
		return;
	}

	MHEGCanvas_drawSector(t->inst.canvas, &pos, &box, start, arc,
			      t->inst.LineWidth, t->inst.LineStyle,
			      &t->inst.RefLineColour, &t->inst.RefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_DrawLine(DynamicLineArtClass *t, DrawLine *params, OctetString *caller_gid)
{
	XYPosition p1;
	XYPosition p2;

	verbose("DynamicLineArtClass: %s; DrawLine", ExternalReference_name(&t->rootClass.inst.ref));

	p1.x_position = GenericInteger_getInteger(&params->x1, caller_gid);
	p1.y_position = GenericInteger_getInteger(&params->y1, caller_gid);
	p2.x_position = GenericInteger_getInteger(&params->x2, caller_gid);
	p2.y_position = GenericInteger_getInteger(&params->y2, caller_gid);

	MHEGCanvas_drawLine(t->inst.canvas, &p1, &p2,
			    t->inst.LineWidth, t->inst.LineStyle,
			    &t->inst.RefLineColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_DrawOval(DynamicLineArtClass *t, DrawOval *params, OctetString *caller_gid)
{
	XYPosition pos;
	OriginalBoxSize box;

	verbose("DynamicLineArtClass: %s; DrawOval", ExternalReference_name(&t->rootClass.inst.ref));

	pos.x_position = GenericInteger_getInteger(&params->x, caller_gid);
	pos.y_position = GenericInteger_getInteger(&params->y, caller_gid);
	box.x_length = GenericInteger_getInteger(&params->ellipse_width, caller_gid);
	box.y_length = GenericInteger_getInteger(&params->ellipse_height, caller_gid);

	/* corrigendum puts these limits on the ellipse size */
	if(box.x_length < 0 || box.y_length < 0)
	{
		error("DynamicLineArtClass_DrawOval: invalid ellipse size (%d,%d)", box.x_length, box.y_length);
		return;
	}

	MHEGCanvas_drawOval(t->inst.canvas, &pos, &box,
			    t->inst.LineWidth, t->inst.LineStyle,
			    &t->inst.RefLineColour, &t->inst.RefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_DrawPolygon(DynamicLineArtClass *t, DrawPolygon *params, OctetString *caller_gid)
{
	LIST_OF(XYPosition) *xy_list = NULL;
	LIST_TYPE(XYPosition) *xy;
	LIST_TYPE(Point) *pt;

	verbose("DynamicLineArtClass: %s; DrawPolygon", ExternalReference_name(&t->rootClass.inst.ref));

	for(pt=params->pointlist; pt; pt=pt->next)
	{
		xy = safe_malloc(sizeof(LIST_TYPE(XYPosition)));
		xy->item.x_position = GenericInteger_getInteger(&pt->item.x, caller_gid);
		xy->item.y_position = GenericInteger_getInteger(&pt->item.y, caller_gid);
		LIST_APPEND(&xy_list, xy);
	}

	MHEGCanvas_drawPolygon(t->inst.canvas, xy_list,
			       t->inst.LineWidth, t->inst.LineStyle,
			       &t->inst.RefLineColour, &t->inst.RefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	LIST_FREE(&xy_list, XYPosition, safe_free);

	return;
}

void
DynamicLineArtClass_DrawPolyline(DynamicLineArtClass *t, DrawPolyline *params, OctetString *caller_gid)
{
	LIST_OF(XYPosition) *xy_list = NULL;
	LIST_TYPE(XYPosition) *xy;
	LIST_TYPE(Point) *pt;

	verbose("DynamicLineArtClass: %s; DrawPolyline", ExternalReference_name(&t->rootClass.inst.ref));

	for(pt=params->pointlist; pt; pt=pt->next)
	{
		xy = safe_malloc(sizeof(LIST_TYPE(XYPosition)));
		xy->item.x_position = GenericInteger_getInteger(&pt->item.x, caller_gid);
		xy->item.y_position = GenericInteger_getInteger(&pt->item.y, caller_gid);
		LIST_APPEND(&xy_list, xy);
	}

	MHEGCanvas_drawPolyline(t->inst.canvas, xy_list,
			       t->inst.LineWidth, t->inst.LineStyle,
			       &t->inst.RefLineColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	LIST_FREE(&xy_list, XYPosition, safe_free);

	return;
}

void
DynamicLineArtClass_DrawRectangle(DynamicLineArtClass *t, DrawRectangle *params, OctetString *caller_gid)
{
	XYPosition pos;
	OriginalBoxSize box;

	verbose("DynamicLineArtClass: %s; DrawRectangle", ExternalReference_name(&t->rootClass.inst.ref));

	pos.x_position = GenericInteger_getInteger(&params->x1, caller_gid);
	pos.y_position = GenericInteger_getInteger(&params->y1, caller_gid);
	box.x_length = GenericInteger_getInteger(&params->x2, caller_gid) - pos.x_position;
	box.y_length = GenericInteger_getInteger(&params->y2, caller_gid) - pos.y_position;

	MHEGCanvas_drawRectangle(t->inst.canvas, &pos, &box,
				 t->inst.LineWidth, t->inst.LineStyle,
				 &t->inst.RefLineColour, &t->inst.RefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_Clear(DynamicLineArtClass *t)
{
	verbose("DynamicLineArtClass: %s; Clear", ExternalReference_name(&t->rootClass.inst.ref));

	/* fill with OriginalRefFillColour */
	MHEGCanvas_clear(t->inst.canvas, &t->inst.OriginalRefFillColour);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
DynamicLineArtClass_render(DynamicLineArtClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;
	XYPosition src;

	verbose("DynamicLineArtClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

	/* work out where the intersection starts on the canvas */
	src.x_position = ins_pos.x_position - t->inst.Position.x_position;
	src.y_position = ins_pos.y_position - t->inst.Position.y_position;

	MHEGDisplay_drawCanvas(d, &src, &ins_box, t->inst.canvas, &ins_pos);

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

