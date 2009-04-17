/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo_adap.c
 * Desc: Support for virtual DVB adapters
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

/* avoid definition of __module_kernel_version in the resulting object file */
#define __NO_VERSION__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>

#include "dvblo.h"
#include "dvblo_adap.h"
#include "dvblo_adap_fe.h"

#define DBGLEV_ADAP1	(DBGLEV_1<<DBGLEV_ADAP)
#define DBGLEV_ADAP2	(DBGLEV_2<<DBGLEV_ADAP)
#define DBGLEV_ADAP3	(DBGLEV_3<<DBGLEV_ADAP)

/* -- Functions offered to others  -- */

int dvblo_adap_get_mac(struct dvblo *dvblo, u8 *mac_out)
{
	int rv = SUCCESS;
	dprintk(DBGLEV_ADAP3, "[%s] dvblo=%p, mac_out=%p\n", dvblo->name, dvblo, mac_out);
	if(dvblo == NULL || mac_out == NULL || dvblo->initdone == 0)
		rv = -EINVAL;
	else 
		memcpy(mac_out, DVBLO_DVB_ADAP(dvblo)->proposed_mac, 6);
	return rv;
}

ssize_t dvblo_adap_deliver_packets(struct dvblo *dvblo, const u8 *buf, size_t len)
{
	ssize_t rv = 0;
	if(dvblo == NULL || buf == NULL || len == 0)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo->sem))
		rv = -ERESTARTSYS;
	else {
		dprintk(DBGLEV_ADAP3, "[%s] dvblo=%p, buf=%p, len=%u\n", dvblo->name, dvblo, buf, len);
		if(dvblo->feeding > 0) {
			dvb_dmx_swfilter_packets(&dvblo->dvb.demux, buf, len / DVBLO_TS_SZ);
			dvblo->stats.ts_count += len / DVBLO_TS_SZ;
		}
		rv = len;
		// else: discard these TS packets
		up(&dvblo->sem);
	}
	return rv;
}
	
/* -- DVB Demux Callbacks -- */

/* called by dmx_ts_feed_start_filtering() and dmx_section_feed_start_filtering() */
static int dvblo_demux_start_feed(struct dvb_demux_feed *feed)
{
	int rv = SUCCESS;
	struct dvb_demux *demux;
	struct dvblo *dvblo;
	if (feed == NULL || (demux = feed->demux) == NULL 
		|| (dvblo = (struct dvblo*) demux->priv) == NULL)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo->sem))
		rv = -ERESTARTSYS;
	else {
		dprintk(DBGLEV_ADAP2, "[%s] feed=%p, demux=%p, dvblo=%p\n", dvblo->name, feed, demux, dvblo);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
		feed->pusi_seen = 0; /* have a clean section start */
#endif
		dvblo->feeding++;
		rv = dvblo->feeding;
		up(&dvblo->sem);
	}
	return rv;
}

/* called by dmx_ts_feed_stop_filtering() and dmx_section_feed_stop_filtering() */
static int dvblo_demux_stop_feed(struct dvb_demux_feed *feed)
{
	int rv = SUCCESS;
	struct dvb_demux *demux;
	struct dvblo *dvblo;
	if (feed == NULL || (demux = feed->demux) == NULL 
		|| (dvblo = (struct dvblo*) demux->priv) == NULL)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo->sem))
		rv = -ERESTARTSYS;
	else {
		dprintk(DBGLEV_ADAP2, "[%s] feed=%p, demux=%p, dvblo=%p\n", dvblo->name, feed, demux, dvblo);
		dvblo->feeding--;
		rv = dvblo->feeding;
		up(&dvblo->sem);
	}
	return rv;
}

/* -- Functions for creating/destroying virtual DVB adapters -- */

/**
 * @note This function can cope with partially initialized dvblo structures
 * by inspecting the init_level member.
 * This is necessary because dvblo_destroy() is used by dvb_init() in case of an
 * error during initialization
 */
int dvblo_adap_destroy(struct dvblo *dvblo)
{
	int rv = SUCCESS, step, i;
	if(dvblo != NULL) {
		if(dvblo->initdone != 0)
			dprintk(DBGLEV_ALL, "destroying virtual DVB adapter: %s\n", dvblo->name);
		/* cleanup dvblo structure
		 * We rollback all init steps starting with the last one.
		 */
		for(step=dvblo->initlev, i=0; step > 0; i=0, step--)
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
					///@todo do have to cleanup the mutex?
					break;
				case 2:
					i = dvb_unregister_adapter(DVBLO_DVB_ADAP(dvblo));
					break;
				case 3:
					i = dvb_unregister_frontend(&dvblo->dvb.frontend);
					break;
				case 4:
					dvb_dmx_release(&dvblo->dvb.demux);
					break;
				case 5:
					dvb_dmxdev_release(&dvblo->dvb.dmxdev);
					break;
				case 6:
					i = dvblo->dvb.demux.dmx.remove_frontend(&dvblo->dvb.demux.dmx, &dvblo->dvb.hw_frontend);
					break;
				case 7:
					i = dvblo->dvb.demux.dmx.remove_frontend(&dvblo->dvb.demux.dmx, &dvblo->dvb.mem_frontend);
					break;
				case 8:
					i = dvblo->dvb.demux.dmx.disconnect_frontend(&dvblo->dvb.demux.dmx);
					break;
				case 9:
					dvb_net_release(&dvblo->dvb.net);
					break;
				default:
					printk(KERN_ERR "%s: Oops! Invalid init step: %i\n", __FUNCTION__, i);
					break;
			}
			if(i < 0) {
				printk(KERN_ALERT "%s: cleanup of init step %i has failed. Continuing cleanup process. Expect more errors...\n", __FUNCTION__, step);
				if(rv == SUCCESS)
					rv = i;
			}
		}
		kfree(dvblo);
	} else {
		printk(KERN_ALERT "%s: Oops! Got NULL as dvblo argument. That's not a good sign...\n", __FUNCTION__);
		rv = -EINVAL;
	}
	return rv;
}

/** 
 * @todo support automatic selection of DVB adapter number
 * if adapnum param is negative.
 * @todo check if the requested adapter number (adapnum) is 
 * available
 */
int dvblo_adap_create(int adapnum, struct dvblo_adap_config *cfg, struct dvblo **dvblo_out) 
{
	int rv = SUCCESS, i;
	struct dvblo *dvblo = NULL;
	do {
		if(adapnum < 0 || adapnum > DVBLO_DEVMAX || dvblo_out == NULL) {
			rv = -EINVAL;
			break;
		}
		dvblo = kmalloc(sizeof(*dvblo), GFP_KERNEL);
		if(dvblo == NULL) {
			rv = -ENOMEM;
			break;
		}
		memset(dvblo, 0, sizeof(*dvblo));
		
		init_MUTEX(&dvblo->sem);
		dvblo->initlev++; /* 1 */
		
		adapnum = 0;
		i = snprintf(dvblo->name, sizeof(dvblo->name), DVBLO_NAME "_adap%d", adapnum);
		if(i < 0 || i >= sizeof(dvblo->name)) {
			if(i < 0)
				rv = i;
			else
				rv = -ENOBUFS;
			break;
		}
		
		dprintk(2, "creating virtual DVB adapter %s...\n", dvblo->name);

		/* returns the adapter number (>= 0) or an error (< 0) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
		/* since kernel version 2.6.18 dvb_register_adapter() has a fourth argument: device */
		/**@todo do we need to supply a value for <device>? */
		i = dvb_register_adapter(&dvblo->dvb.adap, (const char*) dvblo->name, THIS_MODULE, NULL);
#else
		/* NOTE: 
		   The API changed in 2.6.12:
		   	"Modified dvb_register_adapter() to avoid kmalloc/kfree.  Drivers have to embed
			struct dvb_adapter into their private data struct from now on."
		   So now the first paramter of dvb_register_adapter() is a pointer to a struct.
		   In previous versions (<2.6.12) the dvb_adapter structure was kmalloc'ed in 
		   dvb_register_adapter().
		   We do not care about this change here because we always supply
		   a pointer to dvblo->dvb.adap as the first argument.
		   However, in the declaration of struct dvblo, we have to take this into account of course. 
		 */
		i = dvb_register_adapter(&dvblo->dvb.adap, (const char*) dvblo->name, THIS_MODULE);
#endif
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to register virtual DVB adapter\n", __FUNCTION__);
			rv = i; 
			break;
		}
		dvblo->initlev++; /* 2 */
		
		// initialize proposed MAC address at dvblo->dvb.adap.proposed_mac
		if(cfg != NULL && cfg->mac_valid != 0)
			memcpy(DVBLO_DVB_ADAP(dvblo)->proposed_mac, cfg->mac, 6);

		dprintk(DBGLEV_ADAP2, "initializing DVB frontend...\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)		
		dvblo->dvb.frontend.ops = dvblo_adap_fe_ops;
#else
		dvblo->dvb.frontend.ops = &dvblo_adap_fe_ops;
#endif
		FE_PRIV(&dvblo->dvb.frontend) = dvblo;
		/* Note: dvblo->dvb.frontend.frontend_priv is used internally by the DVB
		 * subsystem to hold the context for the frontend device.
		 */
		i = dvb_register_frontend(DVBLO_DVB_ADAP(dvblo), &dvblo->dvb.frontend);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to initialize DVB frontend\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 3 */

		dprintk(DBGLEV_ADAP2, "initializing DVB demux...\n");
		
		dvblo->dvb.demux.priv = (void*) dvblo;
		dvblo->dvb.demux.filternum = 256;
		dvblo->dvb.demux.feednum = 256;
		dvblo->dvb.demux.start_feed = dvblo_demux_start_feed;
		dvblo->dvb.demux.stop_feed = dvblo_demux_stop_feed;
		dvblo->dvb.demux.write_to_decoder = NULL;
		dvblo->dvb.demux.dmx.capabilities = (DMX_TS_FILTERING | DMX_SECTION_FILTERING |
					      DMX_MEMORY_BASED_FILTERING);
	
		i = dvb_dmx_init(&dvblo->dvb.demux);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to initialize DVB demux\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 4 */
		
		dvblo->dvb.dmxdev.filternum = 256;
		dvblo->dvb.dmxdev.demux = &dvblo->dvb.demux.dmx;
		dvblo->dvb.dmxdev.capabilities = 0;
	
		dprintk(DBGLEV_ADAP2, "initializing DVB demux device...\n");
		
		i = dvb_dmxdev_init(&dvblo->dvb.dmxdev, DVBLO_DVB_ADAP(dvblo));
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to initialize DVB demux device\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 5 */
		
		dprintk(DBGLEV_ADAP2, "adding hardware frontend to demux...\n");
		
		///@todo check if hw frontend is needed
		dvblo->dvb.hw_frontend.source = DMX_FRONTEND_0;
		i = dvblo->dvb.demux.dmx.add_frontend(&dvblo->dvb.demux.dmx, &dvblo->dvb.hw_frontend);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to add HW frontend device\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 6 */
		
		dprintk(DBGLEV_ADAP2, "adding memory frontend to demux...\n");
		
		dvblo->dvb.mem_frontend.source = DMX_MEMORY_FE;
		i = dvblo->dvb.demux.dmx.add_frontend(&dvblo->dvb.demux.dmx, &dvblo->dvb.mem_frontend);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to add memory frontend device\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 7 */
	
		dprintk(DBGLEV_ADAP2, "connecting frontend to demux...\n");
		
		i = dvblo->dvb.demux.dmx.connect_frontend(&dvblo->dvb.demux.dmx, &dvblo->dvb.hw_frontend);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to connect HW frontend device\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 8 */
	
		dprintk(DBGLEV_ADAP2, "initializing dvb net...\n");
		
		i = dvb_net_init(DVBLO_DVB_ADAP(dvblo), &dvblo->dvb.net, &dvblo->dvb.demux.dmx);
		if(i < 0)
		{
			printk (KERN_ERR "%s: failed to initialize DVB net\n", __FUNCTION__);
			rv = i;
			break;
		}
		dvblo->initlev++; /* 9 */
		
		/* success */
		dvblo->initdone = 1;
		dprintk(DBGLEV_ALL, "created new virtual DVB adapter: %s\n", dvblo->name);
	} while(0);
	if(rv != 0) {
		if(dvblo != NULL) {
			if(dvblo_adap_destroy(dvblo) < 0)
				printk(KERN_ALERT "%s: failed in error cleanup\n", __FUNCTION__);
			dvblo = NULL;
		}
	}
	*dvblo_out = dvblo;
	return rv;
}

