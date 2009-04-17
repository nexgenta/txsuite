/*
 * ElementaryAction.c
 */

#include "MHEGEngine.h"
#include "ElementaryAction.h"
#include "RootClass.h"
#include "ApplicationClass.h"
#include "SceneClass.h"
#include "ExternalReference.h"
#include "GenericInteger.h"
#include "GenericObjectReference.h"
#include "LinkClass.h"
#include "RemoteProgramClass.h"
#include "ResidentProgramClass.h"
#include "InterchangedProgramClass.h"
#include "AudioClass.h"
#include "VideoClass.h"
#include "RTGraphicsClass.h"
#include "StreamClass.h"
#include "BitmapClass.h"
#include "DynamicLineArtClass.h"
#include "RectangleClass.h"
#include "HotspotClass.h"
#include "SwitchButtonClass.h"
#include "PushButtonClass.h"
#include "TextClass.h"
#include "EntryFieldClass.h"
#include "HyperTextClass.h"
#include "SliderClass.h"
#include "TokenGroupClass.h"
#include "ListGroupClass.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "OctetStringVariableClass.h"
#include "PaletteClass.h"
#include "FontClass.h"
#include "CursorShapeClass.h"
#include "rtti.h"

/*
 * caller_gid should be the group identifier of the object containing the ElementaryAction
 * it is used to resolve the Generic references in the ElementaryAction parameters
 */

void
ElementaryAction_execute(ElementaryAction *e, OctetString *caller_gid)
{
	ObjectReference *ref;
	RootClass *obj;
	int op;

	switch(e->choice)
	{
	case ElementaryAction_activate:
		verbose("ElementaryAction_activate");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.activate, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_LinkClass)
				LinkClass_Activate((LinkClass *) obj);
			else
				error("Activate: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_add:
		verbose("ElementaryAction_add");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.add.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_integer)
				IntegerVariableClass_Add((IntegerVariableClass *) obj, &e->u.add, caller_gid);
			else
				error("Add: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_add_item:
		verbose("ElementaryAction_add_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.add_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_AddItem((ListGroupClass *) obj, &e->u.add_item, caller_gid);
			else
				error("AddItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_append:
		verbose("ElementaryAction_append");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.append.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_octetstring)
				OctetStringVariableClass_Append((OctetStringVariableClass *) obj, &e->u.append, caller_gid);
			else
				error("Append: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_bring_to_front:
		verbose("ElementaryAction_bring_to_front");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.bring_to_front, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_BringToFront((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_BringToFront((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_BringToFront((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_BringToFront((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_BringToFront((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_BringToFront((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_BringToFront((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_BringToFront((VideoClass *) obj);
			else
				error("BringToFront: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_call:
		verbose("ElementaryAction_call");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.call.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			/* UK MHEG Profile says we dont need to support Remote or InterchangedProgramClass */
			if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Call((ResidentProgramClass *) obj, &e->u.call, caller_gid);
			else
				error("Call: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_call_action_slot:
		verbose("ElementaryAction_call_action_slot");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.call_action_slot.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_CallActionSlot((ListGroupClass *) obj, &e->u.call_action_slot, caller_gid);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_CallActionSlot((TokenGroupClass *) obj, &e->u.call_action_slot, caller_gid);
			else
				error("CallActionSlot: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_clear:
		verbose("ElementaryAction_clear");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.clear, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Clear((DynamicLineArtClass *) obj);
			else
				error("Clear: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_clone:
		verbose("ElementaryAction_clone");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.clone.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_Clone((BitmapClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_VariableClass)
				VariableClass_Clone((VariableClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Clone((DynamicLineArtClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_Clone((EntryFieldClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_Clone((HyperTextClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_LinkClass)
				LinkClass_Clone((LinkClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Clone((ListGroupClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_Clone((RectangleClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Clone((ResidentProgramClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Clone((SliderClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_Clone((TextClass *) obj, &e->u.clone, caller_gid);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Clone((TokenGroupClass *) obj, &e->u.clone, caller_gid);
			else
				error("Clone: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_close_connection:
		verbose("ElementaryAction_close_connection");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.close_connection.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_CloseConnection((ApplicationClass *) obj, &e->u.close_connection, caller_gid);
			else
				error("CloseConnection: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_deactivate:
		verbose("ElementaryAction_deactivate");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.deactivate, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_LinkClass)
				LinkClass_Deactivate((LinkClass *) obj);
			else
				error("Deactivate: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_del_item:
		verbose("ElementaryAction_del_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.del_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_DelItem((ListGroupClass *) obj, &e->u.del_item, caller_gid);
			else
				error("DelItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_deselect:
		verbose("ElementaryAction_deselect");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.deselect, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_HotspotClass)
				HotspotClass_Deselect((HotspotClass *) obj);
			else if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_Deselect((PushButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Deselect((SwitchButtonClass *) obj);
			else
				error("Deselect: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_deselect_item:
		verbose("ElementaryAction_deselect_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.deselect_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_DeselectItem((ListGroupClass *) obj, &e->u.deselect_item, caller_gid);
			else
				error("DeselectItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_divide:
		verbose("ElementaryAction_divide");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.divide.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_integer)
				IntegerVariableClass_Divide((IntegerVariableClass *) obj, &e->u.divide, caller_gid);
			else
				error("Divide: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_arc:
		verbose("ElementaryAction_draw_arc");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_arc.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawArc((DynamicLineArtClass *) obj, &e->u.draw_arc, caller_gid);
			else
				error("DrawArc: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_line:
		verbose("ElementaryAction_draw_line");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_line.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawLine((DynamicLineArtClass *) obj, &e->u.draw_line, caller_gid);
			else
				error("DrawLine: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_oval:
		verbose("ElementaryAction_draw_oval");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_oval.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawOval((DynamicLineArtClass *) obj, &e->u.draw_oval, caller_gid);
			else
				error("DrawOval: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_polygon:
		verbose("ElementaryAction_draw_polygon");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_polygon.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawPolygon((DynamicLineArtClass *) obj, &e->u.draw_polygon, caller_gid);
			else
				error("DrawPolygon: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_polyline:
		verbose("ElementaryAction_draw_polyline");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_polyline.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawPolyline((DynamicLineArtClass *) obj, &e->u.draw_polyline, caller_gid);
			else
				error("DrawPolyline: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_rectangle:
		verbose("ElementaryAction_draw_rectangle");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_rectangle.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawRectangle((DynamicLineArtClass *) obj, &e->u.draw_rectangle, caller_gid);
			else
				error("DrawRectangle: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_draw_sector:
		verbose("ElementaryAction_draw_sector");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.draw_sector.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_DrawSector((DynamicLineArtClass *) obj, &e->u.draw_sector, caller_gid);
			else
				error("DrawSector: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_fork:
		verbose("ElementaryAction_fork");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.fork.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			/* UK MHEG Profile says we dont need to support Remote or InterchangedProgramClass */
			if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Fork((ResidentProgramClass *) obj, &e->u.fork, caller_gid);
			else
				error("Call: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_availability_status:
		verbose("ElementaryAction_get_availability_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_availability_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			RootClass_GetAvailabilityStatus(obj, &e->u.get_availability_status.availability_status_var, caller_gid);
		}
		break;

	case ElementaryAction_get_box_size:
		verbose("ElementaryAction_get_box_size");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_box_size.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_GetBoxSize((BitmapClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetBoxSize((DynamicLineArtClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetBoxSize((EntryFieldClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetBoxSize((HyperTextClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_GetBoxSize((RectangleClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetBoxSize((SliderClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_GetBoxSize((TextClass *) obj, &e->u.get_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_GetBoxSize((VideoClass *) obj, &e->u.get_box_size, caller_gid);
			else
				error("GetBoxSize: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_cell_item:
		verbose("ElementaryAction_get_cell_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_cell_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetCellItem((ListGroupClass *) obj, &e->u.get_cell_item, caller_gid);
			else
				error("GetCellItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_cursor_position:
		verbose("ElementaryAction_get_cursor_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_cursor_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_GetCursorPosition((SceneClass *) obj, &e->u.get_cursor_position, caller_gid);
			else
				error("GetCursorPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_engine_support:
		verbose("ElementaryAction_get_engine_support");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_engine_support.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_GetEngineSupport((ApplicationClass *) obj, &e->u.get_engine_support, caller_gid);
			else
				error("GetEngineSupport: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_entry_point:
		verbose("ElementaryAction_get_entry_point");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_entry_point.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetEntryPoint((EntryFieldClass *) obj, &e->u.get_entry_point, caller_gid);
			else
				error("GetEntryPoint: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_fill_colour:
		verbose("ElementaryAction_get_fill_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_fill_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetFillColour((DynamicLineArtClass *) obj, &e->u.get_fill_colour, caller_gid);
			else
				error("GetFillColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_first_item:
		verbose("ElementaryAction_get_first_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_first_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetFirstItem((ListGroupClass *) obj, &e->u.get_first_item, caller_gid);
			else
				error("GetFirstItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_highlight_status:
		verbose("ElementaryAction_get_highlight_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_highlight_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetHighlightStatus((EntryFieldClass *) obj, &e->u.get_highlight_status, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetHighlightStatus((HyperTextClass *) obj, &e->u.get_highlight_status, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetHighlightStatus((SliderClass *) obj, &e->u.get_highlight_status, caller_gid);
			else
				error("GetHighlightStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_interaction_status:
		verbose("ElementaryAction_get_interaction_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_interaction_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetInteractionStatus((EntryFieldClass *) obj, &e->u.get_interaction_status, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetInteractionStatus((HyperTextClass *) obj, &e->u.get_interaction_status, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetInteractionStatus((SliderClass *) obj, &e->u.get_interaction_status, caller_gid);
			else
				error("GetInteractionStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_item_status:
		verbose("ElementaryAction_get_item_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_item_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetItemStatus((ListGroupClass *) obj, &e->u.get_item_status, caller_gid);
			else
				error("GetItemStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_label:
		verbose("ElementaryAction_get_label");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_label.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_GetLabel((PushButtonClass *) obj, &e->u.get_label, caller_gid);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_GetLabel((SwitchButtonClass *) obj, &e->u.get_label, caller_gid);
			else
				error("GetLabel: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_last_anchor_fired:
		verbose("ElementaryAction_get_last_anchor_fired");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_last_anchor_fired.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetLastAnchorFired((HyperTextClass *) obj, &e->u.get_last_anchor_fired, caller_gid);
			else
				error("GetLastAnchorFired: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_line_colour:
		verbose("ElementaryAction_get_line_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_line_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetLineColour((DynamicLineArtClass *) obj, &e->u.get_line_colour, caller_gid);
			else
				error("GetLineColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_line_style:
		verbose("ElementaryAction_get_line_style");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_line_style.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetLineStyle((DynamicLineArtClass *) obj, &e->u.get_line_style, caller_gid);
			else
				error("GetLineStyle: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_line_width:
		verbose("ElementaryAction_get_line_width");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_line_width.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetLineWidth((DynamicLineArtClass *) obj, &e->u.get_line_width, caller_gid);
			else
				error("GetLineWidth: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_list_item:
		verbose("ElementaryAction_get_list_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_list_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetListItem((ListGroupClass *) obj, &e->u.get_list_item, caller_gid);
			else
				error("GetListItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_list_size:
		verbose("ElementaryAction_get_list_size");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_list_size.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetListSize((ListGroupClass *) obj, &e->u.get_list_size, caller_gid);
			else
				error("GetListSize: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_overwrite_mode:
		verbose("ElementaryAction_get_overwrite_mode");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_overwrite_mode.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetOverwriteMode((EntryFieldClass *) obj, &e->u.get_overwrite_mode, caller_gid);
			else
				error("GetOverwriteMode: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_portion:
		verbose("ElementaryAction_get_portion");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_portion.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetPortion((SliderClass *) obj, &e->u.get_portion, caller_gid);
			else
				error("GetPortion: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_position:
		verbose("ElementaryAction_get_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_GetPosition((BitmapClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_GetPosition((DynamicLineArtClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetPosition((EntryFieldClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetPosition((HyperTextClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_GetPosition((RectangleClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetPosition((SliderClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_GetPosition((TextClass *) obj, &e->u.get_position, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_GetPosition((VideoClass *) obj, &e->u.get_position, caller_gid);
			else
				error("GetPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_running_status:
		verbose("ElementaryAction_get_running_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_running_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			RootClass_GetRunningStatus(obj, &e->u.get_running_status.running_status_var, caller_gid);
		}
		break;

	case ElementaryAction_get_selection_status:
		verbose("ElementaryAction_get_selection_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_selection_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_GetSelectionStatus((SwitchButtonClass *) obj, &e->u.get_selection_status, caller_gid);
			else
				error("GetSelectionStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_slider_value:
		verbose("ElementaryAction_get_slider_value");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_slider_value.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_GetSliderValue((SliderClass *) obj, &e->u.get_slider_value, caller_gid);
			else
				error("GetSliderValue: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_text_content:
		verbose("ElementaryAction_get_text_content");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_text_content.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetTextContent((EntryFieldClass *) obj, &e->u.get_text_content, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetTextContent((HyperTextClass *) obj, &e->u.get_text_content, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_GetTextContent((TextClass *) obj, &e->u.get_text_content, caller_gid);
			else
				error("GetTextContent: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_text_data:
		verbose("ElementaryAction_get_text_data");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_text_data.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_TextClass)
				TextClass_GetTextData((TextClass *) obj, &e->u.get_text_data, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_GetTextData((EntryFieldClass *) obj, &e->u.get_text_data, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetTextData((HyperTextClass *) obj, &e->u.get_text_data, caller_gid);
			else
				error("GetTextData: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_token_position:
		verbose("ElementaryAction_get_token_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_token_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_GetTokenPosition((ListGroupClass *) obj, &e->u.get_token_position, caller_gid);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_GetTokenPosition((TokenGroupClass *) obj, &e->u.get_token_position, caller_gid);
			else
				error("GetTokenPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_volume:
		verbose("ElementaryAction_get_volume");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_volume.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_GetVolume((AudioClass *) obj, &e->u.get_volume, caller_gid);
			else
				error("GetVolume: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_launch:
		verbose("ElementaryAction_launch");
		ApplicationClass_Launch(MHEGEngine_getActiveApplication(), &e->u.launch, caller_gid);
		break;

	case ElementaryAction_lock_screen:
		verbose("ElementaryAction_lock_screen");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.lock_screen, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_LockScreen((ApplicationClass *) obj);
			else
				error("LockScreen: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_modulo:
		verbose("ElementaryAction_modulo");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.modulo.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_integer)
				IntegerVariableClass_Modulo((IntegerVariableClass *) obj, &e->u.modulo, caller_gid);
			else
				error("Modulo: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_move:
		verbose("ElementaryAction_move");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.move.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Move((ListGroupClass *) obj, &e->u.move, caller_gid);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Move((TokenGroupClass *) obj, &e->u.move, caller_gid);
			else
				error("Move: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_move_to:
		verbose("ElementaryAction_move_to");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.move_to.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_MoveTo((ListGroupClass *) obj, &e->u.move_to, caller_gid);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_MoveTo((TokenGroupClass *) obj, &e->u.move_to, caller_gid);
			else
				error("MoveTo: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_multiply:
		verbose("ElementaryAction_multiply");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.multiply.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_integer)
				IntegerVariableClass_Multiply((IntegerVariableClass *) obj, &e->u.multiply, caller_gid);
			else
				error("Multiply: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_open_connection:
		verbose("ElementaryAction_open_connection");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.open_connection.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_OpenConnection((ApplicationClass *) obj, &e->u.open_connection, caller_gid);
			else
				error("OpenConnection: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_preload:
		verbose("ElementaryAction_preload");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.preload, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_Preparation((AudioClass *) obj);
			else if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_Preparation((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_VariableClass)
				VariableClass_Preparation((VariableClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Preparation((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_Preparation((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_Preparation((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_LinkClass)
				LinkClass_Preparation((LinkClass *) obj);
			else if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Preparation((ListGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_Preparation((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Preparation((ResidentProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Preparation((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_Preparation((StreamClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_Preparation((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Preparation((TokenGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_Preparation((VideoClass *) obj);
			else
				error("Preload: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_put_before:
		verbose("ElementaryAction_put_before");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.put_before.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_PutBefore((BitmapClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_PutBefore((DynamicLineArtClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_PutBefore((EntryFieldClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_PutBefore((HyperTextClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_PutBefore((RectangleClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_PutBefore((SliderClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_PutBefore((TextClass *) obj, &e->u.put_before, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_PutBefore((VideoClass *) obj, &e->u.put_before, caller_gid);
			else
				error("PutBefore: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_put_behind:
		verbose("ElementaryAction_put_behind");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.put_behind.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_PutBehind((BitmapClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_PutBehind((DynamicLineArtClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_PutBehind((EntryFieldClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_PutBehind((HyperTextClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_PutBehind((RectangleClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_PutBehind((SliderClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_PutBehind((TextClass *) obj, &e->u.put_behind, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_PutBehind((VideoClass *) obj, &e->u.put_behind, caller_gid);
			else
				error("PutBehind: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_quit:
		verbose("ElementaryAction_quit");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.quit, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_Quit((ApplicationClass *) obj);
			else
				error("Quit: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_read_persistent:
		verbose("ElementaryAction_read_persistent");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.read_persistent.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_ReadPersistent((ApplicationClass *) obj, &e->u.read_persistent, caller_gid);
			else
				error("ReadPersistent: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_run:
		verbose("ElementaryAction_run");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.run, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_Activation((AudioClass *) obj);
			else if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_Activation((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Activation((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_Activation((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HotspotClass)
				HotspotClass_Activation((HotspotClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_Activation((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Activation((ListGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_Activation((PushButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_Activation((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_RTGraphicsClass)
				RTGraphicsClass_Activation((RTGraphicsClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Activation((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_Activation((StreamClass *) obj);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Activation((SwitchButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_Activation((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Activation((TokenGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_Activation((VideoClass *) obj);
			else
				error("Run: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_scale_bitmap:
		verbose("ElementaryAction_scale_bitmap");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.scale_bitmap.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_ScaleBitmap((BitmapClass *) obj, &e->u.scale_bitmap, caller_gid);
			else
				error("ScaleBitmap: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_scale_video:
		verbose("ElementaryAction_scale_video");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.scale_video.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_ScaleVideo((VideoClass *) obj, &e->u.scale_video, caller_gid);
			else
				error("ScaleVideo: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_scroll_items:
		verbose("ElementaryAction_scroll_items");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.scroll_items.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_ScrollItems((ListGroupClass *) obj, &e->u.scroll_items, caller_gid);
			else
				error("ScrollItems: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_select:
		verbose("ElementaryAction_select");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.select, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_HotspotClass)
				HotspotClass_Select((HotspotClass *) obj);
			else if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_Select((PushButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Select((SwitchButtonClass *) obj);
			else
				error("Select: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_select_item:
		verbose("ElementaryAction_select_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.select_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_SelectItem((ListGroupClass *) obj, &e->u.select_item, caller_gid);
			else
				error("SelectItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_send_event:
		verbose("ElementaryAction_send_event");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.send_event.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SendEvent((SceneClass *) obj, &e->u.send_event, caller_gid);
			else
				error("SendEvent: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_send_to_back:
		verbose("ElementaryAction_send_to_back");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.send_to_back, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SendToBack((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SendToBack((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SendToBack((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SendToBack((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SendToBack((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SendToBack((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SendToBack((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_SendToBack((VideoClass *) obj);
			else
				error("SendToBack: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_box_size:
		verbose("ElementaryAction_set_box_size");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_box_size.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetBoxSize((BitmapClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetBoxSize((DynamicLineArtClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetBoxSize((EntryFieldClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetBoxSize((HyperTextClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetBoxSize((RectangleClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetBoxSize((SliderClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetBoxSize((TextClass *) obj, &e->u.set_box_size, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_SetBoxSize((VideoClass *) obj, &e->u.set_box_size, caller_gid);
			else
				error("SetBoxSize: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_cache_priority:
		verbose("ElementaryAction_set_cache_priority");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_cache_priority.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_SetCachePriority((ApplicationClass *) obj, &e->u.set_cache_priority, caller_gid);
			else if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SetCachePriority((SceneClass *) obj, &e->u.set_cache_priority, caller_gid);
			else
				error("SetCachePriority: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_counter_end_position:
		verbose("ElementaryAction_set_counter_end_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_counter_end_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_SetCounterEndPosition((StreamClass *) obj, &e->u.set_counter_end_position, caller_gid);
			else
				error("SetCounterEndPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_counter_position:
		verbose("ElementaryAction_set_counter_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_counter_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_SetCounterPosition((StreamClass *) obj, &e->u.set_counter_position, caller_gid);
			else
				error("SetCounterPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_counter_trigger:
		verbose("ElementaryAction_set_counter_trigger");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_counter_trigger.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_SetCounterTrigger((StreamClass *) obj, &e->u.set_counter_trigger, caller_gid);
			else
				error("SetCounterTrigger: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_cursor_position:
		verbose("ElementaryAction_set_cursor_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_cursor_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SetCursorPosition((SceneClass *) obj, &e->u.set_cursor_position, caller_gid);
			else
				error("SetCursorPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_cursor_shape:
		verbose("ElementaryAction_set_cursor_shape");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_cursor_shape.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SetCursorShape((SceneClass *) obj, &e->u.set_cursor_shape, caller_gid);
			else
				error("SetCursorShape: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_data:
		verbose("ElementaryAction_set_data");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_data.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetData((BitmapClass *) obj, &e->u.set_data, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetData((TextClass *) obj, &e->u.set_data, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetData((EntryFieldClass *) obj, &e->u.set_data, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetData((HyperTextClass *) obj, &e->u.set_data, caller_gid);
			else if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_SetData((StreamClass *) obj, &e->u.set_data, caller_gid);
			else
				error("SetData: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_entry_point:
		verbose("ElementaryAction_set_entry_point");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_entry_point.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetEntryPoint((EntryFieldClass *) obj, &e->u.set_entry_point, caller_gid);
			else
				error("SetEntryPoint: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_fill_colour:
		verbose("ElementaryAction_set_fill_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_fill_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetFillColour((DynamicLineArtClass *) obj, &e->u.set_fill_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetFillColour((RectangleClass *) obj, &e->u.set_fill_colour, caller_gid);
			else
				error("SetFillColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_first_item:
		verbose("ElementaryAction_set_first_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_first_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_SetFirstItem((ListGroupClass *) obj, &e->u.set_first_item, caller_gid);
			else
				error("SetFirstItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_font_ref:
		verbose("ElementaryAction_set_font_ref");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_font_ref.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetFontRef((EntryFieldClass *) obj, &e->u.set_font_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetFontRef((HyperTextClass *) obj, &e->u.set_font_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetFontRef((TextClass *) obj, &e->u.set_font_ref, caller_gid);
			else
				error("SetFontRef: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_highlight_status:
		verbose("ElementaryAction_set_highlight_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_highlight_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetHighlightStatus((EntryFieldClass *) obj, &e->u.set_highlight_status, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetHighlightStatus((HyperTextClass *) obj, &e->u.set_highlight_status, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetHighlightStatus((SliderClass *) obj, &e->u.set_highlight_status, caller_gid);
			else
				error("SetHighlightStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_interaction_status:
		verbose("ElementaryAction_set_interaction_status");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_interaction_status.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetInteractionStatus((EntryFieldClass *) obj, &e->u.set_interaction_status, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetInteractionStatus((HyperTextClass *) obj, &e->u.set_interaction_status, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetInteractionStatus((SliderClass *) obj, &e->u.set_interaction_status, caller_gid);
			else
				error("SetInteractionStatus: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_label:
		verbose("ElementaryAction_set_label");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_label.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_SetLabel((PushButtonClass *) obj, &e->u.set_label, caller_gid);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_SetLabel((SwitchButtonClass *) obj, &e->u.set_label, caller_gid);
			else
				error("SetLabel: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_line_colour:
		verbose("ElementaryAction_set_line_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_line_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetLineColour((DynamicLineArtClass *) obj, &e->u.set_line_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetLineColour((RectangleClass *) obj, &e->u.set_line_colour, caller_gid);
			else
				error("SetLineColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_line_style:
		verbose("ElementaryAction_set_line_style");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_line_style.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetLineStyle((DynamicLineArtClass *) obj, &e->u.set_line_style, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetLineStyle((RectangleClass *) obj, &e->u.set_line_style, caller_gid);
			else
				error("SetLineStyle: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_line_width:
		verbose("ElementaryAction_set_line_width");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_line_width.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetLineWidth((DynamicLineArtClass *) obj, &e->u.set_line_width, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetLineWidth((RectangleClass *) obj, &e->u.set_line_width, caller_gid);
			else
				error("SetLineWidth: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_overwrite_mode:
		verbose("ElementaryAction_set_overwrite_mode");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_overwrite_mode.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetOverwriteMode((EntryFieldClass *) obj, &e->u.set_overwrite_mode, caller_gid);
			else
				error("SetOverwriteMode: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_palette_ref:
		verbose("ElementaryAction_set_palette_ref");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_palette_ref.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetPaletteRef((BitmapClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetPaletteRef((DynamicLineArtClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetPaletteRef((EntryFieldClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetPaletteRef((HyperTextClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetPaletteRef((RectangleClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetPaletteRef((SliderClass *) obj, &e->u.set_palette_ref, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetPaletteRef((TextClass *) obj, &e->u.set_palette_ref, caller_gid);
			else
				error("SetPaletteRef: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_portion:
		verbose("ElementaryAction_set_portion");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_portion.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetPortion((SliderClass *) obj, &e->u.set_portion, caller_gid);
			else
				error("SetPortion: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_position:
		verbose("ElementaryAction_set_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetPosition((BitmapClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_SetPosition((DynamicLineArtClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetPosition((EntryFieldClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetPosition((HyperTextClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_SetPosition((RectangleClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetPosition((SliderClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetPosition((TextClass *) obj, &e->u.set_position, caller_gid);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_SetPosition((VideoClass *) obj, &e->u.set_position, caller_gid);
			else
				error("SetPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_slider_value:
		verbose("ElementaryAction_set_slider_value");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_slider_value.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetSliderValue((SliderClass *) obj, &e->u.set_slider_value, caller_gid);
			else
				error("SetSliderValue: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_speed:
		verbose("ElementaryAction_set_speed");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_speed.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_SetSpeed((StreamClass *) obj, &e->u.set_speed, caller_gid);
			else
				error("SetSpeed: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_timer:
		verbose("ElementaryAction_set_timer");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_timer.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_SetTimer((ApplicationClass *) obj, &e->u.set_timer, caller_gid);
			else if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SetTimer((SceneClass *) obj, &e->u.set_timer, caller_gid);
			else
				error("SetTimer: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_transparency:
		verbose("ElementaryAction_set_transparency");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_transparency.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetTransparency((BitmapClass *) obj, &e->u.set_transparency, caller_gid);
			else
				error("SetTransparency: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_variable:
		verbose("ElementaryAction_set_variable");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_variable.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass)
				VariableClass_SetVariable((VariableClass *) obj, &e->u.set_variable.new_variable_value, caller_gid);
			else
				error("SetVariable: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_volume:
		verbose("ElementaryAction_set_volume");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_volume.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_SetVolume((AudioClass *) obj, &e->u.set_volume, caller_gid);
			else
				error("SetVolume: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_spawn:
		verbose("ElementaryAction_spawn");
		ApplicationClass_Spawn(MHEGEngine_getActiveApplication(), &e->u.spawn, caller_gid);
		break;

	case ElementaryAction_step:
		verbose("ElementaryAction_step");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.step.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Step((SliderClass *) obj, &e->u.step, caller_gid);
			else
				error("Step: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_stop:
		verbose("ElementaryAction_stop");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.stop, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_RemoteProgramClass)
				RemoteProgramClass_Deactivation((RemoteProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Deactivation((ResidentProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_InterchangedProgramClass)
				InterchangedProgramClass_Deactivation((InterchangedProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_Deactivation((AudioClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_Deactivation((VideoClass *) obj);
			else if(obj->inst.rtti == RTTI_RTGraphicsClass)
				RTGraphicsClass_Deactivation((RTGraphicsClass *) obj);
			else if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_Deactivation((StreamClass *) obj);
			else if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_Deactivation((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Deactivation((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_Deactivation((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_HotspotClass)
				HotspotClass_Deactivation((HotspotClass *) obj);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Deactivation((SwitchButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_Deactivation((PushButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_Deactivation((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_Deactivation((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_Deactivation((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Deactivation((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Deactivation((TokenGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Deactivation((ListGroupClass *) obj);
			else
				error("Stop: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_store_persistent:
		verbose("ElementaryAction_store_persistent");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.store_persistent.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_StorePersistent((ApplicationClass *) obj, &e->u.store_persistent, caller_gid);
			else
				error("StorePersistent: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_subtract:
		verbose("ElementaryAction_subtract");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.subtract.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VariableClass
			&& VariableClass_type((VariableClass *) obj) == OriginalValue_integer)
				IntegerVariableClass_Subtract((IntegerVariableClass *) obj, &e->u.subtract, caller_gid);
			else
				error("Subtract: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_test_variable:
		verbose("ElementaryAction_test_variable");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.test_variable.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			op = GenericInteger_getInteger(&e->u.test_variable.operator, caller_gid);
			if(obj->inst.rtti == RTTI_VariableClass)
				VariableClass_TestVariable((VariableClass *) obj, op, &e->u.test_variable.comparison_value, caller_gid);
			else
				error("TestVariable: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_toggle:
		verbose("ElementaryAction_toggle");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.toggle, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Toggle((SwitchButtonClass *) obj);
			else
				error("Toggle: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_toggle_item:
		verbose("ElementaryAction_toggle_item");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.toggle_item.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_ToggleItem((ListGroupClass *) obj, &e->u.toggle_item, caller_gid);
			else
				error("ToggleItem: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_transition_to:
		verbose("ElementaryAction_transition_to");
		MHEGEngine_TransitionTo(&e->u.transition_to, caller_gid);
		break;

	case ElementaryAction_unload:
		verbose("ElementaryAction_unload");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.unload, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_RemoteProgramClass)
				RemoteProgramClass_Destruction((RemoteProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_ResidentProgramClass)
				ResidentProgramClass_Destruction((ResidentProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_InterchangedProgramClass)
				InterchangedProgramClass_Destruction((InterchangedProgramClass *) obj);
			else if(obj->inst.rtti == RTTI_PaletteClass)
				PaletteClass_Destruction((PaletteClass *) obj);
			else if(obj->inst.rtti == RTTI_FontClass)
				FontClass_Destruction((FontClass *) obj);
			else if(obj->inst.rtti == RTTI_CursorShapeClass)
				CursorShapeClass_Destruction((CursorShapeClass *) obj);
			else if(obj->inst.rtti == RTTI_VariableClass)
				VariableClass_Destruction((VariableClass *) obj);
			else if(obj->inst.rtti == RTTI_LinkClass)
				LinkClass_Destruction((LinkClass *) obj);
			else if(obj->inst.rtti == RTTI_AudioClass)
				AudioClass_Destruction((AudioClass *) obj);
			else if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_Destruction((VideoClass *) obj);
			else if(obj->inst.rtti == RTTI_RTGraphicsClass)
				RTGraphicsClass_Destruction((RTGraphicsClass *) obj);
			else if(obj->inst.rtti == RTTI_StreamClass)
				StreamClass_Destruction((StreamClass *) obj);
			else if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_Destruction((BitmapClass *) obj);
			else if(obj->inst.rtti == RTTI_DynamicLineArtClass)
				DynamicLineArtClass_Destruction((DynamicLineArtClass *) obj);
			else if(obj->inst.rtti == RTTI_RectangleClass)
				RectangleClass_Destruction((RectangleClass *) obj);
			else if(obj->inst.rtti == RTTI_HotspotClass)
				HotspotClass_Destruction((HotspotClass *) obj);
			else if(obj->inst.rtti == RTTI_SwitchButtonClass)
				SwitchButtonClass_Destruction((SwitchButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_PushButtonClass)
				PushButtonClass_Destruction((PushButtonClass *) obj);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_Destruction((TextClass *) obj);
			else if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_Destruction((EntryFieldClass *) obj);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_Destruction((HyperTextClass *) obj);
			else if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_Destruction((SliderClass *) obj);
			else if(obj->inst.rtti == RTTI_TokenGroupClass)
				TokenGroupClass_Destruction((TokenGroupClass *) obj);
			else if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_Destruction((ListGroupClass *) obj);
			else
				error("Unload: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_unlock_screen:
		verbose("ElementaryAction_unlock_screen");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.unlock_screen, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ApplicationClass)
				ApplicationClass_UnlockScreen((ApplicationClass *) obj);
			else
				error("UnlockScreen: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_background_colour:
		verbose("ElementaryAction_set_background_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_background_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetBackgroundColour((EntryFieldClass *) obj, &e->u.set_background_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetBackgroundColour((HyperTextClass *) obj, &e->u.set_background_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetBackgroundColour((TextClass *) obj, &e->u.set_background_colour, caller_gid);
			else
				error("SetBackgroundColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_cell_position:
		verbose("ElementaryAction_set_cell_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_cell_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_ListGroupClass)
				ListGroupClass_SetCellPosition((ListGroupClass *) obj, &e->u.set_cell_position, caller_gid);
			else
				error("SetCellPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_input_register:
		verbose("ElementaryAction_set_input_register");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_input_register.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SceneClass)
				SceneClass_SetInputRegister((SceneClass *) obj, &e->u.set_input_register, caller_gid);
			else
				error("SetInputRegister: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_text_colour:
		verbose("ElementaryAction_set_text_colour");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_text_colour.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetTextColour((EntryFieldClass *) obj, &e->u.set_text_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetTextColour((HyperTextClass *) obj, &e->u.set_text_colour, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetTextColour((TextClass *) obj, &e->u.set_text_colour, caller_gid);
			else
				error("SetTextColour: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_font_attributes:
		verbose("ElementaryAction_set_font_attributes");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_font_attributes.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_EntryFieldClass)
				EntryFieldClass_SetFontAttributes((EntryFieldClass *) obj, &e->u.set_font_attributes, caller_gid);
			else if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetFontAttributes((HyperTextClass *) obj, &e->u.set_font_attributes, caller_gid);
			else if(obj->inst.rtti == RTTI_TextClass)
				TextClass_SetFontAttributes((TextClass *) obj, &e->u.set_font_attributes, caller_gid);
			else
				error("SetFontAttributes: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_video_decode_offset:
		verbose("ElementaryAction_set_video_decode_offset");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_video_decode_offset.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_SetVideoDecodeOffset((VideoClass *) obj, &e->u.set_video_decode_offset, caller_gid);
			else
				error("SetVideoDecodeOffset: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_video_decode_offset:
		verbose("ElementaryAction_get_video_decode_offset");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_video_decode_offset.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_VideoClass)
				VideoClass_GetVideoDecodeOffset((VideoClass *) obj, &e->u.get_video_decode_offset, caller_gid);
			else
				error("GetVideoDecodeOffset: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_focus_position:
		verbose("ElementaryAction_get_focus_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_focus_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_GetFocusPosition((HyperTextClass *) obj, &e->u.get_focus_position, caller_gid);
			else
				error("GetFocusPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_focus_position:
		verbose("ElementaryAction_set_focus_position");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_focus_position.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_HyperTextClass)
				HyperTextClass_SetFocusPosition((HyperTextClass *) obj, &e->u.set_focus_position, caller_gid);
			else
				error("SetFocusPosition: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_bitmap_decode_offset:
		verbose("ElementaryAction_set_bitmap_decode_offset");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_bitmap_decode_offset.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_SetBitmapDecodeOffset((BitmapClass *) obj, &e->u.set_bitmap_decode_offset, caller_gid);
			else
				error("SetBitmapDecodeOffset: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_get_bitmap_decode_offset:
		verbose("ElementaryAction_get_bitmap_decode_offset");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.get_bitmap_decode_offset.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_BitmapClass)
				BitmapClass_GetBitmapDecodeOffset((BitmapClass *) obj, &e->u.get_bitmap_decode_offset, caller_gid);
			else
				error("GetBitmapDecodeOffset: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	case ElementaryAction_set_slider_parameters:
		verbose("ElementaryAction_set_slider_parameters");
		if(((ref = GenericObjectReference_getObjectReference(&e->u.set_slider_parameters.target, caller_gid)) != NULL)
		&& ((obj = MHEGEngine_findObjectReference(ref, caller_gid)) != NULL))
		{
			if(obj->inst.rtti == RTTI_SliderClass)
				SliderClass_SetSliderParameters((SliderClass *) obj, &e->u.set_slider_parameters, caller_gid);
			else
				error("SetSliderParameters: unexpected target: %s", ExternalReference_name(&obj->inst.ref));
		}
		break;

	default:
		verbose("Unknown ElementaryAction type: %d", e->choice);
		break;
	}

	return;
}

char *
ElementaryAction_name(ElementaryAction *e)
{
	switch(e->choice)
	{
	case ElementaryAction_activate:
		return "activate";

	case ElementaryAction_add:
		return "add";

	case ElementaryAction_add_item:
		return "add_item";

	case ElementaryAction_append:
		return "append";

	case ElementaryAction_bring_to_front:
		return "bring_to_front";

	case ElementaryAction_call:
		return "call";

	case ElementaryAction_call_action_slot:
		return "call_action_slot";

	case ElementaryAction_clear:
		return "clear";

	case ElementaryAction_clone:
		return "clone";

	case ElementaryAction_close_connection:
		return "close_connection";

	case ElementaryAction_deactivate:
		return "deactivate";

	case ElementaryAction_del_item:
		return "del_item";

	case ElementaryAction_deselect:
		return "deselect";

	case ElementaryAction_deselect_item:
		return "deselect_item";

	case ElementaryAction_divide:
		return "divide";

	case ElementaryAction_draw_arc:
		return "draw_arc";

	case ElementaryAction_draw_line:
		return "draw_line";

	case ElementaryAction_draw_oval:
		return "draw_oval";

	case ElementaryAction_draw_polygon:
		return "draw_polygon";

	case ElementaryAction_draw_polyline:
		return "draw_polyline";

	case ElementaryAction_draw_rectangle:
		return "draw_rectangle";

	case ElementaryAction_draw_sector:
		return "draw_sector";

	case ElementaryAction_fork:
		return "fork";

	case ElementaryAction_get_availability_status:
		return "get_availability_status";

	case ElementaryAction_get_box_size:
		return "get_box_size";

	case ElementaryAction_get_cell_item:
		return "get_cell_item";

	case ElementaryAction_get_cursor_position:
		return "get_cursor_position";

	case ElementaryAction_get_engine_support:
		return "get_engine_support";

	case ElementaryAction_get_entry_point:
		return "get_entry_point";

	case ElementaryAction_get_fill_colour:
		return "get_fill_colour";

	case ElementaryAction_get_first_item:
		return "get_first_item";

	case ElementaryAction_get_highlight_status:
		return "get_highlight_status";

	case ElementaryAction_get_interaction_status:
		return "get_interaction_status";

	case ElementaryAction_get_item_status:
		return "get_item_status";

	case ElementaryAction_get_label:
		return "get_label";

	case ElementaryAction_get_last_anchor_fired:
		return "get_last_anchor_fired";

	case ElementaryAction_get_line_colour:
		return "get_line_colour";

	case ElementaryAction_get_line_style:
		return "get_line_style";

	case ElementaryAction_get_line_width:
		return "get_line_width";

	case ElementaryAction_get_list_item:
		return "get_list_item";

	case ElementaryAction_get_list_size:
		return "get_list_size";

	case ElementaryAction_get_overwrite_mode:
		return "get_overwrite_mode";

	case ElementaryAction_get_portion:
		return "get_portion";

	case ElementaryAction_get_position:
		return "get_position";

	case ElementaryAction_get_running_status:
		return "get_running_status";

	case ElementaryAction_get_selection_status:
		return "get_selection_status";

	case ElementaryAction_get_slider_value:
		return "get_slider_value";

	case ElementaryAction_get_text_content:
		return "get_text_content";

	case ElementaryAction_get_text_data:
		return "get_text_data";

	case ElementaryAction_get_token_position:
		return "get_token_position";

	case ElementaryAction_get_volume:
		return "get_volume";

	case ElementaryAction_launch:
		return "launch";

	case ElementaryAction_lock_screen:
		return "lock_screen";

	case ElementaryAction_modulo:
		return "modulo";

	case ElementaryAction_move:
		return "move";

	case ElementaryAction_move_to:
		return "move_to";

	case ElementaryAction_multiply:
		return "multiply";

	case ElementaryAction_open_connection:
		return "open_connection";

	case ElementaryAction_preload:
		return "preload";

	case ElementaryAction_put_before:
		return "put_before";

	case ElementaryAction_put_behind:
		return "put_behind";

	case ElementaryAction_quit:
		return "quit";

	case ElementaryAction_read_persistent:
		return "read_persistent";

	case ElementaryAction_run:
		return "run";

	case ElementaryAction_scale_bitmap:
		return "scale_bitmap";

	case ElementaryAction_scale_video:
		return "scale_video";

	case ElementaryAction_scroll_items:
		return "scroll_items";

	case ElementaryAction_select:
		return "select";

	case ElementaryAction_select_item:
		return "select_item";

	case ElementaryAction_send_event:
		return "send_event";

	case ElementaryAction_send_to_back:
		return "send_to_back";

	case ElementaryAction_set_box_size:
		return "set_box_size";

	case ElementaryAction_set_cache_priority:
		return "set_cache_priority";

	case ElementaryAction_set_counter_end_position:
		return "set_counter_end_position";

	case ElementaryAction_set_counter_position:
		return "set_counter_position";

	case ElementaryAction_set_counter_trigger:
		return "set_counter_trigger";

	case ElementaryAction_set_cursor_position:
		return "set_cursor_position";

	case ElementaryAction_set_cursor_shape:
		return "set_cursor_shape";

	case ElementaryAction_set_data:
		return "set_data";

	case ElementaryAction_set_entry_point:
		return "set_entry_point";

	case ElementaryAction_set_fill_colour:
		return "set_fill_colour";

	case ElementaryAction_set_first_item:
		return "set_first_item";

	case ElementaryAction_set_font_ref:
		return "set_font_ref";

	case ElementaryAction_set_highlight_status:
		return "set_highlight_status";

	case ElementaryAction_set_interaction_status:
		return "set_interaction_status";

	case ElementaryAction_set_label:
		return "set_label";

	case ElementaryAction_set_line_colour:
		return "set_line_colour";

	case ElementaryAction_set_line_style:
		return "set_line_style";

	case ElementaryAction_set_line_width:
		return "set_line_width";

	case ElementaryAction_set_overwrite_mode:
		return "set_overwrite_mode";

	case ElementaryAction_set_palette_ref:
		return "set_palette_ref";

	case ElementaryAction_set_portion:
		return "set_portion";

	case ElementaryAction_set_position:
		return "set_position";

	case ElementaryAction_set_slider_value:
		return "set_slider_value";

	case ElementaryAction_set_speed:
		return "set_speed";

	case ElementaryAction_set_timer:
		return "set_timer";

	case ElementaryAction_set_transparency:
		return "set_transparency";

	case ElementaryAction_set_variable:
		return "set_variable";

	case ElementaryAction_set_volume:
		return "set_volume";

	case ElementaryAction_spawn:
		return "spawn";

	case ElementaryAction_step:
		return "step";

	case ElementaryAction_stop:
		return "stop";

	case ElementaryAction_store_persistent:
		return "store_persistent";

	case ElementaryAction_subtract:
		return "subtract";

	case ElementaryAction_test_variable:
		return "test_variable";

	case ElementaryAction_toggle:
		return "toggle";

	case ElementaryAction_toggle_item:
		return "toggle_item";

	case ElementaryAction_transition_to:
		return "transition_to";

	case ElementaryAction_unload:
		return "unload";

	case ElementaryAction_unlock_screen:
		return "unlock_screen";

	case ElementaryAction_set_background_colour:
		return "set_background_colour";

	case ElementaryAction_set_cell_position:
		return "set_cell_position";

	case ElementaryAction_set_input_register:
		return "set_input_register";

	case ElementaryAction_set_text_colour:
		return "set_text_colour";

	case ElementaryAction_set_font_attributes:
		return "set_font_attributes";

	case ElementaryAction_set_video_decode_offset:
		return "set_video_decode_offset";

	case ElementaryAction_get_video_decode_offset:
		return "get_video_decode_offset";

	case ElementaryAction_get_focus_position:
		return "get_focus_position";

	case ElementaryAction_set_focus_position:
		return "set_focus_position";

	case ElementaryAction_set_bitmap_decode_offset:
		return "set_bitmap_decode_offset";

	case ElementaryAction_get_bitmap_decode_offset:
		return "get_bitmap_decode_offset";

	case ElementaryAction_set_slider_parameters:
		return "set_slider_parameters";

	default:
		error("Unknown ElementaryAction type: %d", e->choice);
		return NULL;
	}
}

