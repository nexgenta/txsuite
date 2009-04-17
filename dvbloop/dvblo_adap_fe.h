/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo_adap.c
 * Desc: Support for virtual DVB adapters - Frontend implementation
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

#ifndef _DVBLO_ADAP_FE_H_
#define _DVBLO_ADAP_FE_H_

#include "dvb-core/dvbdev.h"
#include "dvb-core/dvb_demux.h"
#include "dvb-core/dmxdev.h"
#include "dvb-core/dvb_net.h"
#include "dvb-core/dvb_frontend.h"

extern struct dvb_frontend_ops dvblo_adap_fe_ops;

#endif /* _DVBLO_ADAP_FE_H_ */
