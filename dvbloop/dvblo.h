/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo.h
 * Desc: Common Header File
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

#ifndef _DVBLO_H_
#define _DVBLO_H_

#include <linux/stringify.h>

#define DVBLO_NAME	"dvblo"

#define DVBLO_LONGMANE	"DVB Loopback Adapter"

#define DVBLO_DEVMAX 4

#define DVBLO_TS_SZ 188

#define SUCCESS 0

/* DVBLO_DEFINE_GLOBALS is defined by the file which defines the global
 * variables, which is usally dvblo.c.
 */
#ifndef DVBLO_DEFINE_GLOBALS
/* defined in dvblo.c */
extern unsigned int dvblo_debug;
#endif

#define DVBLO_DEBUG_LEVELS 3

#define DBGLEV_ADAP	DVBLO_DEBUG_LEVELS
#define DBGLEV_ADAP_FE	(DBGLEV_ADAP+DVBLO_DEBUG_LEVELS)
#define DBGLEV_CHAR	(DBGLEV_ADAP_FE+DVBLO_DEBUG_LEVELS)

#define DBGLEV_ALL	0
#define DBGLEV_1 	(1<<0)
#define DBGLEV_2	(1<<1)
#define DBGLEV_3	(1<<2)

#define dprintk(level,args...) \
	do { if ((dvblo_debug & level) == level) { printk(KERN_DEBUG "%s: %s(): ", __stringify(KBUILD_MODNAME), __FUNCTION__); printk(args); } } while (0)

#define mprintk(level, args...) \
	do { printk(level "%s: %s(): ", __stringify(KBUILD_MODNAME), __FUNCTION__); printk(args); } while (0)

#endif /* _DVBLO_H_ */
