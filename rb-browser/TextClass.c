/*
 * TextClass.c
 */

#include "MHEGEngine.h"
#include "ISO13522-MHEG-5.h"
#include "ContentBody.h"
#include "NewContent.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "RootClass.h"
#include "ApplicationClass.h"
#include "VariableClass.h"
#include "OctetStringVariableClass.h"
#include "GenericInteger.h"
#include "IntegerVariableClass.h"
#include "ObjectRefVariableClass.h"
#include "GenericObjectReference.h"
#include "GenericOctetString.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
default_TextClassInstanceVars(TextClass *t, TextClassInstanceVars *v)
{
	bzero(v, sizeof(TextClassInstanceVars));

/* TODO */
/* this code is duplicated in TextClass, EntryFieldClass and HyperTextClass */

	/* VisibleClass */
	memcpy(&v->BoxSize, &t->original_box_size, sizeof(OriginalBoxSize));
	memcpy(&v->Position, &t->original_position, sizeof(XYPosition));
	v->have_PaletteRef = t->have_original_palette_ref;
	if(v->have_PaletteRef)
		ObjectReference_dup(&v->PaletteRef, &t->original_palette_ref);

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

	return;
}

void
free_TextClassInstanceVars(TextClassInstanceVars *v)
{
	if(v->have_PaletteRef)
		free_ObjectReference(&v->PaletteRef);

	free_OctetString(&v->TextData);

	free_MHEGFont(&v->Font);

	LIST_FREE(&v->element, MHEGTextElement, safe_free);

	return;
}

void
TextClass_Preparation(TextClass *t)
{
	verbose("TextClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	/* RootClass Preparation */
	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_TextClassInstanceVars(t, &t->inst);

	/* add it to the DisplayStack of the active application */
	MHEGEngine_addVisibleObject(&t->rootClass);

	return;
}

void
TextClass_Activation(TextClass *t)
{
	verbose("TextClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
		TextClass_Preparation(t);

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
TextClass_Deactivation(TextClass *t)
{
	verbose("TextClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already deactivated */
	if(!RootClass_Deactivation(&t->rootClass))
		return;

	/* now its RunningStatus is false, redraw the area it covered */
	MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_Destruction(TextClass *t)
{
	verbose("TextClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

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
		TextClass_Deactivation(t);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_TextClassInstanceVars(&t->inst);

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
TextClass_SetData(TextClass *t, SetData *set, OctetString *caller_gid)
{
	verbose("TextClass: %s; SetData", ExternalReference_name(&t->rootClass.inst.ref));

	/* get rid of any existing content */
	free_OctetString(&t->inst.TextData);
	LIST_FREE(&t->inst.element, MHEGTextElement, safe_free);

	/*
	 * the content may need to be loaded from an external file
	 * if the file is not available, this returns false and calls TextClass_contentAvailable() when it appears
	 */
	if(NewContent_getContent(&set->new_content, caller_gid, &t->rootClass, &t->inst.TextData))
	{
		/* if it is active, redraw the area it covers */
		if(t->rootClass.inst.RunningStatus)
			MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
		/* ContentPreparation behaviour specified in the ISO MHEG Corrigendum */
		MHEGEngine_generateAsyncEvent(&t->rootClass.inst.ref, EventType_content_available, NULL);
	}

	return;
}

void
TextClass_Clone(TextClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("TextClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, TextClass, text);

	return;
}

void
TextClass_SetPosition(TextClass *t, SetPosition *params, OctetString *caller_gid)
{
	XYPosition old;

	verbose("TextClass: %s; SetPosition", ExternalReference_name(&t->rootClass.inst.ref));

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
TextClass_GetPosition(TextClass *t, GetPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("TextClass: %s; GetPosition", ExternalReference_name(&t->rootClass.inst.ref));

	/* X position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("TextClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.x_position);

	/* Y position */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("TextClass: GetPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.Position.y_position);

	return;
}

void
TextClass_SetBoxSize(TextClass *t, SetBoxSize *params, OctetString *caller_gid)
{
	OriginalBoxSize old;

	verbose("TextClass: %s; SetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	old.x_length = t->inst.BoxSize.x_length;
	old.y_length = t->inst.BoxSize.y_length;

	t->inst.BoxSize.x_length = GenericInteger_getInteger(&params->x_new_box_size, caller_gid);
	t->inst.BoxSize.y_length = GenericInteger_getInteger(&params->y_new_box_size, caller_gid);

	/* remove the previous layout info, gets recalculated when we redraw it */
	LIST_FREE(&t->inst.element, MHEGTextElement, safe_free);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
	{
		MHEGEngine_redrawArea(&t->inst.Position, &old);
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);
	}

	return;
}

void
TextClass_GetBoxSize(TextClass *t, GetBoxSize *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("TextClass: %s; GetBoxSize", ExternalReference_name(&t->rootClass.inst.ref));

	/* width */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->x_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("TextClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.x_length);

	/* height */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->y_box_size_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("TextClass: GetBoxSize: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.BoxSize.y_length);

	return;
}

void
TextClass_BringToFront(TextClass *t)
{
	verbose("TextClass: %s; BringToFront", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_bringToFront(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_SendToBack(TextClass *t)
{
	verbose("TextClass: %s; SendToBack", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_sendToBack(&t->rootClass);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_PutBefore(TextClass *t, PutBefore *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("TextClass: %s; PutBefore", ExternalReference_name(&t->rootClass.inst.ref));

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
TextClass_PutBehind(TextClass *t, PutBehind *params, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;

	verbose("TextClass: %s; PutBehind", ExternalReference_name(&t->rootClass.inst.ref));

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
TextClass_SetPaletteRef(TextClass *t, SetPaletteRef *params, OctetString *caller_gid)
{
	/* UK MHEG Profile says we don't support PaletteClass */
	error("TextClass: %s; SetPaletteRef not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
TextClass_SetBackgroundColour(TextClass *t, SetBackgroundColour *params, OctetString *caller_gid)
{
	verbose("TextClass: %s; SetBackgroundColour", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGColour_fromNewColour(&t->inst.BackgroundColour, &params->new_background_colour, caller_gid);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_SetTextColour(TextClass *t, SetTextColour *params, OctetString *caller_gid)
{
	verbose("TextClass: %s; SetTextColour", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGColour_fromNewColour(&t->inst.TextColour, &params->new_text_colour, caller_gid);

	/* remove the previous layout info, colours get recalculated when we redraw it */
	LIST_FREE(&t->inst.element, MHEGTextElement, safe_free);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_SetFontAttributes(TextClass *t, SetFontAttributes *params, OctetString *caller_gid)
{
	OctetString *attr;

	verbose("TextClass: %s; SetFontAttributes", ExternalReference_name(&t->rootClass.inst.ref));

	attr = GenericOctetString_getOctetString(&params->new_font_attribute, caller_gid);

	MHEGFont_setAttributes(&t->inst.Font, attr);

	/* remove the previous layout info, gets recalculated when we redraw it */
	LIST_FREE(&t->inst.element, MHEGTextElement, safe_free);

	/* if it is active, redraw it */
	if(t->rootClass.inst.RunningStatus)
		MHEGEngine_redrawArea(&t->inst.Position, &t->inst.BoxSize);

	return;
}

void
TextClass_GetTextContent(TextClass *t, GetTextContent *params, OctetString *caller_gid)
{
	verbose("TextClass: %s; GetTextContent", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: TextClass_GetTextContent not yet implemented\n");
	return;
}

/*
 * caller_gid is used to resolve the text_data_var ObjectReference
 */

void
TextClass_GetTextData(TextClass *t, GetTextData *get, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("TextClass: %s; GetTextData", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&get->text_data_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_octetstring)
	{
		error("TextClass: GetTextData: type mismatch");
		return;
	}

	OctetStringVariableClass_setOctetString(var, &t->inst.TextData);

	return;
}

void
TextClass_SetFontRef(TextClass *t, SetFontRef *params, OctetString *caller_gid)
{
	verbose("TextClass: %s; SetFontRef", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: TextClass_SetFontRef not yet implemented\n");
	return;
}

/*
 * called when the content is available
 */

void
TextClass_contentAvailable(TextClass *t, OctetString *file)
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
TextClass_render(TextClass *t, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	XYPosition ins_pos;
	OriginalBoxSize ins_box;
	LIST_TYPE(MHEGTextElement) *element;
	bool tabs;

	verbose("TextClass: %s; render", ExternalReference_name(&t->rootClass.inst.ref));

	if(!intersects(pos, box, &t->inst.Position, &t->inst.BoxSize, &ins_pos, &ins_box))
		return;

	MHEGDisplay_setClipRectangle(d, &ins_pos, &ins_box);

	/* draw the background */
	MHEGDisplay_fillRectangle(d, &ins_pos, &ins_box, &t->inst.BackgroundColour);

	/* layout the text if not already done */
	if(t->inst.element == NULL)
	{
		t->inst.element = MHEGFont_layoutText(&t->inst.Font, &t->inst.TextColour, &t->inst.TextData, &t->inst.BoxSize,
						      t->horizontal_justification, t->vertical_justification,
						      t->line_orientation, t->start_corner, t->text_wrapping);
	}

	/* tabs are treated as spaces if horizontal justification is not Justification_start */
	tabs = (t->horizontal_justification == Justification_start);

	/* draw each text element */
	element = t->inst.element;
	while(element)
	{
		MHEGDisplay_drawTextElement(d, &t->inst.Position, &t->inst.Font, &element->item, tabs);
		element = element->next;
	}

	MHEGDisplay_unsetClipRectangle(d);

	return;
}

