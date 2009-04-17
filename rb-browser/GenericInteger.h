/*
 * GenericInteger.h
 */

#ifndef __GENERICINTEGER_H__
#define __GENERICINTEGER_H__

#include "ISO13522-MHEG-5.h"

int GenericInteger_getInteger(GenericInteger *, OctetString *);
void GenericInteger_setInteger(GenericInteger *, OctetString *, int);

void GenericInteger_print(GenericInteger *, OctetString *);

#endif	/* __GENERICINTEGER_H__ */

