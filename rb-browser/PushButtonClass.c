/*
 * PushButtonClass.c
 */

#include "MHEGEngine.h"
#include "PushButtonClass.h"
#include "ExternalReference.h"

void
PushButtonClass_Preparation(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_Activation(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_Deactivation(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_Destruction(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_Select(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_Deselect(PushButtonClass *t)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_GetLabel(PushButtonClass *t, GetLabel *params, OctetString *caller_gid)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PushButtonClass_SetLabel(PushButtonClass *t, SetLabel *params, OctetString *caller_gid)
{
	error("PushButtonClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

