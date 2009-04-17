/*
 * BooleanVariableClass.h
 */

#ifndef __BOOLEANVARIABLECLASS_H__
#define __BOOLEANVARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void BooleanVariableClass_Clone(BooleanVariableClass *, Clone *, OctetString *);
void BooleanVariableClass_SetVariable(BooleanVariableClass *, NewVariableValue *, OctetString *);
void BooleanVariableClass_TestVariable(BooleanVariableClass *, int, bool);

void BooleanVariableClass_setBoolean(BooleanVariableClass *, bool);

#endif	/* __BOOLEANVARIABLECLASS_H__ */

