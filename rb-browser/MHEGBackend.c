/*
 * MHEGBackend.c
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "MHEGEngine.h"
#include "si.h"
#include "utils.h"

/* internal functions */
static FILE *remote_command(MHEGBackend *, bool, char *);
static unsigned int remote_response(FILE *);

static MHEGStream *open_stream(MHEGBackend *, int, bool, int *, int *, bool, int *, int *);
static void close_stream(MHEGBackend *, MHEGStream *);

static void local_set_service_url(MHEGBackend *);
static void remote_set_service_url(MHEGBackend *);

static const OctetString *get_service_url(MHEGBackend *);

static int parse_addr(char *, struct in_addr *, in_port_t *);
static int get_host_addr(char *, struct in_addr *);

static char *external_filename(MHEGBackend *, OctetString *);

/* local backend funcs */
bool local_checkContentRef(MHEGBackend *, ContentReference *);
bool local_loadFile(MHEGBackend *, OctetString *, OctetString *);
FILE *local_openFile(MHEGBackend *, OctetString *);
void local_retune(MHEGBackend *, OctetString *);
bool local_isServiceAvailable(MHEGBackend *, OctetString *);

static struct MHEGBackendFns local_backend_fns =
{
	local_checkContentRef,		/* checkContentRef */
	local_loadFile,			/* loadFile */
	local_openFile,			/* openFile */
	open_stream,			/* openStream */
	close_stream,			/* closeStream */
	local_retune,			/* retune */
	get_service_url,		/* getServiceURL */
	local_isServiceAvailable,	/* isServiceAvailable */
};

/* remote backend funcs */
bool remote_checkContentRef(MHEGBackend *, ContentReference *);
bool remote_loadFile(MHEGBackend *, OctetString *, OctetString *);
FILE *remote_openFile(MHEGBackend *, OctetString *);
void remote_retune(MHEGBackend *, OctetString *);
bool remote_isServiceAvailable(MHEGBackend *, OctetString *);

static struct MHEGBackendFns remote_backend_fns =
{
	remote_checkContentRef,		/* checkContentRef */
	remote_loadFile,		/* loadFile */
	remote_openFile,		/* openFile */
	open_stream,			/* openStream */
	close_stream,			/* closeStream */
	remote_retune,			/* retune */
	get_service_url,		/* getServiceURL */
	remote_isServiceAvailable,	/* isServiceAvailable */
};

/* public interface */

void
MHEGBackend_init(MHEGBackend *b, bool remote, char *srg_loc)
{
	bzero(b, sizeof(MHEGBackend));

	/* default backend is on the loopback */
	b->addr.sin_family = AF_INET;
	b->addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	b->addr.sin_port = htons(DEFAULT_REMOTE_PORT);

	/* no connection to the backend yet */
	b->be_sock = NULL;

	/* don't know rec://svc/def yet */
	b->rec_svc_def.size = 0;
	b->rec_svc_def.data = NULL;

	if(remote)
	{
		/* backend is on a different host, srg_loc is the remote host[:port] */
		b->fns = &remote_backend_fns;
		b->base_dir = NULL;
		if(parse_addr(srg_loc, &b->addr.sin_addr, &b->addr.sin_port) < 0)
			fatal("Unable to resolve host %s", srg_loc);
		verbose("Remote backend at %s:%u", inet_ntoa(b->addr.sin_addr), ntohs(b->addr.sin_port));
		/* initialise rec://svc/def value */
		remote_set_service_url(b);
	}
	else
	{
		/* backend and frontend on same host, srg_loc is the base directory */
		b->fns = &local_backend_fns;
		b->base_dir = safe_strdup(srg_loc);
		verbose("Local backend; carousel file root '%s'", srg_loc);
		/* initialise rec://svc/def value */
		local_set_service_url(b);
	}

	verbose("Current service is %.*s", b->rec_svc_def.size, b->rec_svc_def.data);

	return;
}

void
MHEGBackend_fini(MHEGBackend *b)
{
	/* send quit command */
	if(b->be_sock != NULL
	&& remote_command(b, true, "quit\n") != NULL)
		fclose(b->be_sock);

	safe_free(b->base_dir);

	safe_free(b->rec_svc_def.data);

	return;
}

/*
 * send the given command to the remote backend
 * if reuse is true, reuse the existing connection to the backend
 * returns a socket FILE to read the response from
 * returns NULL if it can't contact the backend
 */

static FILE *
remote_command(MHEGBackend *t, bool reuse, char *cmd)
{
	int sock;
	FILE *file;

	/* can we use the existing connection */
	if(reuse && t->be_sock != NULL)
	{
		fputs(cmd, t->be_sock);
		return t->be_sock;
	}

	/* need to connect to the backend */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		error("Unable to create backend socket: %s", strerror(errno));
		return NULL;
	}
	if(connect(sock, (struct sockaddr *) &t->addr, sizeof(struct sockaddr_in)) < 0)
	{
		error("Unable to connect to backend: %s", strerror(errno));
		close(sock);
		return NULL;
	}

	/* associate a FILE with the socket (so stdio can do buffering) */
	if((file = fdopen(sock, "r+")) != NULL)
	{
		/* send the command */
		fputs(cmd, file);
	}
	else
	{
		error("Unable to buffer backend connection: %s", strerror(errno));
		close(sock);
	}

	/* remember it if we need to reuse it */
	if(reuse)
		t->be_sock = file;

	return file;
}

/*
 * read the backend response from the given socket FILE
 * returns the OK/error code
 */

#define BACKEND_RESPONSE_OK	200
#define BACKEND_RESPONSE_ERROR	500

static unsigned int
remote_response(FILE *file)
{
	char buf[1024];
	unsigned int rc;

	/* read upto \n */
	if(fgets(buf, sizeof(buf), file) == NULL)
		return BACKEND_RESPONSE_ERROR;

	rc = atoi(buf);

	return rc;
}

/*
 * return a read-only FILE handle for an MPEG Transport Stream (in MHEGStream->ts)
 * the TS will contain an audio stream (if have_audio is true) and a video stream (if have_video is true)
 * the *audio_tag and *video_tag numbers refer to Component/Association Tag values from the DVB PMT
 * if *audio_tag or *video_tag is -1, the default audio and/or video stream for the given Service ID is used
 * if service_id is -1, it uses the Service ID we are downloading the carousel from
 * updates *audio_tag and/or *video_tag to the actual PIDs in the Transport Stream
 * updates *audio_type and/or *video_type to the stream type IDs
 * returns NULL on error
 */

static MHEGStream *
open_stream(MHEGBackend *t,
	    int service_id,
	    bool have_audio, int *audio_tag, int *audio_type,
	    bool have_video, int *video_tag, int *video_type)
{
	MHEGStream *stream;
	bool loopback;
	char *avcmd;
	char cmd[PATH_MAX];
	FILE *be;
	char pids[PATH_MAX];
	unsigned int audio_pid = 0;
	unsigned int video_pid = 0;
	bool err;
	char *ts_dev;
	int tail;

	/* are the backend and frontend on the same host */
	loopback = (t->addr.sin_addr.s_addr == htonl(INADDR_LOOPBACK));
	if(loopback)
		avcmd = "demux";
	else
		avcmd = "stream";

	/* no PIDs required */
	if(!have_audio && !have_video)
		return NULL;
	/* video and audio */
	else if(have_audio && have_video)
		snprintf(cmd, sizeof(cmd), "av%s %d %d %d\n", avcmd, service_id, *audio_tag, *video_tag);
	/* audio only */
	else if(have_audio)
		snprintf(cmd, sizeof(cmd), "a%s %d %d\n", avcmd, service_id, *audio_tag);
	/* video only */
	else
		snprintf(cmd, sizeof(cmd), "v%s %d %d\n", avcmd, service_id, *video_tag);

	/* false => create a new connection to the backend */
	if((be = remote_command(t, false, cmd)) == NULL)
		return NULL;

	/* did it work */
	if(remote_response(be) != BACKEND_RESPONSE_OK
	|| fgets(pids, sizeof(pids), be) == NULL)
	{
		fclose(be);
		return NULL;
	}

	/* update the PID variables */
	if(have_audio && have_video)
		err = (sscanf(pids, "AudioPID %u AudioType %u VideoPID %u VideoType %u",
		       &audio_pid, audio_type, &video_pid, video_type) != 4);
	else if(have_audio)
		err = (sscanf(pids, "AudioPID %u AudioType %u", &audio_pid, audio_type) != 2);
	else
		err = (sscanf(pids, "VideoPID %u VideoType %u", &video_pid, video_type) != 2);

	if(err)
	{
		fclose(be);
		return NULL;
	}

	/* set up the MHEGStream */
	stream = safe_malloc(sizeof(MHEGStream));

	/*
	 * if we sent a "demux" command, open the DVR device
	 * if we sent a "stream" command, the TS is streamed from the backend
	 */
	if(loopback)
	{
		/* backend tells us where the DVR device is */
		if(fgets(pids, sizeof(pids), be) == NULL
		|| strncmp(pids, "Device ", 7) != 0)
		{
			fclose(be);
			safe_free(stream);
			return NULL;
		}
		ts_dev = pids + 7;
		/* chop off any trailing \n */
		tail = strlen(ts_dev) - 1;
		while(tail > 0 && ts_dev[tail] == '\n')
			ts_dev[tail--] = '\0';
		if((stream->ts = fopen(ts_dev, "r")) == NULL)
		{
			fclose(be);
			safe_free(stream);
			return NULL;
		}
		/* backend keeps the PID filters in place until we close this connection */
		stream->demux = be;
	}
	else
	{
		stream->ts = be;
		stream->demux = NULL;
	}

	/* now we are sure it all worked, set up the return values */
	if(have_audio)
		*audio_tag = audio_pid;
	if(have_video)
		*video_tag = video_pid;

	return stream;
}

static void
close_stream(MHEGBackend *t, MHEGStream *stream)
{
	if(stream == NULL)
		return;

	if(stream->ts != NULL)
		fclose(stream->ts);

	if(stream->demux != NULL)
		fclose(stream->demux);

	safe_free(stream);

	return;
}

/*
 * update rec_svc_def to the service directory we are reading the carousel from
 * rec_svc_def will be in dvb:// format, but the network_id will be empty
 * eg if we are reading path/to/services/4165, then rec_svc_def will be dvb://..1045
 */

static void
local_set_service_url(MHEGBackend *t)
{
	char *slash;
	int prefix_len;
	int service_id;
	char url[1024];
	size_t len;

	/* base_dir is: [path/to/services/]<service_id> */
	slash = strrchr(t->base_dir, '/');
	if(slash == NULL)
	{
		/* no preceeding path */
		service_id = atoi(t->base_dir);
	}
	else
	{
		prefix_len = (slash - t->base_dir) + 1;
		service_id = atoi(t->base_dir + prefix_len);
	}

	/* create a fake dvb:// format URL */
	len = snprintf(url, sizeof(url), "dvb://..%x", service_id);

	/* overwrite any existing value */
	t->rec_svc_def.size = len;
	t->rec_svc_def.data = safe_realloc(t->rec_svc_def.data, len);
	memcpy(t->rec_svc_def.data, url, len);

	return;
}

/*
 * update rec_svc_def to the service we are downloading the carousel from
 * rec_svc_def will be in dvb:// format
 */

static void
remote_set_service_url(MHEGBackend *t)
{
	char cmd[32];
	FILE *sock;
	char url[1024];
	size_t len;

	/* send backend a "service" command, response is carousel service in dvb:// format */
	snprintf(cmd, sizeof(cmd), "service\n");

	if((sock = remote_command(t, true, cmd)) == NULL
	|| remote_response(sock) != BACKEND_RESPONSE_OK
	|| fgets(url, sizeof(url), sock) == NULL)
	{
		/* this should never happen, and I don't want a NULL rec_svc_def */
		fatal("Unable to determine current service");
	}

	/* chop any trailing \n off the URL */
	len = strlen(url);
	while(len > 0 && url[len-1] == '\n')
		len --;

	/* overwrite any existing value */
	t->rec_svc_def.size = len;
	t->rec_svc_def.data = safe_realloc(t->rec_svc_def.data, len);
	memcpy(t->rec_svc_def.data, url, len);

	return;
}

static const OctetString *
get_service_url(MHEGBackend *t)
{
	return (const OctetString *) &t->rec_svc_def;
}

/*
 * extract the IP addr and port number from a string in one of these forms:
 * host:port
 * ip-addr:port
 * host
 * ip-addr
 * if the port is not defined in the string, the value passed to this routine is unchanged
 * ip and port are both returned in network byte order
 * returns -1 on error (can't resolve host name)
 */

static int
parse_addr(char *str, struct in_addr *ip, in_port_t *port)
{
	char *p;

	if((p = strchr(str, ':')) != NULL)
	{
		/* its either host:port or ip:port */
		*(p++) = '\0';
		if(get_host_addr(str, ip) < 0)
			return -1;
		*port = htons(atoi(p));
		/* reconstruct the string */
		*(--p) = ':';
	}
	else if(get_host_addr(str, ip) < 0)
	{
		return -1;
	}

	return 0;
}

/*
 * puts the IP address associated with the given host into output buffer
 * host can be a.b.c.d or a host name
 * returns 0 if successful, -1 on error
 */

static int
get_host_addr(char *host, struct in_addr *output)
{
	struct hostent *he;
	int error = 0;

	if(((he = gethostbyname(host)) != NULL) && (he->h_addrtype == AF_INET))
		memcpy(output, he->h_addr, sizeof(struct in_addr));
	else
		error = -1;

	return error;
}

/*
 * returns a filename that can be loaded from the file system
 * ie ~// at the start of the absolute name is replaced with base_dir
 * returns a ptr to a static string that will be overwritten by the next call to this routine
 */

static char _external[PATH_MAX];

static char *
external_filename(MHEGBackend *t, OctetString *name)
{
	char *absolute;

	/* convert it to an absolute group id, ie with a "~//" at the start */
	absolute = MHEGEngine_absoluteFilename(name);

	/* construct the filename */
	snprintf(_external, sizeof(_external), "%s%s", t->base_dir, &absolute[2]);

	return _external;
}

/*
 * local routines
 */

/*
 * returns true if the file exists on the carousel
 */

bool
local_checkContentRef(MHEGBackend *t, ContentReference *name)
{
	bool found = false;
	FILE *file;

	if((file = fopen(external_filename(t, name), "r")) != NULL)
	{
		fclose(file);
		found = true;
	}

	return found;
}

/*
 * file contents are stored in out (out->data will need to be free'd)
 * returns false if it can't load the file (out will be {0,NULL})
 * out should be {0,NULL} before calling this
 */

bool
local_loadFile(MHEGBackend *t, OctetString *name, OctetString *out)
{
	char *fullname;
	FILE *file;

	fullname = external_filename(t, name);

	/* open it */
	if((file = fopen(fullname, "r")) == NULL)
	{
		error("Unable to open '%.*s': %s", name->size, name->data, strerror(errno));
		return false;
	}

	verbose("Loading '%.*s'", name->size, name->data);

	fseek(file, 0, SEEK_END);
	out->size = ftell(file);
	out->data = safe_malloc(out->size);
	rewind(file);
	if(fread(out->data, 1, out->size, file) != out->size)
	{
		error("Unable to load '%.*s'", name->size, name->data);
		safe_free(out->data);
		out->size = 0;
		out->data = NULL;
	}

	fclose(file);

	return (out->data != NULL);
}

/*
 * return a read-only FILE handle for the given carousel file
 * returns NULL on error
 */

FILE *
local_openFile(MHEGBackend *t, OctetString *name)
{
	char *external = external_filename(t, name);

	return fopen(external, "r");
}

/*
 * retune the backend to the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

void
local_retune(MHEGBackend *t, OctetString *service)
{
	unsigned int service_id;
	char service_str[64];
	char *slash;
	int prefix_len;

	/* assert */
	if(service->size < 6 || strncmp(service->data, "dvb://", 6) != 0)
		fatal("local_retune: unable to tune to '%.*s'", service->size, service->data);

	/* extract the service_id */
	service_id = si_get_service_id(service);
	snprintf(service_str, sizeof(service_str), "%u", service_id);

	/*
	 * base_dir is: [path/to/services/]<service_id>
	 * so we just need to replace the last filename component with the new service_id
	 */
	slash = strrchr(t->base_dir, '/');
	if(slash == NULL)
	{
		/* no preceeding path */
		t->base_dir = safe_realloc(t->base_dir, strlen(service_str) + 1);
		strcpy(t->base_dir, service_str);
	}
	else
	{
		prefix_len = (slash - t->base_dir) + 1;
		t->base_dir = safe_realloc(t->base_dir, prefix_len + strlen(service_str) + 1);
		strcpy(t->base_dir + prefix_len, service_str);
	}

	/* update rec://svc/def */
	local_set_service_url(t);

	verbose("Retune: new service gateway is '%s'", t->base_dir);

	return;
}

/*
 * return true if we are able to receive the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

bool
local_isServiceAvailable(MHEGBackend *t, OctetString *service)
{
	unsigned int service_id;
	char service_str[64];
	char *slash;
	int prefix_len;
	char service_dir[PATH_MAX];
	struct stat stats;
	bool exists;

	/* assert */
	if(service->size < 6 || strncmp(service->data, "dvb://", 6) != 0)
		fatal("local_isServiceAvailable: invalid service '%.*s'", service->size, service->data);

	/* extract the service_id */
	service_id = si_get_service_id(service);
	snprintf(service_str, sizeof(service_str), "%u", service_id);

	/*
	 * base_dir is: [path/to/services/]<service_id>
	 * so we just need to replace the last filename component with the new service_id
	 */
	slash = strrchr(t->base_dir, '/');
	if(slash == NULL)
	{
		/* no preceeding path */
		snprintf(service_dir, sizeof(service_dir), "%s", service_str);
	}
	else
	{
		prefix_len = (slash - t->base_dir) + 1;
		snprintf(service_dir, sizeof(service_dir), "%.*s%s", prefix_len, t->base_dir, service_str);
	}

	/* see if the directory for the service exists */
	exists = (stat(service_dir, &stats) == 0);

	return exists;
}

/*
 * remote routines
 */

/*
 * returns true if the file exists on the carousel
 */

bool
remote_checkContentRef(MHEGBackend *t, ContentReference *name)
{
	char cmd[PATH_MAX];
	FILE *sock;
	bool exists;

	snprintf(cmd, sizeof(cmd), "check %s\n", MHEGEngine_absoluteFilename(name));

	if((sock = remote_command(t, true, cmd)) == NULL)
		return false;

	exists = (remote_response(sock) == BACKEND_RESPONSE_OK);

	return exists;
}

/*
 * file contents are stored in out (out->data will need to be free'd)
 * returns false if it can't load the file (out will be {0,NULL})
 * out should be {0,NULL} before calling this
 */

bool
remote_loadFile(MHEGBackend *t, OctetString *name, OctetString *out)
{
	char cmd[PATH_MAX];
	FILE *sock;
	unsigned int size;
	size_t nread;

	snprintf(cmd, sizeof(cmd), "file %s\n", MHEGEngine_absoluteFilename(name));

	if((sock = remote_command(t, true, cmd)) == NULL)
		return false;

	/* if it exists, read the file size */
	if(remote_response(sock) != BACKEND_RESPONSE_OK
	|| fgets(cmd, sizeof(cmd), sock) == NULL
	|| sscanf(cmd, "Length %u", &size) != 1)
	{
		error("Unable to load '%.*s'", name->size, name->data);
		return false;
	}

	verbose("Loading '%.*s'", name->size, name->data);

	out->size = size;
	out->data = safe_malloc(size);

	nread = 0;
	while(!feof(sock) && nread < size)
		nread += fread(out->data + nread, 1, size - nread, sock);

	/* did we read it all */
	if(nread < size)
	{
		error("Unable to load '%.*s'", name->size, name->data);
		safe_free(out->data);
		out->data = NULL;
		out->size = 0;
		return false;
	}

	return true;
}

/*
 * return a read-only FILE handle for the given carousel file
 * returns NULL on error
 */

FILE *
remote_openFile(MHEGBackend *t, OctetString *name)
{
	char cmd[PATH_MAX];
	FILE *sock;
	unsigned int size;
	char buf[8 * 1024];
	size_t nread;
	FILE *out;

	snprintf(cmd, sizeof(cmd), "file %s\n", MHEGEngine_absoluteFilename(name));

	if((sock = remote_command(t, true, cmd)) == NULL)
		return NULL;

	/* if it exists, read the file size */
	if(remote_response(sock) != BACKEND_RESPONSE_OK
	|| fgets(cmd, sizeof(cmd), sock) == NULL
	|| sscanf(cmd, "Length %u", &size) != 1)
	{
		return NULL;
	}

	/* tmpfile() will delete the file when we fclose() it */
	out = tmpfile();
	while(out != NULL && size > 0)
	{
		nread = (size < sizeof(buf)) ? size : sizeof(buf);
		nread = fread(buf, 1, nread, sock);
		if(fwrite(buf, 1, nread, out) != nread)
		{
			error("Unable to write to local file");
			fclose(out);
			out = NULL;
		}
		size -= nread;
	}

	/* rewind the file */
	if(out != NULL)
		rewind(out);

	return out;
}

/*
 * retune the backend to the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

void
remote_retune(MHEGBackend *t, OctetString *service)
{
	char cmd[128];
	FILE *sock;

	/* assert */
	if(service->size < 6 || strncmp(service->data, "dvb://", 6) != 0)
		fatal("remote_retune: unable to tune to '%.*s'", service->size, service->data);

	snprintf(cmd, sizeof(cmd), "retune %u\n", si_get_service_id(service));

	if((sock = remote_command(t, true, cmd)) == NULL
	|| remote_response(sock) != BACKEND_RESPONSE_OK)
	{
		error("Unable to retune to '%.*s' (service_id %u)", service->size, service->data, si_get_service_id(service));
	}

	/* a "retune" command closes the connection to the backend, so close our end */
	if(t->be_sock != NULL)
	{
		fclose(t->be_sock);
		t->be_sock = NULL;
	}

	/* update rec://svc/def */
	remote_set_service_url(t);

	return;
}

/*
 * return true if we are able to receive the given service
 * service should be in the form "dvb://<network_id>..<service_id>", eg "dvb://233a..4C80"
 */

bool
remote_isServiceAvailable(MHEGBackend *t, OctetString *service)
{
	char cmd[128];
	FILE *sock;
	bool available = true;

	/* assert */
	if(service->size < 6 || strncmp(service->data, "dvb://", 6) != 0)
		fatal("remote_isServiceAvailable: invalid service '%.*s'", service->size, service->data);

	snprintf(cmd, sizeof(cmd), "available %u\n", si_get_service_id(service));

	if((sock = remote_command(t, true, cmd)) == NULL
	|| remote_response(sock) != BACKEND_RESPONSE_OK)
	{
		available = false;
	}

	return available;
}
