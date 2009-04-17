/*
 * MHEGTimer.c
 */

#include "MHEGEngine.h"
#include "MHEGTimer.h"
#include "GroupClass.h"

/*
 * timer callback
 */

typedef struct
{
	ExternalReference *ref;		/* object that contains the timer */
	int id;				/* timer ID */
	void *fired_data;		/* passed onto GroupClass_timerFired after the event has been generated */
} TimerCBData;

static void
timer_cb(XtPointer usr_data, XtIntervalId *id)
{
	TimerCBData *data = (TimerCBData *) usr_data;
	EventData event_data;
	XEvent ev;
	MHEGDisplay *d = MHEGEngine_getDisplay();

	/* generate a TimerFired event */
	event_data.choice = EventData_integer;
	event_data.u.integer = data->id;
	MHEGEngine_generateAsyncEvent(data->ref, EventType_timer_fired, &event_data);

	/* let the object do any additional cleaning up */
	GroupClass_timerFired(data->ref, data->id, *id, data->fired_data);

	safe_free(data);

	/*
	 * a timer going off does not get us out of a block in XtAppNextEvent
	 * but we need to process the async event we just generated
	 * we could just call MHEGEngine_processMHEGEvents() here
	 * but if processing that means we want to Launch, Retune etc we will not be able to do it until XtAppNextEvent exits
	 * so generate a fake event here, just to end XtAppNextEvent and get back to the engine main loop
	 */
	ev.xexpose.type = Expose;
	ev.xexpose.display = d->dpy;
	ev.xexpose.window = d->win;
	ev.xexpose.x = 0;
	ev.xexpose.y = 0;
	ev.xexpose.width = 0;
	ev.xexpose.height = 0;
	ev.xexpose.count = 0;
	XSendEvent(d->dpy, d->win, False, 0, &ev);

	return;
}

/*
 * generate a TimerFired event after interval milliseconds
 * the source of the event will be the given ExternalReference
 * the event data will be the given timer ID
 * also calls GroupClass_timerFired after generating the event and passes fired_data to it
 */

MHEGTimer
MHEGTimer_addGroupClassTimer(unsigned int interval, ExternalReference *ref, int id, void *fired_data)
{
	MHEGTimer xtid;
	TimerCBData *data = safe_malloc(sizeof(TimerCBData));

	data->ref = ref;
	data->id = id;
	data->fired_data = fired_data;

	xtid = XtAppAddTimeOut(MHEGEngine_getDisplay()->app, interval, timer_cb, (XtPointer) data);

	return xtid;
}

void
MHEGTimer_removeGroupClassTimer(MHEGTimer id)
{
	XtRemoveTimeOut(id);
}

/*
 * returns the number of milliseconds between t1 and t0
 */

int
time_diff(struct timeval *t1, struct timeval *t0)
{
	return ((t1->tv_sec - t0->tv_sec) * 1000) + ((t1->tv_usec - t0->tv_usec) / 1000);
}

