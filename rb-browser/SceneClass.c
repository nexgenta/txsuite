/*
 * SceneClass.c
 */

#include "MHEGEngine.h"
#include "SceneClass.h"
#include "RootClass.h"
#include "GroupClass.h"
#include "ActionClass.h"
#include "GroupItem.h"
#include "ExternalReference.h"
#include "GenericBoolean.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "GenericObjectReference.h"
#include "EventType.h"

void
default_SceneClassInstanceVars(SceneClass *t, SceneClassInstanceVars *v)
{
	bzero(v, sizeof(SceneClassInstanceVars));

	/* GroupClass */
	v->GroupCachePriority = t->original_group_cache_priority;
	v->Timers = NULL;
	v->removed_timers = NULL;

	v->next_clone = FIRST_CLONED_OBJ_NUM;

	return;
}

void
free_SceneClassInstanceVars(SceneClassInstanceVars *v)
{
	GroupClass_freeTimers(&v->Timers, &v->removed_timers);

	return;
}

void
SceneClass_Preparation(SceneClass *s)
{
	LIST_TYPE(GroupItem) *gi;

	verbose("SceneClass: %s; Preparation", ExternalReference_name(&s->rootClass.inst.ref));

	/* has it been prepared already */
	if(s->rootClass.inst.AvailabilityStatus)
		return;

	default_SceneClassInstanceVars(s, &s->inst);

	/* do Preparation on all Ingredients that are initially_active */
	gi = s->items;
	while(gi)
	{
		if(GroupItem_isInitiallyActive(&gi->item))
			GroupItem_Preparation(&gi->item);
		gi = gi->next;
	}

	/* Preparation inherited from the RootClass */
	RootClass_Preparation(&s->rootClass);

	return;
}

void
SceneClass_Activation(SceneClass *s)
{
	LIST_TYPE(GroupItem) *gi;

	verbose("SceneClass: %s; Activation", ExternalReference_name(&s->rootClass.inst.ref));

	/* do Activation of the RootClass */
	/* is it already activated */
	if(s->rootClass.inst.RunningStatus)
		return;

	/* has it been prepared yet */
	if(!s->rootClass.inst.AvailabilityStatus)
	{
		SceneClass_Preparation(s);
		/* generates an IsAvailable event */
		RootClass_Preparation(&s->rootClass);
	}

	if(s->have_aspect_ratio)
	{
/* TODO */
printf("TODO: SceneClass: %s; aspect_ratio=%d:%d\n", ExternalReference_name(&s->rootClass.inst.ref), s->aspect_ratio.width, s->aspect_ratio.height);
	}

	/* do Activation of the GroupClass */
	if(s->have_on_start_up)
		ActionClass_execute(&s->on_start_up, &s->rootClass.inst.ref.group_identifier);

	/* do Activation on all Ingredients that are initially_active */
	gi = s->items;
	while(gi)
	{
		/* do Activation of all the InitiallyActive group items */
		if(GroupItem_isInitiallyActive(&gi->item))
			GroupItem_Activation(&gi->item);
		gi = gi->next;
	}

	/* set RunningStatus */
	s->rootClass.inst.RunningStatus = true;

	/* generate IsRunning event */
	MHEGEngine_generateEvent(&s->rootClass.inst.ref, EventType_is_running, NULL);

	/* time base for absolute timers */
	gettimeofday(&s->inst.start_time, NULL);

	return;
}

void
SceneClass_Deactivation(SceneClass *s)
{
	LIST_TYPE(GroupItem) *gi;
	LIST_TYPE(GroupItem) *gi_tail;
	RootClass *r;

	verbose("SceneClass: %s; Deactivation", ExternalReference_name(&s->rootClass.inst.ref));

	/* if we are not active ignore it */
	if(!s->rootClass.inst.RunningStatus)
		return;

	/* run the OnCloseDown action */
	if(s->have_on_close_down)
		ActionClass_execute(&s->on_close_down, &s->rootClass.inst.ref.group_identifier);

	/* do Deactivation of all active Ingredients in the reverse order they appear in the list */
	gi = s->items;
	/* find the tail */
	gi_tail = (gi != NULL) ? gi->prev : NULL;
	gi = gi_tail;
	while(gi)
	{
		/* only do Deactivation if it is active */
		if((r = GroupItem_rootClass(&gi->item)) != NULL
		&& r->inst.RunningStatus)
			GroupItem_Deactivation(&gi->item);
		/* have we reached the head */
		gi = (gi->prev != gi_tail) ? gi->prev : NULL;
	}

	/* do RootClass Deactivation */
	RootClass_Deactivation(&s->rootClass);

	return;
}

void
SceneClass_Destruction(SceneClass *s)
{
	LIST_TYPE(GroupItem) *gi;
	LIST_TYPE(GroupItem) *gi_tail;
	RootClass *r;

	verbose("SceneClass: %s; Destruction", ExternalReference_name(&s->rootClass.inst.ref));

	/* is it already destroyed */
	if(!s->rootClass.inst.AvailabilityStatus)
		return;

	/* apply the Destruction behaviour to all Ingredients in the reverse order they appear in the list */
	gi = s->items;
	/* find the tail */
	gi_tail = (gi != NULL) ? gi->prev : NULL;
	gi = gi_tail;
	while(gi)
	{
		/* only do Destruction if it is available */
		if((r = GroupItem_rootClass(&gi->item)) != NULL
		&& r->inst.AvailabilityStatus)
			GroupItem_Destruction(&gi->item);
		/* have we reached the head */
		gi = (gi->prev != gi_tail) ? gi->prev : NULL;
	}

	/* Destruction inherited from RootClass */
	/* Deactivate it if it is running */
	if(s->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		SceneClass_Deactivation(s);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_SceneClassInstanceVars(&s->inst);

	/* generate an IsDeleted event */
	s->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&s->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
SceneClass_SetCachePriority(SceneClass *t, SetCachePriority *params, OctetString *caller_gid)
{
	verbose("SceneClass: %s; SetCachePriority (ignored)", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
SceneClass_SetTimer(SceneClass *t, SetTimer *params, OctetString *caller_gid)
{
	verbose("SceneClass: %s; SetTimer", ExternalReference_name(&t->rootClass.inst.ref));

	GroupClass_SetTimer(&t->rootClass.inst.ref, &t->inst.Timers, &t->inst.removed_timers, &t->inst.start_time, params, caller_gid);

	return;
}

void
SceneClass_SendEvent(SceneClass *t, SendEvent *params, OctetString *caller_gid)
{
	ObjectReference *src_ref;
	RootClass *src_obj;
	EventData data;
	EventData *data_ptr;
	OctetString *oct;

	verbose("SceneClass: %s; SendEvent", ExternalReference_name(&t->rootClass.inst.ref));

	/* want the fully qualified ExternalReference */
	src_ref = GenericObjectReference_getObjectReference(&params->emulated_event_source, caller_gid);
	if((src_obj = MHEGEngine_findObjectReference(src_ref, caller_gid)) == NULL)
		return;

	/* convert EmulatedEventData into EventData */
	if(params->have_emulated_event_data)
	{
		data_ptr = &data;
		switch(params->emulated_event_data.choice)
		{
		case EmulatedEventData_new_generic_boolean:
			data.choice = EventData_boolean;
			data.u.boolean = GenericBoolean_getBoolean(&params->emulated_event_data.u.new_generic_boolean, caller_gid);
			break;

		case EmulatedEventData_new_generic_integer:
			data.choice = EventData_integer;
			data.u.integer = GenericInteger_getInteger(&params->emulated_event_data.u.new_generic_integer, caller_gid);
			break;

		case EmulatedEventData_new_generic_octet_string:
			data.choice = EventData_octetstring;
			oct = GenericOctetString_getOctetString(&params->emulated_event_data.u.new_generic_octet_string, caller_gid);
			data.u.octetstring.size = oct->size;
			data.u.octetstring.data = oct->data;
			break;

		default:
			error("Unknown EmulatedEventData type: %d", params->emulated_event_data.choice);
			data_ptr = NULL;
			break;
		}
	}
	else
	{
		data_ptr = NULL;
	}

	/* should it be synchronous or asynchronous */
	if(EventType_isAsync(params->emulated_event_type))
		MHEGEngine_generateAsyncEvent(&src_obj->inst.ref, params->emulated_event_type, data_ptr);
	else
		MHEGEngine_generateEvent(&src_obj->inst.ref, params->emulated_event_type, data_ptr);

	return;
}

void
SceneClass_SetCursorShape(SceneClass *t, SetCursorShape *params, OctetString *caller_gid)
{
	error("SceneClass: %s; SetCursorShape not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
SceneClass_SetCursorPosition(SceneClass *t, SetCursorPosition *params, OctetString *caller_gid)
{
	error("SceneClass: %s; SetCursorPosition not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
SceneClass_GetCursorPosition(SceneClass *t, GetCursorPosition *params, OctetString *caller_gid)
{
	error("SceneClass: %s; GetCursorPosition not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
SceneClass_SetInputRegister(SceneClass *t, SetInputRegister *params, OctetString *caller_gid)
{
	verbose("SceneClass: %s; SetInputRegister", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: SceneClass_SetInputRegister not yet implemented\n");
	return;
}

