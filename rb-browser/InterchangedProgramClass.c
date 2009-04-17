/*
 * InterchangedProgramClass.c
 */

#include "MHEGEngine.h"
#include "InterchangedProgramClass.h"
#include "ExternalReference.h"

void
InterchangedProgramClass_Preparation(InterchangedProgramClass *t)
{
	error("InterchangedProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
InterchangedProgramClass_Activation(InterchangedProgramClass *t)
{
	error("InterchangedProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
InterchangedProgramClass_Deactivation(InterchangedProgramClass *t)
{
	error("InterchangedProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
InterchangedProgramClass_Destruction(InterchangedProgramClass *t)
{
	error("InterchangedProgramClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

