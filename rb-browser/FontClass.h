/*
 * FontClass.h
 */

#ifndef __FONTCLASS_H__
#define __FONTCLASS_H__

#include "ISO13522-MHEG-5.h"

void FontClass_Preparation(FontClass *);
void FontClass_Activation(FontClass *);
void FontClass_Deactivation(FontClass *);
void FontClass_Destruction(FontClass *);

#endif	/* __FONTCLASS_H__ */

