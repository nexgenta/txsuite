/*
 * NewContent.c
 */

#include "MHEGEngine.h"
#include "NewContent.h"
#include "GenericOctetString.h"
#include "GenericContentReference.h"

/*
 * caller_gid is used to resolve Generic references in the NewContent
 * the content may need to be loaded from a file
 * if it can't load the content it calls MHEGEngine_addMissingContent()
 * with obj as the object that needs the content
 * returns false if it can't load the content
 */

bool
NewContent_getContent(NewContent *n, OctetString *caller_gid, RootClass *obj, OctetString *out)
{
	bool rc = true;
	OctetString *content;
	ContentReference *ref;

	switch(n->choice)
	{
	case NewContent_new_included_content:
		content = GenericOctetString_getOctetString(&n->u.new_included_content, caller_gid);
		out->size = content->size;
		if(out->size > 0)
		{
			out->data = safe_malloc(out->size);
			memcpy(out->data, content->data, out->size);
		}
		else
		{
			out->data = NULL;
		}
		break;

	case NewContent_new_referenced_content:
		ref = GenericContentReference_getContentReference(&n->u.new_referenced_content.generic_content_reference, caller_gid);
		if(!(rc = MHEGEngine_loadFile(ref, out)))
			MHEGEngine_addMissingContent(obj, ref);
		break;

	default:
		rc = false;
		error("Unknown NewContent type: %d", n->choice);
		break;
	}

	return rc;
}

