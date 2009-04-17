/*
 * MHEGEngine.c
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <png.h>

#include "MHEGEngine.h"
#include "RootClass.h"
#include "LinkClass.h"
#include "EventType.h"
#include "ElementaryAction.h"
#include "ExternalReference.h"
#include "ObjectReference.h"
#include "GenericObjectReference.h"
#include "GroupItem.h"
#include "ApplicationClass.h"
#include "SceneClass.h"
#include "VisibleClass.h"
#include "si.h"
#include "clone.h"
#include "rtti.h"
#include "utils.h"

LIST_TYPE(MissingContent) *
new_MissingContentListItem(RootClass *obj, OctetString *file)
{
	LIST_TYPE(MissingContent) *missing;
	struct timeval now;

	missing = safe_malloc(sizeof(LIST_TYPE(MissingContent)));
	bzero(missing, sizeof(LIST_TYPE(MissingContent)));

	missing->item.obj = obj;

	/* copy the filename */
	OctetString_dup(&missing->item.file, file);

	/* current time */
	gettimeofday(&now, NULL);
	missing->item.requested = now.tv_sec;

	return missing;
}

void
free_MissingContentListItem(LIST_TYPE(MissingContent) *missing)
{
	/* free the filename we copied */
	safe_free(missing->item.file.data);

	safe_free(missing);

	return;
}

LIST_TYPE(PersistentData) *
new_PersistentDataListItem(OctetString *filename)
{
	LIST_TYPE(PersistentData) *p;

	p = safe_malloc(sizeof(LIST_TYPE(PersistentData)));
	bzero(p, sizeof(LIST_TYPE(PersistentData)));

	OctetString_dup(&p->item.filename, filename);

	p->item.data = NULL;

	return p;
}

void
free_PersistentDataListItem(LIST_TYPE(PersistentData) *p)
{
	safe_free(p->item.filename.data);

	LIST_FREE_ITEMS(&p->item.data, OriginalValue, free_OriginalValue, safe_free);

	safe_free(p);

	return;
}

LIST_TYPE(MHEGAsyncEvent) *
new_MHEGAsyncEventListItem(ExternalReference *src, EventType type, EventData *data)
{
	LIST_TYPE(MHEGAsyncEvent) *event;

	event = safe_malloc(sizeof(LIST_TYPE(MHEGAsyncEvent)));
	bzero(event, sizeof(LIST_TYPE(MHEGAsyncEvent)));

	/* take a copy of the source in case it disappears before we process the event */
	ExternalReference_dup(&event->item.src, src);

	/* remember the type */
	event->item.type = type;

	/* take a copy of the data in case it disappears before we process the event */
	if(data != NULL)
	{
		event->item.data = safe_malloc(sizeof(EventData));
		EventData_dup(event->item.data, data);
	}
	else
	{
		event->item.data = NULL;
	}

	return event;
}

void
free_MHEGAsyncEventListItem(LIST_TYPE(MHEGAsyncEvent) *event)
{
	/* free the source */
	safe_free(event->item.src.group_identifier.data);

	/* free any event data */
	if(event->item.data != NULL)
	{
		free_EventData(event->item.data);
		safe_free(event->item.data);
	}

	safe_free(event);

	return;
}

/*
 * the group ID and action must remain valid until we execute the action
 */

LIST_TYPE(MHEGAction) *
new_MHEGActionListItem(OctetString *group_id, ElementaryAction *action)
{
	LIST_TYPE(MHEGAction) *item;

	item = safe_malloc(sizeof(LIST_TYPE(MHEGAction)));

	item->item.group_id = group_id;
	item->item.action = action;

	return item;
}

void
free_MHEGActionListItem(LIST_TYPE(MHEGAction) *item)
{
	safe_free(item);

	return;
}

/*
 * rather than passing a ptr to this to every function we write
 * let's be naughty and make it global (to this file)
 */
static MHEGEngine engine;

void
MHEGEngine_init(MHEGEngineOptions *opts)
{
	bzero(&engine, sizeof(MHEGEngine));

	engine.verbose = opts->verbose;
	engine.timeout = opts->timeout;

	MHEGDisplay_init(&engine.display, opts->fullscreen, opts->keymap);

	engine.vo_method = MHEGVideoOutputMethod_fromString(opts->vo_method);
	engine.av_disabled = opts->av_disabled;

	MHEGBackend_init(&engine.backend, opts->remote, opts->srg_loc);

	MHEGApp_init(&engine.active_app);

	return;
}

int
MHEGEngine_run(void)
{
	OctetString boot_obj;
	ApplicationClass *app;
	SceneClass *scene;
	bool block;
	unsigned int i;
	bool found;
	struct timeval start;
	struct timeval now;
	/* search order for the app to boot in the Service Gateway dir */
	char *boot_order[] = { "~//a", "~//startup", NULL };

	do
	{
		/* clear the display (if you want a splash screen, do it here) */
		MHEGDisplay_clearScreen(&engine.display);
		/* search for the boot object for timeout seconds */
		found = false;
		gettimeofday(&start, NULL);
		do
		{
			for(i=0; !found && boot_order[i] != NULL; i++)
			{
				boot_obj.size = strlen(boot_order[i]);
				boot_obj.data = boot_order[i];
				found = MHEGEngine_checkContentRef(&boot_obj);
			}
			gettimeofday(&now, NULL);
		}
		while(!found && now.tv_sec <= (start.tv_sec + engine.timeout));
		if(!found)
		{
			error("Unable to find boot object in service gateway");
			return EXIT_FAILURE;
		}
		do
		{
			/* boot it */
			verbose("Booting '%.*s'", boot_obj.size, boot_obj.data);
			engine.quit_reason = QuitReason_DontQuit;
			/* load the app */
			if((app = MHEGApp_loadApplication(&engine.active_app, &boot_obj)) == NULL)
				return EXIT_FAILURE;
			/* start it up */
			ApplicationClass_Preparation(app);
			ApplicationClass_Activation(app);
			/* main loop */
			while(engine.quit_reason == QuitReason_DontQuit)
			{
				/* poll for files we are waiting for */
				MHEGEngine_pollMissingContent();
				/* process any async events */
				MHEGEngine_processMHEGEvents();
				/*
				 * if we are polling for missing content,
				 * or if we need to quit the current app
				 * don't block waiting for the next GUI event
				 */
				block = (engine.missing_content == NULL && engine.quit_reason == QuitReason_DontQuit);
				/* process any GUI events */
				if(MHEGDisplay_processEvents(&engine.display, block))
					engine.quit_reason = QuitReason_GUIQuit;
			}
			/* do Destruction of Application and Scene */
			if((scene = MHEGEngine_getActiveScene()) != NULL)
			{
				SceneClass_Deactivation(scene);
				SceneClass_Destruction(scene);
			}
			ApplicationClass_Deactivation(app);
			ApplicationClass_Destruction(app);
			/* clean up */
			MHEGApp_fini(&engine.active_app);
			LIST_FREE(&engine.objects, RootClassPtr, safe_free);
			LIST_FREE(&engine.missing_content, MissingContent, free_MissingContentListItem);
			LIST_FREE(&engine.active_links, LinkClassPtr, safe_free);
			LIST_FREE(&engine.async_eventq, MHEGAsyncEvent, free_MHEGAsyncEventListItem);
			LIST_FREE(&engine.main_actionq, MHEGAction, free_MHEGActionListItem);
			LIST_FREE(&engine.temp_actionq, MHEGAction, free_MHEGActionListItem);
			/* do we need to run a new app */
			switch(engine.quit_reason)
			{
			case QuitReason_Launch:
				verbose("Launch '%.*s'", engine.quit_data.size, engine.quit_data.data);
				boot_obj.size = engine.quit_data.size;
				boot_obj.data = engine.quit_data.data;
				break;

			case QuitReason_Spawn:
				verbose("Spawn '%.*s'", engine.quit_data.size, engine.quit_data.data);
/* TODO */
/* need to run on_restart and on_spawn_close_down Actions at some point */
printf("TODO: Spawn '%.*s'; doing Launch instead\n", engine.quit_data.size, engine.quit_data.data);
				boot_obj.size = engine.quit_data.size;
				boot_obj.data = engine.quit_data.data;
				break;

			case QuitReason_Retune:
				verbose("Retune to '%.*s'", engine.quit_data.size, engine.quit_data.data);
				MHEGEngine_retune(&engine.quit_data);
				break;

			default:
				/* nothing to do */
				break;
			}
		}
		while(engine.quit_reason == QuitReason_Launch || engine.quit_reason == QuitReason_Spawn);
	}
	while(engine.quit_reason == QuitReason_Retune);

	return EXIT_SUCCESS;
}

void
MHEGEngine_fini(void)
{
	MHEGDisplay_fini(&engine.display);

	LIST_FREE(&engine.persistent, PersistentData, free_PersistentDataListItem);

	si_free();

	MHEGBackend_fini(&engine.backend);

	free_OctetString(&engine.quit_data);

	return;
}

MHEGDisplay *
MHEGEngine_getDisplay(void)
{
	return &engine.display;
}

MHEGVideoOutputMethod *
MHEGEngine_getVideoOutputMethod(void)
{
	return engine.vo_method;
}

bool
MHEGEngine_avDisabled(void)
{
	return engine.av_disabled;
}

/*
 * according to the ISO MHEG spec this should be part of the SceneClass
 * but we need info about the current app etc too
 */

void
MHEGEngine_TransitionTo(TransitionTo *to, OctetString *caller_gid)
{
	ObjectReference *ref;
	char *absolute;
	OctetString scene_id;
	SceneClass *current_scene;
	ApplicationClass *current_app;
	OctetString *app_gid;
	LIST_TYPE(MHEGAsyncEvent) *ev, *next_ev;
	LIST_TYPE(MHEGAction) *act, *next_act;
	LIST_TYPE(GroupItem) *gi;
	LIST_TYPE(GroupItem) *gi_tail;

	/* check we can find an ExternalReference for the new scene */
	if(((ref = GenericObjectReference_getObjectReference(&to->target, caller_gid)) == NULL)
	|| ref->choice != ObjectReference_external_reference)
	{
		if(ref != NULL)
			error("TransitionTo: unable to transition to an internal reference");
		return;
	}

	/* get the absolute group ID of the new scene */
	absolute = MHEGEngine_absoluteFilename(&ref->u.external_reference.group_identifier);
	/* copy it */
	scene_id.size = strlen(absolute);
	scene_id.data = safe_malloc(scene_id.size);
	memcpy(scene_id.data, absolute, scene_id.size);

	/* check target is not the current scene */
	current_scene = MHEGEngine_getActiveScene();
	if(current_scene == NULL
	|| OctetString_cmp(&current_scene->rootClass.inst.ref.group_identifier, &scene_id) != 0)
	{
		verbose("TransitionTo: %s", ExternalReference_name(&ref->u.external_reference));
		/* UK MHEG Profile says we don't need to support transition effects */
		/* (remember that 'to' will get destroyed when we free the action queues, so use it now if you need it) */
		if(to->have_transition_effect)
			error("Transition effects not supported");
		/* get the active app */
		current_app = MHEGEngine_getActiveApplication();
		/* check the new scene is available */
		if(!MHEGEngine_checkContentRef(&scene_id))
		{
			EventData event_tag;
			error("Unable to open '%.*s': %s", scene_id.size, scene_id.data, strerror(errno));
			/* generate a GroupIDRefError EngineEvent */
			event_tag.choice = EventData_integer;
			event_tag.u.integer = EngineEvent_GroupIDRefError;
			MHEGEngine_generateAsyncEvent(&current_app->rootClass.inst.ref, EventType_engine_event, &event_tag);
			/* clean up */
			safe_free(scene_id.data);
			return;
		}
		/*
		 * do Deactivation of all Ingredients in the current app that are not shared
		 * in the reverse order they appear in the items list
		 */
		gi = current_app->items;
		/* get the tail of the list */
		gi_tail = (current_app->items != NULL) ? current_app->items->prev : NULL;
		while(gi)
		{
			/* only do Deactivation if it is not shared */
			if(!GroupItem_isShared(&gi->item))
				GroupItem_Deactivation(&gi->item);
			/* have we reached the head */
			gi = (gi->prev != gi_tail) ? gi->prev : NULL;
		}
		/* do Deactivation and Destruction on the current scene */
		if(current_scene != NULL)
		{
			SceneClass_Deactivation(current_scene);
			SceneClass_Destruction(current_scene);
		}
		/*
		 * now the old scene is destroyed,
		 * remove its events from the Async event queue and its pending actions
		 * keep events and actions associated with the app
		 */
		app_gid = &current_app->rootClass.inst.ref.group_identifier;
		/* async event queue */
		ev = engine.async_eventq;
		while(ev)
		{
			next_ev = ev->next;
			if(OctetString_cmp(&ev->item.src.group_identifier, app_gid) != 0)
			{
				LIST_REMOVE(&engine.async_eventq, ev);
				free_MHEGAsyncEventListItem(ev);
			}
			ev = next_ev;
		}
		/* main action queue */
		act = engine.main_actionq;
		while(act)
		{
			next_act = act->next;
			if(OctetString_cmp(act->item.group_id, app_gid) != 0)
			{
				LIST_REMOVE(&engine.main_actionq, act);
				free_MHEGActionListItem(act);
			}
			act = next_act;
		}
		/* temp action queue */
		act = engine.temp_actionq;
		while(act)
		{
			next_act = act->next;
			if(OctetString_cmp(act->item.group_id, app_gid) != 0)
			{
				LIST_REMOVE(&engine.temp_actionq, act);
				free_MHEGActionListItem(act);
			}
			act = next_act;
		}
		/* load the new scene (also free's the old one if we have one) */
		if((current_scene = MHEGApp_loadScene(&engine.active_app, &scene_id)) != NULL)
		{
			/* do Preparation and Activation */
			SceneClass_Preparation(current_scene);
			SceneClass_Activation(current_scene);
		}
	}

	/* clean up */
	safe_free(scene_id.data);

	return;
}

void
MHEGEngine_quit(QuitReason reason, OctetString *data)
{
	/*
	 * tell the main loop we want to quit the current app
	 * can't just Launch the new app here cuz we may be in the middle of processing a sequence of ElementaryActions
	 * the rest of the sequence would be free'd if we destroyed the current app here
	 */
	engine.quit_reason = reason;

	/* copy not dup, in case we get called more than once */
	OctetString_copy(&engine.quit_data, data);

	/* empty the Async event queue and any pending actions */
	LIST_FREE(&engine.async_eventq, MHEGAsyncEvent, free_MHEGAsyncEventListItem);
	LIST_FREE(&engine.main_actionq, MHEGAction, free_MHEGActionListItem);
	LIST_FREE(&engine.temp_actionq, MHEGAction, free_MHEGActionListItem);

	return;
}

/*
 * it is a fatal error if no app is loaded
 */

ApplicationClass *
MHEGEngine_getActiveApplication(void)
{
	/* assert */
	if(engine.active_app.app == NULL
	|| engine.active_app.app->choice != InterchangedObject_application)
		fatal("MHEGEngine_getActiveApplication: can't find ApplicationClass");

	return &engine.active_app.app->u.application;
}

/*
 * returns NULL if no scene is loaded
 */

SceneClass *
MHEGEngine_getActiveScene(void)
{
	if(engine.active_app.scene == NULL)
		return NULL;

	/* assert */
	if(engine.active_app.scene->choice != InterchangedObject_scene)
		fatal("MHEGEngine_getActiveScene: can't find SceneClass");

	return &engine.active_app.scene->u.scene;
}

/*
 * adds the given object to the top of the DisplayStack for the active application
 * (if it is not already in the DisplayStack)
 * adds the ptr, so the data must remain valid until it is removed from the DisplayStack
 */

void
MHEGEngine_addVisibleObject(RootClass *obj)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *vis;

	/* check it is not already on the DisplayStack */
	vis = app->inst.DisplayStack;
	while(vis)
	{
		if(vis->item == obj)
			return;
		vis = vis->next;
	}

	vis = safe_malloc(sizeof(LIST_TYPE(RootClassPtr)));
	vis->item = obj;
	LIST_APPEND(&app->inst.DisplayStack, vis);

	return;
}

/*
 * removes the given object from the DisplayStack for the active application
 */

void
MHEGEngine_removeVisibleObject(RootClass *obj)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *vis;

	/* find it */
	vis = app->inst.DisplayStack;
	while(vis)
	{
		if(vis->item == obj)
		{
			LIST_REMOVE(&app->inst.DisplayStack, vis);
			safe_free(vis);
			return;
		}
		vis = vis->next;
	}

	error("Object not found on DisplayStack: %s", ExternalReference_name(&obj->inst.ref));

	return;
}

/*
 * doesn't redraw the screen
 * the end of the DisplayStack list is the top of the screen
 */

void
MHEGEngine_bringToFront(RootClass *obj)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *vis;

	/* find it on the DisplayStack */
	vis = app->inst.DisplayStack;
	while(vis && vis->item != obj)
		vis = vis->next;

	/* if it is not already at the top (ie at the tail of the list) */
	if(vis && vis->next)
	{
		LIST_REMOVE(&app->inst.DisplayStack, vis);
		LIST_APPEND(&app->inst.DisplayStack, vis);
	}

	return;
}

/*
 * doesn't redraw the screen
 * the head of the DisplayStack list is the bottom of the screen
 */

void
MHEGEngine_sendToBack(RootClass *obj)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *vis;

	/* find it on the DisplayStack */
	vis = app->inst.DisplayStack;
	while(vis && vis->item != obj)
		vis = vis->next;

	/* if it is not already at the bottom (ie at the head of the list) */
	if(vis && vis != app->inst.DisplayStack)
	{
		LIST_REMOVE(&app->inst.DisplayStack, vis);
		LIST_PREPEND(&app->inst.DisplayStack, vis);
	}

	return;
}

/*
 * doesn't redraw the screen
 * the head of the DisplayStack list is the bottom of the screen
 */

void
MHEGEngine_putBefore(RootClass *target, RootClass *ref)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *target_item, *ref_item;

	/* find target on the DisplayStack */
	target_item = app->inst.DisplayStack;
	while(target_item && target_item->item != target)
		target_item = target_item->next;
	if(target_item == NULL)
	{
		error("PutBefore: %s is not on the DisplayStack", ExternalReference_name(&target->inst.ref));
		return;
	}

	/* find ref on the DisplayStack */
	ref_item = app->inst.DisplayStack;
	while(ref_item && ref_item->item != ref)
		ref_item = ref_item->next;
	if(ref_item == NULL)
	{
		error("PutBefore: %s is not on the DisplayStack", ExternalReference_name(&ref->inst.ref));
		return;
	}

	/* remove target from the list */
	LIST_REMOVE(&app->inst.DisplayStack, target_item);

	/* insert target after ref in the list */
	LIST_INSERT_AFTER(&app->inst.DisplayStack, target_item, ref_item);

	return;
}

/*
 * doesn't redraw the screen
 * the head of the DisplayStack list is the bottom of the screen
 */

void
MHEGEngine_putBehind(RootClass *target, RootClass *ref)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(RootClassPtr) *target_item, *ref_item;

	/* find target on the DisplayStack */
	target_item = app->inst.DisplayStack;
	while(target_item && target_item->item != target)
		target_item = target_item->next;
	if(target_item == NULL)
	{
		error("PutBefore: %s is not on the DisplayStack", ExternalReference_name(&target->inst.ref));
		return;
	}

	/* find ref on the DisplayStack */
	ref_item = app->inst.DisplayStack;
	while(ref_item && ref_item->item != ref)
		ref_item = ref_item->next;
	if(ref_item == NULL)
	{
		error("PutBefore: %s is not on the DisplayStack", ExternalReference_name(&ref->inst.ref));
		return;
	}

	/* remove target from the list */
	LIST_REMOVE(&app->inst.DisplayStack, target_item);

	/* insert target before ref in the list */
	LIST_INSERT_BEFORE(&app->inst.DisplayStack, target_item, ref_item);

	return;
}

/*
 * redraw all the objects on the DisplayStack in the given area, that have RunningStatus of true
 * area should be given in MHEG coords, ie in the range  0-MHEG_XRES, 0-MHEG_YRES
 */

void
MHEGEngine_redrawArea(XYPosition *pos, OriginalBoxSize *box)
{
	ApplicationClass *app;
	LIST_TYPE(RootClassPtr) *stack;
	RootClass *obj;
	MHEGColour black;

	app = MHEGEngine_getActiveApplication();

	/* only redraw if the display is not locked */
	if(app->inst.LockCount > 0)
		return;

	/* any undrawn on background is black */
	MHEGColour_black(&black);
	MHEGDisplay_fillRectangle(&engine.display, pos, box, &black);

	/* get the display stack from the current ApplicationClass */
	stack = app->inst.DisplayStack;

	/* start at the bottom and redraw each object inside the area */
	while(stack)
	{
		obj = stack->item;
		/* only draw active objects (should all be derived from VisibleClass) */
		if(obj->inst.RunningStatus)
			VisibleClass_render(obj, &engine.display, pos, box);
		stack = stack->next;
	}

	/* use the new objects we have just drawn */
	MHEGDisplay_useOverlay(&engine.display);

	/* refresh the screen */
	MHEGDisplay_refresh(&engine.display, pos, box);

	return;
}

/*
 * adds a ptr to the given LinkClass to the active links list
 * the ptr to the LinkClass data must remain valid until it is removed with MHEGEngine_removeActiveLink()
 */

void
MHEGEngine_addActiveLink(LinkClass *link)
{
	LIST_TYPE(LinkClassPtr) *list;

	list = safe_malloc(sizeof(LIST_TYPE(LinkClassPtr)));
	list->item = link;

	LIST_APPEND(&engine.active_links, list);

	return;
}

void
MHEGEngine_removeActiveLink(LinkClass *link)
{
	LIST_TYPE(LinkClassPtr) *list = engine.active_links;

	while(list)
	{
		if(list->item == link)
		{
			LIST_REMOVE(&engine.active_links, list);
			safe_free(list);
			return;
		}
		list = list->next;
	}

	error("Active link not found: %s", ExternalReference_name(&link->rootClass.inst.ref));

	return;
}

/*
 * key should be one of the MHEGKey_xxx constants
 */

void
MHEGEngine_keyPressed(unsigned int key)
{
	/* source of EngineEvent events */
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	/* source of UserInput events */
	SceneClass *scene = MHEGEngine_getActiveScene();
	/* event data */
	EventData data;

	data.choice = EventData_integer;

	/* do we need an EngineEvent for this key */
	if(key == MHEGKey_Text)
	{
		data.u.integer = EngineEvent_TextKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_EPG)
	{
		data.u.integer = EngineEvent_EPGKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_Cancel)
	{
		data.u.integer = EngineEvent_CancelKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_Red)
	{
		data.u.integer = EngineEvent_RedKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_Green)
	{
		data.u.integer = EngineEvent_GreenKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_Yellow)
	{
		data.u.integer = EngineEvent_YellowKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}
	else if(key == MHEGKey_Blue)
	{
		data.u.integer = EngineEvent_BlueKeyFunction;
		MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &data);
	}

	/* generate the UserInput event */
	if(scene != NULL)
	{
		data.u.integer = key;
		MHEGEngine_generateAsyncEvent(&scene->rootClass.inst.ref, EventType_user_input, &data);
	}

	return;
}

/*
 * returns NULL if the file does not exist and create is false
 * if create is true and the file does not exist, a new (empty one) is created
 */

PersistentData *
MHEGEngine_findPersistentData(OctetString *filename, bool create)
{
	LIST_TYPE(PersistentData) *p = engine.persistent;

	while(p)
	{
		if(OctetString_cmp(&p->item.filename, filename) == 0)
			return &p->item;
		p = p->next;
	}

	/* not found, create it */
	if(create)
	{
		/* add it to the list */
		p = new_PersistentDataListItem(filename);
		LIST_APPEND(&engine.persistent, p);
	}

	return (p != NULL) ? &p->item : NULL;
}

/*
 * the event is processed immediately
 * if any of the active links match the event, add the link actions to the temp_actionq
 * the src should be a fully qualified reference (ie include the group identifier)
 * data (if not NULL) is used immediately, so doesn't need to remain valid after this call
 */

void
MHEGEngine_generateEvent(ExternalReference *src, EventType type, EventData *data)
{
	LIST_TYPE(LinkClassPtr) *link;
	LIST_TYPE(ElementaryAction) *link_action;
	LIST_TYPE(MHEGAction) *temp_action;
	OctetString *gid;

	verbose("Generated event: %s; %s", ExternalReference_name(src), EventType_name(type));

	/* see if any of the active links match this event */
	link = engine.active_links;
	while(link)
	{
		/* if it matches, add the actions to the temp action queue */
		if(LinkClass_conditionMet(link->item, src, type, data))
		{
			/* add a ptr to each ElementaryAction to temp_actionq */
			link_action = link->item->link_effect;
			while(link_action)
			{
				/* remember the group id of the link that caused the action */
				gid = &link->item->rootClass.inst.ref.group_identifier;
				temp_action = new_MHEGActionListItem(gid, &link_action->item);
				LIST_APPEND(&engine.temp_actionq, temp_action);
				link_action = link_action->next;
			}
		}
		link = link->next;
	}

	return;
}

/*
 * the event is added to the async_eventq
 * it will be processed (ie matched against active links) when MHEGEngine_processNextAsyncEvent() is called
 * the src should be a fully qualified reference (ie include the group identifier)
 * makes a copy of src and data so they don't need to stay valid after this call
 */

void
MHEGEngine_generateAsyncEvent(ExternalReference *src, EventType type, EventData *data)
{
	LIST_TYPE(MHEGAsyncEvent) *event;

	verbose("Generated asynchronous event: %s; %s", ExternalReference_name(src), EventType_name(type));

	event = new_MHEGAsyncEventListItem(src, type, data);

	LIST_APPEND(&engine.async_eventq, event);

	return;
}

/*
 * process all outstanding (async) events
 */

void
MHEGEngine_processMHEGEvents(void)
{
	/* assert */
	if(engine.main_actionq != NULL)
		fatal("Outstanding actions on the main action queue");

	/* process the next asynchronous event (if there is one) */
	while(engine.async_eventq)
	{
		/* adds any resulting actions to temp_actionq */
		MHEGEngine_processNextAsyncEvent();
		/* process MHEG event queue as described in UK MHEG Profile */
		engine.main_actionq = engine.temp_actionq;
		engine.temp_actionq = NULL;
		while(engine.main_actionq)
		{
			/* execute the action - adds any resulting actions to temp_actionq */
			ElementaryAction_execute(engine.main_actionq->item.action, engine.main_actionq->item.group_id);
			/* remove the action we just executed from the main_actionq */
			LIST_FREE_HEAD(&engine.main_actionq, MHEGAction, free_MHEGActionListItem);
			/* prepend any temp_actionq actions it generated to the main_actionq */
			LIST_CONCAT(&engine.temp_actionq, MHEGAction, engine.main_actionq);
			engine.main_actionq = engine.temp_actionq;
			engine.temp_actionq = NULL;
		}
	}

	return;
}

/*
 * match the head event in the async_eventq against the active links
 * if any active links fire, add the actions to the temp_actionq
 */

void
MHEGEngine_processNextAsyncEvent(void)
{
	MHEGAsyncEvent *event;

	if(engine.async_eventq)
	{
		verbose("Processing next asynchronous event");
		event = &engine.async_eventq->item;
		/* match it against any active links */
		MHEGEngine_generateEvent(&event->src, event->type, event->data);
		/* remove the event we just processed */
		LIST_FREE_HEAD(&engine.async_eventq, MHEGAsyncEvent, free_MHEGAsyncEventListItem);
	}

	return;
}

/*
 * add a ptr to each ElementaryAction to temp_actionq
 */

void
MHEGEngine_addToTempActionQ(ActionClass *action, OctetString *caller_gid)
{
	LIST_TYPE(ElementaryAction) *list = *action;
	LIST_TYPE(MHEGAction) *temp_action;

	/* add a ptr to each ElementaryAction to temp_actionq */
	while(list)
	{
		/* remember the group id of the object that caused the action */
		temp_action = new_MHEGActionListItem(caller_gid, &list->item);
		LIST_APPEND(&engine.temp_actionq, temp_action);
		list = list->next;
	}

	return;
}

/*
 * sets the group identifier that will be used in MHEGEngine_resolveDERObjectReference()
 * should be an absolute group ID, ie start with ~//
 */

void
MHEGEngine_setDERObject(OctetString *gid)
{
	/* assert */
	if(gid->size < 3 || strncmp(gid->data, "~//", 3) != 0)
		fatal("MHEGEngine_setDERObject: group ID '%.*s' is not absolute", gid->size, gid->data);

	engine.der_object = gid;

	return;
}

/*
 * converts the ObjectReference into an ExternalReference
 * gives it the group identifier that was set with MHEGEngine_setDERObject()
 * this should only be called during DER decoding, all other times use MHEGEngine_findObjectReference()
 */

void
MHEGEngine_resolveDERObjectReference(ObjectReference *ref, ExternalReference *out)
{
	/* always give it the absolute group ID set with MHEGEngine_setDERObject() */
	OctetString_dup(&out->group_identifier, engine.der_object);

	/* find the object number */
	switch(ref->choice)
	{
	case ObjectReference_internal_reference:
		out->object_number = ref->u.internal_reference;
		break;

	case ObjectReference_external_reference:
		out->object_number = ref->u.external_reference.object_number;
		break;

	default:
		fatal("Unknown ObjectReference type: %d", ref->choice);
		break;
	}

	return;
}

/*
 * stores the ptr, so it must remain valid until MHEGEngine_removeObjectReference() is called
 */

void
MHEGEngine_addObjectReference(RootClass *obj)
{
	LIST_TYPE(RootClassPtr) *list = safe_malloc(sizeof(LIST_TYPE(RootClassPtr)));

	list->item = obj;

	LIST_APPEND(&engine.objects, list);

	return;
}

void
MHEGEngine_removeObjectReference(RootClass *obj)
{
	LIST_TYPE(RootClassPtr) *list = engine.objects;

	while(list)
	{
		if(list->item == obj)
		{
			LIST_REMOVE(&engine.objects, list);
			safe_free(list);
			return;
		}
		list = list->next;
	}

	/* assert */
	fatal("ObjectReference not found: %s", ExternalReference_name(&obj->inst.ref));

	/* not reached */
	return;
}

/*
 * if the ObjectReference does not contain a group id of its own, the caller_gid will be used
 * returns NULL if the object has not been added with MHEGEngine_addObjectReference()
 */

RootClass *
MHEGEngine_findObjectReference(ObjectReference *ref, OctetString *caller_gid)
{
	LIST_TYPE(RootClassPtr) *list = engine.objects;
	OctetString *gid = NULL;	/* keep the compiler happy */
	unsigned int num = 0;		/* keep the compiler happy */
	char *fullname;
	OctetString absolute;
	RootClass *obj;

	/* find the group id we need */
	switch(ref->choice)
	{
	case ObjectReference_internal_reference:
		gid = caller_gid;
		num = ref->u.internal_reference;
		break;

	case ObjectReference_external_reference:
		gid = &ref->u.external_reference.group_identifier;
		num = ref->u.external_reference.object_number;
		break;

	default:
		fatal("Unknown ObjectReference type: %d", ref->choice);
		break;
	}

	/* get the absolute group ID */
	fullname = MHEGEngine_absoluteFilename(gid);
	absolute.size = strlen(fullname);
	absolute.data = fullname;
	gid = &absolute;

	while(list)
	{
		obj = list->item;
		if(OctetString_cmp(gid, &obj->inst.ref.group_identifier) == 0
		&& num == obj->inst.ref.object_number)
		{
			return list->item;
		}
		list = list->next;
	}

	error("ObjectReference not found: %.*s %u", gid->size, gid->data, num);

	return NULL;
}

/*
 * returns either the active Scene or the active Application
 * returns NULL if the given group ID does not match either
 * group ID should be absolute (ie start with ~//)
 */

RootClass *
MHEGEngine_findGroupObject(OctetString *gid)
{
	ApplicationClass *app;
	SceneClass *scene;

	/* assert */
	if(gid->size < 3 || strncmp(gid->data, "~//", 3) != 0)
		fatal("MHEGEngine_findGroupObject: group ID '%.*s' is not absolute", gid->size, gid->data);

	/* is it the app */
	app = MHEGEngine_getActiveApplication();
	if(OctetString_cmp(&app->rootClass.inst.ref.group_identifier, gid) == 0)
		return &app->rootClass;

	/* is it the scene */
	scene = MHEGEngine_getActiveScene();
	if(scene != NULL && OctetString_cmp(&scene->rootClass.inst.ref.group_identifier, gid) == 0)
		return &scene->rootClass;

	return NULL;
}

/*
 * returns an object number which is not used in the given GroupClass
 * group should be either an ApplicationClass or a SceneClass object
 * returns 0 if group is NULL or there are no free object numbers left (very unlikely)
 */

unsigned int
MHEGEngine_getUnusedObjectNumber(RootClass *group)
{
	LIST_TYPE(GroupItem) *items, *gi;
	unsigned int next_clone;
	bool unused;
	RootClass *r;

	if(group == NULL)
		return 0;

	/* get a ptr to the list of GroupItems */
	if(group->inst.rtti == RTTI_ApplicationClass)
	{
		items = ((ApplicationClass *) group)->items;
		next_clone = ((ApplicationClass *) group)->inst.next_clone;
	}
	else if(group->inst.rtti == RTTI_SceneClass)
	{
		items = ((SceneClass *) group)->items;
		next_clone = ((SceneClass *) group)->inst.next_clone;
	}
	else
	{
		return 0;
	}

	/* find the next unused object number starting at next_clone */
	unused = false;
	while(!unused)
	{
		next_clone ++;
		/* stop infinite loops */
		if(next_clone == 0)
			return 0;
		/* do any of the existing objects use this number */
		unused = true;
		gi = items;
		while(unused && gi)
		{
			if((r = GroupItem_rootClass(&gi->item)) != NULL)
				unused = (r->inst.ref.object_number != next_clone);
			gi = gi->next;
		}
	}

	/* save the object number search point for next time */
	if(group->inst.rtti == RTTI_ApplicationClass)
		((ApplicationClass *) group)->inst.next_clone = next_clone;
	else
		((SceneClass *) group)->inst.next_clone = next_clone;

	return next_clone;
}

/*
 * add the given file to the missing_content list
 * removes any previous missing content entry for this object
 * sets the objects need_content flag to true
 * the event loop polls for all the files in the missing_content list
 * when a file appears, the associated objects' contentAvailable() method is called
 * and a ContentAvailable event is generated
 * takes a copy of the file OctetString so it doesn't need to remain valid
 */

void
MHEGEngine_addMissingContent(RootClass *obj, OctetString *file)
{
	LIST_TYPE(MissingContent) *missing;

	/* does it have an existing entry */
	if(obj->inst.need_content)
		MHEGEngine_removeMissingContent(obj);

	/* so RootClass_Preparation wont generate a ContentAvailable event */
	obj->inst.need_content = true;

	/* add it to the list */
	missing = new_MissingContentListItem(obj, file);
	LIST_APPEND(&engine.missing_content, missing);

	return;
}

void
MHEGEngine_removeMissingContent(RootClass *obj)
{
	LIST_TYPE(MissingContent) *list = engine.missing_content;

	/* just to avoid any confusion */
	obj->inst.need_content = false;

	while(list)
	{
		if(list->item.obj == obj)
		{
			LIST_REMOVE(&engine.missing_content, list);
			free_MissingContentListItem(list);
			return;
		}
		list = list->next;
	}

	/* assert */
	fatal("MissingContent Object not found: %s", ExternalReference_name(&obj->inst.ref));

	/* not reached */
	return;
}

void
MHEGEngine_pollMissingContent(void)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	LIST_TYPE(MissingContent) *missing, *next;
	bool remove;
	struct timeval now;

	missing = engine.missing_content;
	while(missing)
	{
		remove = false;
		if(MHEGEngine_checkContentRef(&missing->item.file))
		{
			RootClass_contentAvailable(missing->item.obj, &missing->item.file);
			/* remove it from the list */
			remove = true;
		}
		else
		{
			/* has it timed out */
			gettimeofday(&now, NULL);
			/* <= means timeout=0 generates a ContentRefError immediately */
			if(missing->item.requested + engine.timeout <= now.tv_sec)
			{
				/* generate a ContentRefError EngineEvent */
				EventData event_tag;
				event_tag.choice = EventData_integer;
				event_tag.u.integer = EngineEvent_ContentRefError;
				MHEGEngine_generateAsyncEvent(&app->rootClass.inst.ref, EventType_engine_event, &event_tag);
				/* clear the need_content flag */
				missing->item.obj->inst.need_content = false;
				/* remove it from the list */
				remove = true;
			}
		}
		/* do we need to remove it */
		if(remove)
		{
			next = missing->next;
			LIST_REMOVE(&engine.missing_content, missing);
			free_MissingContentListItem(missing);
			missing = next;
		}
		else
		{
			missing = missing->next;
		}
	}

	return;
}

/*
 * returns true if the file exists on the carousel
 */

bool
MHEGEngine_checkContentRef(ContentReference *name)
{
	return (*(engine.backend.fns->checkContentRef))(&engine.backend, name);
}

/*
 * file contents are stored in out (out->data will need to be free'd)
 * returns false if it can't load the file (out will be {0,NULL})
 * out should be uninitialised before calling this
 */

bool
MHEGEngine_loadFile(OctetString *name, OctetString *out)
{
	/* in case it fails */
	out->size = 0;
	out->data = NULL;

	/* just in case */
	if(name->size == 0)
	{
		verbose("MHEGEngine_loadFile: no filename given");
		return false;
	}

	return (*(engine.backend.fns->loadFile))(&engine.backend, name, out);
}

/*
 * returns a read-only FILE handle for the given carousel file
 * returns NULL on error
 */

FILE *
MHEGEngine_openFile(OctetString *name)
{
	return (*(engine.backend.fns->openFile))(&engine.backend, name);
}

/*
 * return a read-only FILE handle for an MPEG Transport Stream
 * the TS will contain an audio stream (if have_audio is true) and a video stream (if have_video is true)
 * the *audio_tag and *video_tag numbers refer to Component/Association Tag values from the DVB PMT
 * if *audio_tag or *video_tag is -1, the default audio and/or video stream for the current Service ID is used
 * updates *audio_tag and/or *video_tag to the actual PIDs in the Transport Stream
 * returns NULL on error
 */

MHEGStream *
MHEGEngine_openStream(int service_id, bool have_audio, int *audio_tag, int *audio_type, bool have_video, int *video_tag, int *video_type)
{
	return (*(engine.backend.fns->openStream))(&engine.backend,
						   service_id,
						   have_audio, audio_tag, audio_type,
						   have_video, video_tag, video_type);
}

void
MHEGEngine_closeStream(MHEGStream *stream)
{
	return (*(engine.backend.fns->closeStream))(&engine.backend, stream);
}

/*
 * retune the backend to the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

void
MHEGEngine_retune(OctetString *service)
{
	return (*(engine.backend.fns->retune))(&engine.backend, service);
}

/*
 * return a read-only dvb:// format value for rec://svc/def
 */

const OctetString *
MHEGEngine_getRecSvcDef(void)
{
	/* ask the backend */
	return (*(engine.backend.fns->getServiceURL))(&engine.backend);
}

/*
 * return a read-only dvb:// format value for rec://svc/cur
 */

const OctetString *
MHEGEngine_getRecSvcCur(void)
{
/* TODO */
// need to keep track of this ourselves
// initially svc/cur = svc/def
// only changes if SetData called on StreamClass
// retune => reset to svc/cur = svc/def again?
printf("TODO: MHEGEngine_getRecSvcCur: returning rec://svc/def instead\n");
return (*(engine.backend.fns->getServiceURL))(&engine.backend);
}

/*
 * return true if the engine is able to receive the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

bool
MHEGEngine_isServiceAvailable(OctetString *service)
{
	/* ask the backend */
	return (*(engine.backend.fns->isServiceAvailable))(&engine.backend, service);
}

/*
 * returns the absolute group ID, ie it always starts with "~//"
 * returns a ptr to static string that will be overwritten by the next call to this routine
 * section 8.3.2 of the UK MHEG Profile says the filename prefixes are:
 * // for the root
 * / for the path to the current active app
 */

static char *active_app_path(void);

static char _absolute[PATH_MAX];

char *
MHEGEngine_absoluteFilename(OctetString *name)
{
	unsigned int size;
	unsigned char *data;

/* TODO */
/* need to cope with CI: at the start */
	if(name->size > 2 && strncmp(name->data, "CI:", 3) == 0)
	{
printf("TODO: absoluteFilename '%.*s'\n", name->size, name->data);
	}

	/* DSM: at the start is equivalent to ~ */
	if(name->size > 3 && strncmp(name->data, "DSM:", 4) == 0)
	{
		size = name->size - 4;
		data = &name->data[4];
	}
	else
	{
		size = name->size;
		data = name->data;
	}

	/* does it already start with a ~// */
	if(size > 2 && strncmp(data, "~//", 3) == 0)
		snprintf(_absolute, sizeof(_absolute), "%.*s", size, data);
	/* starting with // is the same as starting with ~// */
	else if(size > 1 && strncmp(data, "//", 2) == 0)
		snprintf(_absolute, sizeof(_absolute), "~%.*s", size, data);
	/* starting with ~/ means prepend the path to the current active app */
	else if(size > 1 && strncmp(data, "~/", 2) == 0)
		snprintf(_absolute, sizeof(_absolute), "%s%.*s", active_app_path(), size - 1, &data[1]);
	/* starting with / is the same as starting with ~/ */
	else if(size > 0 && data[0] == '/')
		snprintf(_absolute, sizeof(_absolute), "%s%.*s", active_app_path(), size, data);
	/* no / at the start, UK Profile doesn't say what to do, so prepend the path to the current active app */
	else if(size > 0)
		snprintf(_absolute, sizeof(_absolute), "%s/%.*s", active_app_path(), size, data);
	/* no name at all */
	else
		snprintf(_absolute, sizeof(_absolute), "%s/", active_app_path());

	return _absolute;
}

static char _active_app_path[PATH_MAX];

static char *
active_app_path(void)
{
	ApplicationClass *app = MHEGEngine_getActiveApplication();
	OctetString *gid = &app->rootClass.inst.ref.group_identifier;
	char *slash;

	/* assert */
	if(gid->size < 3 || strncmp(gid->data, "~//", 3) != 0)
		fatal("active_app_path: invalid group ID '%.*s'", gid->size, gid->data);

	snprintf(_active_app_path, sizeof(_active_app_path), "%.*s", gid->size, gid->data);

	/* chop off the trailing '/filename' */
	slash = strrchr(_active_app_path, '/');
	/* assert above means it will never be NULL */
	*slash = '\0';

	return _active_app_path;
}

/*
 * if have_hook is true, hook should be either ContentHook_Bitmap_MPEG or ContentHook_Bitmap_PNG
 * if have_hook is false, default hook is ContentHook_Bitmap_PNG
 * Channel 4 sometimes has the wrong content hook, so if the data has a PNG signature treat it as PNG
 */

MHEGBitmap *
MHEGEngine_newBitmap(OctetString *data, bool have_hook, int hook)
{
	MHEGBitmap *bitmap = NULL;

	if(have_hook == false
	|| (have_hook == true && hook == ContentHook_Bitmap_PNG)
	|| (data->size >= 8 && png_check_sig(data->data, 8)))
		bitmap = MHEGDisplay_newPNGBitmap(&engine.display, data);
	else if(have_hook == true && hook == ContentHook_Bitmap_MPEG)
		bitmap = MHEGDisplay_newMPEGBitmap(&engine.display, data);
	else
		error("Unknown BitmapClass content hook: %d,%d", have_hook, hook);

	return bitmap;
}

void
MHEGEngine_freeBitmap(MHEGBitmap *bitmap)
{
	MHEGDisplay_freeBitmap(&engine.display, bitmap);

	return;
}

/* stop verbose messages from different threads overlapping */
pthread_mutex_t stdout_lock = PTHREAD_MUTEX_INITIALIZER;

void
verbose(char *message, ...)
{
	va_list ap;

	if(engine.verbose)
	{
		pthread_mutex_lock(&stdout_lock);
		va_start(ap, message);
		vprintf(message, ap);
		printf("\n");
		va_end(ap);
		pthread_mutex_unlock(&stdout_lock);
	}

	return;
}

