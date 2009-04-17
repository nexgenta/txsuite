/*
 * EntryFieldClass.c
 */

#include "MHEGEngine.h"
#include "EntryFieldClass.h"
#include "RootClass.h"
#include "ApplicationClass.h"
#include "ContentBody.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "VariableClass.h"
#include "OctetStringVariableClass.h"
#include "GenericInteger.h"
#include "GenericObjectReference.h"
#include "clone.h"
#include "rtti.h"

void
default_EntryFieldClassInstanceVars(EntryFieldClass *t, EntryFieldClassInstanceVars *v)
{
	bzero(v, sizeof(EntryFieldClassInstanceVars));

/* TODO */
/* this code is duplicated in TextClass, EntryFieldClass and HyperTextClass */
/* except the InteractibleClass initialisation is not in TextClass */

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

	/* InteractibleClass */
	v->HighlightStatus = false;
	v->InteractionStatus = false;

	/* TextClass */
	if(t->have_original_content)
	{
		ContentBody_getContent(&t->original_content, &t->rootClass, &v->TextData);
	}
	else
	{
		v->TextData.size = 0;
		v->TextData.data = NULL;
	}

	/* defaults come from the ApplicationClass */
	MHEGFont_init(&v->Font);
	if(t->have_original_font)
	{
		MHEGFont_setName(&v->Font, &t->original_font);
	}
	else
	{
		ApplicationClass *app = MHEGEngine_getActiveApplication();
		DefaultAttribute *attr = ApplicationClass_getDefaultAttribute(app, DefaultAttribute_font);
		FontBody *font = (attr != NULL) ? &attr->u.font : NULL;
		if(font != NULL)
			MHEGFont_setName(&v->Font, font);
		else
			MHEGFont_defaultName(&v->Font);
	}

	if(t->have_text_colour)
	{
		MHEGColour_fromColour(&v->TextColour, &t->text_colour);
	}
	else
	{
		ApplicationClass *app = MHEGEngine_getActiveApplication();
		DefaultAttribute *attr = ApplicationClass_getDefaultAttribute(app, DefaultAttribute_text_colour);
		Colour *col = (attr != NULL) ? &attr->u.text_colour : NULL;
		/* UK MHEG Profile says default is white */
		if(col != NULL)
			MHEGColour_fromColour(&v->TextColour, col);
		else
			MHEGColour_white(&v->TextColour);
	}

	if(t->have_background_colour)
	{
		MHEGColour_fromColour(&v->BackgroundColour, &t->background_colour);
	}
	else
	{
		ApplicationClass *app = MHEGEngine_getActiveApplication();
		DefaultAttribute *attr = ApplicationClass_getDefaultAttribute(app, DefaultAttribute_background_colour);
		Colour *col = (attr != NULL) ? &attr->u.background_colour : NULL;
		/* default is transparent */
		if(col != NULL)
			MHEGColour_fromColour(&v->BackgroundColour, col);
		else
			MHEGColour_transparent(&v->BackgroundColour);
	}

	if(t->have_font_attributes)
	{
		MHEGFont_setAttributes(&v->Font, &t->font_attributes);
	}
	else
	{
		ApplicationClass *app = MHEGEngine_getActiveApplication();
		DefaultAttribute *attr = ApplicationClass_getDefaultAttribute(app, DefaultAttribute_font_attributes);
		OctetString *font_attr = (attr != NULL) ? &attr->u.font_attributes : NULL;
		if(font_attr != NULL)
			MHEGFont_setAttributes(&v->Font, font_attr);
		else
			MHEGFont_defaultAttributes(&v->Font);
	}

	v->element = NULL;

	/* EntryFieldClass */
	v->EntryPoint = 0;
	v->OverwriteMode = false;

	return;
}

void
free_EntryFieldClassInstanceVars(EntryFieldClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	free_OctetString(&v->TextData);

	free_MHEGFont(&v->Font);

	LIST_FREE(&v->element, MHEGTextElement, safe_free);

	return;
}

void
EntryFieldClass_Preparation(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_EntryFieldClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
EntryFieldClass_Activation(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		EntryFieldClass_Preparation(t);

	/* has it already been activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

/**************************************************************************/
/* anything you do here, you probably also need to do in contentAvailable() */
/**************************************************************************/

	/* set its RunningStatus */
	t->rootClass.inst.RunningStatus = true;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	/* now its RunningStatus is true, get it drawn at its position in the application's DisplayStack */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
EntryFieldClass_Deactivation(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
EntryFieldClass_Destruction(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		EntryFieldClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_EntryFieldClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
EntryFieldClass_SetData(EntryFieldClass *t, SetData *set, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetData", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetData not yet implemented\n");
	return;
}

void
EntryFieldClass_Clone(EntryFieldClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_Clone not yet implemented\n");
	return;
}

void
EntryFieldClass_SetPosition(EntryFieldClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("EntryFieldClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

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
EntryFieldClass_GetPosition(EntryFieldClass *t, GetPosition *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetPosition not yet implemented\n");
	return;
}

void
EntryFieldClass_SetBoxSize(EntryFieldClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetBoxSize not yet implemented\n");
	return;
}

void
EntryFieldClass_GetBoxSize(EntryFieldClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetBoxSize not yet implemented\n");
	return;
}

void
EntryFieldClass_BringToFront(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
EntryFieldClass_SendToBack(EntryFieldClass *t)
{
	verbose("EntryFieldClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
EntryFieldClass_PutBefore(EntryFieldClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("EntryFieldClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
EntryFieldClass_PutBehind(EntryFieldClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("EntryFieldClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
EntryFieldClass_SetPaletteRef(EntryFieldClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("EntryFieldClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
EntryFieldClass_SetBackgroundColour(EntryFieldClass *t, SetBackgroundColour *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetBackgroundColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetBackgroundColour not yet implemented\n");
	return;
}

void
EntryFieldClass_SetTextColour(EntryFieldClass *t, SetTextColour *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetTextColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetTextColour not yet implemented\n");
	return;
}

void
EntryFieldClass_SetFontAttributes(EntryFieldClass *t, SetFontAttributes *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetFontAttributes", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetFontAttributes not yet implemented\n");
	return;
}

void
EntryFieldClass_GetTextContent(EntryFieldClass *t, GetTextContent *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetTextContent", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetTextContent not yet implemented\n");
	return;
}

/*
 * caller_gid is used to resolve the text_data_var ObjectReference
 */

void
EntryFieldClass_GetTextData(EntryFieldClass *t, GetTextData *get, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("EntryFieldClass: %s; GetTextData", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&get->text_data_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_octetstring)
	{
		error("EntryFieldClass: GetTextData: type mismatch");
		return;
	}

	OctetStringVariableClass_setOctetString(var, &t->inst.TextData);

	return;
}

void
EntryFieldClass_SetFontRef(EntryFieldClass *t, SetFontRef *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetFontRef", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetFontRef not yet implemented\n");
	return;
}

void
EntryFieldClass_SetInteractionStatus(EntryFieldClass *t, SetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetInteractionStatus not yet implemented\n");
	return;
}

void
EntryFieldClass_GetInteractionStatus(EntryFieldClass *t, GetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetInteractionStatus not yet implemented\n");
	return;
}

void
EntryFieldClass_SetHighlightStatus(EntryFieldClass *t, SetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetHighlightStatus not yet implemented\n");
	return;
}

void
EntryFieldClass_GetHighlightStatus(EntryFieldClass *t, GetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetHighlightStatus not yet implemented\n");
	return;
}

void
EntryFieldClass_SetOverwriteMode(EntryFieldClass *t, SetOverwriteMode *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetOverwriteMode", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetOverwriteMode not yet implemented\n");
	return;
}

void
EntryFieldClass_GetOverwriteMode(EntryFieldClass *t, GetOverwriteMode *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetOverwriteMode", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetOverwriteMode not yet implemented\n");
	return;
}

void
EntryFieldClass_SetEntryPoint(EntryFieldClass *t, SetEntryPoint *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; SetEntryPoint", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_SetEntryPoint not yet implemented\n");
	return;
}

void
EntryFieldClass_GetEntryPoint(EntryFieldClass *t, GetEntryPoint *params, OctetString *caller_gid)
{
	verbose("EntryFieldClass: %s; GetEntryPoint", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: EntryFieldClass_GetEntryPoint not yet implemented\n");
	return;
}

/*
 * called when the content is available
 */

void
EntryFieldClass_contentAvailable(EntryFieldClass *t, OctetString *file)
{
	/* get rid of any existing content */
	free_OctetString(&t->inst.TextData);
	LIST_FREE(&t->inst.element, MHEGTextElement, safe_free);

	/* load the new content */
	MHEGEngine_loadFile(file, &t->inst.TextData);

	/* if it is active, redraw the area it covers */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
EntryFieldClass_render(EntryFieldClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;

	verbose("EntryFieldClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

/* TODO */
printf("TODO: EntryFieldClass_render\n");

	MHEGDisplay_unsetClipRectangle(d);

	return;
}


