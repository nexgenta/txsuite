/*
 * GenericOctetString.c
 */

#include "MHEGEngine.h"
#include "GenericOctetString.h"
#include "rtti.h"
#include "utils.h"

OctetString *
GenericOctetString_getOctetString(GenericOctetString *g, OctetString *caller_gid)
{
	OctetString *val = NULL;
	VariableClass *var;

	switch(g->choice)
	{
	case GenericOctetString_octetstring:
		val = &g->u.octetstring;
		break;

	case GenericOctetString_indirect_reference:
		/* points to an OctetStringVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_octetstring)
			{
				error("GenericOctetString indirect reference is not an OctetStringVariable");
			}
			else
			{
				val = &var->inst.Value.u.octetstring;
			}
		}
		break;

	default:
		error("Unknown GenericOctetString type: %d", g->choice);
		break;
	}

	return val;
}

void
GenericOctetString_setOctetString(GenericOctetString *g, OctetString *caller_gid, OctetString *val)
{
	VariableClass *var;

	switch(g->choice)
	{
	case GenericOctetString_octetstring:
		OctetString_copy(&g->u.octetstring, val);
		break;

	case GenericOctetString_indirect_reference:
		/* points to an OctetStringVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_octetstring)
			{
				error("GenericOctetString indirect reference is not an OctetStringVariable");
			}
			else
			{
				OctetString_copy(&var->inst.Value.u.octetstring, val);
			}
		}
		break;

	default:
		error("Unknown GenericOctetString type: %d", g->choice);
		break;
	}

	return;
}

void
GenericOctetString_print(GenericOctetString *g, OctetString *caller_gid)
{
	OctetString *oct = GenericOctetString_getOctetString(g, caller_gid);

	printf("%.*s", oct->size, oct->data);

	return;
}

