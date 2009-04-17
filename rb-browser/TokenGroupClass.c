/*
 * TokenGroupClass.c
 */

#include "MHEGEngine.h"
#include "TokenGroupClass.h"
#include "RootClass.h"
#include "VisibleClass.h"
#include "ExternalReference.h"
#include "GenericInteger.h"
#include "ActionClass.h"
#include "VariableClass.h"
#include "IntegerVariableClass.h"
#include "rtti.h"

void
default_TokenGroupClassInstanceVars(TokenGroupClass *t, TokenGroupClassInstanceVars *v)
{
	bzero(v, sizeof(TokenGroupClassInstanceVars));

	v->TokenPosition = 1;

	return;
}

void
free_TokenGroupClassInstanceVars(TokenGroupClassInstanceVars *v)
{
	return;
}

void
TokenGroupClass_Preparation(TokenGroupClass *t)
{
	verbose("TokenGroupClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	if(!RootClass_Preparation(&t->rootClass))
		return;

	default_TokenGroupClassInstanceVars(t, &t->inst);

	return;
}

void
TokenGroupClass_Activation(TokenGroupClass *t)
{
	LIST_TYPE(TokenGroupItem) *tgi_list;
	RootClass *tgi;
	EventData data;

	verbose("TokenGroupClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it already been activated */
	if(t->rootClass.inst.RunningStatus)
		return;

	/* has it been prepared yet */
	if(!t->rootClass.inst.AvailabilityStatus)
	{
		TokenGroupClass_Preparation(t);
		/* generates an IsAvailable event */
		RootClass_Preparation(&t->rootClass);
	}

	/* do Activation of each TokenGroupItem */
	tgi_list = t->token_group_items;
	while(tgi_list)
	{
		if((tgi = MHEGEngine_findObjectReference(&tgi_list->item.a_visible, &t->rootClass.inst.ref.group_identifier)) != NULL)
			VisibleClass_Activation(tgi);
		tgi_list = tgi_list->next;
	}

	/* generate a TokenMovedTo event */
	data.choice = EventData_integer;
	data.u.integer = t->inst.TokenPosition;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_token_moved_to, &data);

	/* set the RunningStatus */
	t->rootClass.inst.RunningStatus = true;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	return;
}

void
TokenGroupClass_Deactivation(TokenGroupClass *t)
{
	EventData data;

	verbose("TokenGroupClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* are we already deactivated */
	if(!t->rootClass.inst.RunningStatus)
		return;

	/* generate a TokenMovedFrom event */
	data.choice = EventData_integer;
	data.u.integer = t->inst.TokenPosition;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_token_moved_from, &data);

	/* RootClass Deactivation */
	RootClass_Deactivation(&t->rootClass);

	return;
}

void
TokenGroupClass_Destruction(TokenGroupClass *t)
{
	verbose("TokenGroupClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

	if(!RootClass_Destruction(&t->rootClass))
		return;

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_TokenGroupClassInstanceVars(&t->inst);

	return;
}

void
TokenGroupClass_TransferToken(TokenGroupClass *t, unsigned int target)
{
	EventData data;

	data.choice = EventData_integer;
	data.u.integer = t->inst.TokenPosition;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_token_moved_from, &data);

	t->inst.TokenPosition = target;

	data.u.integer = t->inst.TokenPosition;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_token_moved_to, &data);

	return;
}

void
TokenGroupClass_Clone(TokenGroupClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("TokenGroupClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: TokenGroupClass_Clone not yet implemented\n");
	return;
}

void
TokenGroupClass_Move(TokenGroupClass *t, Move *params, OctetString *caller_gid)
{
	unsigned int movement_id;
	unsigned int count;
	LIST_TYPE(Movement) *movement_list;
	LIST_TYPE(int) *target_list;
	unsigned int target;

	verbose("TokenGroupClass: %s; Move", ExternalReference_name(&t->rootClass.inst.ref));

	/* see which Movement list we need */
	movement_id = (unsigned int) GenericInteger_getInteger(&params->movement_identifier, caller_gid);
	/* find the row in the movement_table, movement_id starts at 1 */
	count = movement_id;
	movement_list = t->movement_table;
	while(movement_list && count > 1)
	{
		movement_list = movement_list->next;
		count --;
	}
	/* was movement_id valid */
	if(movement_list == NULL)
	{
		error("TokenGroupClass: Move: invalid movement ID: %u", movement_id);
		return;
	}

	/* find the new TokenPosition in the movement list (index starts at 1) */
	count = t->inst.TokenPosition;
	target_list = movement_list->item;
	while(target_list && count > 1)
	{
		target_list = target_list->next;
		count --;
	}
	/* was it valid */
	if(target_list == NULL)
	{
		error("TokenGroupClass: Move: movement list %u is too short", movement_id);
		return;
	}

	/* get the new TokenPosition */
	target = (unsigned int) target_list->item;

	/* has it moved */
	if(t->inst.TokenPosition != target)
		TokenGroupClass_TransferToken(t, target);

	return;
}

void
TokenGroupClass_MoveTo(TokenGroupClass *t, MoveTo *params, OctetString *caller_gid)
{
	unsigned int index;

	verbose("TokenGroupClass: %s; MoveTo", ExternalReference_name(&t->rootClass.inst.ref));

	index = (unsigned int) GenericInteger_getInteger(&params->index, caller_gid);

	/* has it moved */
	if(t->inst.TokenPosition != index)
		TokenGroupClass_TransferToken(t, index);

	return;
}

void
TokenGroupClass_GetTokenPosition(TokenGroupClass *t, GetTokenPosition *params, OctetString *caller_gid)
{
	VariableClass *var;

	verbose("TokenGroupClass: %s; GetTokenPosition", ExternalReference_name(&t->rootClass.inst.ref));

	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->token_position_var, caller_gid)) == NULL)
		return;

	if(var->rootClass.inst.rtti != RTTI_VariableClass
	|| VariableClass_type(var) != OriginalValue_integer)
	{
		error("TokenGroupClass: GetTokenPosition: type mismatch");
		return;
	}

	IntegerVariableClass_setInteger(var, t->inst.TokenPosition);

	return;
}

void
TokenGroupClass_CallActionSlot(TokenGroupClass *t, CallActionSlot *params, OctetString *caller_gid)
{
	LIST_OF(ActionSlot) *action_slots;
	LIST_TYPE(ActionSlot) *action;
	LIST_TYPE(TokenGroupItem *) tgi;
	unsigned int i;
	unsigned int index;

	verbose("TokenGroupClass: %s; CallActionSlot", ExternalReference_name(&t->rootClass.inst.ref));

	/* if no-one has the token, use the no_token_action_slots */
	if(t->inst.TokenPosition == 0)
	{
		action_slots = t->no_token_action_slots;
	}
	else
	{
		/* use the action_slots from the TokenGroupItem that has the token */
		tgi = t->token_group_items;
		/* counting starts at 1 */
		i = t->inst.TokenPosition - 1;
		while(tgi && i != 0)
		{
			tgi = tgi->next;
			i --;
		}
		/* make sure it exists */
		if(tgi == NULL)
		{
			error("TokenGroupClass: invalid TokenPosition: %u", t->inst.TokenPosition);
			return;
		}
		action_slots = tgi->item.action_slots;
	}

	index = (unsigned int) GenericInteger_getInteger(&params->index, caller_gid);

	/* find the action */
	action = action_slots;
	/*
	 * the spec isn't clear, but from looking at real world apps, index starts at 1
	 * the spec does allow index=0 (though I've never seen it), so we assume this means do nothing
	 */
	if(index == 0)
	{
		verbose("CallActionSlot: index=0");
		return;
	}
	while(action && index != 1)
	{
		action = action->next;
		index --;
	}

	/* check it exists */
	if(action == NULL)
	{
		error("TokenGroupClass: invalid ActionSlot: %u", GenericInteger_getInteger(&params->index, caller_gid));
		return;
	}

	/*
	 * if the action is not Null, add the ElementaryActions to temp_actionq
	 * note, just doing:
	 * ActionClass_execute(&action->item.u.action_class, &t->rootClass.inst.ref.group_identifier);
	 * is not the same as adding the actions to the queue
	 * (also ActionClass_execute will ignore context changing actions - TransitionTo, Launch, Spawn and Quit)
	 */
	if(action->item.choice == ActionSlot_action_class)
		MHEGEngine_addToTempActionQ(&action->item.u.action_class, &t->rootClass.inst.ref.group_identifier);

	return;
}

