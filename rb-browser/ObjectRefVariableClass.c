/*
 * ObjectRefVariableClass.c
 */

#include "MHEGEngine.h"
#include "ObjectRefVariableClass.h"
#include "GenericObjectReference.h"
#include "ObjectReference.h"
#include "ExternalReference.h"
#include "VariableClass.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
ObjectRefVariableClass_Clone(ObjectRefVariableClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("ObjectRefVariableClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, VariableClass, object_ref_variable);

	return;
}

/*
 * caller_gid is used to resolve the GenericObjectReference
 */

void
ObjectRefVariableClass_SetVariable(ObjectRefVariableClass *v, NewVariableValue *new_value, OctetString *caller_gid)
{
	ObjectReference *val;

	verbose("ObjectRefVariableClass: %s; SetVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* check the type of the new value */
	if(new_value->choice != NewVariableValue_new_generic_object_reference)
	{
		error("SetVariable: %s; type mismatch", ExternalReference_name(&v->rootClass.inst.ref));
		return;
	}

	val = GenericObjectReference_getObjectReference(&new_value->u.new_generic_object_reference, caller_gid);

	ObjectReference_copy(&v->inst.Value.u.object_reference, val);

	return;
}

/*
 * if comp does not have a group id, caller_gid is used
 */

void
ObjectRefVariableClass_TestVariable(ObjectRefVariableClass *v, int op, ObjectReference *comp, OctetString *caller_gid)
{
	ObjectReference *ref;
	OctetString *ref_gid;
	EventData result;

	verbose("ObjectRefVariableClass: %s; TestVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_object_reference)
		fatal("ObjectRefVariableClass_TestVariable unexpected type: %d", v->inst.Value.choice);

	/* the ObjectReference in this variable */
	ref = &v->inst.Value.u.object_reference;
	/* its group id (if it is an internal reference) */
	ref_gid = &v->rootClass.inst.ref.group_identifier;

	result.choice = EventData_boolean;

	switch(op)
	{
	case 1:
		/* equal */
		result.u.boolean = (ObjectReference_cmp(ref, ref_gid, comp, caller_gid) == 0);
		break;

	case 2:
		/* not equal */
		result.u.boolean = (ObjectReference_cmp(ref, ref_gid, comp, caller_gid) != 0);
		break;

	default:
		error("Unknown operator type: %d", op);
		return;
	}

	MHEGEngine_generateEvent(&v->rootClass.inst.ref, EventType_test_event, &result);

	return;
}

void
ObjectRefVariableClass_setObjectReference(ObjectRefVariableClass *v, ObjectReference *val)
{
	/* assert */
	if(v->inst.Value.choice != OriginalValue_object_reference)
		fatal("ObjectRefVariableClass_setObjectReference unexpected type: %d", v->inst.Value.choice);

	ObjectReference_copy(&v->inst.Value.u.object_reference, val);

	return;
}
