/*
 * BooleanVariableClass.c
 */

#include "MHEGEngine.h"
#include "BooleanVariableClass.h"
#include "GenericBoolean.h"
#include "ExternalReference.h"
#include "VariableClass.h"
#include "ObjectRefVariableClass.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
BooleanVariableClass_Clone(BooleanVariableClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("BooleanVariableClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, VariableClass, boolean_variable);

	return;
}

/*
 * caller_gid is used to resolve the GenericBoolean
 */

void
BooleanVariableClass_SetVariable(BooleanVariableClass *v, NewVariableValue *new_value, OctetString *caller_gid)
{
	bool val;

	verbose("BooleanVariableClass: %s; SetVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* check the type of the new value */
	if(new_value->choice != NewVariableValue_new_generic_boolean)
	{
		error("SetVariable: %s; type mismatch", ExternalReference_name(&v->rootClass.inst.ref));
		return;
	}

	val = GenericBoolean_getBoolean(&new_value->u.new_generic_boolean, caller_gid);

	v->inst.Value.u.boolean = val;

	return;
}

void
BooleanVariableClass_TestVariable(BooleanVariableClass *v, int op, bool comp)
{
	EventData result;

	verbose("BooleanVariableClass: %s; TestVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_boolean)
		fatal("BooleanVariableClass_TestVariable unexpected type: %d", v->inst.Value.choice);

	result.choice = EventData_boolean;

	switch(op)
	{
	case 1:
		/* equal */
		result.u.boolean = (v->inst.Value.u.boolean == comp);
		break;

	case 2:
		/* not equal */
		result.u.boolean = (v->inst.Value.u.boolean != comp);
		break;

	default:
		error("Unknown operator type: %d", op);
		return;
	}

	MHEGEngine_generateEvent(&v->rootClass.inst.ref, EventType_test_event, &result);

	return;
}

void
BooleanVariableClass_setBoolean(BooleanVariableClass *v, bool val)
{
	/* assert */
	if(v->inst.Value.choice != OriginalValue_boolean)
		fatal("BooleanVariableClass_setBoolean unexpected type: %d", v->inst.Value.choice);

	v->inst.Value.u.boolean = val;

	return;
}

