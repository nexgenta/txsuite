/*
 * RemoteProgramClass.c
 */

#include "MHEGEngine.h"
#include "RemoteProgramClass.h"
#include "ExternalReference.h"

void
RemoteProgramClass_Preparation(RemoteProgramClass *t)
{
	error("RemoteProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RemoteProgramClass_Activation(RemoteProgramClass *t)
{
	error("RemoteProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RemoteProgramClass_Deactivation(RemoteProgramClass *t)
{
	error("RemoteProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RemoteProgramClass_Destruction(RemoteProgramClass *t)
{
	error("RemoteProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

