/*
 * StreamClass.h
 */

#ifndef __STREAMCLASS_H__
#define __STREAMCLASS_H__

#include "ISO13522-MHEG-5.h"

void StreamClass_Preparation(StreamClass *);
void StreamClass_Activation(StreamClass *);
void StreamClass_Deactivation(StreamClass *);
void StreamClass_Destruction(StreamClass *);

void StreamClass_activateVideoComponent(StreamClass *, VideoClass *);
void StreamClass_activateAudioComponent(StreamClass *, AudioClass *);
void StreamClass_deactivateVideoComponent(StreamClass *, VideoClass *);
void StreamClass_deactivateAudioComponent(StreamClass *, AudioClass *);

void StreamClass_SetData(StreamClass *, SetData *, OctetString *);
void StreamClass_SetCounterTrigger(StreamClass *, SetCounterTrigger *, OctetString *);
void StreamClass_SetSpeed(StreamClass *, SetSpeed *, OctetString *);
void StreamClass_SetCounterPosition(StreamClass *, SetCounterPosition *, OctetString *);
void StreamClass_SetCounterEndPosition(StreamClass *, SetCounterEndPosition *, OctetString *);

#endif	/* __STREAMCLASS_H__ */

