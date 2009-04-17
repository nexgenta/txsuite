/*
 * GenericBoolean.h
 */

#ifndef __GENERICBOOLEAN_H__
#define __GENERICBOOLEAN_H__

#include "ISO13522-MHEG-5.h"

bool GenericBoolean_getBoolean(GenericBoolean *, OctetString *);
void GenericBoolean_setBoolean(GenericBoolean *, OctetString *, bool);

void GenericBoolean_print(GenericBoolean *, OctetString *);

#endif	/* __GENERICBOOLEAN_H__ */

