/*
 * utils.h
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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdarg.h>
#include <ffmpeg/avformat.h>

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

enum PixelFormat find_av_pix_fmt(int, unsigned long, unsigned long, unsigned long);

unsigned int char2hex(unsigned char);

int next_utf8(unsigned char *, int, int *);

void *safe_malloc(size_t);
void *safe_mallocz(size_t);
void *safe_realloc(void *, size_t);
void *safe_fast_realloc(void *, size_t *, size_t);
void safe_free(void *);

char *safe_strdup(const char *);

void error(char *, ...);
void fatal(char *, ...);

#endif	/* __UTILS_H__ */

