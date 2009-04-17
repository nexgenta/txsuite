/*
 * GenericContentReference.c
 */

#include "MHEGEngine.h"
#include "GenericContentReference.h"
#include "rtti.h"
#include "utils.h"

ContentReference *
GenericContentReference_getContentReference(GenericContentReference *g, OctetString *caller_gid)
{
	ContentReference *ref = NULL;
	VariableClass *var;

	switch(g->choice)
	{
	case GenericContentReference_content_reference:
		ref = &g->u.content_reference;
		break;

	case GenericContentReference_indirect_reference:
		/* points to a ContentRefVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_content_reference)
			{
				error("GenericContentReference indirect reference is not an ContentRefVariable");
			}
			else
			{
				ref = &var->inst.Value.u.content_reference;
			}
		}
		break;

	default:
		error("Unknown GenericContentReference type: %d", g->choice);
		break;
	}

	return ref;
}

void
GenericContentReference_setContentReference(GenericContentReference *g, OctetString *caller_gid, ContentReference *ref)
{
	VariableClass *var;

	switch(g->choice)
	{
	case GenericContentReference_content_reference:
		/* a ContentReference is just an OctetString */
		OctetString_copy(&g->u.content_reference, ref);
		break;

	case GenericContentReference_indirect_reference:
		/* points to an ContentRefVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_content_reference)
			{
				error("GenericContentReference indirect reference is not an ContentRefVariable");
			}
			else
			{
				/* a ContentReference is just an OctetString */
				OctetString_copy(&var->inst.Value.u.content_reference, ref);
			}
		}
		break;

	default:
		error("Unknown GenericContentReference type: %d", g->choice);
		break;
	}

	return;
}

void
GenericContentReference_print(GenericContentReference *g, OctetString *caller_gid)
{
	ContentReference *ref = GenericContentReference_getContentReference(g, caller_gid);

	printf("%.*s", ref->size, ref->data);

	return;
}

