/*
 * PushButtonClass.h
 */

#ifndef __PUSHBUTTONCLASS_H__
#define __PUSHBUTTONCLASS_H__

#include "ISO13522-MHEG-5.h"

void PushButtonClass_Preparation(PushButtonClass *);
void PushButtonClass_Activation(PushButtonClass *);
void PushButtonClass_Deactivation(PushButtonClass *);
void PushButtonClass_Destruction(PushButtonClass *);

void PushButtonClass_Select(PushButtonClass *);
void PushButtonClass_Deselect(PushButtonClass *);
void PushButtonClass_GetLabel(PushButtonClass *, GetLabel *, OctetString *);
void PushButtonClass_SetLabel(PushButtonClass *, SetLabel *, OctetString *);

#endif	/* __PUSHBUTTONCLASS_H__ */

