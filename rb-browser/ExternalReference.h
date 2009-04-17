/*
 * ExternalReference.h
 */

#ifndef __EXTERNALREFERENCE_H__
#define __EXTERNALREFERENCE_H__

#include "ISO13522-MHEG-5.h"

int ExternalReference_cmp(ExternalReference *, ExternalReference *);

char *ExternalReference_name(ExternalReference *);

#endif	/* __EXTERNALREFERENCE_H__ */

