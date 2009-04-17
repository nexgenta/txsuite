/*
 * findmheg.c
 */

/*
 * Copyright (C) 2005, Simon Kilvington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>

#include "carousel.h"
#include "findmheg.h"
#include "table.h"
#include "assoc.h"
#include "utils.h"

/* stream_types we are interested in */
#define STREAM_TYPE_VIDEO_MPEG2		0x02
#define STREAM_TYPE_AUDIO_MPEG1		0x03
#define STREAM_TYPE_AUDIO_MPEG2		0x04
#define STREAM_TYPE_ISO13818_6_B	0x0b

/* descriptors we want */
#define TAG_LANGUAGE_DESCRIPTOR			0x0a
#define TAG_CAROUSEL_ID_DESCRIPTOR		0x13
#define TAG_STREAM_ID_DESCRIPTOR		0x52
#define TAG_DATA_BROADCAST_ID_DESCRIPTOR	0x66

/* bits of the descriptors we care about */
struct carousel_id_descriptor
{
	uint32_t carousel_id;
};

struct stream_id_descriptor
{
	uint8_t component_tag;
};

struct language_descriptor
{
	char language_code[3];
	uint8_t audio_type;
} __attribute__((__packed__));

struct data_broadcast_id_descriptor
{
	uint16_t data_broadcast_id;
	uint16_t application_type_code;
	uint8_t boot_priority_hint;
} __attribute__((__packed__));

/* data_broadcast_id_descriptor values we want */
#define DATA_BROADCAST_ID		0x0106
#define UK_APPLICATION_TYPE_CODE	0x0101
#define NZ_APPLICATION_TYPE_CODE	0x0505

static struct avstreams *find_current_avstreams(struct carousel *, int, int);
static struct avstreams *find_service_avstreams(struct carousel *, int, int, int);

bool
is_audio_stream(uint8_t stream_type)
{
	switch(stream_type)
	{
	case STREAM_TYPE_AUDIO_MPEG1:
	case STREAM_TYPE_AUDIO_MPEG2:
		return true;

	default:
		return false;
	}
}

/*
 * fills in a struct carousel based on the given service_id
 * returns a ptr to a static struct that will be overwritten be the next call to this routine
 */

static struct carousel _car;

struct carousel *
find_mheg(unsigned int adapter, unsigned int timeout, uint16_t service_id, int carousel_id)
{
	unsigned char pmt[MAX_TABLE_LEN];
	unsigned char *sdt = pmt;
	uint16_t section_length;
	uint16_t offset;
	uint8_t stream_type;
	uint16_t elementary_pid;
	uint16_t info_length;
	uint8_t desc_tag;
	uint8_t desc_length;
	uint16_t component_tag;
	int desc_boot_pid;
	int desc_carousel_id;

	/* carousel data we know so far */
	snprintf(_car.demux_device, sizeof(_car.demux_device), DEMUX_DEVICE, adapter);
	snprintf(_car.dvr_device, sizeof(_car.dvr_device), DVR_DEVICE, adapter);
	_car.timeout = timeout;
	_car.service_id = service_id;

	/* unknown */
	_car.network_id = 0;
	_car.carousel_id = 0;
	_car.boot_pid = 0;
	_car.audio_pid = 0;
	_car.audio_type = 0;
	_car.video_pid = 0;
	_car.video_type = 0;
	_car.current_pid = 0;
	/* map between stream_id_descriptors and elementary_PIDs */
	init_assoc(&_car.assoc);
	/* no PIDs yet */
	_car.npids = 0;
	_car.pids = NULL;
	/* no modules loaded yet */
	_car.got_dsi = false;
	_car.nmodules = 0;
	_car.modules = NULL;

	/* find the original_network_id from the SDT */
	if(!read_sdt(_car.demux_device, timeout, sdt))
		fatal("Unable to read SDT");
	_car.network_id = (sdt[8] << 8) + sdt[9];
	vverbose("original_network_id=%u", _car.network_id);

	/* get the PMT */
	if(!read_pmt(_car.demux_device, service_id, timeout, pmt))
		fatal("Unable to read PMT");

	section_length = 3 + (((pmt[1] & 0x0f) << 8) + pmt[2]);

	/* skip the program_info descriptors */
	offset = 10;
	info_length = ((pmt[offset] & 0x0f) << 8) + pmt[offset+1];
	offset += 2 + info_length;

	/*
	 *  find the Descriptor Tags we are interested in
	 */
	while(offset < (section_length - 4))
	{
		stream_type = pmt[offset];
		offset += 1;
		elementary_pid = ((pmt[offset] & 0x1f) << 8) + pmt[offset+1];
		offset += 2;
		/* is it the default video stream for this service */
		if(stream_type == STREAM_TYPE_VIDEO_MPEG2)
		{
			_car.video_pid = elementary_pid;
			_car.video_type = stream_type;
			vverbose("PID=%u video stream_type=0x%x", elementary_pid, stream_type);
		}
		/* it's not the boot PID yet */
		desc_boot_pid = -1;
		desc_carousel_id = -1;
		/* read the descriptors */
		info_length = ((pmt[offset] & 0x0f) << 8) + pmt[offset+1];
		offset += 2;
		while(info_length != 0)
		{
			desc_tag = pmt[offset];
			desc_length = pmt[offset+1];
			offset += 2;
			info_length -= 2;
			/* ignore boot PID if we explicitly chose a carousel ID */
			if(desc_tag == TAG_DATA_BROADCAST_ID_DESCRIPTOR && carousel_id == -1)
			{
				struct data_broadcast_id_descriptor *desc;
				desc = (struct data_broadcast_id_descriptor *) &pmt[offset];
				if(ntohs(desc->data_broadcast_id) == DATA_BROADCAST_ID)
				{
					desc_boot_pid = elementary_pid;
					vverbose("PID=%u boot_priority_hint=%u", elementary_pid, desc->boot_priority_hint);
					/* haven't seen the NZ MHEG Profile, but let's download the data anyway */
					if(ntohs(desc->application_type_code) == UK_APPLICATION_TYPE_CODE)
						vverbose("UK application_type_code (0x%04x)", UK_APPLICATION_TYPE_CODE);
					else if(ntohs(desc->application_type_code) == NZ_APPLICATION_TYPE_CODE)
						vverbose("NZ application_type_code (0x%04x)", NZ_APPLICATION_TYPE_CODE);
					else
						vverbose("Unknown application_type_code (0x%04x)", ntohs(desc->application_type_code));
				}
				else
				{
					vverbose("PID=%u data_broadcast_id=0x%x", elementary_pid, ntohs(desc->data_broadcast_id));
					vhexdump(&pmt[offset], desc_length);
				}
			}
			else if(desc_tag == TAG_CAROUSEL_ID_DESCRIPTOR)
			{
				struct carousel_id_descriptor *desc;
				desc = (struct carousel_id_descriptor *) &pmt[offset];
				if(carousel_id == -1 || carousel_id == (ntohl(desc->carousel_id)))
				{
					/* if we chose this carousel, set the boot PID */
					if(carousel_id != -1)
						desc_boot_pid = elementary_pid;
					desc_carousel_id = ntohl(desc->carousel_id);
				}
				vverbose("PID=%u carousel_id=%u", elementary_pid, ntohl(desc->carousel_id));
			}
			else if(desc_tag == TAG_STREAM_ID_DESCRIPTOR)
			{
				struct stream_id_descriptor *desc;
				desc = (struct stream_id_descriptor *) &pmt[offset];
				component_tag = desc->component_tag;
				vverbose("PID=%u component_tag=%u", elementary_pid, component_tag);
				add_assoc(&_car.assoc, elementary_pid, desc->component_tag, stream_type);
			}
			else if(desc_tag == TAG_LANGUAGE_DESCRIPTOR && is_audio_stream(stream_type))
			{
				struct language_descriptor *desc;
				desc = (struct language_descriptor *) &pmt[offset];
				/* only remember the normal audio stream (not visually impaired stream) */
				if(desc->audio_type == 0)
				{
					_car.audio_pid = elementary_pid;
					_car.audio_type = stream_type;
					vverbose("PID=%u audio stream_type=0x%x", elementary_pid, stream_type);
				}
			}
			else
			{
				vverbose("PID=%u descriptor=0x%x", elementary_pid, desc_tag);
				vhexdump(&pmt[offset], desc_length);
			}
			offset += desc_length;
			info_length -= desc_length;
		}
		/* is it the boot PID */
		if(desc_boot_pid != -1)
		{
			vverbose("Set boot_pid=%u carousel_id=%u", desc_boot_pid, desc_carousel_id);
			_car.carousel_id = desc_carousel_id;
			_car.boot_pid = desc_boot_pid;
			add_dsmcc_pid(&_car, desc_boot_pid);
		}
	}

	/* did we find a DSM-CC stream */
	if(_car.npids == 0)
		fatal("Unable to find Carousel Descriptor in PMT");

	return &_car;
}

static struct avstreams _streams;

struct avstreams *
find_avstreams(struct carousel *car, int service_id, int audio_tag, int video_tag)
{
	if(service_id == -1)
		return find_current_avstreams(car, audio_tag, video_tag);
	else
		return find_service_avstreams(car, service_id, audio_tag, video_tag);
}

static struct avstreams *
find_current_avstreams(struct carousel *car, int audio_tag, int video_tag)
{
	/* map the tags to PIDs and stream types, or use the defaults */
	if(audio_tag == -1)
	{
		/* maybe 0 if we have no default stream */
		_streams.audio_pid = car->audio_pid;
		_streams.audio_type = car->audio_type;
	}
	else
	{
		_streams.audio_pid = stream2pid(&car->assoc, audio_tag);
		_streams.audio_type = stream2type(&car->assoc, audio_tag);
	}

	if(video_tag == -1)
	{
		/* maybe 0 if we have no default stream */
		_streams.video_pid = car->video_pid;
		_streams.video_type = car->video_type;
	}
	else
	{
		_streams.video_pid = stream2pid(&car->assoc, video_tag);
		_streams.video_type = stream2type(&car->assoc, video_tag);
	}

	return &_streams;
}

static struct avstreams *
find_service_avstreams(struct carousel *car, int service_id, int audio_tag, int video_tag)
{
	unsigned char pmt[MAX_TABLE_LEN];
	uint16_t section_length;
	uint16_t offset;
	uint8_t stream_type;
	uint16_t elementary_pid;
	uint16_t info_length;
	uint8_t desc_tag;
	uint8_t desc_length;
	uint16_t component_tag;

	verbose("find_service_avstreams: %d %d %d", service_id, audio_tag, video_tag);

	/* in case we don't find them */
	bzero(&_streams, sizeof(_streams));

	/* get the PMT */
	if(!read_pmt(car->demux_device, service_id, car->timeout, pmt))
		fatal("Unable to read PMT");

	section_length = 3 + (((pmt[1] & 0x0f) << 8) + pmt[2]);

	/* skip the program_info descriptors */
	offset = 10;
	info_length = ((pmt[offset] & 0x0f) << 8) + pmt[offset+1];
	offset += 2 + info_length;

	/* find the streams */
	while(offset < (section_length - 4))
	{
		stream_type = pmt[offset];
		offset += 1;
		elementary_pid = ((pmt[offset] & 0x1f) << 8) + pmt[offset+1];
		offset += 2;
		/* do we want the default video stream for this service */
		if(video_tag == -1 && stream_type == STREAM_TYPE_VIDEO_MPEG2)
		{
			_streams.video_pid = elementary_pid;
			_streams.video_type = stream_type;
			vverbose("PID=%u video stream_type=0x%x", elementary_pid, stream_type);
		}
		/* read the descriptors */
		info_length = ((pmt[offset] & 0x0f) << 8) + pmt[offset+1];
		offset += 2;
		while(info_length != 0)
		{
			desc_tag = pmt[offset];
			desc_length = pmt[offset+1];
			offset += 2;
			info_length -= 2;
			if(desc_tag == TAG_STREAM_ID_DESCRIPTOR)
			{
				struct stream_id_descriptor *desc;
				desc = (struct stream_id_descriptor *) &pmt[offset];
				component_tag = desc->component_tag;
				vverbose("PID=%u component_tag=%u", elementary_pid, component_tag);
				/* is it one we want */
				if(audio_tag == component_tag)
				{
					_streams.audio_pid = elementary_pid;
					_streams.audio_type = stream_type;
					vverbose("PID=%u audio stream_type=0x%x", elementary_pid, stream_type);
				}
				else if(video_tag == component_tag)
				{
					_streams.video_pid = elementary_pid;
					_streams.video_type = stream_type;
					vverbose("PID=%u video stream_type=0x%x", elementary_pid, stream_type);
				}
			}
			/* do we want the default audio */
			else if(audio_tag == -1 && desc_tag == TAG_LANGUAGE_DESCRIPTOR && is_audio_stream(stream_type))
			{
				struct language_descriptor *desc;
				desc = (struct language_descriptor *) &pmt[offset];
				/* only remember the normal audio stream (not visually impaired stream) */
				if(desc->audio_type == 0)
				{
					_streams.audio_pid = elementary_pid;
					_streams.audio_type = stream_type;
					vverbose("PID=%u audio stream_type=0x%x", elementary_pid, stream_type);
				}
			}
			else
			{
				vverbose("PID=%u descriptor=0x%x", elementary_pid, desc_tag);
				vhexdump(&pmt[offset], desc_length);
			}
			offset += desc_length;
			info_length -= desc_length;
		}
	}

	verbose("Audio PID=%u type=0x%x", _streams.audio_pid, _streams.audio_type);
	verbose("Video PID=%u type=0x%x", _streams.video_pid, _streams.video_type);

	return &_streams;
}

