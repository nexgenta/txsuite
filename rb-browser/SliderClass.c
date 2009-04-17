/*
 * SliderClass.c
 */

#include "MHEGEngine.h"
#include "SliderClass.h"
#include "RootClass.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "GenericInteger.h"
#include "GenericObjectReference.h"
#include "clone.h"

void
default_SliderClassInstanceVars(SliderClass *t, SliderClassInstanceVars *v)
{
	bzero(v, sizeof(SliderClassInstanceVars));

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

	/* InteractibleClass */
	v->HighlightStatus = false;
	v->InteractionStatus = false;

	/* SliderClass */
	if(t->have_initial_value)
		v->SliderValue = t->initial_value;
	else
		v->SliderValue = 0;

	if(t->have_initial_portion)
		v->Portion = t->initial_portion;
	else
		v->Portion = 0;

	v->MinValue = t->min_value;
	v->MaxValue = t->max_value;
	v->StepSize = t->step_size;

	return;
}

void
free_SliderClassInstanceVars(SliderClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	return;
}

void
SliderClass_Preparation(SliderClass *t)
{
	verbose("SliderClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_SliderClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
SliderClass_Activation(SliderClass *t)
{
	verbose("SliderClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_Activation not yet implemented\n");
	return;
}

void
SliderClass_Deactivation(SliderClass *t)
{
	verbose("SliderClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_Deactivation not yet implemented\n");
	return;
}

void
SliderClass_Destruction(SliderClass *t)
{
	verbose("SliderClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		SliderClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_SliderClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
SliderClass_Clone(SliderClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_Clone not yet implemented\n");
	return;
}

void
SliderClass_SetPosition(SliderClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("SliderClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

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
SliderClass_GetPosition(SliderClass *t, GetPosition *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetPosition not yet implemented\n");
	return;
}

void
SliderClass_SetBoxSize(SliderClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetBoxSize not yet implemented\n");
	return;
}

void
SliderClass_GetBoxSize(SliderClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetBoxSize not yet implemented\n");
	return;
}

void
SliderClass_BringToFront(SliderClass *t)
{
	verbose("SliderClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
SliderClass_SendToBack(SliderClass *t)
{
	verbose("SliderClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
SliderClass_PutBefore(SliderClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("SliderClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
SliderClass_PutBehind(SliderClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("SliderClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
SliderClass_SetPaletteRef(SliderClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("SliderClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
SliderClass_SetSliderParameters(SliderClass *t, SetSliderParameters *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetSliderParameters", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetSliderParameters not yet implemented\n");
	return;
}

void
SliderClass_SetInteractionStatus(SliderClass *t, SetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetInteractionStatus not yet implemented\n");
	return;
}

void
SliderClass_GetInteractionStatus(SliderClass *t, GetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetInteractionStatus not yet implemented\n");
	return;
}

void
SliderClass_SetHighlightStatus(SliderClass *t, SetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetHighlightStatus not yet implemented\n");
	return;
}

void
SliderClass_GetHighlightStatus(SliderClass *t, GetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetHighlightStatus not yet implemented\n");
	return;
}

void
SliderClass_Step(SliderClass *t, Step *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; Step", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_Step not yet implemented\n");
	return;
}

void
SliderClass_SetSliderValue(SliderClass *t, SetSliderValue *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetSliderValue", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetSliderValue not yet implemented\n");
	return;
}

void
SliderClass_GetSliderValue(SliderClass *t, GetSliderValue *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetSliderValue", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetSliderValue not yet implemented\n");
	return;
}

void
SliderClass_SetPortion(SliderClass *t, SetPortion *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; SetPortion", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_SetPortion not yet implemented\n");
	return;
}

void
SliderClass_GetPortion(SliderClass *t, GetPortion *params, OctetString *caller_gid)
{
	verbose("SliderClass: %s; GetPortion", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SliderClass_GetPortion not yet implemented\n");
	return;
}

void
SliderClass_render(SliderClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;

	verbose("SliderClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

/* TODO */
printf("TODO: SliderClass_render\n");

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

