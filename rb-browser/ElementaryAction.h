/*
 * ElementaryAction.h
 */

#ifndef __ELEMENTARYACTION_H__
#define __ELEMENTARYACTION_H__

#include "ISO13522-MHEG-5.h"

void ElementaryAction_execute(ElementaryAction *, OctetString *);
char *ElementaryAction_name(ElementaryAction *);

#endif	/* __ELEMENTARYACTION_H__ */

