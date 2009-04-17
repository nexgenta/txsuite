/*
 * ActionClass.c
 */

#include "MHEGEngine.h"
#include "ActionClass.h"
#include "ElementaryAction.h"

/*
 * caller_gid should be the group identifier of the object containing the ActionClass
 * it is used to resolve the ObjectReference's in the ElementaryAction's
 * this function should only be called to run OnStartUp and OnCloseDown actions
 * all other times (ie CallActionSlot) use MHEGEngine_addToTempActionQ instead
 * Corrigendum says we should ignore context changing actions - TransitionTo, Launch, Spawn and Quit
 */

void
ActionClass_execute(ActionClass *a, OctetString *caller_gid)
{
	LIST_TYPE(ElementaryAction) *list = *a;

	while(list)
	{
		unsigned int type = list->item.choice;
		if(type == ElementaryAction_transition_to
		|| type == ElementaryAction_launch
		|| type == ElementaryAction_spawn
		|| type == ElementaryAction_quit)
			error("ActionClass: ignoring %s in OnStartup/OnCloseDown actions", ElementaryAction_name(&list->item));
		else
			ElementaryAction_execute(&list->item, caller_gid);
		list = list->next;
	}

	return;
}

