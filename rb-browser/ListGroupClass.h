/*
 * ListGroupClass.h
 */

#ifndef __LISTGROUPCLASS_H__
#define __LISTGROUPCLASS_H__

#include "ISO13522-MHEG-5.h"

void ListGroupClass_Preparation(ListGroupClass *);
void ListGroupClass_Activation(ListGroupClass *);
void ListGroupClass_Deactivation(ListGroupClass *);
void ListGroupClass_Destruction(ListGroupClass *);

void ListGroupClass_Clone(ListGroupClass *, Clone *, OctetString *);
void ListGroupClass_Move(ListGroupClass *, Move *, OctetString *);
void ListGroupClass_MoveTo(ListGroupClass *, MoveTo *, OctetString *);
void ListGroupClass_GetTokenPosition(ListGroupClass *, GetTokenPosition *, OctetString *);
void ListGroupClass_CallActionSlot(ListGroupClass *, CallActionSlot *, OctetString *);
void ListGroupClass_SetCellPosition(ListGroupClass *, SetCellPosition *, OctetString *);
void ListGroupClass_AddItem(ListGroupClass *, AddItem *, OctetString *);
void ListGroupClass_DelItem(ListGroupClass *, DelItem *, OctetString *);
void ListGroupClass_GetListItem(ListGroupClass *, GetListItem *, OctetString *);
void ListGroupClass_GetCellItem(ListGroupClass *, GetCellItem *, OctetString *);
void ListGroupClass_GetItemStatus(ListGroupClass *, GetItemStatus *, OctetString *);
void ListGroupClass_SelectItem(ListGroupClass *, SelectItem *, OctetString *);
void ListGroupClass_DeselectItem(ListGroupClass *, DeselectItem *, OctetString *);
void ListGroupClass_ToggleItem(ListGroupClass *, ToggleItem *, OctetString *);
void ListGroupClass_ScrollItems(ListGroupClass *, ScrollItems *, OctetString *);
void ListGroupClass_SetFirstItem(ListGroupClass *, SetFirstItem *, OctetString *);
void ListGroupClass_GetFirstItem(ListGroupClass *, GetFirstItem *, OctetString *);
void ListGroupClass_GetListSize(ListGroupClass *, GetListSize *, OctetString *);

#endif	/* __LISTGROUPCLASS_H__ */

