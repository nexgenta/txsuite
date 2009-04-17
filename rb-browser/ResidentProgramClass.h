/*
 * ResidentProgramClass.h
 */

#ifndef __RESIDENTPROGRAMCLASS_H__
#define __RESIDENTPROGRAMCLASS_H__

#include "ISO13522-MHEG-5.h"

void ResidentProgramClass_Preparation(ResidentProgramClass *);
void ResidentProgramClass_Activation(ResidentProgramClass *);
void ResidentProgramClass_Deactivation(ResidentProgramClass *);
void ResidentProgramClass_Destruction(ResidentProgramClass *);

void ResidentProgramClass_Clone(ResidentProgramClass *, Clone *, OctetString *);
void ResidentProgramClass_Call(ResidentProgramClass *, Call *, OctetString *);
void ResidentProgramClass_Fork(ResidentProgramClass *, Fork *, OctetString *);

#endif	/* __RESIDENTPROGRAMCLASS_H__ */

