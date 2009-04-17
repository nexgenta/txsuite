/*
 * CursorShapeClass.c
 */

#include "MHEGEngine.h"
#include "CursorShapeClass.h"
#include "ExternalReference.h"

void
CursorShapeClass_Preparation(CursorShapeClass *t)
{
	error("CursorShapeClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
CursorShapeClass_Activation(CursorShapeClass *t)
{
	error("CursorShapeClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
CursorShapeClass_Deactivation(CursorShapeClass *t)
{
	error("CursorShapeClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
CursorShapeClass_Destruction(CursorShapeClass *t)
{
	error("CursorShapeClass: %s; not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}


