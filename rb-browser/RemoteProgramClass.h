/*
 * RemoteProgramClass.h
 */

#ifndef __REMOTEPROGRAMCLASS_H__
#define __REMOTEPROGRAMCLASS_H__

#include "ISO13522-MHEG-5.h"

void RemoteProgramClass_Preparation(RemoteProgramClass *);
void RemoteProgramClass_Activation(RemoteProgramClass *);
void RemoteProgramClass_Deactivation(RemoteProgramClass *);
void RemoteProgramClass_Destruction(RemoteProgramClass *);

#endif	/* __REMOTEPROGRAMCLASS_H__ */

