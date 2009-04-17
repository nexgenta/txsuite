/*
 * MHEGVideoOutput.c
 */

#include <stdbool.h>
#include <string.h>

#include "MHEGEngine.h"
#include "MHEGVideoOutput.h"
#include "videoout_null.h"
#include "videoout_xshm.h"
#include "utils.h"

static struct
{
	char *name;
	char *desc;
	MHEGVideoOutputMethod *fns;
} vo_methods[] =
{
	{ "null", "No video output", &vo_null_fns},
	{ "xshm", "Uses X11 Shared Memory", &vo_xshm_fns},
	{ NULL, NULL}
};

#define DEFAULT_VO_METHOD	&vo_xshm_fns

/*
 * pass NULL to use the default
 */

MHEGVideoOutputMethod *
MHEGVideoOutputMethod_fromString(char *name)
{
	unsigned int i;

	if(name == NULL)
		return DEFAULT_VO_METHOD;

	for(i=0; vo_methods[i].name; i++)
		if(strcasecmp(name, vo_methods[i].name) == 0)
			return vo_methods[i].fns;

	fatal("Unknown video output method '%s'. %s", name, MHEGVideoOutputMethod_getUsage());

	/* not reached */
	return NULL;
}

/* must be big enough to hold the names of them all */
static char _usage[512];

char *
MHEGVideoOutputMethod_getUsage(void)
{
	unsigned int i;
	char method[80];

	snprintf(_usage, sizeof(_usage), "Available video output methods are:");

	for(i=0; vo_methods[i].name; i++)
	{
		bool dflt = (vo_methods[i].fns == DEFAULT_VO_METHOD);
		snprintf(method, sizeof(method), "\n%s\t%s%s", vo_methods[i].name, vo_methods[i].desc, dflt ? " (default)" : "");
		/* assumes _usage[] is big enough */
		strcat(_usage, method);
	}

	return _usage;
}

void
MHEGVideoOutput_init(MHEGVideoOutput *v, MHEGVideoOutputMethod *fns)
{
	/* assert */
	if(fns == NULL)
		fatal("MHEGVideoOutput_init: video output method not defined");

	v->fns = fns;
	v->ctx = (*(v->fns->init))();

	return;
}

void
MHEGVideoOutput_fini(MHEGVideoOutput *v)
{
	return (*(v->fns->fini))(v->ctx);
}

/*
 * get ready to draw the given frame at the given output size
 */

void
MHEGVideoOutput_prepareFrame(MHEGVideoOutput *v, VideoFrame *f, unsigned int out_width, unsigned int out_height)
{
	return (*(v->fns->prepareFrame))(v->ctx, f, out_width, out_height);
}

/*
 * draw the frame set up by MHEGVideoOutput_prepareFrame() at the given position on the contents Pixmap
 */

void
MHEGVideoOutput_drawFrame(MHEGVideoOutput *v, int x, int y)
{
	return (*(v->fns->drawFrame))(v->ctx, x, y);
}

