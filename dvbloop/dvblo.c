/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo.c
 * Desc: This is the module core
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */
 
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/errno.h> 
#include <linux/stat.h>

#define DVBLO_DEFINE_GLOBALS 1

#include "dvblo.h"
#include "dvblo_char.h"
#include "dvblo_util.h"

static unsigned int dvblo_devcount = 1;
static char *dvblo_dvb_macaddrv[DVBLO_DEVMAX];
static int dvblo_dvb_macaddrc = 0;
// dvblo_debug is a global symbol (should be available in all .c files)
unsigned int dvblo_debug = 0;

module_param_named(devcount, dvblo_devcount, uint, S_IRUGO);
MODULE_PARM_DESC(devcount, "The initial number of virtual DVB adapters");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
module_param_array_named(macaddrs, dvblo_dvb_macaddrv, charp, &dvblo_dvb_macaddrc, S_IRUGO);
#else
module_param_array_named(macaddrs, dvblo_dvb_macaddrv, charp, dvblo_dvb_macaddrc, S_IRUGO);
#endif
MODULE_PARM_DESC(macaddrs, "A (comma-separated) list of MAC addresses assigned to the virtual DVB adapters");

module_param_named(debug, dvblo_debug, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(debug, "The debug level. Higher values will generate more debug info");

EXPORT_SYMBOL_GPL(dvblo_devcount);
EXPORT_SYMBOL_GPL(dvblo_dvb_macaddrv);
EXPORT_SYMBOL_GPL(dvblo_dvb_macaddrc);
EXPORT_SYMBOL_GPL(dvblo_debug);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Christian Praehauser");
MODULE_DESCRIPTION("DVB loopback device");
MODULE_VERSION("0.1");

/// Module initialization level
static int initlev = 0;

/**
 * @note This function can cope with a partially initialized dvblo module
 * by inspecting the init_level member.
 * This is necessary because dvblo_cleanup() is used by dvb_init() in case of an
 * error during initialization
 */
static int dvblo_cleanup(void)
{
	int rv = SUCCESS, step, i = 0;
	/* cleanup dvblo module
	 * We rollback all init steps starting with the last one.
	 */
	for(step=initlev; step > 0; i=0, step--)
	{
		/* The items of the switch statement resemble the initialization
		 * steps which were taken in dvblo_init()
		 *
		 * Yeah, I know it would be elegent to reverse the order of the
		 * switch entries (the highest init step at the top) and remove
		 * all break statements, so that when entering one init level (say 6)
		 * all other levels (below it, i.e. 5,4,3,2,1) are also processed by falling-through.
		 *
		 * The reason why we (un)do every step in a for loop is that
		 * we can easily check for errors (using the variable i) and
		 * report the step which failed and then continue (although it 
		 * might be dangerous).
		 */
		switch(step)
		{
			case 1:
				i = dvblo_char_exit();
				break;
			default:
				mprintk(KERN_ERR, "Oops! Invalid init step: %i\n", i);
				break;
		}
		if(i < 0) {
			mprintk(KERN_ALERT, "cleanup of init step %i has failed. Continuing cleanup process. Expect more errors...\n", step);
			if(rv == SUCCESS)
				rv = i;
		}
	}
	return rv;
}

static int __init dvblo_init(void) 
{
	int rv = SUCCESS, i;
	unsigned int devnum;
	struct dvblo_chardev_config devcfg;
	do {
		rv = dvblo_char_init();
		if(rv < 0)
		{
			mprintk(KERN_ERR, "failed to initialize char driver\n");
			break;
		}
		initlev++;
		
		if(dvblo_devcount > 0) {
			/* open initial number of dvbloop devices */
			dprintk(DBGLEV_1, "adding %i " DVBLO_NAME " device%s...\n",
				dvblo_devcount, dvblo_devcount == 1 ? "" : "s"); 
			for(i=0; i<dvblo_devcount && rv == SUCCESS; i++) {
				if(dvblo_dvb_macaddrc > i && dvblo_dvb_macaddrv[i] != NULL) {
					/* MAC address was specified for this device */
					rv = dvblo_parse_mac(dvblo_dvb_macaddrv[i], devcfg.dvbcfg.mac);
					if(rv < 0)
					{
						mprintk(KERN_ERR, "invalid MAC address: \"%s\"\n", dvblo_dvb_macaddrv[i]);
						break;
					}
					else {
						dprintk(DBGLEV_1, "Using MAC address %s for DVB adapter %d\n", dvblo_dvb_macaddrv[i], i);
						devcfg.dvbcfg.mac_valid = 1;
					}
				} else
					devcfg.dvbcfg.mac_valid = 0;
				rv = dvblo_char_add_dev(&devcfg, &devnum);
			}
			if(rv != SUCCESS) {
				mprintk(KERN_ERR, "FAILED to add DVB adapter %d\n", i);
				break;
			}
		}
		
		/* success */
		mprintk(KERN_INFO, "successfully initialized " DVBLO_LONGMANE "\n");
	} while(0);
	if(rv != SUCCESS) {
		mprintk(KERN_ERR, "FAILED to initialize " DVBLO_LONGMANE "\n");
		i = dvblo_cleanup();
		if(i < 0)
			mprintk(KERN_ALERT, "dvblo_cleanup() returned %d\n", i);
	}
	return rv;
}

static void __exit dvblo_exit(void)
{
	int i;
	i = dvblo_cleanup();
	if(i < 0)
		mprintk(KERN_ALERT, "dvblo_cleanup() returned %d\n", i);
}

module_init(dvblo_init);
module_exit(dvblo_exit);

