/*
 * ObjectReference.h
 */

#ifndef __OBJECTREFERENCE_H__
#define __OBJECTREFERENCE_H__

#include "ISO13522-MHEG-5.h"

int ObjectReference_cmp(ObjectReference *, OctetString *, ObjectReference *, OctetString *);

char *ObjectReference_name(ObjectReference *);

#endif	/* __OBJECTREFERENCE_H__ */
