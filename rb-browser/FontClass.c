/*
 * FontClass.c
 */

#include "MHEGEngine.h"
#include "FontClass.h"
#include "ExternalReference.h"

void
FontClass_Preparation(FontClass *t)
{
	error("FontClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
FontClass_Activation(FontClass *t)
{
	error("FontClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
FontClass_Deactivation(FontClass *t)
{
	error("FontClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
FontClass_Destruction(FontClass *t)
{
	error("FontClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}


