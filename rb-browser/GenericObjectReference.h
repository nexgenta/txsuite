/*
 * GenericObjectReference.h
 */

#ifndef __GENERICOBJECTREFERENCE_H__
#define __GENERICOBJECTREFERENCE_H__

#include "ISO13522-MHEG-5.h"

ObjectReference *GenericObjectReference_getObjectReference(GenericObjectReference *, OctetString *);
void GenericObjectReference_setObjectReference(GenericObjectReference *, OctetString *, ObjectReference *);

void GenericObjectReference_print(GenericObjectReference *, OctetString *);

#endif	/* __GENERICOBJECTREFERENCE_H__ */

