/*
 * GenericContentReference.h
 */

#ifndef __GENERICCONTENTREFERENCE_H__
#define __GENERICCONTENTREFERENCE_H__

#include "ISO13522-MHEG-5.h"

ContentReference *GenericContentReference_getContentReference(GenericContentReference *, OctetString *);
void GenericContentReference_setContentReference(GenericContentReference *, OctetString *, ContentReference *);

void GenericContentReference_print(GenericContentReference *, OctetString *);

#endif	/* __GENERICCONTENTREFERENCE_H__ */

