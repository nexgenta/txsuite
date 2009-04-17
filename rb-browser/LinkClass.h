/*
 * LinkClass.h
 */

#ifndef __LINKCLASS_H__
#define __LINKCLASS_H__

#include "ISO13522-MHEG-5.h"

void LinkClass_Preparation(LinkClass *);
void LinkClass_Activation(LinkClass *);
void LinkClass_Deactivation(LinkClass *);
void LinkClass_Destruction(LinkClass *);

void LinkClass_Clone(LinkClass *, Clone *, OctetString *);
void LinkClass_Activate(LinkClass *);
void LinkClass_Deactivate(LinkClass *);

bool LinkClass_conditionMet(LinkClass *, ExternalReference *, EventType, EventData *);

#endif	/* __LINKCLASS_H__ */

