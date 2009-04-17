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

#ifndef _DVBLO_IOCTL_H_
#define _DVBLO_IOCTL_H_

#include <linux/ioctl.h>

/**
 * Maximum number of devices
 */
#define DVBLO_IOC_MAGIC		'd'

struct dvblo_ioc_dev
{
	/// The MAC address of the virtual DVB adapter
	u8 mac[6];
	/**
	 * This is set to the number of the new device when ioctl(DVBLO_IOCADDDEV)
	 * was successful.
	 * @note This corresponds to the minor device number.
	 */
	int num;
};

/**
 * @brief Add a new DVBLoop adapter device
 */
#define DVBLO_IOCADDDEV		_IO(DVBLO_IOC_MAGIC, 1, struct dvblo_ioc_dev)
/**
 * @brief Remove the DVBLoop adapter device with the specified number
 */
#define DVBLO_IOCDELDEV		_IO(DVBLO_IOC_MAGIC, 1, unsigned int)


#endif /* _DVBLO_IOCTL_H_ */
