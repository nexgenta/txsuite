/*
 * GenericInteger.c
 */

#include "MHEGEngine.h"
#include "GenericInteger.h"
#include "rtti.h"
#include "utils.h"

int
GenericInteger_getInteger(GenericInteger *g, OctetString *caller_gid)
{
	int val = 0;
	VariableClass *var;

	switch(g->choice)
	{
	case GenericInteger_integer:
		val = g->u.integer;
		break;

	case GenericInteger_indirect_reference:
		/* points to an IntegerVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_integer)
			{
				error("GenericInteger indirect reference is not an IntegerVariable");
			}
			else
			{
				val = var->inst.Value.u.integer;
			}
		}
		break;

	default:
		error("Unknown GenericInteger type: %d", g->choice);
		break;
	}

	return val;
}

void
GenericInteger_setInteger(GenericInteger *g, OctetString *caller_gid, int val)
{
	VariableClass *var;

	switch(g->choice)
	{
	case GenericInteger_integer:
		g->u.integer = val;
		break;

	case GenericInteger_indirect_reference:
		/* points to an IntegerVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_integer)
			{
				error("GenericInteger indirect reference is not an IntegerVariable");
			}
			else
			{
				var->inst.Value.u.integer = val;
			}
		}
		break;

	default:
		error("Unknown GenericInteger type: %d", g->choice);
		break;
	}

	return;
}

void
GenericInteger_print(GenericInteger *g, OctetString *caller_gid)
{
	printf("%d", GenericInteger_getInteger(g, caller_gid));

	return;
}

