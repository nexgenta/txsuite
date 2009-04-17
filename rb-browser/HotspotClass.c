/*
 * HotspotClass.c
 */

#include "MHEGEngine.h"
#include "HotspotClass.h"
#include "ExternalReference.h"

void
HotspotClass_Preparation(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HotspotClass_Activation(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HotspotClass_Deactivation(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HotspotClass_Destruction(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HotspotClass_Select(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
HotspotClass_Deselect(HotspotClass *t)
{
	error("HotspotClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

