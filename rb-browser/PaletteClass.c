/*
 * PaletteClass.c
 */

#include "MHEGEngine.h"
#include "PaletteClass.h"
#include "ExternalReference.h"

void
PaletteClass_Preparation(PaletteClass *t)
{
	error("PaletteClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PaletteClass_Activation(PaletteClass *t)
{
	error("PaletteClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PaletteClass_Deactivation(PaletteClass *t)
{
	error("PaletteClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
PaletteClass_Destruction(PaletteClass *t)
{
	error("PaletteClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}


