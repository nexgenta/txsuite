/*
 * VariableClass.h
 */

#ifndef __VARIABLECLASS_H__
#define __VARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void VariableClass_Preparation(VariableClass *);
void VariableClass_Activation(VariableClass *);
void VariableClass_Deactivation(VariableClass *);
void VariableClass_Destruction(VariableClass *);

void VariableClass_Clone(VariableClass *, Clone *, OctetString *);
void VariableClass_SetVariable(VariableClass *, NewVariableValue *, OctetString *);
void VariableClass_TestVariable(VariableClass *, int, ComparisonValue *, OctetString *);

unsigned int VariableClass_type(VariableClass *);

char *VariableClass_stringValue(VariableClass *);

#endif	/* __VARIABLECLASS_H__ */

