/*
 * EventType.c
 */

#include "EventType.h"
#include "utils.h"

char *
EventType_name(EventType t)
{
	switch(t)
	{
	case EventType_is_available:
		return "is_available";

	case EventType_content_available:
		return "content_available";

	case EventType_is_deleted:
		return "is_deleted";

	case EventType_is_running:
		return "is_running";

	case EventType_is_stopped:
		return "is_stopped";

	case EventType_user_input:
		return "user_input";

	case EventType_anchor_fired:
		return "anchor_fired";

	case EventType_timer_fired:
		return "timer_fired";

	case EventType_asynch_stopped:
		return "asynch_stopped";

	case EventType_interaction_completed:
		return "interaction_completed";

	case EventType_token_moved_from:
		return "token_moved_from";

	case EventType_token_moved_to:
		return "token_moved_to";

	case EventType_stream_event:
		return "stream_event";

	case EventType_stream_playing:
		return "stream_playing";

	case EventType_stream_stopped:
		return "stream_stopped";

	case EventType_counter_trigger:
		return "counter_trigger";

	case EventType_highlight_on:
		return "highlight_on";

	case EventType_highlight_off:
		return "highlight_off";

	case EventType_cursor_enter:
		return "cursor_enter";

	case EventType_cursor_leave:
		return "cursor_leave";

	case EventType_is_selected:
		return "is_selected";

	case EventType_is_deselected:
		return "is_deselected";

	case EventType_test_event:
		return "test_event";

	case EventType_first_item_presented:
		return "first_item_presented";

	case EventType_last_item_presented:
		return "last_item_presented";

	case EventType_head_items:
		return "head_items";

	case EventType_tail_items:
		return "tail_items";

	case EventType_item_selected:
		return "item_selected";

	case EventType_item_deselected:
		return "item_deselected";

	case EventType_entry_field_full:
		return "entry_field_full";

	case EventType_engine_event:
		return "engine_event";

	case EventType_focus_moved:
		return "focus_moved";

	case EventType_slider_value_changed:
		return "slider_value_changed";

	default:
		error("Unknown EventType: %d", t);
		return NULL;
	}
}

bool
EventType_isAsync(EventType t)
{
	switch(t)
	{
	case EventType_is_available:
		return false;

	case EventType_content_available:
		return true;

	case EventType_is_deleted:
		return false;

	case EventType_is_running:
		return false;

	case EventType_is_stopped:
		return false;

	case EventType_user_input:
		return true;

	case EventType_anchor_fired:
		return true;

	case EventType_timer_fired:
		return true;

	case EventType_asynch_stopped:
		return true;

	case EventType_interaction_completed:
		return true;

	case EventType_token_moved_from:
		return false;

	case EventType_token_moved_to:
		return false;

	case EventType_stream_event:
		return true;

	case EventType_stream_playing:
		return true;

	case EventType_stream_stopped:
		return true;

	case EventType_counter_trigger:
		return true;

	case EventType_highlight_on:
		return false;

	case EventType_highlight_off:
		return false;

	case EventType_cursor_enter:
		return true;

	case EventType_cursor_leave:
		return true;

	case EventType_is_selected:
		return false;

	case EventType_is_deselected:
		return false;

	case EventType_test_event:
		return false;

	case EventType_first_item_presented:
		return false;

	case EventType_last_item_presented:
		return false;

	case EventType_head_items:
		return false;

	case EventType_tail_items:
		return false;

	case EventType_item_selected:
		return false;

	case EventType_item_deselected:
		return false;

	case EventType_entry_field_full:
		return true;

	case EventType_engine_event:
		return true;

	case EventType_focus_moved:
		return true;

	case EventType_slider_value_changed:
		return true;

	default:
		error("Unknown EventType: %d", t);
		return false;
	}
}

