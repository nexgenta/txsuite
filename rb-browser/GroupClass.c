/*
 * GroupClass.c
 */

#include <stdbool.h>

#include "MHEGEngine.h"
#include "MHEGTimer.h"
#include "GroupClass.h"
#include "GenericInteger.h"
#include "GenericBoolean.h"
#include "ExternalReference.h"

void
GroupClass_SetTimer(ExternalReference *ref, LIST_OF(Timer) **timer_list, LIST_OF(MHEGTimer) **removed_timers,
		    struct timeval *start_time, SetTimer *params, OctetString *caller_gid)
{
	int id;
	LIST_TYPE(Timer) *timer;
	LIST_TYPE(MHEGTimer) *old_timer;
	int value;
	bool absolute;
	int interval;
	struct timeval now;

	/* find the timer */
	id = GenericInteger_getInteger(&params->timer_id, caller_gid);
	timer = *timer_list;
	while(timer && timer->item.id != id)
		timer = timer->next;

	/* is there a new timer value */
	if(params->have_new_timer)
	{
		value = GenericInteger_getInteger(&params->new_timer.timer_value, caller_gid);
		/* are we updating an existing timer */
		if(timer != NULL)
		{
			/* changing a timer does not surpress events from the previous timer */
			/* cannot change the absolute_time attribute of an existing timer */
			/* absolute time is the time since we generated an IsRunning event */
			if(timer->item.absolute_time)
			{
				gettimeofday(&now, NULL);
				interval = time_diff(&now, start_time) + value;
			}
			else
			{
				interval = value;
			}
			/* should it have already gone off */
			if(interval < 0)
				interval = 0;
			timer->item.position = value;
			/*
			 * don't want timers going off after the GroupClass has been deleted
			 * so, remember the old mheg_id
			 */
			old_timer = safe_malloc(sizeof(LIST_TYPE(MHEGTimer)));
			old_timer->item = timer->item.mheg_id;
			LIST_APPEND(removed_timers, old_timer);
			/* update its mheg_id */
			timer->item.mheg_id = MHEGTimer_addGroupClassTimer(interval, ref, timer->item.id, timer_list);
		}
		else
		{
			/* new timer */
			if(params->new_timer.have_absolute_time)
				absolute = GenericBoolean_getBoolean(&params->new_timer.absolute_time, caller_gid);
			else
				absolute = false;
			/* absolute time is the time since we generated an IsRunning event */
			if(absolute)
			{
				gettimeofday(&now, NULL);
				interval = time_diff(&now, start_time) + value;
			}
			else
			{
				interval = value;
			}
			/* should it have already gone off */
			if(interval < 0)
				interval = 0;
			timer = safe_malloc(sizeof(LIST_TYPE(Timer)));
			timer->item.id = id;
			timer->item.position = value;
			timer->item.absolute_time = absolute;
			timer->item.mheg_id = MHEGTimer_addGroupClassTimer(interval, ref, id, timer_list);
			LIST_APPEND(timer_list, timer);
		}
	}
	else
	{
		/* no new timer value => remove the timer */
		if(timer != NULL)
		{
			/* remember the old mheg_id */
			old_timer = safe_malloc(sizeof(LIST_TYPE(MHEGTimer)));
			old_timer->item = timer->item.mheg_id;
			LIST_APPEND(removed_timers, old_timer);
			/* removing a timer does not surpress events from the previous timer */
			LIST_REMOVE(timer_list, timer);
			safe_free(timer);
		}
	}

	return;
}

/*
 * called when a timer goes off
 */

void
GroupClass_timerFired(ExternalReference *ref, int id, MHEGTimer mheg_id, LIST_OF(Timer) **timer_list)
{
	LIST_TYPE(Timer) *timer;

	verbose("GroupClass: %s; Timer %d fired", ExternalReference_name(ref), id);

	/* remove it from the list */
	timer = *timer_list;
	while(timer)
	{
		/*
		 * will not be removed if it has been changed
		 * in this case the mheg_id will not match
		 */
		if(timer->item.id == id
		&& timer->item.mheg_id == mheg_id)
		{
			LIST_REMOVE(timer_list, timer);
			safe_free(timer);
			return;
		}
		timer = timer->next;
	}

	return;
}

void
GroupClass_freeTimers(LIST_OF(Timer) **timer_list, LIST_OF(MHEGTimer) **removed_timers)
{
	LIST_TYPE(Timer) *timer = *timer_list;
	LIST_TYPE(MHEGTimer) *old_timer = *removed_timers;

	while(timer)
	{
		MHEGTimer_removeGroupClassTimer(timer->item.mheg_id);
		timer = timer->next;
	}

	LIST_FREE(timer_list, Timer, safe_free);

	/* remove the timers that have been updated or removed */
	while(old_timer)
	{
		MHEGTimer_removeGroupClassTimer(old_timer->item);
		old_timer = old_timer->next;
	}

	LIST_FREE(removed_timers, MHEGTimer, safe_free);

	return;
}

