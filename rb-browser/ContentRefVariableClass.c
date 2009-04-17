/*
 * ContentRefVariableClass.c
 */

#include "MHEGEngine.h"
#include "ContentRefVariableClass.h"
#include "GenericContentReference.h"
#include "ExternalReference.h"
#include "VariableClass.h"
#include "ObjectRefVariableClass.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
ContentRefVariableClass_Clone(ContentRefVariableClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("ContentRefVariableClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, VariableClass, content_ref_variable);

	return;
}

/*
 * caller_gid is used to resolve the GenericContentReference
 */

void
ContentRefVariableClass_SetVariable(ContentRefVariableClass *v, NewVariableValue *new_value, OctetString *caller_gid)
{
	ContentReference *val;

	verbose("ContentRefVariableClass: %s; SetVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* check the type of the new value */
	if(new_value->choice != NewVariableValue_new_generic_content_reference)
	{
		error("SetVariable: %s; type mismatch", ExternalReference_name(&v->rootClass.inst.ref));
		return;
	}

	val = GenericContentReference_getContentReference(&new_value->u.new_generic_content_reference, caller_gid);

	OctetString_copy(&v->inst.Value.u.content_reference, val);

	return;
}

void
ContentRefVariableClass_TestVariable(ContentRefVariableClass *v, int op, ContentReference *comp)
{
	EventData result;

	verbose("ContentRefVariableClass: %s; TestVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_content_reference)
		fatal("ContentRefVariableClass_TestVariable unexpected type: %d", v->inst.Value.choice);

	result.choice = EventData_boolean;

	switch(op)
	{
	case 1:
		/* equal */
		result.u.boolean = (OctetString_cmp(&v->inst.Value.u.content_reference, comp) == 0);
		break;

	case 2:
		/* not equal */
		result.u.boolean = (OctetString_cmp(&v->inst.Value.u.content_reference, comp) != 0);
		break;

	default:
		error("Unknown operator type: %d", op);
		return;
	}

	MHEGEngine_generateEvent(&v->rootClass.inst.ref, EventType_test_event, &result);

	return;
}

