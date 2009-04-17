/*
 * ApplicationClass.h
 */

#ifndef __APPLICATIONCLASS_H__
#define __APPLICATIONCLASS_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

void ApplicationClass_Preparation(ApplicationClass *);
void ApplicationClass_Activation(ApplicationClass *);
void ApplicationClass_Deactivation(ApplicationClass *);
void ApplicationClass_Destruction(ApplicationClass *);

void ApplicationClass_SetCachePriority(ApplicationClass *, SetCachePriority *, OctetString *);
void ApplicationClass_StorePersistent(ApplicationClass *, StorePersistent *, OctetString *);
void ApplicationClass_ReadPersistent(ApplicationClass *, ReadPersistent *, OctetString *);
void ApplicationClass_Launch(ApplicationClass *, GenericObjectReference *, OctetString *);
void ApplicationClass_Spawn(ApplicationClass *, GenericObjectReference *, OctetString *);
void ApplicationClass_Quit(ApplicationClass *);
void ApplicationClass_LockScreen(ApplicationClass *);
void ApplicationClass_UnlockScreen(ApplicationClass *);
void ApplicationClass_OpenConnection(ApplicationClass *, OpenConnection *, OctetString *);
void ApplicationClass_CloseConnection(ApplicationClass *, CloseConnection *, OctetString *);
void ApplicationClass_GetEngineSupport(ApplicationClass *, GetEngineSupport *, OctetString *);
void ApplicationClass_SetTimer(ApplicationClass *, SetTimer *, OctetString *);

DefaultAttribute *ApplicationClass_getDefaultAttribute(ApplicationClass *, unsigned int);

#endif	/* __APPLICATIONCLASS_H__ */

