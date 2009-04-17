/*
 * MHEGBackend.h
 */

#ifndef __MHEGBACKEND_H__
#define __MHEGBACKEND_H__

#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>

/* default TCP port to contact backend on */
#define DEFAULT_REMOTE_PORT	10101

#define DEFAULT_BACKEND		"127.0.0.1"

/* MPEG Transport Stream */
typedef struct
{
	FILE *ts;	/* the Transport Stream */
	FILE *demux;	/* private */
} MHEGStream;

typedef struct MHEGBackend
{
	OctetString rec_svc_def;	/* service we are downloading the carousel from */
	char *base_dir;			/* local Service Gateway root directory */
	struct sockaddr_in addr;	/* remote backend IP and port */
	FILE *be_sock;			/* connection to remote backend */
	/* function pointers */
	struct MHEGBackendFns
	{
		/* check a carousel file exists */
		bool (*checkContentRef)(struct MHEGBackend *, ContentReference *);
		/* load a carousel file */
		bool (*loadFile)(struct MHEGBackend *, OctetString *, OctetString *);
		/* open a carousel file */
		FILE *(*openFile)(struct MHEGBackend *, OctetString *);
		/* open an MPEG Transport Stream */
		MHEGStream *(*openStream)(struct MHEGBackend *, int, bool, int *, int *, bool, int *, int *);
		/* close an MPEG Transport Stream */
		void (*closeStream)(struct MHEGBackend *, MHEGStream *);
		/* tune to the given service */
		void (*retune)(struct MHEGBackend *, OctetString *);
		/* return a dvb:// URL for the service we are currently downloading the carousel from */
		const OctetString *(*getServiceURL)(struct MHEGBackend *);
		/* return true if the engine is able to receive the given service (dvb:// URL format) */
		bool (*isServiceAvailable)(struct MHEGBackend *, OctetString *);
	} *fns;
} MHEGBackend;

void MHEGBackend_init(MHEGBackend *, bool, char *);
void MHEGBackend_fini(MHEGBackend *);

#endif	/* __MHEGBACKEND_H__ */
