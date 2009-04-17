/*
 * ContentRefVariableClass.h
 */

#ifndef __CONTENTREFVARIABLECLASS_H__
#define __CONTENTREFVARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void ContentRefVariableClass_Clone(ContentRefVariableClass *, Clone *, OctetString *);
void ContentRefVariableClass_SetVariable(ContentRefVariableClass *, NewVariableValue *, OctetString *);
void ContentRefVariableClass_TestVariable(ContentRefVariableClass *, int, ContentReference *);

#endif	/* __CONTENTREFVARIABLECLASS_H__ */

