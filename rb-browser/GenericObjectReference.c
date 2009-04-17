/*
 * GenericObjectReference.c
 */

#include "MHEGEngine.h"
#include "GenericObjectReference.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "clone.h"
#include "rtti.h"
#include "utils.h"

/*
 * if we have a Scene with a GenericObjectReference in it
 * and the GenericObjectReference points to an ObjectRefVariable in the Application
 * and the ObjectReference it contains does not have a group identifier
 * then when should we resolve the group identifier for the ObjectReference?
 * when we read the ObjectReference from the ObjectRefVariable (=> it gets the Application group id)
 * or when we use the ObjectReference in the Scene (=> it gets the Scene group id)
 *
 * we choose to only resolve ObjectReferences when we use them (so we will get the Scene group id)
 * if we did it the other way, we would need to rewrite this function to always return an ExternalReference
 */

ObjectReference *
GenericObjectReference_getObjectReference(GenericObjectReference *g, OctetString *caller_gid)
{
	ObjectReference *ref = NULL;
	VariableClass *var;

	switch(g->choice)
	{
	case GenericObjectReference_direct_reference:
		ref = &g->u.direct_reference;
		break;

	case GenericObjectReference_indirect_reference:
		/* points to an ObjectRefVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_object_reference)
			{
				error("GenericObjectReference indirect reference is not an ObjectRefVariable");
			}
			else
			{
				ref = &var->inst.Value.u.object_reference;
			}
		}
		break;

	default:
		error("Unknown GenericObjectReference type: %d", g->choice);
		break;
	}

	return ref;
}

void
GenericObjectReference_setObjectReference(GenericObjectReference *g, OctetString *caller_gid, ObjectReference *ref)
{
	VariableClass *var;

	switch(g->choice)
	{
	case GenericObjectReference_direct_reference:
		ObjectReference_copy(&g->u.direct_reference, ref);
		break;

	case GenericObjectReference_indirect_reference:
		/* points to an ObjectRefVariable */
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&g->u.indirect_reference, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti != RTTI_VariableClass
			|| var->inst.Value.choice != OriginalValue_object_reference)
			{
				error("GenericObjectReference indirect reference is not an ObjectRefVariable");
			}
			else
			{
				ObjectReference_copy(&var->inst.Value.u.object_reference, ref);
			}
		}
		break;

	default:
		error("Unknown GenericObjectReference type: %d", g->choice);
		break;
	}

	return;
}

void
GenericObjectReference_print(GenericObjectReference *g, OctetString *caller_gid)
{
	ObjectReference *ref = GenericObjectReference_getObjectReference(g, caller_gid);

	switch(ref->choice)
	{
	case ObjectReference_internal_reference:
		printf("%.*s %u", caller_gid->size, caller_gid->data, ref->u.internal_reference);
		break;

	case ObjectReference_external_reference:
		printf("%s", ExternalReference_name(&ref->u.external_reference));
		break;

	default:
		error("Unknown ObjectReference type: %d", ref->choice);
		break;
	}

	return;
}

