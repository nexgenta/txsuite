/*
 * GroupItem.h
 */

#ifndef __GROUPITEM_H__
#define __GROUPITEM_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

RootClass *GroupItem_rootClass(GroupItem *);
bool GroupItem_isInitiallyActive(GroupItem *);
bool GroupItem_isShared(GroupItem *);
bool GroupItem_isVisibleClass(GroupItem *);

void GroupItem_Preparation(GroupItem *);
void GroupItem_Activation(GroupItem *);
void GroupItem_Deactivation(GroupItem *);
void GroupItem_Destruction(GroupItem *);

#endif	/* __GROUPITEM_H__ */

