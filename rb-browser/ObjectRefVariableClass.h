/*
 * ObjectRefVariableClass.h
 */

#ifndef __OBJECTREFVARIABLECLASS_H__
#define __OBJECTREFVARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void ObjectRefVariableClass_Clone(ObjectRefVariableClass *, Clone *, OctetString *);
void ObjectRefVariableClass_SetVariable(ObjectRefVariableClass *, NewVariableValue *, OctetString *);
void ObjectRefVariableClass_TestVariable(ObjectRefVariableClass *, int, ObjectReference *, OctetString *);

void ObjectRefVariableClass_setObjectReference(ObjectRefVariableClass *, ObjectReference *);

#endif	/* __OBJECTREFVARIABLECLASS_H__ */

