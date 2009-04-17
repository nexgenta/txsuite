/*
 * videoout_xshm.c
 */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <ffmpeg/avformat.h>

#include "MHEGEngine.h"
#include "MHEGVideoOutput.h"
#include "videoout_xshm.h"
#include "utils.h"

void *vo_xshm_init(void);
void vo_xshm_fini(void *);
void vo_xshm_prepareFrame(void *, VideoFrame *, unsigned int, unsigned int);
void vo_xshm_drawFrame(void *, int, int);

MHEGVideoOutputMethod vo_xshm_fns =
{
	vo_xshm_init,
	vo_xshm_fini,
	vo_xshm_prepareFrame,
	vo_xshm_drawFrame
};

static void vo_xshm_create_frame(vo_xshm_ctx *, unsigned int, unsigned int);
static void vo_xshm_resize_frame(vo_xshm_ctx *, unsigned int, unsigned int);
static void vo_xshm_destroy_frame(vo_xshm_ctx *);

void *
vo_xshm_init(void)
{
	vo_xshm_ctx *v = safe_mallocz(sizeof(vo_xshm_ctx));

	v->current_frame = NULL;

	v->resize_ctx = NULL;
	v->resized_data = NULL;

	return v;
}

void
vo_xshm_fini(void *ctx)
{
	vo_xshm_ctx *v = (vo_xshm_ctx *) ctx;

	if(v->resize_ctx != NULL)
	{
		img_resample_close(v->resize_ctx);
		safe_free(v->resized_data);
	}

	if(v->current_frame != NULL)
		vo_xshm_destroy_frame(v);

	safe_free(ctx);

	return;
}

void
vo_xshm_prepareFrame(void *ctx, VideoFrame *f, unsigned int out_width, unsigned int out_height)
{
	vo_xshm_ctx *v = (vo_xshm_ctx *) ctx;
	AVPicture *yuv_frame;
	int resized_size;

	/* have we created the output frame yet */
	if(v->current_frame == NULL)
		vo_xshm_create_frame(v, out_width, out_height);

	/* see if the output size has changed since the last frame */
	if(v->current_frame->width != out_width || v->current_frame->height != out_height)
		vo_xshm_resize_frame(v, out_width, out_height);

	/* see if the input size is different than the output size */
	if(f->width != out_width || f->height != out_height)
	{
		/* have the resize input or output dimensions changed */
		if(v->resize_ctx == NULL
		|| v->resize_in.width != f->width || v->resize_in.height != f->height
		|| v->resize_out.width != out_width || v->resize_out.height != out_height)
		{
			/* get rid of any existing resize context */
			if(v->resize_ctx != NULL)
				img_resample_close(v->resize_ctx);
			if((v->resize_ctx = img_resample_init(out_width, out_height, f->width, f->height)) == NULL)
				fatal("Out of memory");
			/* remember the resize input and output dimensions */
			v->resize_in.width = f->width;
			v->resize_in.height = f->height;
			v->resize_out.width = out_width;
			v->resize_out.height = out_height;
			/* somewhere to store the resized frame */
			if((resized_size = avpicture_get_size(f->pix_fmt, out_width, out_height)) < 0)
				fatal("vo_xshm_prepareFrame: invalid frame size");
			v->resized_data = safe_realloc(v->resized_data, resized_size);
			avpicture_fill(&v->resized_frame, v->resized_data, f->pix_fmt, out_width, out_height);
		}
		/* resize it */
		img_resample(v->resize_ctx, &v->resized_frame, &f->frame);
		yuv_frame = &v->resized_frame;
	}
	else
	{
		yuv_frame = &f->frame;
	}

	/* convert the frame to RGB */
	img_convert(&v->rgb_frame, v->out_format, yuv_frame, f->pix_fmt, out_width, out_height);

	return;
}

void
vo_xshm_drawFrame(void *ctx, int x, int y)
{
	vo_xshm_ctx *v = (vo_xshm_ctx *) ctx;
	MHEGDisplay *d = MHEGEngine_getDisplay();
	unsigned int out_width;
	unsigned int out_height;

	if(v->current_frame != NULL)
	{
		/* video frame is already scaled as needed */
		out_width = v->current_frame->width;
		out_height = v->current_frame->height;
		/* draw it onto the Window contents Pixmap */
		XShmPutImage(d->dpy, d->contents, d->win_gc, v->current_frame, 0, 0, x, y, out_width, out_height, False);
		/* get it drawn straight away */
		XFlush(d->dpy);
	}

	return;
}

static void
vo_xshm_create_frame(vo_xshm_ctx *v, unsigned int out_width, unsigned int out_height)
{
	MHEGDisplay *d = MHEGEngine_getDisplay();
	int rgb_size;

	if((v->current_frame = XShmCreateImage(d->dpy, d->vis, d->depth, ZPixmap, NULL, &v->shm, out_width, out_height)) == NULL)
		fatal("XShmCreateImage failed");

	/* work out what ffmpeg pixel format matches our XImage format */
	if((v->out_format = find_av_pix_fmt(v->current_frame->bits_per_pixel,
					    d->vis->red_mask, d->vis->green_mask, d->vis->blue_mask)) == PIX_FMT_NONE)
		fatal("Unsupported XImage pixel format");

	rgb_size = v->current_frame->bytes_per_line * out_height;

	if(rgb_size != avpicture_get_size(v->out_format, out_width, out_height))
		fatal("XImage and ffmpeg pixel formats differ");

	if((v->shm.shmid = shmget(IPC_PRIVATE, rgb_size, IPC_CREAT | 0777)) == -1)
		fatal("shmget failed");
	if((v->shm.shmaddr = shmat(v->shm.shmid, NULL, 0)) == (void *) -1)
		fatal("shmat failed");
	v->shm.readOnly = True;
	if(!XShmAttach(d->dpy, &v->shm))
		fatal("XShmAttach failed");

	/* we made sure these pixel formats are the same */
	v->current_frame->data = v->shm.shmaddr;
	avpicture_fill(&v->rgb_frame, v->shm.shmaddr, v->out_format, out_width, out_height);

	return;
}

static void
vo_xshm_resize_frame(vo_xshm_ctx  *v, unsigned int out_width, unsigned int out_height)
{
/* TODO */
/* better if create_frame makes the max size shm we will need (ie d->xres, d->yres) */
/* then this just updates the XImage and AVFrame params in current_frame and rgb_frame */

	if(v->current_frame != NULL)
		vo_xshm_destroy_frame(v);

	vo_xshm_create_frame(v, out_width, out_height);

	return;
}

static void
vo_xshm_destroy_frame(vo_xshm_ctx  *v)
{
	MHEGDisplay *d = MHEGEngine_getDisplay();

	/* get rid of the current frame */
	/* the XImage data is our shared memory, make sure XDestroyImage doesn't try to free it */
	v->current_frame->data = NULL;
	XDestroyImage(v->current_frame);
	/* make sure no-one tries to use it */
	v->current_frame = NULL;

	/* get rid of the shared memory */
	XShmDetach(d->dpy, &v->shm);
	shmdt(v->shm.shmaddr);
	shmctl(v->shm.shmid, IPC_RMID, NULL);

	return;
}

