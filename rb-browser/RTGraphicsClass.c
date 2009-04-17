/*
 * RTGraphicsClass.c
 */

#include "MHEGEngine.h"
#include "RTGraphicsClass.h"
#include "ExternalReference.h"

void
RTGraphicsClass_Preparation(RTGraphicsClass *t)
{
	error("RTGraphicsClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RTGraphicsClass_Activation(RTGraphicsClass *t)
{
	error("RTGraphicsClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RTGraphicsClass_Deactivation(RTGraphicsClass *t)
{
	error("RTGraphicsClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
RTGraphicsClass_Destruction(RTGraphicsClass *t)
{
	error("RTGraphicsClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

