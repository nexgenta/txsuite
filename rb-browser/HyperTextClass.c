/*
 * HyperTextClass.c
 */

#include "MHEGEngine.h"
#include "HyperTextClass.h"
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
default_HyperTextClassInstanceVars(HyperTextClass *t, HyperTextClassInstanceVars *v)
{
	bzero(v, sizeof(HyperTextClassInstanceVars));

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

	/* HyperTextClass */
	v->LastAnchorFired.size = 0;
	v->LastAnchorFired.data = NULL;

	v->FocusPosition = 0;

	return;
}

void
free_HyperTextClassInstanceVars(HyperTextClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	free_OctetString(&v->TextData);

	free_MHEGFont(&v->Font);

	LIST_FREE(&v->element, MHEGTextElement, safe_free);

	free_OctetString(&v->LastAnchorFired);

	return;
}

void
HyperTextClass_Preparation(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_HyperTextClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
HyperTextClass_Activation(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		HyperTextClass_Preparation(t);

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
HyperTextClass_Deactivation(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
HyperTextClass_Destruction(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		HyperTextClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_HyperTextClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
HyperTextClass_SetData(HyperTextClass *t, SetData *set, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetData", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetData not yet implemented\n");
	return;
}

void
HyperTextClass_Clone(HyperTextClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_Clone not yet implemented\n");
	return;
}

void
HyperTextClass_SetPosition(HyperTextClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("HyperTextClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

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
HyperTextClass_GetPosition(HyperTextClass *t, GetPosition *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetPosition not yet implemented\n");
	return;
}

void
HyperTextClass_SetBoxSize(HyperTextClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetBoxSize not yet implemented\n");
	return;
}

void
HyperTextClass_GetBoxSize(HyperTextClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetBoxSize not yet implemented\n");
	return;
}

void
HyperTextClass_BringToFront(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
HyperTextClass_SendToBack(HyperTextClass *t)
{
	verbose("HyperTextClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
HyperTextClass_PutBefore(HyperTextClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("HyperTextClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
HyperTextClass_PutBehind(HyperTextClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("HyperTextClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
HyperTextClass_SetPaletteRef(HyperTextClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("HyperTextClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HyperTextClass_SetBackgroundColour(HyperTextClass *t, SetBackgroundColour *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetBackgroundColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetBackgroundColour not yet implemented\n");
	return;
}

void
HyperTextClass_SetTextColour(HyperTextClass *t, SetTextColour *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetTextColour", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetTextColour not yet implemented\n");
	return;
}

void
HyperTextClass_SetFontAttributes(HyperTextClass *t, SetFontAttributes *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetFontAttributes", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetFontAttributes not yet implemented\n");
	return;
}

void
HyperTextClass_GetFocusPosition(HyperTextClass *t, GetFocusPosition *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetFocusPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetFocusPosition not yet implemented\n");
	return;
}

void
HyperTextClass_SetFocusPosition(HyperTextClass *t, SetFocusPosition *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetFocusPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetFocusPosition not yet implemented\n");
	return;
}

void
HyperTextClass_GetTextContent(HyperTextClass *t, GetTextContent *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetTextContent", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetTextContent not yet implemented\n");
	return;
}

/*
 * caller_gid is used to resolve the text_data_var ObjectReference
 */

void
HyperTextClass_GetTextData(HyperTextClass *t, GetTextData *get, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("HyperTextClass: %s; GetTextData", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&get->text_data_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_octetstring)
	{
		error("HyperTextClass: GetTextData: type mismatch");
		return;
	}

	OctetStringVariableClass_setOctetString(var, &t->inst.TextData);

	return;
}

void
HyperTextClass_SetFontRef(HyperTextClass *t, SetFontRef *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetFontRef", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetFontRef not yet implemented\n");
	return;
}

void
HyperTextClass_SetInteractionStatus(HyperTextClass *t, SetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetInteractionStatus not yet implemented\n");
	return;
}

void
HyperTextClass_GetInteractionStatus(HyperTextClass *t, GetInteractionStatus *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetInteractionStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetInteractionStatus not yet implemented\n");
	return;
}

void
HyperTextClass_SetHighlightStatus(HyperTextClass *t, SetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; SetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_SetHighlightStatus not yet implemented\n");
	return;
}

void
HyperTextClass_GetHighlightStatus(HyperTextClass *t, GetHighlightStatus *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetHighlightStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetHighlightStatus not yet implemented\n");
	return;
}

void
HyperTextClass_GetLastAnchorFired(HyperTextClass *t, GetLastAnchorFired *params, OctetString *caller_gid)
{
	verbose("HyperTextClass: %s; GetLastAnchorFired", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: HyperTextClass_GetLastAnchorFired not yet implemented\n");
	return;
}

/*
 * called when the content is available
 */

void
HyperTextClass_contentAvailable(HyperTextClass *t, OctetString *file)
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
HyperTextClass_render(HyperTextClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;

	verbose("HyperTextClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

/* TODO */
printf("TODO: HyperTextClass_render\n");

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

