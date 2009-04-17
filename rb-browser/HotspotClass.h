/*
 * HotspotClass.h
 */

#ifndef __HOTSPOTCLASS_H__
#define __HOTSPOTCLASS_H__

#include "ISO13522-MHEG-5.h"

void HotspotClass_Preparation(HotspotClass *);
void HotspotClass_Activation(HotspotClass *);
void HotspotClass_Deactivation(HotspotClass *);
void HotspotClass_Destruction(HotspotClass *);

void HotspotClass_Select(HotspotClass *);
void HotspotClass_Deselect(HotspotClass *);

#endif	/* __HOTSPOTCLASS_H__ */

