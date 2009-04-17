/*
 * IntegerVariableClass.c
 */

#include <ctype.h>

#include "MHEGEngine.h"
#include "IntegerVariableClass.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "ExternalReference.h"
#include "VariableClass.h"
#include "ObjectRefVariableClass.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
IntegerVariableClass_Clone(IntegerVariableClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("IntegerVariableClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, VariableClass, integer_variable);

	return;
}

/*
 * caller_gid is used to resolve the GenericInteger or GenericOctetString
 */

void
IntegerVariableClass_SetVariable(IntegerVariableClass *v, NewVariableValue *new_value, OctetString *caller_gid)
{
	int val;
	OctetString *oct;
	unsigned int i;

	verbose("IntegerVariableClass: %s; SetVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* check the type of the new value */
	if(new_value->choice != NewVariableValue_new_generic_integer
	&& new_value->choice != NewVariableValue_new_generic_octet_string)
	{
		error("SetVariable: %s; type mismatch", ExternalReference_name(&v->rootClass.inst.ref));
		return;
	}

	if(new_value->choice == NewVariableValue_new_generic_integer)
	{
		val = GenericInteger_getInteger(&new_value->u.new_generic_integer, caller_gid);
	}
	else
	{
		/* convert the OctetString to an int */
		oct = GenericOctetString_getOctetString(&new_value->u.new_generic_octet_string, caller_gid);
		val = 0;
		i = (oct->size != 0 && oct->data[0] == '-') ? 1 : 0;
		while(i < oct->size && isdigit(oct->data[i]))
		{
			val *= 10;
			val += oct->data[i] - '0';
			i ++;
		}
		if(oct->size != 0 && oct->data[0] == '-')
			val = -val;
	}

	v->inst.Value.u.integer = val;

	return;
}

void
IntegerVariableClass_TestVariable(IntegerVariableClass *v, int op, int comp)
{
	EventData result;

	verbose("IntegerVariableClass: %s; TestVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_TestVariable unexpected type: %d", v->inst.Value.choice);

	result.choice = EventData_boolean;

	switch(op)
	{
	case 1:
		/* equal */
		result.u.boolean = (v->inst.Value.u.integer == comp);
		break;

	case 2:
		/* not equal */
		result.u.boolean = (v->inst.Value.u.integer != comp);
		break;

	case 3:
		/* less than */
		result.u.boolean = (v->inst.Value.u.integer < comp);
		break;

	case 4:
		/* less than or equal to */
		result.u.boolean = (v->inst.Value.u.integer <= comp);
		break;

	case 5:
		/* greater than */
		result.u.boolean = (v->inst.Value.u.integer > comp);
		break;

	case 6:
		/* greater than or equal to */
		result.u.boolean = (v->inst.Value.u.integer >= comp);
		break;

	default:
		error("Unknown operator type: %d", op);
		return;
	}

	MHEGEngine_generateEvent(&v->rootClass.inst.ref, EventType_test_event, &result);

	return;
}

void
IntegerVariableClass_Add(IntegerVariableClass *v, Add *params, OctetString *caller_gid)
{
	int val;

	verbose("IntegerVariableClass: %s; Add", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_Add unexpected type: %d", v->inst.Value.choice);

	val = GenericInteger_getInteger(&params->value, caller_gid);

	verbose("Add: %d + %d", v->inst.Value.u.integer, val);

	v->inst.Value.u.integer += val;

	return;
}

void
IntegerVariableClass_Subtract(IntegerVariableClass *v, Subtract *params, OctetString *caller_gid)
{
	int val;

	verbose("IntegerVariableClass: %s; Subtract", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_Subtract unexpected type: %d", v->inst.Value.choice);

	val = GenericInteger_getInteger(&params->value, caller_gid);

	verbose("Subtract: %d - %d", v->inst.Value.u.integer, val);

	v->inst.Value.u.integer -= val;

	return;
}

void
IntegerVariableClass_Multiply(IntegerVariableClass *v, Multiply *params, OctetString *caller_gid)
{
	int val;

	verbose("IntegerVariableClass: %s; Multiply", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_Multiply unexpected type: %d", v->inst.Value.choice);

	val = GenericInteger_getInteger(&params->value, caller_gid);

	verbose("Multiply: %d * %d", v->inst.Value.u.integer, val);

	v->inst.Value.u.integer *= val;

	return;
}

void
IntegerVariableClass_Divide(IntegerVariableClass *v, Divide *params, OctetString *caller_gid)
{
	int val;

	verbose("IntegerVariableClass: %s; Divide", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_Divide unexpected type: %d", v->inst.Value.choice);

	val = GenericInteger_getInteger(&params->value, caller_gid);

	verbose("Divide: %d / %d", v->inst.Value.u.integer, val);

	v->inst.Value.u.integer /= val;

	return;
}

void
IntegerVariableClass_Modulo(IntegerVariableClass *v, Modulo *params, OctetString *caller_gid)
{
	int val;

	verbose("IntegerVariableClass: %s; Modulo", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_Modulo unexpected type: %d", v->inst.Value.choice);

	val = GenericInteger_getInteger(&params->value, caller_gid);

	verbose("Modulo: %d %% %d", v->inst.Value.u.integer, val);

	v->inst.Value.u.integer %= val;

	return;
}

void
IntegerVariableClass_setInteger(IntegerVariableClass *v, int val)
{
	/* assert */
	if(v->inst.Value.choice != OriginalValue_integer)
		fatal("IntegerVariableClass_setInteger unexpected type: %d", v->inst.Value.choice);

	v->inst.Value.u.integer = val;

	return;
}
