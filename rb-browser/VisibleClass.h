/*
 * VisibleClass.h
 */

#ifndef __VISIBLECLASS_H__
#define __VISIBLECLASS_H__

#include "ISO13522-MHEG-5.h"

void VisibleClass_Activation(RootClass *);

void VisibleClass_render(RootClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __VISIBLECLASS_H__ */

