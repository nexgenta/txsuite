/*
 * GenericOctetString.h
 */

#ifndef __GENERICOCTETSTRING_H__
#define __GENERICOCTETSTRING_H__

#include "ISO13522-MHEG-5.h"

OctetString *GenericOctetString_getOctetString(GenericOctetString *, OctetString *);
void GenericOctetString_setOctetString(GenericOctetString *, OctetString *, OctetString *);

void GenericOctetString_print(GenericOctetString *, OctetString *);

#endif	/* __GENERICOCTETSTRING_H__ */

