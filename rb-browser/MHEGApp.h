/*
 * MHEGApp.h
 */

#ifndef __MHEGAPP_H__
#define __MHEGAPP_H__

#include "ISO13522-MHEG-5.h"

typedef struct
{
	InterchangedObject *app;
	InterchangedObject *scene;
} MHEGApp;

void MHEGApp_init(MHEGApp *);
void MHEGApp_fini(MHEGApp *);

ApplicationClass *MHEGApp_loadApplication(MHEGApp *, OctetString *);
SceneClass *MHEGApp_loadScene(MHEGApp *, OctetString *);

#endif	/* __MHEGAPP_H__ */

