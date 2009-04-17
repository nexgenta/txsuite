/*
 * RootClass.h
 */

#ifndef __ROOTCLASS_H__
#define __ROOTCLASS_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

void RootClass_dup(RootClass *, RootClass *);

void RootClass_registerObject(RootClass *);
void RootClass_registerClonedObject(LIST_TYPE(GroupItem) *, RootClass *, unsigned int, unsigned int);
void RootClass_unregisterObject(RootClass *);

bool RootClass_Preparation(RootClass *);
bool RootClass_Activation(RootClass *);
bool RootClass_Deactivation(RootClass *);
bool RootClass_Destruction(RootClass *);

void RootClass_GetAvailabilityStatus(RootClass *, ObjectReference *, OctetString *);
void RootClass_GetRunningStatus(RootClass *, ObjectReference *, OctetString *);

void RootClass_contentAvailable(RootClass *, OctetString *);

#endif	/* __ROOTCLASS_H__ */

