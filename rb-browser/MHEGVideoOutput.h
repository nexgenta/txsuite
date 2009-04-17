/*
 * MHEGVideoOutput.h
 */

#ifndef __MHEGVIDEOOUTPUT_H__
#define __MHEGVIDEOOUTPUT_H__

typedef struct
{
	void *ctx;			/* context passed to MHEGVideoOutputFns */
	struct MHEGVideoOutputFns
	{
		/* return a new ctx */
		void *(*init)(void);
		/* free the given ctx */
		void (*fini)(void *);
		/* get ready to draw the given YUV frame at the given size */
		void (*prepareFrame)(void *, VideoFrame *, unsigned int, unsigned int);
		/* draw the frame setup by prepareFrame at the given location */
		void (*drawFrame)(void *, int, int);
	} *fns;
} MHEGVideoOutput;

typedef struct MHEGVideoOutputFns MHEGVideoOutputMethod;

MHEGVideoOutputMethod *MHEGVideoOutputMethod_fromString(char *);
char *MHEGVideoOutputMethod_getUsage(void);

void MHEGVideoOutput_init(MHEGVideoOutput *, MHEGVideoOutputMethod *);
void MHEGVideoOutput_fini(MHEGVideoOutput *);

void MHEGVideoOutput_prepareFrame(MHEGVideoOutput *, VideoFrame *, unsigned int, unsigned int);
void MHEGVideoOutput_drawFrame(MHEGVideoOutput *, int, int);

#endif 	/* __MHEGVIDEOOUTPUT_H__ */

