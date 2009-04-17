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

#ifndef _DVBLO_UTIL_H_
#define _DVBLO_UTIL_H_

#include <linux/types.h>

int dvblo_parse_mac(const char *macstr, u8 *mac_out);

#if 0
/**
 * Ring buffer implementation
 * @todo maybe use kfifo which is provided by Linux kernels >= 2.6.10
 */
struct dvblo_ringbuf {
	u8 *buf;
	size_t size;
	unsigned int wr;
	unsigned int rd;
};

typedef struct dvblo_ringbuf dvblo_ringbuf_t;

static inline int dvblo_rb_alloc(size_t size, dvblo_ringbuf_t *rb_out)
{
	rb_out->buf = kmalloc(size, GFP_KERNEL);
	if(rb_out->buf == NULL)
		return -ENOMEM;
	else {
		rb_out->size = size;
		rb_out->in = rb_out->out = 0;
	}
	return 0;
}

static inline ssize_t dvblo_rb_write(dvblo_ringbuf_t *rb_out, 
#endif

#endif /* _DVBLO_UTIL_H_ */
