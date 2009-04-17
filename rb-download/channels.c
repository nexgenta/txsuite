/*
 * channels.c
 */

/*
 * Copyright (C) 2007, Simon Kilvington
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

#include "utils.h"

/* magic DVB-S values from dvbtune */
#define SLOF (11700*1000UL)
#define LOF1 (9750*1000UL)
#define LOF2 (10600*1000UL)
#define ONE_kHz 1000UL

/* internal functions */
static bool get_tune_params(fe_type_t, uint16_t, struct dvb_frontend_parameters *, char *, unsigned int *);

static bool get_dvbt_tune_params(uint16_t, struct dvb_frontend_parameters *);
static bool get_dvbs_tune_params(uint16_t, struct dvb_frontend_parameters *, char *, unsigned int *);
static bool get_dvbc_tune_params(uint16_t, struct dvb_frontend_parameters *);
static bool get_atsc_tune_params(uint16_t, struct dvb_frontend_parameters *);

/* DISEQC code from dvbtune, written by Dave Chapman */
struct diseqc_cmd
{
	struct dvb_diseqc_master_cmd cmd;
	uint32_t wait;
};

static int do_diseqc(int, int, char, bool);
static int diseqc_send_msg(int, fe_sec_voltage_t, struct diseqc_cmd *, fe_sec_tone_mode_t, fe_sec_mini_cmd_t);

/*
 * returns "tzap" if the given DVB adapter is DVB-T,
 * "szap" if it is DVB-S
 * "czap" if it is DVB-C
 * or "azap" if it is ATSC
 * (may have been better to call one of these to retune rather than doing it ourselves?)
 */

char *
zap_name(unsigned int adapter)
{
	char fe_dev[PATH_MAX];
	int fe_fd;
	struct dvb_frontend_info fe_info;
	bool got_info;

	/* see what type of DVB device the adapter is */
	snprintf(fe_dev, sizeof(fe_dev), FE_DEVICE, adapter);

	if((fe_fd = open(fe_dev, O_RDONLY | O_NONBLOCK)) < 0)
		fatal("open '%s': %s", fe_dev, strerror(errno));

	vverbose("Getting frontend info");

	do
	{
		/* maybe interrupted by a signal */
		got_info = (ioctl(fe_fd, FE_GET_INFO, &fe_info) >= 0);
		if(!got_info && errno != EINTR)
			fatal("ioctl FE_GET_INFO: %s", strerror(errno));
	}
	while(!got_info);

	close(fe_fd);

	if(fe_info.type == FE_OFDM)
	{
		vverbose("Adapter %u is a '%s' DVB-T card", adapter, fe_info.name);
		return "tzap";
	}
	else if(fe_info.type == FE_QPSK)
	{
		vverbose("Adapter %u is a '%s' DVB-S card", adapter, fe_info.name);
		return "szap";
	}
	else if(fe_info.type == FE_QAM)
	{
		vverbose("Adapter %u is a '%s' DVB-C card", adapter, fe_info.name);
		return "czap";
	}
	else if(fe_info.type == FE_ATSC)
	{
		vverbose("Adapter %u is a '%s' ATSC card", adapter, fe_info.name);
		return "azap";
	}
	else
	{
		vverbose("Adapter %u (%s); unknown card type %d", adapter, fe_info.name, fe_info.type);
		return "";
	}
}

static FILE *_channels = NULL;

/*
 * if filename is NULL, it searches for:
 * ~/.<zap_name>/channels.conf
 * /etc/channels.conf
 * zap_name should be tzap for DVB-T, szap for DVB-S, czap for DVB-C or azap for ATSC cards
 */

bool
init_channels_conf(char *zap_name, char *filename)
{
	char *home;
	char pathname[PATH_MAX];

	if(_channels != NULL)
		fatal("init_channels_conf: already initialised");

	if(filename == NULL)
	{
		if((home = getenv("HOME")) != NULL)
		{
			snprintf(pathname, sizeof(pathname), "%s/.%s/channels.conf", home, zap_name);
			verbose("Trying to open %s", pathname);
			_channels = fopen(pathname, "r");
		}
		if(_channels == NULL)
		{
			verbose("Trying to open /etc/channels.conf");
			_channels = fopen("/etc/channels.conf", "r");
		}
	}
	else
	{
		verbose("Trying to open %s", filename);
		_channels = fopen(filename, "r");
	}

	return (_channels != NULL);
}

/*
 * map strings to frontend enum values
 * based on tzap utility in linuxtv dvb-apps package
 */

struct param
{
	char *name;
	int value;
};

static const struct param inversion_list[] =
{
	{ "INVERSION_OFF", INVERSION_OFF },
	{ "INVERSION_ON", INVERSION_ON },
	{ "INVERSION_AUTO", INVERSION_AUTO }
};

static const struct param bw_list[] =
{
	{ "BANDWIDTH_6_MHZ", BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", BANDWIDTH_8_MHZ }
};

static const struct param fec_list[] =
{
	{ "FEC_1_2", FEC_1_2 },
	{ "FEC_2_3", FEC_2_3 },
	{ "FEC_3_4", FEC_3_4 },
	{ "FEC_4_5", FEC_4_5 },
	{ "FEC_5_6", FEC_5_6 },
	{ "FEC_6_7", FEC_6_7 },
	{ "FEC_7_8", FEC_7_8 },
	{ "FEC_8_9", FEC_8_9 },
	{ "FEC_AUTO", FEC_AUTO },
	{ "FEC_NONE", FEC_NONE }
};

static const struct param guard_list[] =
{
	{"GUARD_INTERVAL_1_16", GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4", GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8", GUARD_INTERVAL_1_8}
};

static const struct param hierarchy_list[] =
{
	{ "HIERARCHY_1", HIERARCHY_1 },
	{ "HIERARCHY_2", HIERARCHY_2 },
	{ "HIERARCHY_4", HIERARCHY_4 },
	{ "HIERARCHY_NONE", HIERARCHY_NONE }
};

static const struct param qam_list[] =
{
	{ "QPSK", QPSK },
	{ "QAM_16", QAM_16 },
	{ "QAM_32", QAM_32 },
	{ "QAM_64", QAM_64 },
	{ "QAM_128", QAM_128 },
	{ "QAM_256", QAM_256 },
	{ "8VSB", VSB_8 },
	{ "16VSB", VSB_16 }
};

static const struct param transmissionmode_list[] =
{
	{ "TRANSMISSION_MODE_2K", TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K", TRANSMISSION_MODE_8K },
};

#define LIST_SIZE(x)	(sizeof(x) / sizeof(struct param))

static int
str2enum(char *str, const struct param *map, int map_size)
{
	while(map_size >= 0)
	{
		map_size --;
		if(strcmp(str, map[map_size].name) == 0)
			return map[map_size].value;
	}

	fatal("Invalid parameter '%s' in channels.conf file", str);

	/* not reached */
	return -1;
}

/*
 * return the params needed to tune to the given service_id
 * the data is read from the channels.conf file
 * returns false if the service_id is not found
 */

static bool
get_tune_params(fe_type_t fe_type, uint16_t service_id, struct dvb_frontend_parameters *out, char *pol, unsigned int *sat_no)
{
	if(_channels == NULL)
	{
		verbose("No channels.conf file available");
		return false;
	}

	bzero(out, sizeof(struct dvb_frontend_parameters));

	verbose("Searching channels.conf for service_id %u", service_id);

	rewind(_channels);

	if(fe_type == FE_OFDM)
		return get_dvbt_tune_params(service_id, out);
	else if(fe_type == FE_QPSK)
		return get_dvbs_tune_params(service_id, out, pol, sat_no);
	else if(fe_type == FE_QAM)
		return get_dvbc_tune_params(service_id, out);
	else if(fe_type == FE_ATSC)
		return get_atsc_tune_params(service_id, out);
	else
		error("Unknown DVB device type (%d)", fe_type);

	return false;
}

/*
 * DVB-T channels.conf format is:
 * name:freq:inversion:bandwidth:code_rate_HP:code_rate_LP:constellation:transmission_mode:guard_interval:hierarchy:vpid:apid:service_id
 * eg:
 * BBC ONE:722166670:INVERSION_AUTO:BANDWIDTH_8_MHZ:FEC_3_4:FEC_3_4:QAM_16:TRANSMISSION_MODE_2K:GUARD_INTERVAL_1_32:HIERARCHY_NONE:600:601:4165
 */

static bool
get_dvbt_tune_params(uint16_t service_id, struct dvb_frontend_parameters *out)
{
	char line[1024];
	unsigned int freq;
	char inv[32];
	char bw[32];
	char hp[32];
	char lp[32];
	char qam[32];
	char trans[32];
	char gi[32];
	char hier[32];
	unsigned int id;
	int len;

	while(!feof(_channels))
	{
		if(fgets(line, sizeof(line), _channels) == NULL
		|| sscanf(line, "%*[^:]:%u:%32[^:]:%32[^:]:%32[^:]:%32[^:]:%32[^:]:%32[^:]:%32[^:]:%32[^:]:%*[^:]:%*[^:]:%u", &freq, inv, bw, hp, lp, qam, trans, gi, hier, &id) != 10
		|| id != service_id)
			continue;
		/* chop off trailing \n */
		len = strlen(line) - 1;
		while(len >= 0 && line[len] == '\n')
			line[len--] = '\0';
		verbose("%s", line);
		out->frequency = freq;
		out->inversion = str2enum(inv, inversion_list, LIST_SIZE(inversion_list));
		out->u.ofdm.bandwidth = str2enum(bw, bw_list, LIST_SIZE(bw_list));
		out->u.ofdm.code_rate_HP = str2enum(hp, fec_list, LIST_SIZE(fec_list));
		out->u.ofdm.code_rate_LP = str2enum(lp, fec_list, LIST_SIZE(fec_list));
		out->u.ofdm.constellation = str2enum(qam, qam_list, LIST_SIZE(qam_list));
		out->u.ofdm.transmission_mode = str2enum(trans, transmissionmode_list, LIST_SIZE(transmissionmode_list));
		out->u.ofdm.guard_interval = str2enum(gi, guard_list, LIST_SIZE(guard_list));
		out->u.ofdm.hierarchy_information = str2enum(hier, hierarchy_list, LIST_SIZE(hierarchy_list));
		return true;
	}

	return false;
}

/*
 * DVB-S channels.conf format is:
 * name:freq:polarity:sat_number:symbol_rate:vpid:apid:service_id
 * eg:
 * Cartoon Network:10949:v:0:27500:6601:6611:7457
 */

static bool
get_dvbs_tune_params(uint16_t service_id, struct dvb_frontend_parameters *out, char *pol, unsigned int *sat_no)
{
	char line[1024];
	unsigned int freq;
	unsigned int sr;
	unsigned int id;
	int len;

	while(!feof(_channels))
	{
		if(fgets(line, sizeof(line), _channels) == NULL
		|| sscanf(line, "%*[^:]:%u:%c:%u:%u:%*[^:]:%*[^:]:%u", &freq, pol, sat_no, &sr, &id) != 5
		|| id != service_id)
			continue;
		/* chop off trailing \n */
		len = strlen(line) - 1;
		while(len >= 0 && line[len] == '\n')
			line[len--] = '\0';
		verbose("%s", line);
		out->frequency = freq * 1000;
		out->inversion = INVERSION_AUTO;
		out->u.qpsk.symbol_rate = sr * 1000;
		out->u.qpsk.fec_inner = FEC_AUTO;
		return true;
	}

	return false;
}

/*
 * DVB-C channels.conf format is:
 * name:freq:inversion:symbol_rate:fec:modulation:vpid:apid:service_id
 * eg:
 * Eurosport:394000000:INVERSION_OFF:6900000:FEC_NONE:QAM_64:410:420:16000
 */

static bool
get_dvbc_tune_params(uint16_t service_id, struct dvb_frontend_parameters *out)
{
	char line[1024];
	unsigned int freq;
	char inv[32];
	unsigned int sr;
	char fec[32];
	char mod[32];
	unsigned int id;
	int len;

	while(!feof(_channels))
	{
		if(fgets(line, sizeof(line), _channels) == NULL
		|| sscanf(line, "%*[^:]:%u:%32[^:]:%u:%32[^:]:%32[^:]:%*[^:]:%*[^:]:%u", &freq, inv, &sr, fec, mod, &id) != 6
		|| id != service_id)
			continue;
		/* chop off trailing \n */
		len = strlen(line) - 1;
		while(len >= 0 && line[len] == '\n')
			line[len--] = '\0';
		verbose("%s", line);
		out->frequency = freq;
		out->inversion = str2enum(inv, inversion_list, LIST_SIZE(inversion_list));
		out->u.qam.symbol_rate = sr;
		out->u.qam.fec_inner = str2enum(fec, fec_list, LIST_SIZE(fec_list));
		out->u.qam.modulation = str2enum(mod, qam_list, LIST_SIZE(qam_list));
		return true;
	}

	return false;
}

/*
 * ATSC channels.conf format is:
 * name:frequency:modulation:vpid:apid:service_id
 * eg:
 * Jazz-TV:647000000:QAM_256:49:52:3
 */

static bool
get_atsc_tune_params(uint16_t service_id, struct dvb_frontend_parameters *out)
{
	char line[1024];
	unsigned int freq;
	char mod[32];
	unsigned int id;
	int len;

	while(!feof(_channels))
	{
		if(fgets(line, sizeof(line), _channels) == NULL
		|| sscanf(line, "%*[^:]:%u:%32[^:]:%*[^:]:%*[^:]:%u", &freq, mod, &id) != 3
		|| id != service_id)
			continue;
		/* chop off trailing \n */
		len = strlen(line) - 1;
		while(len >= 0 && line[len] == '\n')
			line[len--] = '\0';
		verbose("%s", line);
		out->frequency = freq;
		/* out->inversion is not set by azap */
		out->u.vsb.modulation = str2enum(mod, qam_list, LIST_SIZE(qam_list));
		return true;
	}

	return false;
}

/* DISEQC code from dvbtune, written by Dave Chapman */

/*
 * digital satellite equipment control,
 * specification is available from http://www.eutelsat.com/
 */

static int
do_diseqc(int secfd, int sat_no, char pol, bool hi_lo)
{
	bool polv = (toupper(pol) == 'V');
	struct diseqc_cmd cmd =  { {{0xe0, 0x10, 0x38, 0xf0, 0x00, 0x00}, 4}, 0 };

	/*
	 * param: high nibble: reset bits, low nibble set bits,
	 * bits are: option, position, polarizaion, band
	 */
	cmd.cmd.msg[3] = 0xf0 | (((sat_no * 4) & 0x0f) | (hi_lo ? 1 : 0) | (polv ? 0 : 2));

	return diseqc_send_msg( secfd,
				polv ? SEC_VOLTAGE_13 : SEC_VOLTAGE_18,
				&cmd,
				hi_lo ? SEC_TONE_ON : SEC_TONE_OFF,
				(sat_no / 4) % 2 ? SEC_MINI_B : SEC_MINI_A);
}

static int
diseqc_send_msg(int fd, fe_sec_voltage_t v, struct diseqc_cmd *cmd, fe_sec_tone_mode_t t, fe_sec_mini_cmd_t b)
{
	if(ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) < 0)
		return -1;
	if(ioctl(fd, FE_SET_VOLTAGE, v) < 0)
		return -1;
	usleep(15 * 1000);
	if(ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd->cmd) < 0)
		return -1;
	usleep(cmd->wait * 1000);
	usleep(15 * 1000);
	if(ioctl(fd, FE_DISEQC_SEND_BURST, b) < 0)
		return -1;
	usleep(15 * 1000);
	if(ioctl(fd, FE_SET_TONE, t) < 0)
		return -1;

	return 0;
}

/*
 * retune to the frequency the given service_id is on
 */

bool
tune_service_id(unsigned int adapter, unsigned int timeout, uint16_t service_id)
{
	char fe_dev[PATH_MAX];
	bool got_info;
	struct dvb_frontend_info fe_info;
	struct dvb_frontend_parameters current_params;
	struct dvb_frontend_parameters needed_params;
	char polarity;
	unsigned int sat_no;
	bool hi_lo;
	struct dvb_frontend_event event;
//	fe_status_t status;
	bool lock;
	/* need to keep the frontend device open to stop it untuning itself */
	static int fe_fd = -1;
	static bool first_time = true;

	if(fe_fd < 0)
	{
		snprintf(fe_dev, sizeof(fe_dev), FE_DEVICE, adapter);
		/*
		 * need O_RDWR if you want to tune, O_RDONLY is okay for getting info
		 * if someone else is using the frontend, we can only open O_RDONLY
		 * => we can still download data, but just not retune
		 */
		if((fe_fd = open(fe_dev, O_RDWR | O_NONBLOCK)) < 0)
		{
			error("Unable to open '%s' read/write; you will not be able to retune", fe_dev);
			if((fe_fd = open(fe_dev, O_RDONLY | O_NONBLOCK)) < 0)
				fatal("open '%s': %s", fe_dev, strerror(errno));
			/* don't try to tune in */
			first_time = false;
		}
	}

	vverbose("Getting frontend info");

	do
	{
		/* maybe interrupted by a signal */
		got_info = (ioctl(fe_fd, FE_GET_INFO, &fe_info) >= 0);
		if(!got_info && errno != EINTR)
			fatal("ioctl FE_GET_INFO: %s", strerror(errno));
	}
	while(!got_info);

	/* see what we are currently tuned to */
	if(ioctl(fe_fd, FE_GET_FRONTEND, &current_params) < 0)
		fatal("ioctl FE_GET_FRONTEND: %s", strerror(errno));

	/* find the tuning params for the service */
	if(!get_tune_params(fe_info.type, service_id, &needed_params, &polarity, &sat_no))
	{
		error("service_id %u not found in channels.conf file", service_id);
		return false;
	}

	/*
	 * if no-one was using the frontend when we open it
	 * FE_GET_FRONTEND may say we are tuned to the frequency we want
	 * but when we try to read any data, it fails
	 * checking if we have a lock doesn't seem to work
	 * so, always retune the first time we are called
	 */
#if 0
	if(ioctl(fe_fd, FE_READ_STATUS, &status) < 0)
		lock = false;
	else
		lock = status & FE_HAS_LOCK;
#endif

	/* are we already tuned to the right frequency */
	vverbose("Current frequency %u; needed %u; first_time=%d", current_params.frequency, needed_params.frequency, first_time);

	/* frequency resolution is up to 1 kHz */
	if(first_time
	|| abs(current_params.frequency - needed_params.frequency) >= ONE_kHz)
	{
		first_time = false;
		verbose("Retuning to frequency %u", needed_params.frequency);
		/* empty event queue */
		while(ioctl(fe_fd, FE_GET_EVENT, &event) >= 0)
			; /* do nothing */
		/* do DISEQC (whatever that is) for DVB-S */
		if(fe_info.type == FE_QPSK)
		{
			if(needed_params.frequency < SLOF)
			{
				needed_params.frequency -= LOF1;
				hi_lo = false;
			}
			else
			{
				needed_params.frequency -= LOF2;
				hi_lo = true;
			}
			if(do_diseqc(fe_fd, sat_no, polarity, hi_lo) < 0)
				error("DISEQC command failed for service_id %u", service_id);
		}
		/* tune in */
		if(ioctl(fe_fd, FE_SET_FRONTEND, &needed_params) < 0)
			fatal("Unable to retune: ioctl FE_SET_FRONTEND: %s", strerror(errno));
		/* wait for lock */
		vverbose("Waiting for tuner to lock on");
		/* TODO: use timeout value here */
		lock = false;
		while(!lock)
		{
			if(ioctl(fe_fd, FE_GET_EVENT, &event) >= 0)
				lock = event.status & FE_HAS_LOCK;
		}
		vverbose("Retuned");
	}

	return true;
}

/*
 * returns true if service_id is listed in the channels file
 */

bool
service_available(uint16_t service_id)
{
	char line[1024];
	char *p;
	unsigned long id;

	rewind(_channels);

	while(!feof(_channels))
	{
		/* service_id is the last field for all channels.conf file formats */
		if(fgets(line, sizeof(line), _channels) == NULL
		|| (p = rindex(line, ':')) == NULL)
			continue;
		id = strtoul(p + 1, NULL, 0);
		if(id == service_id)
			return true;
	}

	return false;
}

