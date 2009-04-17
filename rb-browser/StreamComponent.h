/*
 * StreamComponent.h
 */

#ifndef __STREAMCOMPONENT_H__
#define __STREAMCOMPONENT_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

void StreamComponent_registerStreamClass(StreamComponent *, StreamClass *);

RootClass *StreamComponent_rootClass(StreamComponent *);
bool StreamComponent_isInitiallyActive(StreamComponent *);

void StreamComponent_Activation(StreamComponent *);
void StreamComponent_Destruction(StreamComponent *);

void StreamComponent_play(StreamComponent *, MHEGStreamPlayer *);
void StreamComponent_stop(StreamComponent *, MHEGStreamPlayer *);

#endif	/* __STREAMCOMPONENT_H__ */

