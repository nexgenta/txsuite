/*
 * BitmapClass.c
 */

#include "MHEGEngine.h"
#include "ISO13522-MHEG-5.h"
#include "RootClass.h"
#include "ContentBody.h"
#include "NewContent.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "GenericInteger.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "ObjectRefVariableClass.h"
#include "GenericObjectReference.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
default_BitmapClassInstanceVars(BitmapClass *t, BitmapClassInstanceVars *v)
{
	bzero(v, sizeof(BitmapClassInstanceVars));

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

	/* BitmapClass */
	v->Transparency = t->original_transparency;
	v->BitmapDecodeOffset.x_position = 0;
	v->BitmapDecodeOffset.y_position = 0;
	/* get the bitmap */
	if(t->have_original_content)
	{
		ContentBody_getContent(&t->original_content, &t->rootClass, &v->BitmapData);
	}
	else
	{
		v->BitmapData.size = 0;
		v->BitmapData.data = NULL;
	}
	/* no internal representation yet, as the content may not be available */
	v->Bitmap = NULL;

	return;
}

void
free_BitmapClassInstanceVars(BitmapClassInstanceVars *v)
{
	MHEGEngine_freeBitmap(v->Bitmap);

	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	free_OctetString(&v->BitmapData);

	return;
}

void
BitmapClass_Preparation(BitmapClass *t)
{
	verbose("BitmapClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_BitmapClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
BitmapClass_Activation(BitmapClass *t)
{
	verbose("BitmapClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		BitmapClass_Preparation(t);

	/* has it already been activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

	/* convert it to MHEGDisplay's internal format */
	t->inst.Bitmap = MHEGEngine_newBitmap(&t->inst.BitmapData, t->have_content_hook, t->content_hook);

	/* set its RunningStatus */
	t->rootClass.inst.RunningStatus = true;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	/* now its RunningStatus is true, get it drawn at its position in the application's DisplayStack */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
BitmapClass_Deactivation(BitmapClass *t)
{
	verbose("BitmapClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
BitmapClass_Destruction(BitmapClass *t)
{
	verbose("BitmapClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already destroyed */
	if(!t->rootClass.inst.AvailabilityStatus)
		return;

	/* if we are waiting for content, remove ourselves from the missing content list */
	if(t->rootClass.inst.need_content)
		MHEGEngine_removeMissingContent(&t->rootClass);

	/* remove it from the DisplayStack */
	MHEGEngine_removeVisibleObject(&t->rootClass);

	/* RootClass Destruction */
	/* Deactivate it if it is running */
	if(t->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		BitmapClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_BitmapClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
BitmapClass_SetData(BitmapClass *t, SetData *set, OctetString *caller_gid)
{
	verbose("BitmapClass: %s; SetData", ExternalReference_name(&t->rootClass.inst.ref));

	/* get rid of any existing content */
	free_OctetString(&t->inst.BitmapData);
	MHEGEngine_freeBitmap(t->inst.Bitmap);
	t->inst.Bitmap = NULL;

	/*
	 * the content may need to be loaded from an external file
	 * if the file is not available, this returns false and calls BitmapClass_contentAvailable() when it appears
	 */
	if(NewContent_getContent(&set->new_content, caller_gid, &t->rootClass, &t->inst.BitmapData))
	{
		/* convert it to MHEGDisplay's internal format */
		t->inst.Bitmap = MHEGEngine_newBitmap(&t->inst.BitmapData, t->have_content_hook, t->content_hook);
		/* if it is active, redraw the area it covers */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
		/* ContentPreparation behaviour specified in the ISO MHEG Corrigendum */
		MHEGEngine_generateAsyncEvent(&t->rootClass.inst.ref, EventType_content_available, NULL);
	}

	return;
}

void
BitmapClass_Clone(BitmapClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("BitmapClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, BitmapClass, bitmap);

	return;
}

void
BitmapClass_SetPosition(BitmapClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("BitmapClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

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
BitmapClass_GetPosition(BitmapClass *t, GetPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("BitmapClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* X position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("BitmapClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.x_position);

	/* Y position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("BitmapClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.y_position);

	return;
}

void
BitmapClass_SetBoxSize(BitmapClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	OriginalBoxSize old;

	verbose("BitmapClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

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
BitmapClass_GetBoxSize(BitmapClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("BitmapClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	/* width */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("BitmapClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.x_length);

	/* height */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("BitmapClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.y_length);

	return;
}

void
BitmapClass_BringToFront(BitmapClass *t)
{
	verbose("BitmapClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
BitmapClass_SendToBack(BitmapClass *t)
{
	verbose("BitmapClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
BitmapClass_PutBefore(BitmapClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("BitmapClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
BitmapClass_PutBehind(BitmapClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("BitmapClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
BitmapClass_SetPaletteRef(BitmapClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("BitmapClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
BitmapClass_SetBitmapDecodeOffset(BitmapClass *t, SetBitmapDecodeOffset *params, OctetString *caller_gid)
{
	verbose("BitmapClass: %s; SetBitmapDecodeOffset", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: BitmapClass_SetBitmapDecodeOffset not yet implemented\n");
	return;
}

void
BitmapClass_GetBitmapDecodeOffset(BitmapClass *t, GetBitmapDecodeOffset *params, OctetString *caller_gid)
{
	verbose("BitmapClass: %s; GetBitmapDecodeOffset", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: BitmapClass_GetBitmapDecodeOffset not yet implemented\n");
	return;
}

void
BitmapClass_ScaleBitmap(BitmapClass *t, ScaleBitmap *params, OctetString *caller_gid)
{
	int x_scale;
	int y_scale;

	verbose("BitmapClass: %s; ScaleBitmap", ExternalReference_name(&t->rootClass.inst.ref));

	x_scale = GenericInteger_getInteger(&params->x_scale, caller_gid);
	y_scale = GenericInteger_getInteger(&params->y_scale, caller_gid);

/* TODO */
printf("TODO: BitmapClass_ScaleBitmap(%d, %d) not yet implemented\n", x_scale, y_scale);
	return;
}

void
BitmapClass_SetTransparency(BitmapClass *t, SetTransparency *params, OctetString *caller_gid)
{
	verbose("BitmapClass: %s; SetTransparency", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: BitmapClass_SetTransparency not yet implemented\n");
	return;
}

/*
 * called when the content is available
 */

void
BitmapClass_contentAvailable(BitmapClass *t, OctetString *file)
{
	/* get rid of any existing content */
	free_OctetString(&t->inst.BitmapData);
	MHEGEngine_freeBitmap(t->inst.Bitmap);

	/* load the new content */
	MHEGEngine_loadFile(file, &t->inst.BitmapData);

	/* convert it to MHEGDisplay's internal format */
	t->inst.Bitmap = MHEGEngine_newBitmap(&t->inst.BitmapData, t->have_content_hook, t->content_hook);

	/* if it is active, redraw the area it covers */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
BitmapClass_render(BitmapClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;
	XYPosition src;

	verbose("BitmapClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

	/* work out where the intersection starts on the bitmap */
	src.x_position = ins_pos.x_position - t->inst.Position.x_position;
	src.y_position = ins_pos.y_position - t->inst.Position.y_position;

/****************************************************************************************/
/*TODO */
/* take tiling into account */
/* take BitmapDecodeOffset into account */
/* if we are not tiling and the actual bitmap is a different size than t->inst.BoxSize do we need to scale it? */
/* can probably get XRender to tile it for us */
/****************************************************************************************/

	MHEGDisplay_drawBitmap(d, &src, &ins_box, t->inst.Bitmap, &ins_pos);

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

