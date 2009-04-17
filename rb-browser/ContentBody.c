/*
 * ContentBody.c
 */

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"
#include "MHEGEngine.h"
#include "utils.h"

/*
 * if it can't load the content it calls MHEGEngine_addMissingContent()
 * with obj as the object that needs the content
 * returns false if it can't load the content
 */

bool
ContentBody_getContent(ContentBody *c, RootClass *obj, OctetString *out)
{
	bool rc = true;

	switch(c->choice)
	{
	case ContentBody_included_content:
		out->size = c->u.included_content.size;
		if(out->size > 0)
		{
			out->data = safe_malloc(out->size);
			memcpy(out->data, c->u.included_content.data, out->size);
		}
		else
		{
			out->data = NULL;
		}
		break;

	case ContentBody_referenced_content:
		if(!(rc = MHEGEngine_loadFile(&c->u.referenced_content.content_reference, out)))
			MHEGEngine_addMissingContent(obj, &c->u.referenced_content.content_reference);
		break;

	default:
		rc = false;
		error("Unknown ContentBody type: %d", c->choice);
		break;
	}

	return rc;
}

/*
 * returns a ptr to the Referenced Content OctetString
 * retuns NULL if the content is included
 */

OctetString *
ContentBody_getReference(ContentBody *c)
{
	OctetString *rc = NULL;

	switch(c->choice)
	{
	case ContentBody_included_content:
		break;

	case ContentBody_referenced_content:
		rc = &c->u.referenced_content.content_reference;
		break;

	default:
		error("Unknown ContentBody type: %d", c->choice);
		break;
	}

	return rc;
}

