/*
 * ListGroupClass.c
 */

#include "MHEGEngine.h"
#include "ListGroupClass.h"
#include "ExternalReference.h"
#include "utils.h"

void
default_ListGroupClassInstanceVars(ListGroupClass *t, ListGroupClassInstanceVars *v)
{
	bzero(v, sizeof(ListGroupClassInstanceVars));

	/* TokenManagerClass */
	v->TokenPosition = 1;

	/* ListGroupClass */
	v->ItemList = NULL;
	v->FirstItem = 1;
/******************************************************************************************************/
/* we need to take a copy of this, so if we change it, the originals stay the same */
	v->Positions = t->positions;
/******************************************************************************************************/

	return;
}

void
free_ListGroupItem(ListGroupItem *i)
{
/***********************************************************************************/
/* free_TokenGroupItem will use der_free */
/***********************************************************************************/
	free_TokenGroupItem(&i->item);

	return;
}

void
free_ListGroupClassInstanceVars(ListGroupClassInstanceVars *v)
{
	LIST_FREE_ITEMS(&v->ItemList, ListGroupItem, free_ListGroupItem, safe_free);

	return;
}

void
ListGroupClass_Preparation(ListGroupClass *t)
{
	verbose("ListGroupClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Preparation not yet implemented\n");
	return;
}

void
ListGroupClass_Activation(ListGroupClass *t)
{
	verbose("ListGroupClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Activation not yet implemented\n");
	return;
}

void
ListGroupClass_Deactivation(ListGroupClass *t)
{
	verbose("ListGroupClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Deactivation not yet implemented\n");
	return;
}

void
ListGroupClass_Destruction(ListGroupClass *t)
{
	verbose("ListGroupClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Destruction not yet implemented\n");
	return;
}

void
ListGroupClass_Clone(ListGroupClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Clone not yet implemented\n");
	return;
}

void
ListGroupClass_Move(ListGroupClass *t, Move *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; Move", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_Move not yet implemented\n");
	return;
}

void
ListGroupClass_MoveTo(ListGroupClass *t, MoveTo *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; MoveTo", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_MoveTo not yet implemented\n");
	return;
}

void
ListGroupClass_GetTokenPosition(ListGroupClass *t, GetTokenPosition *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetTokenPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetTokenPosition not yet implemented\n");
	return;
}

void
ListGroupClass_CallActionSlot(ListGroupClass *t, CallActionSlot *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; CallActionSlot", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_CallActionSlot not yet implemented\n");
	return;
}

void
ListGroupClass_SetCellPosition(ListGroupClass *t, SetCellPosition *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; SetCellPosition", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_SetCellPosition not yet implemented\n");
	return;
}

void
ListGroupClass_AddItem(ListGroupClass *t, AddItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; AddItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_AddItem not yet implemented\n");
	return;
}

void
ListGroupClass_DelItem(ListGroupClass *t, DelItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; DelItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_DelItem not yet implemented\n");
	return;
}

void
ListGroupClass_GetListItem(ListGroupClass *t, GetListItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetListItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetListItem not yet implemented\n");
	return;
}

void
ListGroupClass_GetCellItem(ListGroupClass *t, GetCellItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetCellItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetCellItem not yet implemented\n");
	return;
}

void
ListGroupClass_GetItemStatus(ListGroupClass *t, GetItemStatus *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetItemStatus", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetItemStatus not yet implemented\n");
	return;
}

void
ListGroupClass_SelectItem(ListGroupClass *t, SelectItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; SelectItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_SelectItem not yet implemented\n");
	return;
}

void
ListGroupClass_DeselectItem(ListGroupClass *t, DeselectItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; DeselectItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_DeselectItem not yet implemented\n");
	return;
}

void
ListGroupClass_ToggleItem(ListGroupClass *t, ToggleItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; ToggleItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_ToggleItem not yet implemented\n");
	return;
}

void
ListGroupClass_ScrollItems(ListGroupClass *t, ScrollItems *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; ScrollItems", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_ScrollItems not yet implemented\n");
	return;
}

void
ListGroupClass_SetFirstItem(ListGroupClass *t, SetFirstItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; SetFirstItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_SetFirstItem not yet implemented\n");
	return;
}

void
ListGroupClass_GetFirstItem(ListGroupClass *t, GetFirstItem *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetFirstItem", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetFirstItem not yet implemented\n");
	return;
}

void
ListGroupClass_GetListSize(ListGroupClass *t, GetListSize *params, OctetString *caller_gid)
{
	verbose("ListGroupClass: %s; GetListSize", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ListGroupClass_GetListSize not yet implemented\n");
	return;
}

