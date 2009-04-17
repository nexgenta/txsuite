/*
 * SwitchButtonClass.h
 */

#ifndef __SWITCHBUTTONCLASS_H__
#define __SWITCHBUTTONCLASS_H__

#include "ISO13522-MHEG-5.h"

void SwitchButtonClass_Preparation(SwitchButtonClass *);
void SwitchButtonClass_Activation(SwitchButtonClass *);
void SwitchButtonClass_Deactivation(SwitchButtonClass *);
void SwitchButtonClass_Destruction(SwitchButtonClass *);

void SwitchButtonClass_GetSelectionStatus(SwitchButtonClass *, GetSelectionStatus *, OctetString *);
void SwitchButtonClass_Select(SwitchButtonClass *);
void SwitchButtonClass_Deselect(SwitchButtonClass *);
void SwitchButtonClass_GetLabel(SwitchButtonClass *, GetLabel *, OctetString *);
void SwitchButtonClass_SetLabel(SwitchButtonClass *, SetLabel *, OctetString *);
void SwitchButtonClass_Toggle(SwitchButtonClass *);

#endif	/* __SWITCHBUTTONCLASS_H__ */

