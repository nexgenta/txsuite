/* png2rgb: convert one or more PNG files to a raw 24-bit RGB sequence */

/* Copyright (c) 2009 Mo McRoberts.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the author(s) of this software may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <png.h>
#include <setjmp.h>
#include <unistd.h>

typedef struct convertbuf_struct convertbuf_t;

struct convertbuf_struct
{
	png_bytep *rows;
	size_t nrows;
	size_t width;
	size_t rowbytes;
};

static const char *progname = "png2rgb";

static void
updateprogname(char *arg)
{
	char *t;
	
	if(NULL != arg)
	{
		if(t = strrchr(arg, '/'))
		{
			progname = &(t[1]);
		}
		else
		{
			progname = arg;
		}
	}	
}

int
png2rgb(convertbuf_t *cb, const char *filename, FILE *fout)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height, y;
	int depth, coltype;
	png_bytep *pp;
	size_t rowbytes, passes, n;
	FILE *fp;
	png_byte header[8];
	
	if(NULL == (fp = fopen(filename, "rb")))
	{
		perror(filename);
		return -1;
	}
	fread(header, 1, sizeof(header), fp);
	if(0 != png_sig_cmp(header, 0, sizeof(header)))
	{
		fprintf(stderr, "%s: does not appear to be a PNG file\n", filename);
		fclose(fp);
		return -1;
	}
	if(NULL == (png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		fprintf(stderr, "%s: failed to initialise libpng (png_create_read_struct)\n", progname);
		fclose(fp);
		return -1;
	}
	if(NULL == (info_ptr = png_create_info_struct(png_ptr)))
	{
		fprintf(stderr, "%s: failed to initialise libpng (png_create_info_struct)\n", progname);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		return -1;
	}
#ifdef PNG_SETJMP_SUPPORTED
	if((setjmp(png_jmpbuf(png_ptr))))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return -1;
	}
#endif
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sizeof(header));
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &coltype, NULL, NULL, NULL);
	if(cb->nrows  && (cb->nrows != height || cb->width != width))
	{
		fprintf(stderr, "%s: image dimensions differ from first frame (was %lu x %lu, now %lu x %lu), skipping.\n",
			filename, cb->width, cb->nrows, width, height);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return -1;
	}
	if(0 == cb->nrows)
	{
		if(NULL == (pp = (png_bytep *) realloc(cb->rows, sizeof(png_bytep) * height)))
		{
			fprintf(stderr, "%s: failed to realloc to %lu bytes\n", progname, sizeof(png_bytep) * height);
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(fp);
			return -1;
		}
		cb->rows = pp;
		cb->nrows = height;
		cb->width = width;
		memset(cb->rows, 0, sizeof(png_bytep) * height);
	}
	if(coltype == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}
	if(coltype == PNG_COLOR_TYPE_GRAY || coltype == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		if(depth < 8)
		{
			png_set_gray_1_2_4_to_8(png_ptr);
		}
		png_set_gray_to_rgb(png_ptr);
	}
	if(depth == 16)
	{
		png_set_strip_16(png_ptr);
	}
	if(coltype & PNG_COLOR_MASK_ALPHA)
	{
		png_set_strip_alpha(png_ptr);
	}
	if(depth < 8)
	{
		png_set_packing(png_ptr);
	}
	passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	if(cb->rowbytes != rowbytes)
	{
		for (y = 0; y < height; y++)
		{
			cb->rows[y] = (png_bytep) realloc(cb->rows[y], rowbytes);
		}
	}
	for(n = 0; n < passes; n++)
	{
		png_read_rows(png_ptr, cb->rows, NULL, height);
	}
	for(y = 0; y < height; y++)
	{
		if(1 != fwrite(cb->rows[y], 1, rowbytes, fout))
		{
			perror(filename);
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(fp);
			return -2;
		}
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return 0;
}

int
main(int argc, char **argv)
{
	int c, fatal, r;
	FILE *fp, *fout;
	uint8_t header[8];
	convertbuf_t cbuf;
	unsigned max, seq, n, errs, written;
	char filename[PATH_MAX], sb[16], *t;
	
	updateprogname(argv[0]);
	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s OUTPUT SOURCE [...]\n", progname);
		return 1;
	}
	if(NULL == (fout = fopen(argv[1], "wb")))
	{
		perror(argv[1]);
		return 1;
	}
	memset(&cbuf, 0, sizeof(cbuf));
	for(c = 2, errs = 0, written = 0; c < argc; c++)
	{
		if(NULL != (t = strchr(argv[c], '?')))
		{
			/* It's an image sequence pattern */
			max = 1;
			n = 0;
			while(*t == '?')
			{
				*t = 0;
				t++;
				max *= 10;
				n++;
			}
			sprintf(sb, "%%s%%0%uu%%s", n);
			for(n = 0, fatal = 0; n < max; n++)
			{
				sprintf(filename, sb, argv[c], n, t);
				if(-1 == access(filename, R_OK) && ENOENT == errno)
				{
					if(0 == n)
					{
						continue;
					}
					break;
				}
				fprintf(stderr, "%s: processing %s\n", progname, filename);
				if(-1 == (r = png2rgb(&cbuf, filename, fout)))
				{
					errs++;
					break;
				}
				else if(-2 == r)
				{
					errs++;
					fatal = 1;
					break;
				}
				written++;
			}
			if(fatal)
			{
				break;
			}
		}
		else
		{
			fprintf(stderr, "%s: processing %s\n", progname, argv[c]);
			if(-1 == (r = png2rgb(&cbuf, argv[c], fout)))
			{
				errs++;
			}
			else if(-2 == r)
			{
				errs++;
				break;
			}
			else
			{
				written++;
			}
		}
	}
	fclose(fout);
	for(c = 0; c < cbuf.nrows; c++)
	{
		free(cbuf.rows[c]);
	}
	free(cbuf.rows);
	fprintf(stderr, "%s: written %u frames to %s at %u x %u (8 bits per channel)", progname, written, argv[1], cbuf.width, cbuf.nrows);
	if(errs)
	{
		fprintf(stderr, "; %u errors", errs);
	}
	fputc('\n', stderr);
	return 0;
}
