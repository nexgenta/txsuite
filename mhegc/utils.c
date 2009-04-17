/*
 * utils.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

void
write_all(FILE *out, unsigned char *buf, size_t len)
{
	if(fwrite(buf, 1, len, out) != len)
		fatal("Error writing to file: %s", strerror(errno));

	return;
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
 * I don't want to double the size of my code just to deal with malloc failures
 * if you've run out of memory you're fscked anyway, me trying to recover is not gonna help...
 */

void *
safe_malloc(size_t nbytes)
{
	void *buf;

	if((buf = malloc(nbytes)) == NULL)
		fatal("Out of memory");

	return buf;
}

/*
 * safe_realloc(NULL, n) == safe_malloc(n)
 */

void *
safe_realloc(void *oldbuf, size_t nbytes)
{
	void *newbuf;

	if(oldbuf == NULL)
		return safe_malloc(nbytes);

	if((newbuf = realloc(oldbuf, nbytes)) == NULL)
		fatal("Out of memory");

	return newbuf;
}

/*
 * safe_free(NULL) is okay
 */

void
safe_free(void *buf)
{
	if(buf != NULL)
		free(buf);

	return;
}

void
error(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	return;
}

void
fatal(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	exit(EXIT_FAILURE);
}

/* number of bytes per line */
#define HEXDUMP_WIDTH	16

void
hexdump(FILE *out, unsigned char *data, size_t nbytes)
{
	size_t nout;
	int i;

	nout = 0;
	while(nout < nbytes)
	{
		/* byte offset at start of line */
		if((nout % HEXDUMP_WIDTH) == 0)
			fprintf(out, "0x%.8x  ", nout);
		/* the byte value in hex */
		fprintf(out, "%.2x ", data[nout]);
		/* the ascii equivalent at the end of the line */
		if((nout % HEXDUMP_WIDTH) == (HEXDUMP_WIDTH - 1))
		{
			fprintf(out, " ");
			for(i=HEXDUMP_WIDTH-1; i>=0; i--)
				fprintf(out, "%c", isprint(data[nout - i]) ? data[nout - i] : '.');
			fprintf(out, "\n");
		}
		nout ++;
	}

	/* the ascii equivalent if we haven't just done it */
	if((nout % HEXDUMP_WIDTH) != 0)
	{
		/* pad to the start of the ascii equivalent */
		for(i=(nout % HEXDUMP_WIDTH); i<HEXDUMP_WIDTH; i++)
			fprintf(out, "   ");
		fprintf(out, " ");
		/* print the ascii equivalent */
		nout --;
		for(i=(nout % HEXDUMP_WIDTH); i>=0; i--)
			fprintf(out, "%c", isprint(data[nout - i]) ? data[nout - i] : '.');
		fprintf(out, "\n");
	}

	return;
}

