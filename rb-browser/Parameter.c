/*
 * Parameter.c
 */

#include "Parameter.h"
#include "GenericBoolean.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "GenericObjectReference.h"
#include "GenericContentReference.h"

void
Parameter_print(Parameter *p, OctetString *caller_gid)
{
	switch(p->choice)
	{
	case Parameter_new_generic_boolean:
		GenericBoolean_print(&p->u.new_generic_boolean, caller_gid);
		break;

	case Parameter_new_generic_integer:
		GenericInteger_print(&p->u.new_generic_integer, caller_gid);
		break;

	case Parameter_new_generic_octetstring:
		GenericOctetString_print(&p->u.new_generic_octetstring, caller_gid);
		break;

	case Parameter_new_generic_object_reference:
		GenericObjectReference_print(&p->u.new_generic_object_reference, caller_gid);
		break;

	case Parameter_new_generic_content_reference:
		GenericContentReference_print(&p->u.new_generic_content_reference, caller_gid);
		break;

	default:
		error("Unknown Parameter type: %d", p->choice);
		break;
	}

	return;
}

