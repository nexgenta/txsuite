/*
 * NewContent.h
 */

#ifndef __NEWCONTENT_H__
#define __NEWCONTENT_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

bool NewContent_getContent(NewContent *, OctetString *, RootClass *, OctetString *);

#endif	/* __NEWCONTENT_H__ */

