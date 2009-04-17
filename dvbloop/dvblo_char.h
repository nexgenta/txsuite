/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo_char.h
 * Desc: Char device support for dvblo
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

#ifndef _DVBLO_CHAR_H_
#define _DVBLO_CHAR_H_

#include "dvblo.h"
#include "dvblo_adap.h"

/**
 * Maximum number of devices
 */
#define DVBLO_CHAR_DEVMAX 4

struct dvblo_chardev_config
{
	/// The configuration for the corresponding virtual DVB adapter
	struct dvblo_adap_config dvbcfg;
};

int dvblo_char_init(void);
int dvblo_char_exit(void);

int dvblo_char_add_dev(struct dvblo_chardev_config *cfg, unsigned int *devnum_out);
int dvblo_char_del_dev(unsigned int devnum);

#endif /* _DVBLO_CHAR_H_ */
