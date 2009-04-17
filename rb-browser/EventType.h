/*
 * EventType.h
 */

#ifndef __EVENTTYPE_H__
#define __EVENTTYPE_H__

#include <stdbool.h>

#include "ISO13522-MHEG-5.h"

char *EventType_name(EventType);

bool EventType_isAsync(EventType);

#endif	/* __EVENTTYPE_H__ */
