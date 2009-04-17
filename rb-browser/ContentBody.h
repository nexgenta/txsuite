/*
 * ContentBody.h
 */

#ifndef __CONTENTBODY_H__
#define __CONTENTBODY_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

bool ContentBody_getContent(ContentBody *, RootClass *, OctetString *);

OctetString *ContentBody_getReference(ContentBody *);

#endif	/* __CONTENTBODY_H__ */
