/*
 * IntegerVariableClass.h
 */

#ifndef __INTEGERVARIABLECLASS_H__
#define __INTEGERVARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void IntegerVariableClass_Clone(IntegerVariableClass *, Clone *, OctetString *);
void IntegerVariableClass_SetVariable(IntegerVariableClass *, NewVariableValue *, OctetString *);
void IntegerVariableClass_TestVariable(IntegerVariableClass *, int, int);
void IntegerVariableClass_Add(IntegerVariableClass *, Add *, OctetString *);
void IntegerVariableClass_Subtract(IntegerVariableClass *, Subtract *, OctetString *);
void IntegerVariableClass_Multiply(IntegerVariableClass *, Multiply *, OctetString *);
void IntegerVariableClass_Divide(IntegerVariableClass *, Divide *, OctetString *);
void IntegerVariableClass_Modulo(IntegerVariableClass *, Modulo *, OctetString *);

void IntegerVariableClass_setInteger(IntegerVariableClass *, int);

#endif	/* __INTEGERVARIABLECLASS_H__ */

