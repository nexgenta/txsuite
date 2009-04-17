/*
 * RootClass.c
 */

#include <stdbool.h>

#include "MHEGEngine.h"
#include "RootClass.h"
#include "GroupItem.h"
#include "BitmapClass.h"
#include "ExternalReference.h"
#include "TextClass.h"
#include "EntryFieldClass.h"
#include "HyperTextClass.h"
#include "VariableClass.h"
#include "BooleanVariableClass.h"
#include "clone.h"
#include "rtti.h"

/*
 * any existing data is dst will be lost
 * (can't auto-gen this as add_instance_vars changes RootClass from the XSD definition)
 */

void
RootClass_dup(RootClass *dst, RootClass *src)
{
	ObjectReference_dup(&dst->ObjectReference, &src->ObjectReference);

	return;
}

/*
 * sets the RootClassInstanceVars
 * adds the object to the engine with MHEGEngine_addObjectReference
 */

void
RootClass_registerObject(RootClass *t)
{
	bzero(&t->inst, sizeof(RootClassInstanceVars));

	t->inst.AvailabilityStatus = false;
	t->inst.RunningStatus = false;

	/* set the RTTI to an invalid value, the der_decode_TYPE() routines set it to the correct value */
	t->inst.rtti = RTTI_Unknown;

	/*
	 * we always store a fully resolved object reference (ie including the group id)
	 * this means we don't need a ptr to the enclosing app/scene in every object
	 */
	MHEGEngine_resolveDERObjectReference(&t->ObjectReference, &t->inst.ref);

	/* remember it */
	MHEGEngine_addObjectReference(t);

	/* this will be set if a subclass calls MHEGEngine_needContent() */
	t->inst.need_content = false;

	return;
}

/*
 * registers the object contained in the given GroupItem
 * sets the RootClassInstanceVars to default values
 * sets the object's group ID and object number
 * sets the object's RTTI type
 * adds the object to the GroupItems list of the given GroupClass
 * adds the object to the engine with MHEGEngine_addObjectReference
 */

void
RootClass_registerClonedObject(LIST_TYPE(GroupItem) *gi, RootClass *group, unsigned int object_num, unsigned int rtti)
{
	RootClass *r;
	LIST_OF(GroupItem) **items = NULL;

	/* find the RootClass of the GroupItem */
	if((r = GroupItem_rootClass(&gi->item)) == NULL)
		fatal("Trying to register an invalid cloned object");

	bzero(&r->inst, sizeof(RootClassInstanceVars));

	r->inst.AvailabilityStatus = false;
	r->inst.RunningStatus = false;

	r->inst.rtti = rtti;

	/*
	 * we always store a fully resolved object reference (ie including the group id)
	 * this means we don't need a ptr to the enclosing app/scene in every object
	 */
	OctetString_dup(&r->inst.ref.group_identifier, &group->inst.ref.group_identifier);
	r->inst.ref.object_number = object_num;

	/* remember the object */
	MHEGEngine_addObjectReference(r);

	/* this will be set if a subclass calls MHEGEngine_needContent() */
	r->inst.need_content = false;

	/* add the object to its parent group */
	if(group->inst.rtti == RTTI_ApplicationClass)
		items = &(((ApplicationClass *) group)->items);
	else if(group->inst.rtti == RTTI_SceneClass)
		items = &(((SceneClass *) group)->items);
	else
		fatal("Trying to add a clone to an invalid object: %s", ExternalReference_name(&group->inst.ref));

	LIST_APPEND(items, gi);

	return;
}

void
RootClass_unregisterObject(RootClass *t)
{
	/* assert */
	if(t->inst.AvailabilityStatus)
		fatal("Trying to unregister an active object: %s", ExternalReference_name(&t->inst.ref));

	/* tell the engine we are going */
	MHEGEngine_removeObjectReference(t);

	free_ExternalReference(&t->inst.ref);

	return;
}

/*
 * returns false if we have already prepared it
 */

bool
RootClass_Preparation(RootClass *r)
{
//	verbose("RootClass: %s; Preparation", ExternalReference_name(&r->inst.ref));

	/* have we already done it */
	if(r->inst.AvailabilityStatus)
		return false;

	/*
	 * the subclass will need to:
	 * - retrieve the object from an entity outside the engine
	 * - set each internal attribute of the object to its initial value
	 */

	r->inst.AvailabilityStatus = true;

	/* generate IsAvailable event */
	MHEGEngine_generateEvent(&r->inst.ref, EventType_is_available, NULL);

	/* if the content is ready, generate an asynchronous ContentAvailable event */
	if(!r->inst.need_content)
		MHEGEngine_generateAsyncEvent(&r->inst.ref, EventType_content_available, NULL);

	return true;
}

/*
 * returns false if we have already activated it
 */

bool
RootClass_Activation(RootClass *r)
{
//	verbose("RootClass: %s; Activation", ExternalReference_name(&r->inst.ref));

	/* is it already activated */
	if(r->inst.RunningStatus)
		return false;

	/* has it been prepared yet */
	if(!r->inst.AvailabilityStatus)
	{
		/* generates an IsAvailable event */
		RootClass_Preparation(r);
	}

	return true;
}

/*
 * returns false if we have already deactivated it
 */

bool
RootClass_Deactivation(RootClass *r)
{
//	verbose("RootClass: %s; Deactivation", ExternalReference_name(&r->inst.ref));

	/* is it already deactivated */
	if(!r->inst.RunningStatus)
		return false;

	r->inst.RunningStatus = false;

	/* generate an IsStopped event */
	MHEGEngine_generateEvent(&r->inst.ref, EventType_is_stopped, NULL);

	return true;
}

/*
 * returns false if we have already destroyed it
 */

bool
RootClass_Destruction(RootClass *r)
{
//	verbose("RootClass: %s; Destruction", ExternalReference_name(&r->inst.ref));

	/* is it already destroyed */
	if(!r->inst.AvailabilityStatus)
		return false;

	/* Deactivate it if it is running */
	if(r->inst.RunningStatus)
	{
		/* generates an IsStopped event */
		RootClass_Deactivation(r);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	/* generate an IsDeleted event */
	r->inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&r->inst.ref, EventType_is_deleted, NULL);

	return true;
}

/*
 * value is stored in status BooleanVariable
 * caller_gid is used to resolve the status ObjectReference
 */

void
RootClass_GetAvailabilityStatus(RootClass *r, ObjectReference *status, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("RootClass: %s; GetAvailabilityStatus", ExternalReference_name(&r->inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(status, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_boolean)
	{
		error("RootClass: GetAvailabilityStatus: type mismatch");
		return;
	}

	BooleanVariableClass_setBoolean(var, r->inst.AvailabilityStatus);

	return;
}

/*
 * value is stored in status BooleanVariable
 * caller_gid is used to resolve the status ObjectReference
 */

void
RootClass_GetRunningStatus(RootClass *r, ObjectReference *status, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("RootClass: %s; GetRunningStatus", ExternalReference_name(&r->inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(status, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_boolean)
	{
		error("RootClass: GetRunningStatus: type mismatch");
		return;
	}

	BooleanVariableClass_setBoolean(var, r->inst.RunningStatus);

	return;
}

/*
 * calls the contentAvailable method for the subtype
 * generates a ContentAvailable event
 */

void
RootClass_contentAvailable(RootClass *r, OctetString *file)
{
	/* clear the need_content flag */
	r->inst.need_content = false;

	switch(r->inst.rtti)
	{
/**********************************************************************/
/* every class that calls ContentBody_getContent() will need an entry here */
/**********************************************************************/
	case RTTI_BitmapClass:
		BitmapClass_contentAvailable((BitmapClass *) r, file);
		break;

	case RTTI_TextClass:
		TextClass_contentAvailable((TextClass *) r, file);
		break;

	case RTTI_EntryFieldClass:
		EntryFieldClass_contentAvailable((EntryFieldClass *) r, file);
		break;

	case RTTI_HyperTextClass:
		HyperTextClass_contentAvailable((HyperTextClass *) r, file);
		break;

	default:
		fatal("Unexpected ContentBody in type %d", r->inst.rtti);
		break;
	}

	MHEGEngine_generateAsyncEvent(&r->inst.ref, EventType_content_available, NULL);

	return;
}

