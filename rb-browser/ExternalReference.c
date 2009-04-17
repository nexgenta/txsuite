/*
 * ExternalReference.c
 */

#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#include "ExternalReference.h"

/*
 * returns 0 if both ExternalReferences refer to the same object
 */

int
ExternalReference_cmp(ExternalReference *e1, ExternalReference *e2)
{
	/* fastest first */
	if(e1->object_number == e2->object_number)
		return OctetString_cmp(&e1->group_identifier, &e2->group_identifier);
	else
		return (e1->object_number - e2->object_number);
}

/*
 * returns a static string that will be overwritten by the next call to this routine
 */

static char _name[PATH_MAX + 16];

char *
ExternalReference_name(ExternalReference *e)
{
	snprintf(_name, sizeof(_name), "%.*s %u", e->group_identifier.size, e->group_identifier.data, e->object_number);

	return _name;
}

