/*
 * LinkClass.c
 */

#include <stdbool.h>

#include "MHEGEngine.h"
#include "LinkClass.h"
#include "RootClass.h"
#include "EventType.h"
#include "ExternalReference.h"
#include "der_decode.h"

void
LinkClass_Preparation(LinkClass *t)
{
	verbose("LinkClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	RootClass_Preparation(&t->rootClass);

	return;
}

void
LinkClass_Activation(LinkClass *t)
{
	verbose("LinkClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* do the Activation behaviour of its base class, returns false if it is already activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

	/* add it to the list of active links */
	MHEGEngine_addActiveLink(t);

	/* set RunningStatus */
	t->rootClass.inst.RunningStatus = true;

	/* generate IsRunning event */
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	return;
}

void
LinkClass_Deactivation(LinkClass *t)
{
	verbose("LinkClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it already been deactivated */
	if(!t->rootClass.inst.RunningStatus)
		return;

	MHEGEngine_removeActiveLink(t);

	RootClass_Deactivation(&t->rootClass);

	return;
}

void
LinkClass_Destruction(LinkClass *t)
{
	verbose("LinkClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already destroyed */
	if(!t->rootClass.inst.AvailabilityStatus)
		return;

	/* Deactivate it if it is running */
	if(t->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		LinkClass_Deactivation(t);
	}

	/*
	 * we always cache the objects (ie dont free them)
	 * until free_InterchangedObject is called on the whole app or scene
	 */

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
LinkClass_Clone(LinkClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("LinkClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: LinkClass_Clone not yet implemented\n");
	return;
}

void
LinkClass_Activate(LinkClass *l)
{
	verbose("LinkClass: %s; Activate", ExternalReference_name(&l->rootClass.inst.ref));

	/* is it already active */
	if(l->rootClass.inst.RunningStatus)
		return;

	LinkClass_Activation(l);

	return;
}

void
LinkClass_Deactivate(LinkClass *l)
{
	verbose("LinkClass: %s; Deactivate", ExternalReference_name(&l->rootClass.inst.ref));

	/* is it already deactivated */
	if(!l->rootClass.inst.RunningStatus)
		return;

	LinkClass_Deactivation(l);

	return;
}

/*
 * returns true if the given src/type/data match the LinkCondition for this LinkClass
 */

bool
LinkClass_conditionMet(LinkClass *l, ExternalReference *src, EventType type, EventData *data)
{
	ExternalReference *link_ref;
	ObjectReference *link_src;
	char *fullname;
	OctetString absolute;
	OctetString *link_gid;

	/* easiest first */
	if(l->link_condition.event_type != type)
		return false;

	/* shouldn't happen, event type determines if there is any data or not, but just in case */
	if(l->link_condition.have_event_data && data == NULL)
		return false;

	/*
	 * if we have any data, does it match
	 * if the link condition has no event data, then it matches
	 */
	if(l->link_condition.have_event_data && data != NULL)
	{
		switch(data->choice)
		{
		case EventData_boolean:
			if(data->u.boolean != l->link_condition.event_data.u.boolean)
				return false;
			break;

		case EventData_integer:
			if(data->u.integer != l->link_condition.event_data.u.integer)
				return false;
			break;

		case EventData_octetstring:
			if(OctetString_cmp(&data->u.octetstring, &l->link_condition.event_data.u.octetstring) != 0)
				return false;
			break;

		default:
			error("Unknown EventData type: %d\n", data->choice);
			return false;
		}
	}

	/*
	 * check the source group id and object number
	 * if the group id is not specified in the link condition, it defaults to the enclosing app/scene (ie link_ref)
	 */
	link_ref = &l->rootClass.inst.ref;
	link_src = &l->link_condition.event_source;
	switch(link_src->choice)
	{
	case ObjectReference_internal_reference:
		if(OctetString_cmp(&src->group_identifier, &link_ref->group_identifier) != 0
		|| src->object_number != link_src->u.internal_reference)
			return false;
		break;

	case ObjectReference_external_reference:
		/* make sure the event src is an absolute group id (ie starts with ~//) */
		fullname = MHEGEngine_absoluteFilename(&link_src->u.external_reference.group_identifier);
		absolute.size = strlen(fullname);
		absolute.data = fullname;
		link_gid = &absolute;
		if(OctetString_cmp(&src->group_identifier, link_gid) != 0
		|| src->object_number != link_src->u.external_reference.object_number)
			return false;
		break;

	default:
		error("Unknown ObjectReference type: %d", link_src->choice);
		return false;
	}

	verbose("LinkCondition met: %s; %s", ExternalReference_name(src), EventType_name(type));

	return true;
}

