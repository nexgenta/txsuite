/* dvbloop - A DVB Loopback Device
 * Copyright (C) 2006 Christian Praehauser
 -----------------------------------------
 * File: dvblo_char.c
 * Desc: Char device support for dvblo
 * Date: October 2006
 * Author: Christian Praehauser <cpreahaus@cosy.sbg.ac.at>
 *
 * This file is released under the GPLv2.
 */

/**
 * @todo Add sort of control socket which enables userspace programs to 
 * control the creation and deletion of DVB loopback devices.
 */

/* avoid definition of __module_kernel_version in the resulting object file */
#define __NO_VERSION__

/**
 * Whether to use the new char device interface provided since the 2.6 versions
 * of the Linux kernel
 */
#define USE_CDEV 1

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/ctype.h>
#if defined(USE_CDEV) && USE_CDEV != 0
#include <linux/cdev.h>
#endif
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include "dvblo.h"
#include "dvblo_adap.h"
#include "dvblo_char.h"
#include "dvblo_ioctl.h"

#define DBGLEV_CHAR1	(DBGLEV_1<<DBGLEV_CHAR)
#define DBGLEV_CHAR2	(DBGLEV_2<<DBGLEV_CHAR)
#define DBGLEV_CHAR3	(DBGLEV_3<<DBGLEV_CHAR)

/**
 * Context structure for the char device
 */
struct dvblo_chardev {
	/// Whether this device is used
	unsigned int used:1;
	// The minor device number of this char device
	unsigned int minor:7;
	/// Set to 1 if this device is completely initialized and ready to use
	unsigned int initdone:1;
	unsigned int initlev:7;
	/// Number of open file handles
	unsigned int hcount;
	/// Maximum number of open file handles allowed. 0 means "no limit"
	unsigned int hcountmax;
	struct class_device *clsdev;
	struct dvblo *dvblo;
	struct {
		/// Size of TS packets (i.e. L2-framing mode, 188 or 204)
		unsigned short ts_sz;
		/// The default per-handle write buffer size (in bytes)
		size_t hwrbuf_sz;
	} cfg;
	/// The device name
	char name[16];
};

/**
 * Context structure for open file handles
 */
struct dvblo_chardev_handle {
	unsigned int initlev:7;
	unsigned int initdone:1;
	struct dvblo_chardev *chardev;
	struct {
		u8 *base;
		size_t size;
		unsigned int wr;
		unsigned int rd;
	} buf;
};

/// The major device number (0 means dynamic allocation)
static int dvblo_char_major = 0;
#if defined(USE_CDEV) && USE_CDEV != 0
static struct cdev dvblo_char_cdev;
#endif

/// Array of char devices
static struct dvblo_chardev chardevs[DVBLO_CHAR_DEVMAX];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
static ssize_t clsdev_op_show_dvb_mac(struct class_device *dev, char *buf);
static ssize_t clsdev_op_show_ts_sz(struct class_device *dev, char *buf);
static ssize_t clsdev_op_store_ts_sz(struct class_device *dev, const char *buf, size_t count);
static ssize_t clsdev_op_show_hwrbuf_sz(struct class_device *dev, char *buf);
static ssize_t clsdev_op_store_hwrbuf_sz(struct class_device *dev, const char *buf, size_t count);

static struct class_device_attribute clsdev_attrs[] = {
	{{"mac" /*name*/, THIS_MODULE /*owner*/, S_IRUGO /*mode*/} /*attr*/, clsdev_op_show_dvb_mac /*show*/, NULL /*store*/},
	{{"ts_sz" /*name*/, THIS_MODULE /*owner*/, S_IRUGO|S_IWUSR /*mode*/} /*attr*/, clsdev_op_show_ts_sz /*show*/, clsdev_op_store_ts_sz /*store*/},
	{{"hwrbuf_sz" /*name*/, THIS_MODULE /*owner*/, S_IRUGO|S_IWUSR /*mode*/} /*attr*/, clsdev_op_show_hwrbuf_sz /*show*/, clsdev_op_store_hwrbuf_sz /*store*/}
};

static struct class *dvblo_class = NULL;
#else
/* The old-style "simple" class API */
static struct class_simple *dvblo_class = NULL;
#endif

static int initlev = 0;

DECLARE_MUTEX(dvblo_char_chardevs_sem);
DECLARE_WAIT_QUEUE_HEAD(dvblo_char_chardevs_wq);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
static ssize_t clsdev_op_store(int what, struct class_device *dev, const char *buf, size_t count)
{
	char sbuf[64];
	ssize_t rv = SUCCESS, minor = MINOR(dev->devt);
	dprintk(DBGLEV_CHAR3, "what=%d, dev=%p, buf=%p, count=%u\n", what, dev, buf, count);
	if(minor >= DVBLO_CHAR_DEVMAX || chardevs[minor].used == 0)
		rv = -ENODEV;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else {
		strncat(sbuf, buf, min(sizeof(sbuf)-1, count));
		if(chardevs[minor].used == 0)
			rv = -ENODEV;
		else switch(what) {
			case 1: /* Set the L2-framing mode */
			{
				char *end = NULL;
				unsigned long val = simple_strtoul(sbuf, &end, 0);
				if(end == NULL || (end[0] != '\0' && !isspace(end[0])))
					rv = -EINVAL;
				else if(val != 188) {
					mprintk(KERN_ERR, "Invalid value for TS packet size: the supplied value (%lu) was wrong: must be 188\n", val);
					rv = -EINVAL;
				} else {
					chardevs[minor].cfg.ts_sz = val;
					rv = count;
				}
				break;
			}
			case 2: /* Set the (default) per-handle write buffer size */
			{
				char *end = NULL;
				unsigned long val = simple_strtoul(sbuf, &end, 0);
				if(end == NULL || (end[0] != '\0' && !isspace(end[0])))
					rv = -EINVAL;
				else if((val % chardevs[minor].cfg.ts_sz) != 0) {
					mprintk(KERN_ERR, "Invalid value for per-handle write buffer size: the supplied value (%lu) was wrong: must be a multiple of %u\n", val, chardevs[minor].cfg.ts_sz);
					rv = -EINVAL;
				} else {
					chardevs[minor].cfg.hwrbuf_sz = val;
					rv = count;
				}
				break;
			}
			default:
				rv = -EINVAL;
				break;
		}
		up(&dvblo_char_chardevs_sem);
	}
	return rv;
}

/* NOTE: buf is of size PAGE_SIZE */
static ssize_t clsdev_op_show(int what, struct class_device *dev, char *buf)
{
	ssize_t rv = SUCCESS, minor = MINOR(dev->devt);
	dprintk(DBGLEV_CHAR3, "what=%d, dev=%p, buf=%p\n", what, dev, buf);
	if(minor >= DVBLO_CHAR_DEVMAX || chardevs[minor].used == 0)
		rv = -ENODEV;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else {
		if(chardevs[minor].used == 0)
			rv = -ENODEV;
		else switch(what) {
			case 0: /* Get the MAC address of the DVB adapter */
			{
				u8 mac[6];
				rv = dvblo_adap_get_mac(chardevs[minor].dvblo, mac);
				if(rv == SUCCESS) {
					int i = snprintf(buf, PAGE_SIZE, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					if(i < 0 || i >= PAGE_SIZE) {
						if(i < 0)
							rv = i;
						else
							rv = -ENOBUFS;
					}
					else
						rv = i; // return number of characters written
				}
				break;
			}
			case 1: /* Get the L2-framing mode */
			{
				int i = snprintf(buf, PAGE_SIZE, "%u", chardevs[minor].cfg.ts_sz);
				if(i < 0 || i >= PAGE_SIZE) {
					if(i < 0)
						rv = i;
					else
						rv = -ENOBUFS;
				}
				else
					rv = i; // return number of characters written
				break;
			}
			case 2: /* Get the (default) per-handle write buffer size */
			{
				int i = snprintf(buf, PAGE_SIZE, "%u", chardevs[minor].cfg.hwrbuf_sz);
				if(i < 0 || i >= PAGE_SIZE) {
					if(i < 0)
						rv = i;
					else
						rv = -ENOBUFS;
				}
				else
					rv = i; // return number of characters written
				break;
			}
			default:
				rv = -EINVAL;
				break;
		}
		up(&dvblo_char_chardevs_sem);
	}
	return rv;
}

static ssize_t clsdev_op_show_dvb_mac(struct class_device *dev, char *buf)
{
	dprintk(DBGLEV_CHAR3, "dev=%p, buf=%p\n", dev, buf);
	return clsdev_op_show(0, dev, buf);
}

static ssize_t clsdev_op_show_ts_sz(struct class_device *dev, char *buf)
{
	dprintk(DBGLEV_CHAR3, "dev=%p, buf=%p\n", dev, buf);
	return clsdev_op_show(1, dev, buf);
}

static ssize_t clsdev_op_store_ts_sz(struct class_device *dev, const char *buf, size_t count)
{
	dprintk(DBGLEV_CHAR3, "dev=%p, buf=%p\n", dev, buf);
	return clsdev_op_store(1, dev, buf, count);
}

static ssize_t clsdev_op_show_hwrbuf_sz(struct class_device *dev, char *buf)
{
	dprintk(DBGLEV_CHAR3, "dev=%p, buf=%p\n", dev, buf);
	return clsdev_op_show(2, dev, buf);
}

static ssize_t clsdev_op_store_hwrbuf_sz(struct class_device *dev, const char *buf, size_t count)
{
	dprintk(DBGLEV_CHAR3, "dev=%p, buf=%p\n", dev, buf);
	return clsdev_op_store(2, dev, buf, count);
}
#endif

static int dvblo_char_handle_destroy(struct dvblo_chardev_handle *handle)
{
	int rv = SUCCESS;
	dprintk(DBGLEV_CHAR3, "handle=%p\n", handle);
	if(handle == NULL)
		rv = -EINVAL;
	else switch(handle->initlev) {
		case 1:
			if(handle->buf.base != NULL)
				kfree(handle->buf.base);
		default:
			break;
	}
	return rv;
}

static int dvblo_char_handle_create(struct dvblo_chardev *chardev, struct inode *inode, struct dvblo_chardev_handle **handle_out)
{
	int rv = SUCCESS;
	struct dvblo_chardev_handle *handle = NULL;
	dprintk(DBGLEV_CHAR3, "chardev=%p, inode=%p, handle_out=%p\n", chardev, inode, handle_out);
	if(chardev == NULL || inode == NULL || handle_out == NULL)
		rv = -EINVAL;
	else if((handle = kmalloc(sizeof(*handle), GFP_KERNEL)) == NULL)
		rv = -ENOMEM;
	else do {
		handle->initlev = 0;
		handle->initdone = 0;
		handle->chardev = chardev;
		handle->buf.size = chardev->cfg.hwrbuf_sz;
		handle->buf.base = kmalloc(handle->buf.size, GFP_KERNEL);
		if(handle->buf.base == NULL) {
			rv = -ENOMEM;
			break;
		}
		handle->initlev++; /* 1 */
		
		handle->buf.rd = handle->buf.wr = 0;
		
		/* SUCCESS */
		handle->initdone = 1;
	} while(0);
	if(rv != SUCCESS) {
		*handle_out = NULL;
		if(handle != NULL)
			dvblo_char_handle_destroy(handle);
	} else
		*handle_out = handle;
	return rv;
}

static int dvblo_char_op_open(struct inode *inode, struct file *filp)
{
	int rv = SUCCESS, minor = iminor(inode);
	dprintk(DBGLEV_CHAR3, "inode=%p, filp=%p\n", inode, filp);
	if(minor >= DVBLO_CHAR_DEVMAX)
		rv = -ENODEV;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else {
		if(chardevs[minor].used == 0)
			rv = -ENODEV;
		else {
			if(!chardevs[minor].hcountmax || chardevs[minor].hcount < chardevs[minor].hcountmax) {
				
				///@todo allocate per-file handle context structure
				struct dvblo_chardev_handle *handle;
				rv = dvblo_char_handle_create(&chardevs[minor], inode, &handle);
				if(rv == SUCCESS) {
					chardevs[minor].hcount++;
					filp->private_data = handle;
					try_module_get(THIS_MODULE);
				}
			} else
				rv = -EBUSY; // Too many open file handles
		}
		up(&dvblo_char_chardevs_sem);
	}
	return rv;
}

/* dvblo_char_write
 *
 * This function forwards TS packets written to the char device to the virtual DVB adapter
 */
static ssize_t dvblo_char_op_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	ssize_t rv = 0, cbytes;
	struct dvblo_chardev_handle *handle = (struct dvblo_chardev_handle*) filp->private_data;
	size_t ts_sz = handle->chardev->cfg.ts_sz, wrleft, rdleft, cbytes_req, bytes_left;
	dprintk(DBGLEV_CHAR3, "filp=%p, buf=%p, len=%u, off=%p\n", filp, buf, len, off);
	/* we only accept complete TS packets */
	bytes_left = (len / ts_sz) * ts_sz;
	while(bytes_left > 0) {
		do {
			wrleft = handle->buf.wr < handle->buf.rd ? (handle->buf.rd - handle->buf.wr) : (handle->buf.size - handle->buf.wr);
			if(wrleft == 0) {
				/* we can't write anymore */
				if(rv == 0) {
					if((filp->f_flags & O_NONBLOCK) != 0)  
						rv = -EAGAIN; /* we were not able to write anything */
					else {
						///@todo support blocking operation
						rv = -ENOSYS;
					}
				}
			}
		} while(wrleft == 0 && rv >= 0);
		if(wrleft == 0 || rv < 0)
			break; /* no buffer space left */
		if((wrleft % ts_sz) != 0) {
			mprintk(KERN_ERR, "Lost TS packet boundary (write) in ring buffer. Please report this to the maintainer\n");
			rv = -EBADFD;
			break;
		}
		/* calc. how much we can write, at most */
		cbytes_req = bytes_left < wrleft ? bytes_left : wrleft;
		/* NOTE: copy_from_user() returns number of bytes that could not be copied.
		 * On success, this will be zero.
		 */
		if(copy_from_user(&handle->buf.base[handle->buf.wr], &buf[rv], cbytes_req) != 0) {
			rv = -EFAULT;
			break; /* error while copying */
		}
		dprintk(DBGLEV_CHAR3, "wr=%u, wrleft=%u, bytes_left=%u, cbytes_req=%u, rv=%d\n",
			handle->buf.wr, wrleft, bytes_left, cbytes_req, rv);
		/* update write offset (with wrap-around) */
		handle->buf.wr = (handle->buf.wr + cbytes_req) % handle->buf.size;
		rv += cbytes_req;
		/* @note At this point we could invoke a "virtual multiplexer" which reads from 
		 * different input streams
		 */
		rdleft = handle->buf.rd > handle->buf.wr ? (handle->buf.size - handle->buf.rd) : (handle->buf.wr - handle->buf.rd);
		if((rdleft % ts_sz) != 0) {
			mprintk(KERN_ERR, "Lost read TS packet boundary in ring buffer. Please report this to the maintainer\n");
			rv = -EBADFD;
			break;
		}
		cbytes_req = rdleft;
		cbytes = dvblo_adap_deliver_packets(handle->chardev->dvblo, &handle->buf.base[handle->buf.rd], cbytes_req);
		if(cbytes < 0) {
			rv = cbytes;
			break; /* failed while delivering TS packets to DVB adapter */
		} else if((cbytes % ts_sz) != 0) {
			rv = -EBADFD;
			break;
		}
		dprintk(DBGLEV_CHAR3, "rd=%u, rdleft=%u, cbytes_req=%u, cbytes=%d\n",
			handle->buf.rd, rdleft, cbytes_req, cbytes);
		/* update read offset (with wrap-around) */
		handle->buf.rd = (handle->buf.rd + cbytes) % handle->buf.size;
		bytes_left -= cbytes;
	}
	return rv;
}

static int dvblo_char_op_release(struct inode *inode, struct file *filp)
{
	int rv = SUCCESS;
	struct dvblo_chardev_handle *handle = (struct dvblo_chardev_handle*) filp->private_data;
	dprintk(DBGLEV_CHAR3, "inode=%p, filp=%p\n", inode, filp);
	if(handle == NULL || handle->chardev == NULL)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else {
		struct dvblo_chardev *chardev = handle->chardev;
		if(chardev->used == 0)
			rv = -ENODEV;
		else {
			rv = dvblo_char_handle_destroy(handle);
			if(rv == SUCCESS) {
				handle->chardev->hcount--;
				/* 
				 * Decrement the usage count, or else once you opened the file, you'll
				 * never get get rid of the module. 
				 */
				module_put(THIS_MODULE);
				up(&dvblo_char_chardevs_sem);
				if(handle->chardev->hcount == 0)
					wake_up_interruptible(&dvblo_char_chardevs_wq);
			}
		}
	}
	return rv;
}

static int dvblo_char_op_ioctl(struct inode *inode, struct file *filp, 
	unsigned int cmd, unsigned long arg)
{
	return -ENOSYS;
}

struct file_operations dvblo_char_fops = {
	.open = dvblo_char_op_open,
	.write = dvblo_char_op_write,
	.release = dvblo_char_op_release,
	.ioctl = dvblo_char_op_ioctl,
	.owner = THIS_MODULE
};

static int dvblo_chardev_release(struct dvblo_chardev *chardev)
{
	int rv = SUCCESS, i;
	if(chardev->used == 0) {
		rv = -EINVAL;
	} else if(chardev->hcount > 0) {
		rv = -EAGAIN;
	} else {
		if(chardev->initdone != 0)
			dprintk(0, "releasing char device with minor device number %u\n", chardev->minor);
		switch(chardev->initlev)
		{
			case 3:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
			for(i=0; i<(sizeof(clsdev_attrs)/sizeof(clsdev_attrs[0])) && rv == SUCCESS; i++)
				class_device_remove_file(chardev->clsdev, &clsdev_attrs[i]);
#endif
			case 2:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
				class_device_destroy(dvblo_class, MKDEV(dvblo_char_major, chardev->minor));
#else
				/* The old-style "simple" class API */
				class_simple_device_remove(MKDEV(dvblo_char_major, chardev->minor));
#endif
			case 1:
				i = dvblo_adap_destroy(chardev->dvblo);
				if(i < 0) {
					mprintk(KERN_ALERT, "failed to destroy virtual DVB adapter: %d\n", i);
					if(rv == 0)
						rv = i;
				}
			default:
				break;
		}
		if(chardev->initdone != 0)		
			mprintk(KERN_INFO, "removed character device %s\n", chardev->name);
		chardev->initlev = 0;
		chardev->initdone = 0;
		chardev->used = 0;
	}
	return rv;
}

static int dvblo_chardev_init(struct dvblo_chardev *chardev, struct dvblo_chardev_config *cfg)
{
	int rv = SUCCESS, i;
	
	if(chardev->used != 0) {
		rv = -EINVAL;
	} else {
		chardev->used = 1;
		chardev->initlev = 0;
		chardev->initdone = 0;
		do {
			dprintk(DBGLEV_ALL, "initializing char device with minor device number %u\n", chardev->minor);
			i = snprintf(chardev->name, sizeof(chardev->name), DVBLO_NAME "%d", chardev->minor);
			if(i < 0 || i >= sizeof(chardev->name)) {
				if(i < 0)
					rv = i;
				else
					rv = -ENOBUFS;
				break;
			}
			
			chardev->hcount = 0;
			chardev->cfg.ts_sz = DVBLO_TS_SZ;
			chardev->cfg.hwrbuf_sz = 20 * chardev->cfg.ts_sz;
			
			i = dvblo_adap_create(chardev->minor, cfg ? &cfg->dvbcfg : NULL, &chardev->dvblo);
			if(i < 0) {
				mprintk(KERN_ALERT, "failed to create virtual DVB adapter: %d\n", i);
				rv = i;
				break;
			}
			chardev->initlev++;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
			/* In 2.6.15, class_device_create() got a pointer to the parent device (if any) as its second param */ 
			chardev->clsdev = class_device_create(dvblo_class, NULL, MKDEV(dvblo_char_major, chardev->minor), NULL, chardev->name);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
			/* class_device_create() first appeared in 2.6.13 */
			chardev->clsdev = class_device_create(dvblo_class, MKDEV(dvblo_char_major, chardev->minor), NULL, chardev->name);
#else
			/* The old-style "simple" class API */
			chardev->clsdev = class_simple_device_add(dvblo_class, MKDEV(dvblo_char_major, chardev->minor), NULL, chardev->name);
#endif
			if(IS_ERR(chardev->clsdev)) {
				rv = PTR_ERR(chardev->clsdev);
				mprintk(KERN_ALERT, "failed to create device class \"%s\": %d\n", chardev->name, rv);
				break;
			}
			chardev->initlev++;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
			for(i=0; i<(sizeof(clsdev_attrs)/sizeof(clsdev_attrs[0])) && rv == SUCCESS; i++)
				rv = class_device_create_file(chardev->clsdev, &clsdev_attrs[i]);
			if(rv != SUCCESS)
				break;
#endif
			chardev->initlev++;
			mprintk(KERN_INFO, "added character device %s\n", chardev->name);
		} while(0);
		if(rv != 0)
			dvblo_chardev_release(chardev); // error cleanup
	}
	return rv;
}

int dvblo_char_add_dev(struct dvblo_chardev_config *cfg, unsigned int *devnum_out)
{
	int rv = SUCCESS, i;
	if(devnum_out == NULL)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else {
		for(i=0; i<DVBLO_CHAR_DEVMAX && chardevs[i].used != 0; i++);
		if(i == DVBLO_CHAR_DEVMAX)
			rv = -ENFILE;
		else {
			rv = dvblo_chardev_init(&chardevs[i], cfg);
			if(rv == SUCCESS)
				*devnum_out = i;
		}
		up(&dvblo_char_chardevs_sem);
	}
	return rv;
}

int dvblo_char_del_dev(unsigned int devnum)
{
	int rv = SUCCESS;
	if(devnum >= DVBLO_CHAR_DEVMAX || chardevs[devnum].used == 0)
		rv = -EINVAL;
	else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
		rv = -ERESTARTSYS;
	else
	{
		// Wait, until all open file handles have been closed
		while(rv == SUCCESS && chardevs[devnum].hcount > 0) {
			up(&dvblo_char_chardevs_sem);
			if(wait_event_interruptible(dvblo_char_chardevs_wq, (chardevs[devnum].hcount == 0)) != 0)
				rv = -ERESTARTSYS;
			else if(down_interruptible(&dvblo_char_chardevs_sem) != 0)
				rv = -ERESTARTSYS;
			// else: loop again and test condition
		}
		if(rv == SUCCESS) {
			rv = dvblo_chardev_release(&chardevs[devnum]);
			up(&dvblo_char_chardevs_sem);
		}
	}
	return rv;
}

int dvblo_char_exit(void)
{
	int rv = SUCCESS, i, j;
	switch(initlev) {
		case 3:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
			class_destroy(dvblo_class);
#else
			class_simple_destroy(dvblo_class);
#endif
		case 2:
			/* NOTE: we have to lock dvblo_char_chardevs_sem to avoid that after
			 * releasing all devices a new device is created again, becaus our char 
			 * device driver is still registered.
			 */
			down(&dvblo_char_chardevs_sem);
			for(j=0; j<DVBLO_CHAR_DEVMAX; j++) {
				if(chardevs[j].used != 0) {
					// Wait, until all open file handles have been closed
					int waitc = 0;
					while(chardevs[j].hcount > 0) {
						mprintk(KERN_INFO, "There are %i open file handles for device %s. %s...\n", 
							chardevs[j].hcount, chardevs[j].name, waitc > 0 ? "Still waiting" : "Waiting");
						up(&dvblo_char_chardevs_sem);
						wait_event(dvblo_char_chardevs_wq, (chardevs[j].hcount == 0));
						down(&dvblo_char_chardevs_sem);
						waitc++;
					}
					// now that all file handles are closed, release the device
					i = dvblo_chardev_release(&chardevs[j]);
					if(i != SUCCESS)
					{
						mprintk(KERN_ALERT, "failed to release char device %s: %d\n", chardevs[j].name, i);
						if(rv == 0)
							rv = i;
					}
				}
			}
#if defined(USE_CDEV) && USE_CDEV != 0
			cdev_del(&dvblo_char_cdev);
#else
			i = unregister_chrdev(dvblo_char_major, DVBLO_NAME);
			if (i < 0) {
				mprintk(KERN_ALERT, "failed to unregister char device: %d\n", i);
				if(rv == 0)
					rv = i;
			}
#endif
			up(&dvblo_char_chardevs_sem);
		case 1:
#if defined(USE_CDEV) && USE_CDEV != 0
			unregister_chrdev_region(MKDEV(dvblo_char_major, 0), DVBLO_CHAR_DEVMAX);
#endif
			dvblo_char_major = 0;
		default:
			break;
	}
	initlev = 0;
	return rv;
}

int dvblo_char_init(void)
{
	int rv = SUCCESS, i;
	do
	{
		if(initlev > 0) {
			rv = -EINVAL;
			break;
		}
		/* initialize array of char devices */
		for(i=0; i<DVBLO_CHAR_DEVMAX; i++) {
			chardevs[i].used = 0;
			chardevs[i].minor = i;
		}
#if defined(USE_CDEV) && USE_CDEV != 0
		if(dvblo_char_major > 0) {
			/* we explicitly request the major device number dvblo_char_major */
			i = register_chrdev_region(MKDEV(dvblo_char_major, 0), DVBLO_CHAR_DEVMAX, DVBLO_NAME);
			if(i < 0) {
				mprintk(KERN_ALERT, "failed to register char device number region (major=%d, count=%d): %d\n", 
					dvblo_char_major, DVBLO_CHAR_DEVMAX, i);
				rv = dvblo_char_major;
				break;
			}
		} else {
			/* dynamic allocation of the major device number */
			dev_t dev;
			i = alloc_chrdev_region(&dev, 0, DVBLO_CHAR_DEVMAX, DVBLO_NAME);
			if(i < 0) {
				mprintk(KERN_ALERT, "failed to allocate char device number region (count=%d): %d\n", 
					DVBLO_CHAR_DEVMAX, i);
				rv = dvblo_char_major;
				break;
			}
			dvblo_char_major = MAJOR(dev);
		}
#endif
		initlev++; /* 1 */
		
#if defined(USE_CDEV) && USE_CDEV != 0
		cdev_init(&dvblo_char_cdev, &dvblo_char_fops);
		// fops are assigned in cdev_init()
		//dvblo_char_cdev.ops = &dvblo_char_fops;
		dvblo_char_cdev.owner = THIS_MODULE;
		i = cdev_add(&dvblo_char_cdev, MKDEV(dvblo_char_major, 0), DVBLO_CHAR_DEVMAX);
		if(i < 0)
		{
			mprintk(KERN_ALERT, "failed to add char device: %d\n", i);
			rv = dvblo_char_major;
			break;
		}
#else
		/* NOTE: by setting <major> to 0 we request a dynamic major device number
		 */
		i = register_chrdev(dvblo_char_major, DVBLO_NAME, &dvblo_char_fops);
		if(i < 0)
		{
			mprintk(KERN_ALERT, "failed to register char device: %d\n", i);
			rv = i;
			break;
		} else if(dvblo_char_major == 0)
			dvblo_char_major = i;
		// else requested device major number was accepted
#endif
		initlev++; /* 2 */
		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
		dvblo_class = class_create(THIS_MODULE, DVBLO_NAME);
#else
		dvblo_class = class_simple_create(THIS_MODULE, DVBLO_NAME);
#endif
		if(IS_ERR(dvblo_class)) {
			rv = PTR_ERR(dvblo_class);
			mprintk(KERN_ALERT, "failed to create class \"" DVBLO_NAME "\": %d\n", rv);
			break;
		}
		initlev++; /* 3 */
	} while(0);
	if(rv != 0)
		dvblo_char_exit(); // err cleanup
	return rv;
}

