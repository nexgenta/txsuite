/*
 * ApplicationClass.c
 */

#include <ctype.h>

#include "MHEGEngine.h"
#include "RootClass.h"
#include "GroupClass.h"
#include "ActionClass.h"
#include "GroupItem.h"
#include "ExternalReference.h"
#include "GenericOctetString.h"
#include "VariableClass.h"
#include "BooleanVariableClass.h"
#include "GenericObjectReference.h"
#include "clone.h"
#include "rtti.h"

/* internal funcs */
bool feature_AncillaryConnections(OctetString *);
bool feature_ApplicationStacking(OctetString *);
bool feature_Cloning(OctetString *);
bool feature_FreeMovingCursor(OctetString *);
bool feature_MultipleAudioStreams(OctetString *);
bool feature_MultipleVideoStreams(OctetString *);
bool feature_OverlappingVisibles(OctetString *);
bool feature_Scaling(OctetString *);
bool feature_SceneAspectRatio(OctetString *);
bool feature_SceneCoordinateSystem(OctetString *);
bool feature_TrickModes(OctetString *);
bool feature_VideoScaling(OctetString *);
bool feature_BitmapScaling(OctetString *);
bool feature_VideoDecodeOffset(OctetString *);
bool feature_BitmapDecodeOffset(OctetString *);
bool feature_UKEngineProfile(OctetString *);

int get_int_param(unsigned char **, unsigned int *);

void
default_ApplicationClassInstanceVars(ApplicationClass *t, ApplicationClassInstanceVars *v)
{
	bzero(v, sizeof(ApplicationClassInstanceVars));

	/* GroupClass */
	v->GroupCachePriority = t->original_group_cache_priority;
	v->Timers = NULL;
	v->removed_timers = NULL;

	/* ApplicationClass */
	v->LockCount = 0;
	v->DisplayStack = NULL;

	v->next_clone = FIRST_CLONED_OBJ_NUM;

	return;
}

void
free_ApplicationClassInstanceVars(ApplicationClassInstanceVars *v)
{
	GroupClass_freeTimers(&v->Timers, &v->removed_timers);

	LIST_FREE(&v->DisplayStack, RootClassPtr, safe_free);

	return;
}

void
ApplicationClass_Preparation(ApplicationClass *a)
{
	LIST_TYPE(GroupItem) *gi;

	verbose("ApplicationClass: %s; Preparation", ExternalReference_name(&a->rootClass.inst.ref));

	/* has it been prepared already */
	if(a->rootClass.inst.AvailabilityStatus)
		return;

	default_ApplicationClassInstanceVars(a, &a->inst);

	/* do Preparation on all Ingredients that are initially_active */
	gi = a->items;
	while(gi)
	{
		if(GroupItem_isInitiallyActive(&gi->item))
			GroupItem_Preparation(&gi->item);
		gi = gi->next;
	}

	/* Preparation inherited from the RootClass */
	RootClass_Preparation(&a->rootClass);

	return;
}

bool
ApplicationClass_Activation(ApplicationClass *a)
{
	LIST_TYPE(GroupItem) *gi;

	verbose("ApplicationClass: %s; Activation", ExternalReference_name(&a->rootClass.inst.ref));

	/* do Activation of the RootClass */
	/* is it already activated */
	if(a->rootClass.inst.RunningStatus)
		return false;

	/* has it been prepared yet */
	if(!a->rootClass.inst.AvailabilityStatus)
	{
		ApplicationClass_Preparation(a);
		/* generates an IsAvailable event */
		RootClass_Preparation(&a->rootClass);
	}

	/* do Activation of the GroupClass */
	if(a->have_on_start_up)
		ActionClass_execute(&a->on_start_up, &a->rootClass.inst.ref.group_identifier);

	/* do Activation on all Ingredients that are initially_active */
	gi = a->items;
	while(gi)
	{
		/* do Activation of all InitiallyActive group items */
		if(GroupItem_isInitiallyActive(&gi->item))
			GroupItem_Activation(&gi->item);
		gi = gi->next;
	}

	/* set RunningStatus */
	a->rootClass.inst.RunningStatus = true;

	/* generate IsRunning event */
	MHEGEngine_generateEvent(&a->rootClass.inst.ref, EventType_is_running, NULL);

	/* time base for absolute timers */
	gettimeofday(&a->inst.start_time, NULL);

	return true;
}

void
ApplicationClass_Deactivation(ApplicationClass *a)
{
	LIST_TYPE(GroupItem) *gi;
	LIST_TYPE(GroupItem) *gi_tail;
	RootClass *r;

	verbose("ApplicationClass: %s; Deactivation", ExternalReference_name(&a->rootClass.inst.ref));

	/* if we are not active ignore it */
	if(!a->rootClass.inst.RunningStatus)
		return;

	/*
	 * UK MHEG Profile says we don't need to support Open/CloseConnection
	 * so no need to perform a CloseConnection here
	 */

	/* run the OnCloseDown action */
	if(a->have_on_close_down)
		ActionClass_execute(&a->on_close_down, &a->rootClass.inst.ref.group_identifier);

	/* do Deactivation of all active Ingredients in the reverse order they appear in the list */
	gi = a->items;
	/* find the tail */
	gi_tail = (gi != NULL) ? gi->prev : NULL;
	gi = gi_tail;
	while(gi)
	{
		/* only do Deactivation if it is active */
		if((r = GroupItem_rootClass(&gi->item)) != NULL
		&& r->inst.RunningStatus)
			GroupItem_Deactivation(&gi->item);
		/* have we reached the head */
		gi = (gi->prev != gi_tail) ? gi->prev : NULL;
	}

	/* do RootClass Deactivation */
	RootClass_Deactivation(&a->rootClass);

	return;
}

void
ApplicationClass_Destruction(ApplicationClass *a)
{
	LIST_TYPE(GroupItem) *gi;
	LIST_TYPE(GroupItem) *gi_tail;
	RootClass *r;

	verbose("ApplicationClass: %s; Destruction", ExternalReference_name(&a->rootClass.inst.ref));

	/* is it already destroyed */
	if(!a->rootClass.inst.AvailabilityStatus)
		return;

	/* apply the Destruction behaviour to all Ingredients in the reverse order they appear in the list */
	gi = a->items;
	/* find the tail */
	gi_tail = (gi != NULL) ? gi->prev : NULL;
	gi = gi_tail;
	while(gi)
	{
		/* only do Destruction if it is available */
		if((r = GroupItem_rootClass(&gi->item)) != NULL
		&& r->inst.AvailabilityStatus)
			GroupItem_Destruction(&gi->item);
		/* have we reached the head */
		gi = (gi->prev != gi_tail) ? gi->prev : NULL;
	}

	/* Destruction inherited from RootClass */
	/* Deactivate it if it is running */
	if(a->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		ApplicationClass_Deactivation(a);
	}

	/*
	 * spec says we should handle caching here
	 * rb-download caches everything
	 */

	free_ApplicationClassInstanceVars(&a->inst);

	/* generate an IsDeleted event */
	a->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&a->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
ApplicationClass_SetCachePriority(ApplicationClass *t, SetCachePriority *params, OctetString *caller_gid)
{
	verbose("ApplicationClass: %s; SetCachePriority (ignored)", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
ApplicationClass_StorePersistent(ApplicationClass *t, StorePersistent *params, OctetString *caller_gid)
{
	OctetString *filename;
	PersistentData *p;
	LIST_TYPE(ObjectReference) *ref;
	VariableClass *var;
	LIST_TYPE(OriginalValue) *val;
	bool succeed = true;

	verbose("ApplicationClass: %s; StorePersistent", ExternalReference_name(&t->rootClass.inst.ref));

	filename = GenericOctetString_getOctetString(&params->out_file_name, caller_gid);

	/* create a new file if it does not exist */
	p = MHEGEngine_findPersistentData(filename, true);

	/* free any existing contents */
	LIST_FREE_ITEMS(&p->data, OriginalValue, free_OriginalValue, safe_free);

	verbose("StorePersistent: filename '%.*s'", filename->size, filename->data);

	/* add the new values */
	ref = params->in_variables;
	while(ref && succeed)
	{
		if((var = (VariableClass *) MHEGEngine_findObjectReference(&ref->item, caller_gid)) != NULL)
		{
			if(var->rootClass.inst.rtti == RTTI_VariableClass)
			{
				verbose("StorePersistent: variable '%s'", VariableClass_stringValue(var));
				val = safe_malloc(sizeof(LIST_TYPE(OriginalValue)));
				OriginalValue_dup(&val->item, &var->inst.Value);
				LIST_APPEND(&p->data, val);
			}
			else
			{
				error("StorePersistent: %s is not a VariableClass", ExternalReference_name(&var->rootClass.inst.ref));
				succeed = false;
			}
		}
		ref = ref->next;
	}

	/* set the succeeded variable */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->store_succeeded, caller_gid)) != NULL)
	{
		if(var->rootClass.inst.rtti == RTTI_VariableClass
		&& VariableClass_type(var) == OriginalValue_boolean)
		{
			BooleanVariableClass_setBoolean(var, succeed);
		}
		else
		{
			error("StorePersistent: %s is not a BooleanVariableClass", ExternalReference_name(&var->rootClass.inst.ref));
		}
	}

	return;
}

void
ApplicationClass_ReadPersistent(ApplicationClass *t, ReadPersistent *params, OctetString *caller_gid)
{
	OctetString *filename;
	PersistentData *p;
	LIST_TYPE(ObjectReference) *ref = NULL;
	VariableClass *var;
	LIST_TYPE(OriginalValue) *val;
	bool succeed = true;

	verbose("ApplicationClass: %s; ReadPersistent", ExternalReference_name(&t->rootClass.inst.ref));

	filename = GenericOctetString_getOctetString(&params->in_file_name, caller_gid);

	/* find the file */
	if((p = MHEGEngine_findPersistentData(filename, false)) != NULL)
	{
		verbose("ReadPersistent: filename '%.*s'", filename->size, filename->data);
		/* read the values into the variables */
		ref = params->out_variables;
		val = p->data;
		while(ref && val && succeed)
		{
			if((var = (VariableClass *) MHEGEngine_findObjectReference(&ref->item, caller_gid)) != NULL)
			{
				if(var->rootClass.inst.rtti == RTTI_VariableClass
				&& VariableClass_type(var) == val->item.choice)
				{
					/* free any existing data */
					OriginalValue_copy(&var->inst.Value, &val->item);
					verbose("ReadPersistent: variable '%s'", VariableClass_stringValue(var));
				}
				else
				{
					error("ReadPersistent: %s: type mismatch", ExternalReference_name(&var->rootClass.inst.ref));
					succeed = false;
				}
			}
			ref = ref->next;
			val = val->next;
		}
	}
	else
	{
		/* not an error message */
		verbose("ReadPersistent: '%.*s' not found", filename->size, filename->data);
		succeed = false;
	}

	/* did we set all the variables */
	if(ref)
	{
		error("ReadPersistent: '%.*s': unable to read all the variables", filename->size, filename->data);
		succeed = false;
	}

	/* set the succeeded variable */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->read_succeeded, caller_gid)) != NULL)
	{
		if(var->rootClass.inst.rtti == RTTI_VariableClass
		&& VariableClass_type(var) == OriginalValue_boolean)
		{
			BooleanVariableClass_setBoolean(var, succeed);
		}
		else
		{
			error("ReadPersistent: %s is not a BooleanVariableClass", ExternalReference_name(&var->rootClass.inst.ref));
		}
	}

	return;
}

void
ApplicationClass_Launch(ApplicationClass *t, GenericObjectReference *launch, OctetString *caller_gid)
{
	ObjectReference *ref;
	char *absolute;
	OctetString quit_data;

	verbose("ApplicationClass: %s; Launch", ExternalReference_name(&t->rootClass.inst.ref));

	/* check we can find an ExternalReference for the new app */
	if(((ref = GenericObjectReference_getObjectReference(launch, caller_gid)) == NULL)
	|| ref->choice != ObjectReference_external_reference)
	{
		if(ref != NULL)
			error("Launch: unable to launch an internal reference");
		return;
	}

	/* get the absolute group ID of the new app */
	absolute = MHEGEngine_absoluteFilename(&ref->u.external_reference.group_identifier);
	quit_data.size = strlen(absolute);
	quit_data.data = absolute;

	MHEGEngine_quit(QuitReason_Launch, &quit_data);

	return;
}

void
ApplicationClass_Spawn(ApplicationClass *t, GenericObjectReference *spawn, OctetString *caller_gid)
{
	ObjectReference *ref;
	char *absolute;
	OctetString quit_data;

	verbose("ApplicationClass: %s; Spawn", ExternalReference_name(&t->rootClass.inst.ref));

	/* check we can find an ExternalReference for the new app */
	if(((ref = GenericObjectReference_getObjectReference(spawn, caller_gid)) == NULL)
	|| ref->choice != ObjectReference_external_reference)
	{
		if(ref != NULL)
			error("Spawn: unable to spawn an internal reference");
		return;
	}

	/* get the absolute group ID of the new app */
	absolute = MHEGEngine_absoluteFilename(&ref->u.external_reference.group_identifier);
	quit_data.size = strlen(absolute);
	quit_data.data = absolute;

	MHEGEngine_quit(QuitReason_Spawn, &quit_data);

/* TODO */
/* on_spawn_close_down Actions */

	return;
}

void
ApplicationClass_Quit(ApplicationClass *t)
{
	verbose("ApplicationClass: %s; Quit", ExternalReference_name(&t->rootClass.inst.ref));

	MHEGEngine_quit(QuitReason_Quit, NULL);

	return;
}

void
ApplicationClass_LockScreen(ApplicationClass *t)
{
	verbose("ApplicationClass: %s; LockScreen", ExternalReference_name(&t->rootClass.inst.ref));

	t->inst.LockCount ++;

	return;
}

void
ApplicationClass_UnlockScreen(ApplicationClass *t)
{
	XYPosition pos;
	OriginalBoxSize box;

	verbose("ApplicationClass: %s; UnlockScreen", ExternalReference_name(&t->rootClass.inst.ref));

	/* if it is not already unlocked */
	if(t->inst.LockCount > 0)
	{
		t->inst.LockCount --;
		/* update the screen if it is now unlocked */
		if(t->inst.LockCount == 0)
		{
			pos.x_position = 0;
			pos.y_position = 0;
			box.x_length = MHEG_XRES;
			box.y_length = MHEG_YRES;
			MHEGEngine_redrawArea(&pos, &box);
		}
	}

	return;
}

void
ApplicationClass_OpenConnection(ApplicationClass *t, OpenConnection *params, OctetString *caller_gid)
{
	error("ApplicationClass: %s; OpenConnection not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
ApplicationClass_CloseConnection(ApplicationClass *t, CloseConnection *params, OctetString *caller_gid)
{
	error("ApplicationClass: %s; CloseConnection not supported", ExternalReference_name(&t->rootClass.inst.ref));

	return;
}

void
ApplicationClass_GetEngineSupport(ApplicationClass *t, GetEngineSupport *params, OctetString *caller_gid)
{
	OctetString *feature;
	unsigned char *pars_start;
	unsigned int feature_len;
	OctetString feature_pars;
	unsigned int i;
	bool found;
	bool answer = false;
	VariableClass *var;
	struct
	{
		char *long_name;
		char *short_name;
		bool (*func)(OctetString *);
	} features[] =
	{
		{ "AncillaryConnections", "ACo", feature_AncillaryConnections },
		{ "ApplicationStacking", "ASt", feature_ApplicationStacking },
		{ "Cloning", "Clo", feature_Cloning },
		{ "FreeMovingCursor", "FMC", feature_FreeMovingCursor },
		{ "MultipleAudioStreams(", "MAS(", feature_MultipleAudioStreams },
		{ "MultipleVideoStreams(", "MVS(", feature_MultipleVideoStreams },
		{ "OverlappingVisibles(", "OvV(", feature_OverlappingVisibles },
		{ "Scaling", "Sca", feature_Scaling },
		{ "SceneAspectRatio(", "SAR(", feature_SceneAspectRatio },
		{ "SceneCoordinateSystem(", "SCS(", feature_SceneCoordinateSystem },
		{ "TrickModes", "TrM", feature_TrickModes },
		{ "VideoScaling(", "VSc(", feature_VideoScaling },
		{ "BitmapScaling(", "BSc(", feature_BitmapScaling },
		{ "VideoDecodeOffset(", "VDO(", feature_VideoDecodeOffset },
		{ "BitmapDecodeOffset(", "BDO(", feature_BitmapDecodeOffset },
		{ "UKEngineProfile(", "UEP(", feature_UKEngineProfile },
		{ NULL, NULL, NULL }
	};

	feature = GenericOctetString_getOctetString(&params->feature, caller_gid);

	/* does the feature take parameters */
	pars_start = memchr(feature->data, '(', feature->size);
	if(pars_start != NULL)
		feature_len = pars_start - feature->data;
	else
		feature_len = feature->size;

	found = false;
	for(i=0; features[i].func != NULL && !found; i++)
	{
		if(strncmp(feature->data, features[i].short_name, feature_len) == 0
		|| strncmp(feature->data, features[i].long_name, feature_len) == 0)
		{
			feature_pars.data = pars_start;
			feature_pars.size = feature->size - feature_len;
			answer = (*(features[i].func))(&feature_pars);
			found = true;
		}
	}

	if(!found)
	{
		error("GetEngineSupport: unknown feature '%.*s'", feature->size, feature->data);
		answer = false;
	}

	verbose("ApplicationClass: %s; GetEngineSupport(\"%.*s\") = %s",
		ExternalReference_name(&t->rootClass.inst.ref),
		feature->size, feature->data,
		answer ? "true" : "false");


	/* set the answer variable */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->answer, caller_gid)) != NULL)
	{
		if(var->rootClass.inst.rtti == RTTI_VariableClass
		&& VariableClass_type(var) == OriginalValue_boolean)
		{
			BooleanVariableClass_setBoolean(var, answer);
		}
		else
		{
			error("GetEngineSupport: %s is not a BooleanVariableClass", ExternalReference_name(&var->rootClass.inst.ref));
		}
	}

	return;
}

void
ApplicationClass_SetTimer(ApplicationClass *t, SetTimer *params, OctetString *caller_gid)
{
	verbose("ApplicationClass: %s; SetTimer", ExternalReference_name(&t->rootClass.inst.ref));

	GroupClass_SetTimer(&t->rootClass.inst.ref, &t->inst.Timers, &t->inst.removed_timers, &t->inst.start_time, params, caller_gid);

	return;
}

/*
 * returns NULL if the attribute is not specified in the ApplicationClass
 */

DefaultAttribute *
ApplicationClass_getDefaultAttribute(ApplicationClass *a, unsigned int attr)
{
	LIST_TYPE(DefaultAttribute) *list = a->default_attributes;

	while(list)
	{
		if(list->item.choice == attr)
			return &list->item;
		list = list->next;
	}

	return NULL;
}

/*
 * GetEngineSupport features
 * params->data will either be NULL or contain the full params string enclosed in brackets
 * if the feature takes a parameter, ie the name includes a (, then params will never be NULL
 * and params->size will always be at least 1
 */

bool
feature_AncillaryConnections(OctetString *params)
{
	return false;
}

bool
feature_ApplicationStacking(OctetString *params)
{
	return true;
}

bool
feature_Cloning(OctetString *params)
{
	return true;
}

bool
feature_FreeMovingCursor(OctetString *params)
{
	return false;
}

bool
feature_MultipleAudioStreams(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int n;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	n = get_int_param(&data, &size);

	return (n <= 1);
}

bool
feature_MultipleVideoStreams(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int n;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	n = get_int_param(&data, &size);

	return (n <= 1);
}

bool
feature_OverlappingVisibles(OctetString *params)
{
	return true;
}

bool
feature_Scaling(OctetString *params)
{
	return false;
}

bool
feature_SceneAspectRatio(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int w, h;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	w = get_int_param(&data, &size);
	h = get_int_param(&data, &size);

	return ((w == 4) && (h == 3)) || ((w == 16) && (h == 9));
}

bool
feature_SceneCoordinateSystem(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int x, y;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	x = get_int_param(&data, &size);
	y = get_int_param(&data, &size);

	return (x == MHEG_XRES) && (y == MHEG_YRES);
}

bool
feature_TrickModes(OctetString *params)
{
	return false;
}

bool
feature_VideoScaling(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int chook, x, y;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	chook = get_int_param(&data, &size);
	x = get_int_param(&data, &size);
	y = get_int_param(&data, &size);

	if(chook != ContentHook_Stream_MPEG)
		return false;

	return ((x == MHEG_XRES * 2) && (y == MHEG_YRES * 2))
	    || ((x == MHEG_XRES) && (y == MHEG_YRES))
	    || ((x == MHEG_XRES / 2) && (y == MHEG_YRES / 2));
}

bool
feature_BitmapScaling(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int chook, x, y;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	chook = get_int_param(&data, &size);
	x = get_int_param(&data, &size);
	y = get_int_param(&data, &size);

	if(chook != ContentHook_Bitmap_MPEG)
		return false;

	return ((x == MHEG_XRES * 2) && (y == MHEG_YRES * 2))
	    || ((x == MHEG_XRES) && (y == MHEG_YRES))
	    || ((x == MHEG_XRES / 2) && (y == MHEG_YRES / 2));
}

bool
feature_VideoDecodeOffset(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int chook, level;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	chook = get_int_param(&data, &size);
	level = get_int_param(&data, &size);

	if(chook != ContentHook_Stream_MPEG)
		return false;

	return (level == 0 || level == 1);
}

bool
feature_BitmapDecodeOffset(OctetString *params)
{
	unsigned char *data;
	unsigned int size;
	int chook, level;

	/* skip the opening bracket */
	data = params->data + 1;
	size = params->size - 1;

	chook = get_int_param(&data, &size);
	level = get_int_param(&data, &size);

	if(chook != ContentHook_Bitmap_MPEG)
		return false;

	return (level == 0 || level == 1);
}

#define UKEP_MATCH(STR)		if(params->size == strlen(STR)				\
				&& strncmp(params->data, STR, params->size) == 0)	\
					return true

bool
feature_UKEngineProfile(OctetString *params)
{
	/* UK MHEG Profile launch ID */
	UKEP_MATCH("(2)");

	/* WhoAmI strings */
	UKEP_MATCH("(" MHEG_RECEIVER_ID ")");
	UKEP_MATCH("(" MHEG_ENGINE_ID ")");
	UKEP_MATCH("(" MHEG_DSMCC_ID ")");

	return false;
}

/*
 * gets the next base 10 integer parameter
 * updates *data and *size
 */

int
get_int_param(unsigned char **data, unsigned int *size)
{
	int val = 0;

	while(*size > 0 && isdigit(**data))
	{
		val *= 10;
		val += (**data) - '0';
		(*data) ++;
		(*size) --;
	}

	/* skip the terminator */
	if(*size > 0)
	{
		(*data) ++;
		(*size) --;
	}

	return val;
}

