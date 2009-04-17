/*
 * OctetStringVariableClass.c
 */

#include <stdio.h>
#include <string.h>

#include "MHEGEngine.h"
#include "OctetStringVariableClass.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "ExternalReference.h"
#include "VariableClass.h"
#include "ObjectRefVariableClass.h"
#include "cloneobj.h"
#include "clone.h"
#include "rtti.h"

void
OctetStringVariableClass_Clone(OctetStringVariableClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("OctetStringVariableClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

	CLONE_OBJECT(t, VariableClass, octet_string_variable);

	return;
}

/*
 * caller_gid is used to resolve the GenericInteger or GenericOctetString
 */

void
OctetStringVariableClass_SetVariable(OctetStringVariableClass *v, NewVariableValue *new_value, OctetString *caller_gid)
{
	OctetString *oct;
	int val;
	char val_str[16];

	verbose("OctetStringVariableClass: %s; SetVariable", ExternalReference_name(&v->rootClass.inst.ref));

	/* check the type of the new value */
	if(new_value->choice != NewVariableValue_new_generic_integer
	&& new_value->choice != NewVariableValue_new_generic_octet_string)
	{
		error("SetVariable: %s; type mismatch", ExternalReference_name(&v->rootClass.inst.ref));
		return;
	}

	if(new_value->choice == NewVariableValue_new_generic_octet_string)
	{
		oct = GenericOctetString_getOctetString(&new_value->u.new_generic_octet_string, caller_gid);
		OctetString_copy(&v->inst.Value.u.octetstring, oct);
	}
	else
	{
		/* convert the int to an OctetString */
		val = GenericInteger_getInteger(&new_value->u.new_generic_integer, caller_gid);
		snprintf(val_str, sizeof(val_str), "%d", val);
		/* free any existing data */
		safe_free(v->inst.Value.u.octetstring.data);
		v->inst.Value.u.octetstring.size = strlen(val_str);
		v->inst.Value.u.octetstring.data = safe_malloc(v->inst.Value.u.octetstring.size);
		strncpy(v->inst.Value.u.octetstring.data, val_str, v->inst.Value.u.octetstring.size);
	}

	return;
}

void
OctetStringVariableClass_TestVariable(OctetStringVariableClass *v, int op, OctetString *comp)
{
	EventData result;

	/* assert */
	if(v->inst.Value.choice != OriginalValue_octetstring)
		fatal("OctetStringVariableClass_TestVariable unexpected type: %d", v->inst.Value.choice);

	result.choice = EventData_boolean;

	switch(op)
	{
	case 1:
		/* equal */
		result.u.boolean = (OctetString_cmp(&v->inst.Value.u.octetstring, comp) == 0);
		break;

	case 2:
		/* not equal */
		result.u.boolean = (OctetString_cmp(&v->inst.Value.u.octetstring, comp) != 0);
		break;

	default:
		error("Unknown operator type: %d", op);
		return;
	}

	MHEGEngine_generateEvent(&v->rootClass.inst.ref, EventType_test_event, &result);

	return;
}

void
OctetStringVariableClass_Append(OctetStringVariableClass *v, Append *params, OctetString *caller_gid)
{
	OctetString *target;
	OctetString *append;

	verbose("OctetStringVariableClass: %s; Append", ExternalReference_name(&v->rootClass.inst.ref));

	/* assert */
	if(v->inst.Value.choice != OriginalValue_octetstring)
		fatal("OctetStringVariableClass_Append unexpected type: %d", v->inst.Value.choice);

	target = &v->inst.Value.u.octetstring;
	append = GenericOctetString_getOctetString(&params->append_value, caller_gid);

	verbose("Append: '%.*s' + '%.*s'", target->size, target->data, append->size, append->data);

	target->data = safe_realloc(target->data, target->size + append->size);
	memcpy(&target->data[target->size], append->data, append->size);
	target->size += append->size;

	return;
}

/*
 * takes a copy of the source data
 */

void
OctetStringVariableClass_setOctetString(OctetStringVariableClass *v, OctetString *val)
{
	/* assert */
	if(v->inst.Value.choice != OriginalValue_octetstring)
		fatal("OctetStringVariableClass_setOctetString unexpected type: %d", v->inst.Value.choice);

	OctetString_copy(&v->inst.Value.u.octetstring, val);

	return;
}

