/*
 * command.c
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "command.h"
#include "findmheg.h"
#include "assoc.h"
#include "fs.h"
#include "stream.h"
#include "channels.h"
#include "utils.h"

/* max number of args that can be passed to a command (arbitrary) */
#define ARGV_MAX	10

/* the commands */
bool cmd_assoc(struct listen_data *, FILE *, int, char **);
bool cmd_ademux(struct listen_data *, FILE *, int, char **);
bool cmd_astream(struct listen_data *, FILE *, int, char **);
bool cmd_available(struct listen_data *, FILE *, int, char **);
bool cmd_avdemux(struct listen_data *, FILE *, int, char **);
bool cmd_avstream(struct listen_data *, FILE *, int, char **);
bool cmd_check(struct listen_data *, FILE *, int, char **);
bool cmd_file(struct listen_data *, FILE *, int, char **);
bool cmd_help(struct listen_data *, FILE *, int, char **);
bool cmd_quit(struct listen_data *, FILE *, int, char **);
bool cmd_retune(struct listen_data *, FILE *, int, char **);
bool cmd_service(struct listen_data *, FILE *, int, char **);
bool cmd_vdemux(struct listen_data *, FILE *, int, char **);
bool cmd_vstream(struct listen_data *, FILE *, int, char **);

static struct
{
	char *name;
	char *args;
	bool (*proc)(struct listen_data *, FILE *, int, char **);
	char *help;
} command[] =
{
	{ "assoc", "",						cmd_assoc,	"List component tag to PID mappings" },
	{ "ademux", "[<ServiceID>] <ComponentTag>",		cmd_ademux,	"Demux the given audio component tag" },
	{ "astream", "[<ServiceID>] <ComponentTag>",		cmd_astream,	"Stream the given audio component tag" },
	{ "available", "<ServiceID>",				cmd_available,	"Return OK if the ServiceID is available" },
	{ "avdemux", "[<ServiceID>] <AudioTag> <VideoTag>",	cmd_avdemux,	"Demux the given audio and video component tags" },
	{ "avstream", "[<ServiceID>] <AudioTag> <VideoTag>",	cmd_avstream,	"Stream the given audio and video component tags" },
	{ "check", "<ContentReference>",			cmd_check,	"Check if the given file exists on the carousel" },
	{ "exit", "",						cmd_quit,	"Close the connection" },
	{ "file", "<ContentReference>",				cmd_file,	"Retrieve the given file from the carousel" },
	{ "help", "",						cmd_help,	"List available commands" },
	{ "quit", "",						cmd_quit,	"Close the connection" },
	{ "retune", "<ServiceID>",				cmd_retune,	"Start downloading the carousel from ServiceID" },
	{ "service", "",					cmd_service,	"Show the current service ID" },
	{ "vdemux", "[<ServiceID>] <ComponentTag>",		cmd_vdemux,	"Demux the given video component tag" },
	{ "vstream", "[<ServiceID>] <ComponentTag>",		cmd_vstream,	"Stream the given video component tag" },
	{ NULL, NULL, NULL, NULL }
};

/* send an OK/error code etc response down client_sock */
#define SEND_RESPONSE(RC, MESSAGE)	fputs(#RC " " MESSAGE "\n", client)

/* internal routines */
char *external_filename(struct listen_data *, char *);
char *canonical_filename(char *);

/*
 * process the given command
 * return true if we should close the connection
 */

bool
process_command(struct listen_data *listen_data, FILE *client, char *cmd)
{
	int argc;
	char *argv[ARGV_MAX];
	char term;
	unsigned int cmd_len;
	int i;

	/* chop it into words, complicated by quoting */
	argc = 0;
	while(argc < ARGV_MAX && *cmd != '\0')
	{
		argv[argc++] = cmd;
		/* do we need to find the next space, or the next quote */
		if(*cmd == '\'' || *cmd == '"')
		{
			term = *cmd;
			/* remove/skip the opening quote */
			argv[argc-1] ++;
			cmd ++;
		}
		else
		{
			/* stop at the next space */
			term = ' ';
		}
		/* find the next terminating character */
		while(*cmd != term && *cmd != '\0')
			cmd ++;
		/* if this is not the end, skip to the start of the next word */
		if(*cmd != '\0')
		{
			/* terminate the last word */
			*cmd = '\0';
			/* move onto the next non-space character */
			cmd = skip_ws(cmd + 1);
		}
	}

	cmd_len = strlen(argv[0]);
	for(i=0; command[i].name != NULL; i++)
	{
		if(strncmp(argv[0], command[i].name, cmd_len) == 0)
			return (command[i].proc)(listen_data, client, argc, argv);
	}

	SEND_RESPONSE(500, "Unrecognised command");

	return false;
}

/*
 * the commands
 * listen_data is global data needed by listener commands
 * client is where any response data should go
 * argc is the number of arguments passed to it
 * argv[0] is the command name (or the abreviation the user used)
 * return true if we should close the connection
 */

#define CHECK_USAGE(ARGC, SYNTAX)		\
if(argc != ARGC)				\
{						\
	SEND_RESPONSE(500, "Syntax: " SYNTAX);	\
	return false;				\
}

/* variable number of arguments */
#define CHECK_VUSAGE(MIN, MAX, SYNTAX)		\
if(argc < MIN || argc > MAX)			\
{						\
	SEND_RESPONSE(500, "Syntax: " SYNTAX);	\
	return false;				\
}

/*
 * assoc
 * show the association/component tag to PID mappings
 * also shows the default audio and video PIDs
 * (just for debugging)
 */

bool
cmd_assoc(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	unsigned int i;

	SEND_RESPONSE(200, "OK");

	fprintf(client, "Tag\tPID\tType\n");
	fprintf(client, "===\t===\t====\n");

	/* default audio and video PIDs */
	fprintf(client, "(audio)\t%u\t%u\n", car->audio_pid, car->audio_type);
	fprintf(client, "(video)\t%u\t%u\n", car->video_pid, car->video_type);

	/* component tag mappings */
	for(i=0; i<car->assoc.nassocs; i++)
		fprintf(client, "%u\t%u\t%u\n", car->assoc.sids[i], car->assoc.pids[i], car->assoc.types[i]);

	/* terminator */
	fprintf(client, ".\n");

	return false;
}

/*
 * ademux [<service_id>] <tag>
 * add the given audio stream to the PID filter
 * return the name of the dvr device to read the transport stream from
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tag should be an association/component_tag number as found in the PMT
 * the tag is converted to a PID and that PID is used as a demux filter
 * if tag is -1, the default audio stream for the service_id is used
 */

bool
cmd_ademux(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int tag;
	struct avstreams *streams;
	int audio_fd;
	char hdr[64];

	CHECK_VUSAGE(2, 3, "ademux [<ServiceID>] <ComponentTag>");

	if(argc == 2)
	{
		service = -1;
		tag = strtol(argv[1], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		tag = strtol(argv[2], NULL, 0);
	}

	streams = find_avstreams(car, service, tag, -1);

	/* check we have a default stream */
	if(streams->audio_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve audio PID");
		return false;
	}

	/* add the PID to the demux device */
	if((audio_fd = add_demux_filter(car->demux_device, streams->audio_pid, DMX_PES_AUDIO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open audio PID");
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PID and stream type the component tag resolved to */
	snprintf(hdr, sizeof(hdr), "AudioPID %u AudioType %u\n", streams->audio_pid, streams->audio_type);
	fputs(hdr, client);

	/* tell the client where the dvr device is */
	snprintf(hdr, sizeof(hdr), "Device %s\n", car->dvr_device);
	fputs(hdr, client);

	fflush(client);

	/* keep the filter in place until the client closes or we get an error */
	while(!feof(client))
		sleep(1);

	/* clean up */
	ioctl(audio_fd, DMX_STOP);
	close(audio_fd);

	/* close the connection */
	return true;
}

/*
 * astream [<service_id>] <tag>
 * send the given audio stream down the connection
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tag should be an association/component_tag number as found in the PMT
 * the tag is converted to a PID and that PID is sent as a MPEG Transport Stream down the connection
 * if tag is -1, the default audio stream for the service_id is sent
 */

bool
cmd_astream(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int tag;
	struct avstreams *streams;
	int audio_fd;
	int ts_fd;
	char hdr[64];

	CHECK_VUSAGE(2, 3, "astream [<ServiceID>] <ComponentTag>");

	if(argc == 2)
	{
		service = -1;
		tag = strtol(argv[1], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		tag = strtol(argv[2], NULL, 0);
	}

	streams = find_avstreams(car, service, tag, -1);

	/* check we have a default stream */
	if(streams->audio_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve audio PID");
		return false;
	}

	/* add the PID to the demux device */
	if((audio_fd = add_demux_filter(car->demux_device, streams->audio_pid, DMX_PES_AUDIO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open audio PID");
		return false;
	}

	/* we can now read a transport stream from the dvr device */
	if((ts_fd = open(car->dvr_device, O_RDONLY)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open DVB device");
		close(audio_fd);
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PID and stream type the component tag resolved to */
	snprintf(hdr, sizeof(hdr), "AudioPID %u AudioType %u\n", streams->audio_pid, streams->audio_type);
	fputs(hdr, client);

	/* shovel the transport stream to client until the client closes or we get an error */
	stream_ts(ts_fd, client);

	/* clean up */
	ioctl(audio_fd, DMX_STOP);
	close(audio_fd);
	close(ts_fd);

	/* close the connection */
	return true;
}

/*
 * vdemux [<service_id>] <tag>
 * add the given video stream to the PID filter
 * return the name of the dvr device to read the transport stream from
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tag should be an association/component_tag number as found in the PMT
 * the tag is converted to a PID and that PID is used as a demux filter
 * if tag is -1, the default video stream for the service_id is used
 */

bool
cmd_vdemux(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int tag;
	struct avstreams *streams;
	int video_fd;
	char hdr[64];

	CHECK_VUSAGE(2, 3, "vdemux [<ServiceID>] <ComponentTag>");

	if(argc == 2)
	{
		service = -1;
		tag = strtol(argv[1], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		tag = strtol(argv[2], NULL, 0);
	}

	streams = find_avstreams(car, service, -1, tag);

	/* check we have a default stream */
	if(streams->video_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve video PID");
		return false;
	}

	/* add the PID to the demux device */
	if((video_fd = add_demux_filter(car->demux_device, streams->video_pid, DMX_PES_VIDEO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open video PID");
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PID and stream type the component tag resolved to */
	snprintf(hdr, sizeof(hdr), "VideoPID %u VideoType %u\n", streams->video_pid, streams->video_type);
	fputs(hdr, client);

	/* tell the client where the dvr device is */
	snprintf(hdr, sizeof(hdr), "Device %s\n", car->dvr_device);
	fputs(hdr, client);

	fflush(client);

	/* keep the filter in place until the client closes or we get an error */
	while(!feof(client))
		sleep(1);

	/* clean up */
	ioctl(video_fd, DMX_STOP);
	close(video_fd);

	/* close the connection */
	return true;
}

/*
 * vstream [<service_id>] <tag>
 * send the given video stream down the connection
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tag should be an association/component_tag number as found in the PMT
 * the tag is converted to a PID and that PID is sent as a MPEG Transport Stream down the connection
 * if tag is -1, the default video stream for the service_id is sent
 */

bool
cmd_vstream(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int tag;
	struct avstreams *streams;
	int video_fd;
	int ts_fd;
	char hdr[64];

	CHECK_VUSAGE(2, 3, "vstream [<ServiceID>] <ComponentTag>");

	if(argc == 2)
	{
		service = -1;
		tag = strtol(argv[1], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		tag = strtol(argv[2], NULL, 0);
	}

	streams = find_avstreams(car, service, -1, tag);

	/* check we have a default stream */
	if(streams->video_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve video PID");
		return false;
	}

	/* add the PID to the demux device */
	if((video_fd = add_demux_filter(car->demux_device, streams->video_pid, DMX_PES_VIDEO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open video PID");
		return false;
	}

	/* we can now read a transport stream from the dvr device */
	if((ts_fd = open(car->dvr_device, O_RDONLY)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open DVB device");
		close(video_fd);
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PID and stream type the component tag resolved to */
	snprintf(hdr, sizeof(hdr), "VideoPID %u VideoType %u\n", streams->video_pid, streams->video_type);
	fputs(hdr, client);

	/* shovel the transport stream down client_sock until the client closes it or we get an error */
	stream_ts(ts_fd, client);

	/* clean up */
	ioctl(video_fd, DMX_STOP);
	close(video_fd);
	close(ts_fd);

	/* close the connection */
	return true;
}

/*
 * avdemux <audio_tag> <video_tag>
 * add the given audio and video streams to the PID filter
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tags should be association/component_tag numbers as found in the PMT
 * the tags are converted to PIDs and those PIDs are used as the demux filter
 * if a tag is -1, the default audio or video stream for the service_id is used
 */

bool
cmd_avdemux(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int audio_tag;
	int video_tag;
	struct avstreams *streams;
	int audio_fd;
	int video_fd;
	char hdr[64];

	CHECK_VUSAGE(3, 4, "avdemux [<ServiceID>] <AudioTag> <VideoTag>");

	if(argc == 3)
	{
		service = -1;
		audio_tag = strtol(argv[1], NULL, 0);
		video_tag = strtol(argv[2], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		audio_tag = strtol(argv[2], NULL, 0);
		video_tag = strtol(argv[3], NULL, 0);
	}

	streams = find_avstreams(car, service, audio_tag, video_tag);

	/* check we have a default stream */
	if(streams->audio_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve audio PID");
		return false;
	}
	if(streams->video_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve video PID");
		return false;
	}

	/* add the PIDs to the demux device */
	if((audio_fd = add_demux_filter(car->demux_device, streams->audio_pid, DMX_PES_AUDIO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open audio PID");
		return false;
	}
	if((video_fd = add_demux_filter(car->demux_device, streams->video_pid, DMX_PES_VIDEO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open video PID");
		close(audio_fd);
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PIDs and stream types the component tags resolved to */
	snprintf(hdr, sizeof(hdr), "AudioPID %u AudioType %u VideoPID %u VideoType %u\n",
				    streams->audio_pid, streams->audio_type, streams->video_pid, streams->video_type);
	fputs(hdr, client);

	/* tell the client where the dvr device is */
	snprintf(hdr, sizeof(hdr), "Device %s\n", car->dvr_device);
	fputs(hdr, client);

	fflush(client);

	/* keep the filter in place until the client closes or we get an error */
	while(!feof(client))
		sleep(1);

	/* clean up */
	ioctl(audio_fd, DMX_STOP);
	ioctl(video_fd, DMX_STOP);
	close(audio_fd);
	close(video_fd);

	/* close the connection */
	return true;
}

/*
 * avstream <audio_tag> <video_tag>
 * send the given audio and video streams down the connection
 * if service_id is not specified or is -1, use the service we are downloading the carousel from
 * the tags should be association/component_tag numbers as found in the PMT
 * the tags are converted to PIDs and those PIDs are sent as a MPEG Transport Stream down the connection
 * if a tag is -1, the default audio or video stream for the service_id is sent
 */

bool
cmd_avstream(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	int service;
	int audio_tag;
	int video_tag;
	struct avstreams *streams;
	int audio_fd;
	int video_fd;
	int ts_fd;
	char hdr[64];

	CHECK_VUSAGE(3, 4, "avstream [<ServiceID>] <AudioTag> <VideoTag>");

	if(argc == 3)
	{
		service = -1;
		audio_tag = strtol(argv[1], NULL, 0);
		video_tag = strtol(argv[2], NULL, 0);
	}
	else
	{
		service = strtol(argv[1], NULL, 0);
		audio_tag = strtol(argv[2], NULL, 0);
		video_tag = strtol(argv[3], NULL, 0);
	}

	streams = find_avstreams(car, service, audio_tag, video_tag);

	/* check we have a default stream */
	if(streams->audio_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve audio PID");
		return false;
	}
	if(streams->video_pid == 0)
	{
		SEND_RESPONSE(500, "Unable to resolve video PID");
		return false;
	}

	/* add the PIDs to the demux device */
	if((audio_fd = add_demux_filter(car->demux_device, streams->audio_pid, DMX_PES_AUDIO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open audio PID");
		return false;
	}
	if((video_fd = add_demux_filter(car->demux_device, streams->video_pid, DMX_PES_VIDEO)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open video PID");
		close(audio_fd);
		return false;
	}

	/* we can now read a transport stream from the dvr device */
	if((ts_fd = open(car->dvr_device, O_RDONLY)) < 0)
	{
		SEND_RESPONSE(500, "Unable to open DVB device");
		close(audio_fd);
		close(video_fd);
		return false;
	}

	/* send the OK code */
	SEND_RESPONSE(200, "OK");

	/* tell the client what PIDs and stream types the component tags resolved to */
	snprintf(hdr, sizeof(hdr), "AudioPID %u AudioType %u VideoPID %u VideoType %u\n",
				    streams->audio_pid, streams->audio_type, streams->video_pid, streams->video_type);
	fputs(hdr, client);

	/* shovel the transport stream down client_sock until the client closes it or we get an error */
	stream_ts(ts_fd, client);

	/* clean up */
	ioctl(audio_fd, DMX_STOP);
	ioctl(video_fd, DMX_STOP);
	close(audio_fd);
	close(video_fd);
	close(ts_fd);

	/* close the connection */
	return true;
}

/*
 * available <ServiceID>
 * returns 200 OK if ServiceID is listed in the channels.conf file
 */

bool
cmd_available(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	unsigned int service_id;

	CHECK_USAGE(2, "available <ServiceID>");

	service_id = strtoul(argv[1], NULL, 0);

	if(service_available(service_id))
		SEND_RESPONSE(200, "OK");
	else
		SEND_RESPONSE(404, "Not Found");

	return false;
}

/*
 * check <ContentReference>
 * check if the given file is on the carousel
 * ContentReference should be absolute, ie start with "~//"
 */

bool
cmd_check(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	char *filename;
	FILE *file;

	CHECK_USAGE(2, "check <ContentReference>");

	if((filename = external_filename(listen_data, argv[1])) == NULL)
	{
		SEND_RESPONSE(500, "Invalid ContentReference");
		return false;
	}

	if((file = fopen(filename, "r")) != NULL)
	{
		fclose(file);
		SEND_RESPONSE(200, "OK");
	}
	else
	{
		SEND_RESPONSE(404, "Not found");
	}

	return false;
}

/*
 * file <ContentReference>
 * send the given file down client_sock
 * ContentReference should be absolute, ie start with "~//"
 */

bool
cmd_file(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	char *filename;
	struct stat info;
	FILE *file;
	long size;
	char hdr[64];
	long left;
	size_t nread;
	char buff[1024 * 8];

	CHECK_USAGE(2, "file <ContentReference>");

	if((filename = external_filename(listen_data, argv[1])) == NULL)
	{
		SEND_RESPONSE(500, "Invalid ContentReference");
		return false;
	}

	/* check it is a regular file */
	if(stat(filename, &info) < 0
	|| !S_ISREG(info.st_mode))
	{
		SEND_RESPONSE(500, "Invalid file");
		return false;
	}

	if((file = fopen(filename, "r")) == NULL)
	{
		SEND_RESPONSE(404, "Not found");
		return false;
	}

	/* find the file length */
	if(fseek(file, 0, SEEK_END) < 0
	|| (size = ftell(file)) < 0)
	{
		SEND_RESPONSE(500, "Error reading file");
		return false;
	}
	rewind(file);

	SEND_RESPONSE(200, "OK");

	/* send the file length */
	snprintf(hdr, sizeof(hdr), "Length %ld\n", size);
	fputs(hdr, client);

	/* send the file contents */
	left = size;
	while(left > 0)
	{
		nread = fread(buff, 1, sizeof(buff), file);
		fwrite(buff, 1, nread, client);
		left -= nread;
	}

	fclose(file);

	return false;
}

/*
 * retune <ServiceID>
 * stop downloading the current carousel
 * start downloading the carousel on the given ServiceID
 */

bool
cmd_retune(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;
	unsigned int service_id;
	union sigval value;

	CHECK_USAGE(2, "retune <ServiceID>");

	service_id = strtoul(argv[1], NULL, 0);

	/* do we need to retune */
	if(service_id != car->service_id)
	{
		/* send a SIGHUP to the main listener process */
		value.sival_int = service_id;
		sigqueue(getppid(), SIGHUP, value);
	}

	SEND_RESPONSE(200, "OK");

	/* need to close the connection as this process now has stale listen_data->carousel */
	return true;
}

/*
 * service
 * show the service ID we are downloading the carousel from
 * will be in the form, "dvb://network_id..service_id"
 */

bool
cmd_service(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	struct carousel *car = listen_data->carousel;

	SEND_RESPONSE(200, "OK");

	fprintf(client, "dvb://%x..%x\n", car->network_id, car->service_id);

	return false;
}

/*
 * help
 */

bool
cmd_help(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	int i;
	char name_args[64];
	char help_line[128];

	SEND_RESPONSE(200, "OK");

	for(i=0; command[i].name != NULL; i++)
	{
		snprintf(name_args, sizeof(name_args), "%s %s", command[i].name, command[i].args);
		snprintf(help_line, sizeof(help_line), "%-30s %s\n", name_args, command[i].help);
		fputs(help_line, client);
	}

	return false;
}

/*
 * quit
 */

bool
cmd_quit(struct listen_data *listen_data, FILE *client, int argc, char *argv[])
{
	return true;
}

/*
 * return a filename that can be used to load the given ContentReference from the filesystem
 * returns a static string that will be overwritten by the next call to this routine
 * returns NULL if the ContentReference is invalid (does not start with ~// or has too many .. components)
 */

static char _external[PATH_MAX];

char *
external_filename(struct listen_data *listen_data, char *cref)
{
	char *canon_cref;

	/* is ContentReference absolute */
	if(strlen(cref) < 3 || strncmp(cref, "~//", 3) != 0)
		return NULL;

	/* strip off the ~// prefix, and canonicalise the reference */
	canon_cref = canonical_filename(cref + 3);

	/* if the canonical name starts with "../", it is invalid */
	if(strcmp(canon_cref, "..") == 0 || strncmp(canon_cref, "../", 3) == 0)
		return NULL;

	/* create the carousel filename, ie prepend the servive gateway directory */
	snprintf(_external, sizeof(_external), "%s/%u/%s", SERVICES_DIR, listen_data->carousel->service_id, canon_cref);

	return _external;
}

/*
 * return a string that recursively removes all sequences of the form '/x/../' in path
 * returns a static string that will be overwritten by the next call to this routine
 */

static char _canon[PATH_MAX];

char *
canonical_filename(char *path)
{
	char *start;
	char *slash;
	size_t len;

	/* copy path into the output buffer */
	strncpy(_canon, path, sizeof(_canon));
	/* just in case */
	_canon[sizeof(_canon)-1] = '\0';

	/* keep removing "/x/../" until there are none left */
	start = _canon;
	while(true)
	{
		/* find the start of the first path component that is not "../" */
		while(strncmp(start, "../", 3) == 0)
			start += 3;
		/* find the next slash in the path */
		slash = start;
		while(*slash != '/' && *slash != '\0')
			slash ++;
		/* no more slashes => nothing left to do */
		if(*slash == '\0')
			return _canon;
		/* if this component is empty (ie ./ or /) remove it */
		if(strncmp(start, "./", 2) == 0 || *start == '/')
		{
			/* include \0 terminator */
			len = strlen(slash + 1) + 1;
			memmove(start, slash + 1, len);
			/* restart the search */
			start = _canon;
		}
		/* if the next path component is "../", eat this one */
		else if(strncmp(slash, "/../", 4) == 0)
		{
			/* include \0 terminator */
			len = strlen(slash + 4) + 1;
			memmove(start, slash + 4, len);
			/* restart the search */
			start = _canon;
		}
		else
		{
			/* move to the next component */
			start = slash + 1;
		}
	}

	/* not reached */
	return NULL;
}

