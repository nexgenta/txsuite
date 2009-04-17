/*
 * GenericBoolean.c
 */

#include "MHEGEngine.h"
#include "GenericBoolean.h"
#include "rtti.h"
#include "utils.h"

bool
GenericBoolean_getBoolean(GenericBoolean *g, OctetString *caller_gid)
{
	bool val = false;
	VariableClass *var;

	switch(g->choice)
	{
	case GenericBoolean_boolean:
		val = g->u.boolean;
		break;

	case GenericBoolean_indirect_reference:
		/* points to a BooleanVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_boolean)
			{
				error("GenericBoolean indirect reference is not a BooleanVariable");
			}
			else
			{
				val = var->inst.Value.u.boolean;
			}
		}
		break;

	default:
		error("Unknown GenericBoolean type: %d", g->choice);
		break;
	}

	return val;
}

void
GenericBoolean_setBoolean(GenericBoolean *g, OctetString *caller_gid, bool val)
{
	VariableClass *var;

	switch(g->choice)
	{
	case GenericBoolean_boolean:
		g->u.boolean = val;
		break;

	case GenericBoolean_indirect_reference:
		/* points to a BooleanVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_boolean)
			{
				error("GenericBoolean indirect reference is not a BooleanVariable");
			}
			else
			{
				var->inst.Value.u.boolean = val;
			}
		}
		break;

	default:
		error("Unknown GenericBoolean type: %d", g->choice);
		break;
	}

	return;
}

void
GenericBoolean_print(GenericBoolean *g, OctetString *caller_gid)
{
	printf("%s", GenericBoolean_getBoolean(g, caller_gid) ? "true" : "false");

	return;
}

