/*
 * MHEGTimer.h
 */

#ifndef __MHEGTIMER_H__
#define __MHEGTIMER_H__

#include <sys/time.h>
#include <X11/Intrinsic.h>

typedef XtIntervalId MHEGTimer;

DEFINE_LIST_OF(MHEGTimer);

MHEGTimer MHEGTimer_addGroupClassTimer(unsigned int, ExternalReference *, int, void *);
void MHEGTimer_removeGroupClassTimer(MHEGTimer);

int time_diff(struct timeval *, struct timeval *);

#endif	/* __MHEGTIMER_H__ */

