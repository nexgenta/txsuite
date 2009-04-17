/*
 * GroupItem.c
 */

#include <stdbool.h>

#include "MHEGEngine.h"
#include "GroupItem.h"
#include "ResidentProgramClass.h"
#include "RemoteProgramClass.h"
#include "InterchangedProgramClass.h"
#include "PaletteClass.h"
#include "FontClass.h"
#include "CursorShapeClass.h"
#include "VariableClass.h"
#include "LinkClass.h"
#include "StreamClass.h"
#include "BitmapClass.h"
#include "LineArtClass.h"
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
#include "utils.h"

RootClass *
GroupItem_rootClass(GroupItem *g)
{
	switch(g->choice)
	{
	case GroupItem_resident_program:
		return &g->u.resident_program.rootClass;

	case GroupItem_remote_program:
		return &g->u.remote_program.rootClass;

	case GroupItem_interchanged_program:
		return &g->u.interchanged_program.rootClass;

	case GroupItem_palette:
		return &g->u.palette.rootClass;

	case GroupItem_font:
		return &g->u.font.rootClass;

	case GroupItem_cursor_shape:
		return &g->u.cursor_shape.rootClass;

	case GroupItem_boolean_variable:
		return &g->u.boolean_variable.rootClass;

	case GroupItem_integer_variable:
		return &g->u.integer_variable.rootClass;

	case GroupItem_octet_string_variable:
		return &g->u.octet_string_variable.rootClass;

	case GroupItem_object_ref_variable:
		return &g->u.object_ref_variable.rootClass;

	case GroupItem_content_ref_variable:
		return &g->u.content_ref_variable.rootClass;

	case GroupItem_link:
		return &g->u.link.rootClass;

	case GroupItem_stream:
		return &g->u.stream.rootClass;

	case GroupItem_bitmap:
		return &g->u.bitmap.rootClass;

	case GroupItem_line_art:
		return &g->u.line_art.rootClass;

	case GroupItem_dynamic_line_art:
		return &g->u.dynamic_line_art.rootClass;

	case GroupItem_rectangle:
		return &g->u.rectangle.rootClass;

	case GroupItem_hotspot:
		return &g->u.hotspot.rootClass;

	case GroupItem_switch_button:
		return &g->u.switch_button.rootClass;

	case GroupItem_push_button:
		return &g->u.push_button.rootClass;

	case GroupItem_text:
		return &g->u.text.rootClass;

	case GroupItem_entry_field:
		return &g->u.entry_field.rootClass;

	case GroupItem_hyper_text:
		return &g->u.hyper_text.rootClass;

	case GroupItem_slider:
		return &g->u.slider.rootClass;

	case GroupItem_token_group:
		return &g->u.token_group.rootClass;

	case GroupItem_list_group:
		return &g->u.list_group.rootClass;

	default:
		error("Unknown GroupItem type: %d", g->choice);
		return NULL;
	}
}

bool
GroupItem_isInitiallyActive(GroupItem *g)
{
	bool active = false;

	switch(g->choice)
	{
	case GroupItem_resident_program:
		active = g->u.resident_program.initially_active;
		break;

	case GroupItem_remote_program:
		active = g->u.remote_program.initially_active;
		break;

	case GroupItem_interchanged_program:
		active = g->u.interchanged_program.initially_active;
		break;

	case GroupItem_palette:
		active = g->u.palette.initially_active;
		break;

	case GroupItem_font:
		active = g->u.font.initially_active;
		break;

	case GroupItem_cursor_shape:
		active = g->u.cursor_shape.initially_active;
		break;

	case GroupItem_boolean_variable:
		active = g->u.boolean_variable.initially_active;
		break;

	case GroupItem_integer_variable:
		active = g->u.integer_variable.initially_active;
		break;

	case GroupItem_octet_string_variable:
		active = g->u.octet_string_variable.initially_active;
		break;

	case GroupItem_object_ref_variable:
		active = g->u.object_ref_variable.initially_active;
		break;

	case GroupItem_content_ref_variable:
		active = g->u.content_ref_variable.initially_active;
		break;

	case GroupItem_link:
		active = g->u.link.initially_active;
		break;

	case GroupItem_stream:
		active = g->u.stream.initially_active;
		break;

	case GroupItem_bitmap:
		active = g->u.bitmap.initially_active;
		break;

	case GroupItem_line_art:
		active = g->u.line_art.initially_active;
		break;

	case GroupItem_dynamic_line_art:
		active = g->u.dynamic_line_art.initially_active;
		break;

	case GroupItem_rectangle:
		active = g->u.rectangle.initially_active;
		break;

	case GroupItem_hotspot:
		active = g->u.hotspot.initially_active;
		break;

	case GroupItem_switch_button:
		active = g->u.switch_button.initially_active;
		break;

	case GroupItem_push_button:
		active = g->u.push_button.initially_active;
		break;

	case GroupItem_text:
		active = g->u.text.initially_active;
		break;

	case GroupItem_entry_field:
		active = g->u.entry_field.initially_active;
		break;

	case GroupItem_hyper_text:
		active = g->u.hyper_text.initially_active;
		break;

	case GroupItem_slider:
		active = g->u.slider.initially_active;
		break;

	case GroupItem_token_group:
		active = g->u.token_group.initially_active;
		break;

	case GroupItem_list_group:
		active = g->u.list_group.initially_active;
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return active;
}

/*
 * returns the shared attribute of the item
 * shared is OPTIONAL, defaults to false
 */

bool
GroupItem_isShared(GroupItem *g)
{
	bool shared = false;

	switch(g->choice)
	{
	case GroupItem_resident_program:
		shared = g->u.resident_program.have_shared && g->u.resident_program.shared;
		break;

	case GroupItem_remote_program:
		shared = g->u.remote_program.have_shared && g->u.remote_program.shared;
		break;

	case GroupItem_interchanged_program:
		shared = g->u.interchanged_program.have_shared && g->u.interchanged_program.shared;
		break;

	case GroupItem_palette:
		shared = g->u.palette.have_shared && g->u.palette.shared;
		break;

	case GroupItem_font:
		shared = g->u.font.have_shared && g->u.font.shared;
		break;

	case GroupItem_cursor_shape:
		shared = g->u.cursor_shape.have_shared && g->u.cursor_shape.shared;
		break;

	case GroupItem_boolean_variable:
		shared = g->u.boolean_variable.have_shared && g->u.boolean_variable.shared;
		break;

	case GroupItem_integer_variable:
		shared = g->u.integer_variable.have_shared && g->u.integer_variable.shared;
		break;

	case GroupItem_octet_string_variable:
		shared = g->u.octet_string_variable.have_shared && g->u.octet_string_variable.shared;
		break;

	case GroupItem_object_ref_variable:
		shared = g->u.object_ref_variable.have_shared && g->u.object_ref_variable.shared;
		break;

	case GroupItem_content_ref_variable:
		shared = g->u.content_ref_variable.have_shared && g->u.content_ref_variable.shared;
		break;

	case GroupItem_link:
		shared = g->u.link.have_shared && g->u.link.shared;
		break;

	case GroupItem_stream:
		shared = g->u.stream.have_shared && g->u.stream.shared;
		break;

	case GroupItem_bitmap:
		shared = g->u.bitmap.have_shared && g->u.bitmap.shared;
		break;

	case GroupItem_line_art:
		shared = g->u.line_art.have_shared && g->u.line_art.shared;
		break;

	case GroupItem_dynamic_line_art:
		shared = g->u.dynamic_line_art.have_shared && g->u.dynamic_line_art.shared;
		break;

	case GroupItem_rectangle:
		shared = g->u.rectangle.have_shared && g->u.rectangle.shared;
		break;

	case GroupItem_hotspot:
		shared = g->u.hotspot.have_shared && g->u.hotspot.shared;
		break;

	case GroupItem_switch_button:
		shared = g->u.switch_button.have_shared && g->u.switch_button.shared;
		break;

	case GroupItem_push_button:
		shared = g->u.push_button.have_shared && g->u.push_button.shared;
		break;

	case GroupItem_text:
		shared = g->u.text.have_shared && g->u.text.shared;
		break;

	case GroupItem_entry_field:
		shared = g->u.entry_field.have_shared && g->u.entry_field.shared;
		break;

	case GroupItem_hyper_text:
		shared = g->u.hyper_text.have_shared && g->u.hyper_text.shared;
		break;

	case GroupItem_slider:
		shared = g->u.slider.have_shared && g->u.slider.shared;
		break;

	case GroupItem_token_group:
		shared = g->u.token_group.have_shared && g->u.token_group.shared;
		break;

	case GroupItem_list_group:
		shared = g->u.list_group.have_shared && g->u.list_group.shared;
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return shared;
}

void
GroupItem_Preparation(GroupItem *g)
{
	switch(g->choice)
	{
	case GroupItem_resident_program:
		ResidentProgramClass_Preparation(&g->u.resident_program);
		break;

	case GroupItem_remote_program:
		RemoteProgramClass_Preparation(&g->u.remote_program);
		break;

	case GroupItem_interchanged_program:
		InterchangedProgramClass_Preparation(&g->u.interchanged_program);
		break;

	case GroupItem_palette:
		PaletteClass_Preparation(&g->u.palette);
		break;

	case GroupItem_font:
		FontClass_Preparation(&g->u.font);
		break;

	case GroupItem_cursor_shape:
		CursorShapeClass_Preparation(&g->u.cursor_shape);
		break;

	case GroupItem_boolean_variable:
		VariableClass_Preparation(&g->u.boolean_variable);
		break;

	case GroupItem_integer_variable:
		VariableClass_Preparation(&g->u.integer_variable);
		break;

	case GroupItem_octet_string_variable:
		VariableClass_Preparation(&g->u.octet_string_variable);
		break;

	case GroupItem_object_ref_variable:
		VariableClass_Preparation(&g->u.object_ref_variable);
		break;

	case GroupItem_content_ref_variable:
		VariableClass_Preparation(&g->u.content_ref_variable);
		break;

	case GroupItem_link:
		LinkClass_Preparation(&g->u.link);
		break;

	case GroupItem_stream:
		StreamClass_Preparation(&g->u.stream);
		break;

	case GroupItem_bitmap:
		BitmapClass_Preparation(&g->u.bitmap);
		break;

	case GroupItem_line_art:
		LineArtClass_Preparation(&g->u.line_art);
		break;

	case GroupItem_dynamic_line_art:
		DynamicLineArtClass_Preparation(&g->u.dynamic_line_art);
		break;

	case GroupItem_rectangle:
		RectangleClass_Preparation(&g->u.rectangle);
		break;

	case GroupItem_hotspot:
		HotspotClass_Preparation(&g->u.hotspot);
		break;

	case GroupItem_switch_button:
		SwitchButtonClass_Preparation(&g->u.switch_button);
		break;

	case GroupItem_push_button:
		PushButtonClass_Preparation(&g->u.push_button);
		break;

	case GroupItem_text:
		TextClass_Preparation(&g->u.text);
		break;

	case GroupItem_entry_field:
		EntryFieldClass_Preparation(&g->u.entry_field);
		break;

	case GroupItem_hyper_text:
		HyperTextClass_Preparation(&g->u.hyper_text);
		break;

	case GroupItem_slider:
		SliderClass_Preparation(&g->u.slider);
		break;

	case GroupItem_token_group:
		TokenGroupClass_Preparation(&g->u.token_group);
		break;

	case GroupItem_list_group:
		ListGroupClass_Preparation(&g->u.list_group);
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return;
}

void
GroupItem_Activation(GroupItem *g)
{
	switch(g->choice)
	{
	case GroupItem_resident_program:
		ResidentProgramClass_Activation(&g->u.resident_program);
		break;

	case GroupItem_remote_program:
		RemoteProgramClass_Activation(&g->u.remote_program);
		break;

	case GroupItem_interchanged_program:
		InterchangedProgramClass_Activation(&g->u.interchanged_program);
		break;

	case GroupItem_palette:
		PaletteClass_Activation(&g->u.palette);
		break;

	case GroupItem_font:
		FontClass_Activation(&g->u.font);
		break;

	case GroupItem_cursor_shape:
		CursorShapeClass_Activation(&g->u.cursor_shape);
		break;

	case GroupItem_boolean_variable:
		VariableClass_Activation(&g->u.boolean_variable);
		break;

	case GroupItem_integer_variable:
		VariableClass_Activation(&g->u.integer_variable);
		break;

	case GroupItem_octet_string_variable:
		VariableClass_Activation(&g->u.octet_string_variable);
		break;

	case GroupItem_object_ref_variable:
		VariableClass_Activation(&g->u.object_ref_variable);
		break;

	case GroupItem_content_ref_variable:
		VariableClass_Activation(&g->u.content_ref_variable);
		break;

	case GroupItem_link:
		LinkClass_Activation(&g->u.link);
		break;

	case GroupItem_stream:
		StreamClass_Activation(&g->u.stream);
		break;

	case GroupItem_bitmap:
		BitmapClass_Activation(&g->u.bitmap);
		break;

	case GroupItem_line_art:
		LineArtClass_Activation(&g->u.line_art);
		break;

	case GroupItem_dynamic_line_art:
		DynamicLineArtClass_Activation(&g->u.dynamic_line_art);
		break;

	case GroupItem_rectangle:
		RectangleClass_Activation(&g->u.rectangle);
		break;

	case GroupItem_hotspot:
		HotspotClass_Activation(&g->u.hotspot);
		break;

	case GroupItem_switch_button:
		SwitchButtonClass_Activation(&g->u.switch_button);
		break;

	case GroupItem_push_button:
		PushButtonClass_Activation(&g->u.push_button);
		break;

	case GroupItem_text:
		TextClass_Activation(&g->u.text);
		break;

	case GroupItem_entry_field:
		EntryFieldClass_Activation(&g->u.entry_field);
		break;

	case GroupItem_hyper_text:
		HyperTextClass_Activation(&g->u.hyper_text);
		break;

	case GroupItem_slider:
		SliderClass_Activation(&g->u.slider);
		break;

	case GroupItem_token_group:
		TokenGroupClass_Activation(&g->u.token_group);
		break;

	case GroupItem_list_group:
		ListGroupClass_Activation(&g->u.list_group);
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return;
}

void
GroupItem_Deactivation(GroupItem *g)
{
	switch(g->choice)
	{
	case GroupItem_resident_program:
		ResidentProgramClass_Deactivation(&g->u.resident_program);
		break;

	case GroupItem_remote_program:
		RemoteProgramClass_Deactivation(&g->u.remote_program);
		break;

	case GroupItem_interchanged_program:
		InterchangedProgramClass_Deactivation(&g->u.interchanged_program);
		break;

	case GroupItem_palette:
		PaletteClass_Deactivation(&g->u.palette);
		break;

	case GroupItem_font:
		FontClass_Deactivation(&g->u.font);
		break;

	case GroupItem_cursor_shape:
		CursorShapeClass_Deactivation(&g->u.cursor_shape);
		break;

	case GroupItem_boolean_variable:
		VariableClass_Deactivation(&g->u.boolean_variable);
		break;

	case GroupItem_integer_variable:
		VariableClass_Deactivation(&g->u.integer_variable);
		break;

	case GroupItem_octet_string_variable:
		VariableClass_Deactivation(&g->u.octet_string_variable);
		break;

	case GroupItem_object_ref_variable:
		VariableClass_Deactivation(&g->u.object_ref_variable);
		break;

	case GroupItem_content_ref_variable:
		VariableClass_Deactivation(&g->u.content_ref_variable);
		break;

	case GroupItem_link:
		LinkClass_Deactivation(&g->u.link);
		break;

	case GroupItem_stream:
		StreamClass_Deactivation(&g->u.stream);
		break;

	case GroupItem_bitmap:
		BitmapClass_Deactivation(&g->u.bitmap);
		break;

	case GroupItem_line_art:
		LineArtClass_Deactivation(&g->u.line_art);
		break;

	case GroupItem_dynamic_line_art:
		DynamicLineArtClass_Deactivation(&g->u.dynamic_line_art);
		break;

	case GroupItem_rectangle:
		RectangleClass_Deactivation(&g->u.rectangle);
		break;

	case GroupItem_hotspot:
		HotspotClass_Deactivation(&g->u.hotspot);
		break;

	case GroupItem_switch_button:
		SwitchButtonClass_Deactivation(&g->u.switch_button);
		break;

	case GroupItem_push_button:
		PushButtonClass_Deactivation(&g->u.push_button);
		break;

	case GroupItem_text:
		TextClass_Deactivation(&g->u.text);
		break;

	case GroupItem_entry_field:
		EntryFieldClass_Deactivation(&g->u.entry_field);
		break;

	case GroupItem_hyper_text:
		HyperTextClass_Deactivation(&g->u.hyper_text);
		break;

	case GroupItem_slider:
		SliderClass_Deactivation(&g->u.slider);
		break;

	case GroupItem_token_group:
		TokenGroupClass_Deactivation(&g->u.token_group);
		break;

	case GroupItem_list_group:
		ListGroupClass_Deactivation(&g->u.list_group);
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return;
}

void
GroupItem_Destruction(GroupItem *g)
{
	switch(g->choice)
	{
	case GroupItem_resident_program:
		ResidentProgramClass_Destruction(&g->u.resident_program);
		break;

	case GroupItem_remote_program:
		RemoteProgramClass_Destruction(&g->u.remote_program);
		break;

	case GroupItem_interchanged_program:
		InterchangedProgramClass_Destruction(&g->u.interchanged_program);
		break;

	case GroupItem_palette:
		PaletteClass_Destruction(&g->u.palette);
		break;

	case GroupItem_font:
		FontClass_Destruction(&g->u.font);
		break;

	case GroupItem_cursor_shape:
		CursorShapeClass_Destruction(&g->u.cursor_shape);
		break;

	case GroupItem_boolean_variable:
		VariableClass_Destruction(&g->u.boolean_variable);
		break;

	case GroupItem_integer_variable:
		VariableClass_Destruction(&g->u.integer_variable);
		break;

	case GroupItem_octet_string_variable:
		VariableClass_Destruction(&g->u.octet_string_variable);
		break;

	case GroupItem_object_ref_variable:
		VariableClass_Destruction(&g->u.object_ref_variable);
		break;

	case GroupItem_content_ref_variable:
		VariableClass_Destruction(&g->u.content_ref_variable);
		break;

	case GroupItem_link:
		LinkClass_Destruction(&g->u.link);
		break;

	case GroupItem_stream:
		StreamClass_Destruction(&g->u.stream);
		break;

	case GroupItem_bitmap:
		BitmapClass_Destruction(&g->u.bitmap);
		break;

	case GroupItem_line_art:
		LineArtClass_Destruction(&g->u.line_art);
		break;

	case GroupItem_dynamic_line_art:
		DynamicLineArtClass_Destruction(&g->u.dynamic_line_art);
		break;

	case GroupItem_rectangle:
		RectangleClass_Destruction(&g->u.rectangle);
		break;

	case GroupItem_hotspot:
		HotspotClass_Destruction(&g->u.hotspot);
		break;

	case GroupItem_switch_button:
		SwitchButtonClass_Destruction(&g->u.switch_button);
		break;

	case GroupItem_push_button:
		PushButtonClass_Destruction(&g->u.push_button);
		break;

	case GroupItem_text:
		TextClass_Destruction(&g->u.text);
		break;

	case GroupItem_entry_field:
		EntryFieldClass_Destruction(&g->u.entry_field);
		break;

	case GroupItem_hyper_text:
		HyperTextClass_Destruction(&g->u.hyper_text);
		break;

	case GroupItem_slider:
		SliderClass_Destruction(&g->u.slider);
		break;

	case GroupItem_token_group:
		TokenGroupClass_Destruction(&g->u.token_group);
		break;

	case GroupItem_list_group:
		ListGroupClass_Destruction(&g->u.list_group);
		break;

	default:
		fatal("Unknown GroupItem type: %d", g->choice);
		break;
	}

	return;
}

