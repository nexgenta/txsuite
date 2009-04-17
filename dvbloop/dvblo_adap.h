/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo_adap.h
 * Desc: Support for virtual DVB adapters
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

#ifndef _DVBLO_ADAP_H_
#define _DVBLO_ADAP_H_

#include <linux/types.h>

#include "dvb-core/dvbdev.h"
#include "dvb-core/dvb_demux.h"
#include "dvb-core/dmxdev.h"
#include "dvb-core/dvb_net.h"
#include "dvb-core/dvb_frontend.h"

struct dvblo_adap_statistics {
	/// Number of TS packets received on the adapter
	unsigned long ts_count;
};

/**
 * Structure that represents a virtual DVB adapter instance
 * @todo rename this to dvblo_adap
 */
struct dvblo {
	/**
	 * Level of initialization
	 * This help dvblo_destroy() to determine which things have to be 
	 * cleaned/unregistered as it is used by dvblo_init() when an error occurs
	 */
	unsigned int initlev:8;
	/// Flag that is set to 1 if this dvblo structure is completely initialized
	unsigned int initdone:1;
	/// The name of this adapter, e.g. "dvblo_adap0"
	char name[16];
	struct {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
		/* Since kernel version 2.6.12 the dvb_adapter structure has to be 
		 * embedded into our structure
		 */
		struct dvb_adapter adap;
#define DVBLO_DVB_ADAP(dvblop) (&(dvblop)->dvb.adap)
#else
		struct dvb_adapter *adap;
#define DVBLO_DVB_ADAP(dvblop) ((dvblop)->dvb.adap)
#endif
		struct dvb_device dev;
		struct dmxdev dmxdev;
		struct dvb_demux demux;
		struct dvb_net net;
		struct dvb_frontend frontend;
		/* struct dvb_frontend: tuner_priv was added in 2.6.18 */
#define FE_PRIV(fep) ((fep)->misc_priv)
#define DVBLO_DVB_ADAP_FEPRIV(dvblop) FE_PRIV(&((dvblop)->dvb.frontend))
		struct dmx_frontend hw_frontend;
		struct dmx_frontend mem_frontend;
	} dvb;
	/// count, how many times dvblo_demux_start_feed() has been called
	int feeding;
	struct semaphore sem;
	struct dvblo_adap_statistics stats;
	struct {
		struct dvb_frontend_parameters params;
		struct {
			struct dvb_frontend_parameters params;
		} tuner;
	} fe;
};

/**
 * Adapter configuration paramters
 */
struct dvblo_adap_config
{
	/// Whether a MAC address is specified by this structure
	unsigned int mac_valid:1;
	/// The MAC address of the DVB adapter (if mac_valid == 1) 
	u8 mac[6];
};

/** 
 * Creates a new virtual DVB adapter
 * @param adapnum The desired adapter number (set to -1 for automatic assignment)
 * @param cfg Adapter configuration (may be NULL)
 * @param dvblo_out A pointer to the newly allocated DVB adapter context is
 * returned via this parameter
 */
int dvblo_adap_create(int adapnum, struct dvblo_adap_config *cfg, struct dvblo **dvblo_out);
/** 
 * Destroys a virtual DVB adapter
 */
int dvblo_adap_destroy(struct dvblo *dvblo);

/**
 * Deliver TS packets to the virtual DVB adapter
 * @param dvblo The dvblo adapter context
 * @param buf Pointer to buffer containing TS packets
 * @param len Length of buf in bytes
 */
ssize_t dvblo_adap_deliver_packets(struct dvblo *dvblo, const u8 *buf, size_t len);

/**
 * Get MAC address of virtual DVB adapter
 */
int dvblo_adap_get_mac(struct dvblo *dvblo, u8 *mac_out);

#endif /* _DVBLO_ADAP_H_ */
