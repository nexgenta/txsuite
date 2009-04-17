/*
 * OctetStringVariableClass.h
 */

#ifndef __OCTETSTRINGVARIABLECLASS_H__
#define __OCTETSTRINGVARIABLECLASS_H__

#include "ISO13522-MHEG-5.h"

void OctetStringVariableClass_Clone(OctetStringVariableClass *, Clone *, OctetString *);
void OctetStringVariableClass_SetVariable(OctetStringVariableClass *, NewVariableValue *, OctetString *);
void OctetStringVariableClass_TestVariable(OctetStringVariableClass *, int, OctetString *);
void OctetStringVariableClass_Append(OctetStringVariableClass *, Append *, OctetString *);

void OctetStringVariableClass_setOctetString(OctetStringVariableClass *, OctetString *);

#endif	/* __OCTETSTRINGVARIABLECLASS_H__ */

