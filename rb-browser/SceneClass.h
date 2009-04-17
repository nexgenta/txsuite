/*
 * SceneClass.h
 */

#ifndef __SCENECLASS_H__
#define __SCENECLASS_H__

#include "ISO13522-MHEG-5.h"

void SceneClass_Preparation(SceneClass *);
void SceneClass_Activation(SceneClass *);
void SceneClass_Deactivation(SceneClass *);
void SceneClass_Destruction(SceneClass *);

void SceneClass_SetCachePriority(SceneClass *, SetCachePriority *, OctetString *);
void SceneClass_SetTimer(SceneClass *, SetTimer *, OctetString *);
void SceneClass_SendEvent(SceneClass *, SendEvent *, OctetString *);
void SceneClass_SetCursorShape(SceneClass *, SetCursorShape *, OctetString *);
void SceneClass_SetCursorPosition(SceneClass *, SetCursorPosition *, OctetString *);
void SceneClass_GetCursorPosition(SceneClass *, GetCursorPosition *, OctetString *);
void SceneClass_SetInputRegister(SceneClass *, SetInputRegister *, OctetString *);
#endif	/* __SCENECLASS_H__ */

