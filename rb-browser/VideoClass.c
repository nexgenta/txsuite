/*
 * VideoClass.c
 */

#include "MHEGEngine.h"
#include "ISO13522-MHEG-5.h"
#include "RootClass.h"
#include "StreamClass.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "GenericInteger.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "GenericObjectReference.h"
#include "clone.h"
#include "rtti.h"

void
default_VideoClassInstanceVars(VideoClass *t, VideoClassInstanceVars *v)
{
	/*
	 * don't do:
	 * bzero(v, sizeof(VideoClassInstanceVars));
	 * or:
	 * v->owner = NULL;
	 * the whole VideoClass including these instance vars is zero'd when it is DER decoded
	 * we need to make sure v->owner is not set to NULL here
	 * in case our StreamClass is already active and has set our owner
	 */

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

	/* VideoClass */
	v->VideoDecodeOffset.x_position = 0;
	v->VideoDecodeOffset.y_position = 0;

	pthread_mutex_init(&v->bbox_lock, NULL);
	pthread_mutex_init(&v->scaled_lock, NULL);
	v->scaled = false;

	v->no_video = false;

	return;
}

void
free_VideoClassInstanceVars(VideoClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	pthread_mutex_destroy(&v->bbox_lock);
	pthread_mutex_destroy(&v->scaled_lock);

	return;
}

void
VideoClass_Preparation(VideoClass *t)
{
	verbose("VideoClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_VideoClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
VideoClass_Activation(VideoClass *t)
{
	verbose("VideoClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		VideoClass_Preparation(t);

	/* has it already been activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

	/* set its RunningStatus */
	t->rootClass.inst.RunningStatus = true;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	/*
	 * tell our StreamClass to start playing us
	 * owner maybe NULL if our StreamClass is in the process of activating itself
	 * in which case, it will start us when needed
	 */
	if(t->inst.owner != NULL)
		StreamClass_activateVideoComponent(t->inst.owner, t);

	/* now its RunningStatus is true, get it drawn at its position in the application's DisplayStack */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
VideoClass_Deactivation(VideoClass *t)
{
	verbose("VideoClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/*
	 * tell our StreamClass to stop playing us
	 * owner maybe NULL if our StreamClass is in the process of deactivating itself
	 * in which case, it will stop us when needed
	 */
	if(t->inst.owner != NULL)
		StreamClass_deactivateVideoComponent(t->inst.owner, t);

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
VideoClass_Destruction(VideoClass *t)
{
	verbose("VideoClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		VideoClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_VideoClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
VideoClass_SetPosition(VideoClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("VideoClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_position = t->inst.Position.x_position;
	old.y_position = t->inst.Position.y_position;

	pthread_mutex_lock(&t->inst.bbox_lock);
	t->inst.Position.x_position = GenericInteger_getInteger(&params->new_x_position, caller_gid);
	t->inst.Position.y_position = GenericInteger_getInteger(&params->new_y_position, caller_gid);
	pthread_mutex_unlock(&t->inst.bbox_lock);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&old, &t->inst.BoxSize);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
VideoClass_GetPosition(VideoClass *t, GetPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("VideoClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* X position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("VideoClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.x_position);

	/* Y position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("VideoClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.y_position);

	return;
}

void
VideoClass_SetBoxSize(VideoClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	OriginalBoxSize old;

	verbose("VideoClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_length = t->inst.BoxSize.x_length;
	old.y_length = t->inst.BoxSize.y_length;

	pthread_mutex_lock(&t->inst.bbox_lock);
	t->inst.BoxSize.x_length = GenericInteger_getInteger(&params->x_new_box_size, caller_gid);
	t->inst.BoxSize.y_length = GenericInteger_getInteger(&params->y_new_box_size, caller_gid);
	pthread_mutex_unlock(&t->inst.bbox_lock);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&t->inst.Position, &old);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
VideoClass_GetBoxSize(VideoClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("VideoClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	/* width */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("VideoClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.x_length);

	/* height */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("VideoClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.y_length);

	return;
}

void
VideoClass_BringToFront(VideoClass *t)
{
	verbose("VideoClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
VideoClass_SendToBack(VideoClass *t)
{
	verbose("VideoClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
VideoClass_PutBefore(VideoClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("VideoClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
VideoClass_PutBehind(VideoClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("VideoClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
VideoClass_SetVideoDecodeOffset(VideoClass *t, SetVideoDecodeOffset *params, OctetString *caller_gid)
{
	verbose("VideoClass: %s; SetVideoDecodeOffset", ExternalReference_name(&t->rootClass.inst.ref));

	pthread_mutex_lock(&t->inst.bbox_lock);
	t->inst.VideoDecodeOffset.x_position = GenericInteger_getInteger(&params->new_x_offset, caller_gid);
	t->inst.VideoDecodeOffset.y_position = GenericInteger_getInteger(&params->new_y_offset, caller_gid);
	pthread_mutex_unlock(&t->inst.bbox_lock);

	/* screen will be updated next time we draw a video frame */
/* TODO */
/* should probably clear content Pixmap to black */
/* x,y = t->inst.Position, w,h = t->inst.BoxSize */

	return;
}

void
VideoClass_GetVideoDecodeOffset(VideoClass *t, GetVideoDecodeOffset *params, OctetString *caller_gid)
{
	verbose("VideoClass: %s; GetVideoDecodeOffset", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: VideoClass_GetVideoDecodeOffset not yet implemented\n");
	return;
}

void
VideoClass_ScaleVideo(VideoClass *t, ScaleVideo *params, OctetString *caller_gid)
{
	verbose("VideoClass: %s; ScaleVideo", ExternalReference_name(&t->rootClass.inst.ref));

	pthread_mutex_lock(&t->inst.scaled_lock);
	t->inst.scaled = true;
	t->inst.scaled_width = GenericInteger_getInteger(&params->x_scale, caller_gid);
	t->inst.scaled_height = GenericInteger_getInteger(&params->y_scale, caller_gid);
	pthread_mutex_unlock(&t->inst.scaled_lock);

	return;
}

void
VideoClass_render(VideoClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;
	MHEGColour black;

	verbose("VideoClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	/*
	 * if we have no video stream, just draw a black rectangle
	 * if we do have a video stream, make a transparent hole in the MHEG overlay so we can see the video below it
	 */
	if(t->inst.no_video)
	{
		MHEGColour_black(&black);
		MHEGDisplay_fillRectangle(d, &ins_pos, &ins_box, &black);
	}
	else
	{
		MHEGDisplay_fillTransparentRectangle(d, &ins_pos, &ins_box);
	}

	return;
}

