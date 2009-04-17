/*
 * ObjectReference.c
 */

#include <stdio.h>
#include <limits.h>

#include "MHEGEngine.h"
#include "ObjectReference.h"
#include "ExternalReference.h"

/*
 * returns 0 if the ObjectReferences refer to the same object
 * if r1 is an internal reference, r1_gid is used as its group id
 * similarly for r2
 */

int
ObjectReference_cmp(ObjectReference *r1, OctetString *r1_gid, ObjectReference *r2, OctetString *r2_gid)
{
	unsigned int num1, num2;
	OctetString *gid1, *gid2;
	int cmp;

	/* complicated by the fact that they can be either internal or external references */
	switch(r1->choice)
	{
	case ObjectReference_internal_reference:
		num1 = r1->u.internal_reference;
		gid1 = r1_gid;
		break;

	case ObjectReference_external_reference:
		num1 = r1->u.external_reference.object_number;
		gid1 = &r1->u.external_reference.group_identifier;
		break;

	default:
		error("Unknown ObjectReference type: %d", r1->choice);
		return -1;
	}

	switch(r2->choice)
	{
	case ObjectReference_internal_reference:
		num2 = r2->u.internal_reference;
		gid2 = r2_gid;
		break;

	case ObjectReference_external_reference:
		num2 = r2->u.external_reference.object_number;
		gid2 = &r2->u.external_reference.group_identifier;
		break;

	default:
		error("Unknown ObjectReference type: %d", r2->choice);
		return -1;
	}

	if(num1 == num2)
		cmp = OctetString_cmp(gid1, gid2);
	else
		cmp = num1 - num2;

	verbose("ObjectReference_cmp(%.*s %d, %.*s %d) = %d", gid1->size, gid1->data, num1, gid2->size, gid2->data, num2, cmp);

	return cmp;
}

/*
 * returns a static string that will be overwritten by the next call to this routine
 */

static char _name[PATH_MAX + 16];

char *
ObjectReference_name(ObjectReference *r)
{
	switch(r->choice)
	{
	case ObjectReference_internal_reference:
		snprintf(_name, sizeof(_name), "%u", r->u.internal_reference);
		return _name;

	case ObjectReference_external_reference:
		return ExternalReference_name(&r->u.external_reference);

	default:
		error("Unknown ObjectReference type: %d", r->choice);
		snprintf(_name, sizeof(_name), "Invalid ObjectReference");
		return _name;
	}
}
