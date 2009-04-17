/*
 * utils.c
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "utils.h"

/*
 * returns a PIX_FMT_xxx type that matches the given bits per pixel and RGB bit mask values
 * returns PIX_FMT_NONE if none match
 */

enum PixelFormat
find_av_pix_fmt(int bpp, unsigned long rmask, unsigned long gmask, unsigned long bmask)
{
	enum PixelFormat fmt;

	fmt = PIX_FMT_NONE;
	switch(bpp)
	{
	case 32:
		if(rmask == 0xff0000 && gmask == 0xff00 && bmask == 0xff)
			fmt = PIX_FMT_RGBA32;
		break;

	case 24:
		if(rmask == 0xff0000 && gmask == 0xff00 && bmask == 0xff)
			fmt = PIX_FMT_RGB24;
		else if(rmask == 0xff && gmask == 0xff00 && bmask == 0xff0000)
			fmt = PIX_FMT_BGR24;
		break;

	case 16:
		if(rmask == 0xf800 && gmask == 0x07e0 && bmask == 0x001f)
			fmt = PIX_FMT_RGB565;
		else if(rmask == 0x7c00 && gmask == 0x03e0 && bmask == 0x001f)
			fmt = PIX_FMT_RGB555;
		break;

	default:
		break;
	}

	return fmt;
}

/*
 * returns 15 for 'f' etc
 */

unsigned int
char2hex(unsigned char c)
{
	if(!isxdigit(c))
		return 0;
	else if(c >= '0' && c <= '9')
		return c - '0';
	else
		return 10 + (tolower(c) - 'a');
}

/*
 * returns the next UTF8 character in the given text
 * size should be the amount of data available in text
 * sets *used to the number of bytes in the UTF8 character
 * gives up if the char is more than 4 bytes long
 */

int
next_utf8(unsigned char *text, int size, int *used)
{
	if(size >= 1 && (text[0] & 0x80) == 0)
	{
		*used = 1;
		return text[0];
	}
	else if(size >= 2 && (text[0] & 0xe0) == 0xc0)
	{
		*used = 2;
		return ((text[0] & 0x1f) << 6) + (text[1] & 0x3f);
	}
	else if(size >= 3 && (text[0] & 0xf0) == 0xe0)
	{
		*used = 3;
		return ((text[0] & 0x0f) << 12) + ((text[1] & 0x3f) << 6) + (text[2] & 0x3f);
	}
	else if(size >= 4 && (text[0] & 0xf8) == 0xf0)
	{
		*used = 4;
		return ((text[0] & 0x07) << 18) + ((text[1] & 0x3f) << 12) + ((text[2] & 0x3f) << 6) + (text[3] & 0x3f);
	}
	else if(size > 0)
	{
		/* error, return the next char */
		*used = 1;
		return text[0];
	}
	else
	{
		*used = 0;
		return 0;
	}
}

/*
 * I don't want to double the size of my code just to deal with malloc failures
 * if you've run out of memory you're fscked anyway, me trying to recover is not gonna help...
 */

#ifdef DEBUG_ALLOC
static int _nallocs = 0;
#endif

void *
safe_malloc(size_t nbytes)
{
	void *buf;

#ifdef DEBUG_ALLOC
	_nallocs ++;
	fprintf(stderr, "safe_malloc: %d\n", _nallocs);
#endif

	if((buf = malloc(nbytes)) == NULL)
		fatal("Out of memory");

	return buf;
}

void *
safe_mallocz(size_t nbytes)
{
	void *buf = safe_malloc(nbytes);

	bzero(buf, nbytes);

	return buf;
}

/*
 * safe_realloc(NULL, n) == safe_malloc(n)
 * safe_realloc(x, 0) == safe_free(x) and returns NULL
 */

void *
safe_realloc(void *oldbuf, size_t nbytes)
{
	void *newbuf;

	if(nbytes == 0)
	{
		safe_free(oldbuf);
		return NULL;
	}

	if(oldbuf == NULL)
		return safe_malloc(nbytes);

	if((newbuf = realloc(oldbuf, nbytes)) == NULL)
		fatal("Out of memory");

	return newbuf;
}

/*
 * only calls safe_realloc if nbytes > *oldsize
 * updates *oldsize if it calls safe_realloc
 */

void *
safe_fast_realloc(void *oldbuf, size_t *oldsize, size_t nbytes)
{
	if(nbytes > *oldsize)
	{
		oldbuf = safe_realloc(oldbuf, nbytes);
		*oldsize = nbytes;
	}

	return oldbuf;
}

/*
 * safe_free(NULL) is okay
 */

void
safe_free(void *buf)
{
	if(buf != NULL)
	{
		free(buf);
#ifdef DEBUG_ALLOC
		_nallocs--;
		fprintf(stderr, "safe_free: %d\n", _nallocs);
#endif
	}

	return;
}

/*
 * safe_strdup(NULL) == NULL
 */

char *
safe_strdup(const char *src)
{
	char *dst;

	if(src == NULL)
		return NULL;

	dst = (char *) safe_malloc(strlen(src) + 1);
	strcpy(dst, src);

	return dst;
}

void
error(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vprintf(message, ap);
	printf("\n");
	va_end(ap);

	return;
}

void
fatal(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vprintf(message, ap);
	printf("\n");
	va_end(ap);

	exit(EXIT_FAILURE);
}

