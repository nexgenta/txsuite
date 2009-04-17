/*
 * mhegd.c
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

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "asn1decode.h"
#include "der_decode.h"
#include "output.h"
#include "utils.h"

void verbose(const char *, ...);
void vverbose(const char *, ...);

void usage(char *);

static int _verbose = 0;

int
main(int argc, char *argv[])
{
	char *prog_name = argv[0];
	int arg;
	FILE *in_file;
	FILE *out_file = stdout;
	int filesize;
	int used;

	while((arg = getopt(argc, argv, "o:v")) != EOF)
	{
		switch(arg)
		{
		case 'o':
			if((out_file = fopen(optarg, "w")) == NULL)
				fatal("Unable to write '%s': %s", optarg, strerror(errno));
			break;

		case 'v':
			_verbose ++;
			break;

		default:
			usage(prog_name);
			break;
		}
	}

	/* the single param is the name of a DER file */
	if(optind != argc - 1)
		usage(prog_name);

	if((in_file = fopen(argv[optind], "r")) == NULL)
		fatal("Unable to open '%s': %s", argv[optind], strerror(errno));

	verbose("Reading '%s':\n", argv[optind]);

	/* see how long the file is */
	fseek(in_file, 0, SEEK_END);
	filesize = (int) ftell(in_file);
	rewind(in_file);

	/* write text form of DER encoded in_file to out_file */
	output_init();
	used = asn1decode_InterchangedObject(in_file, out_file, filesize);
	fprintf(out_file, "\n");
	if(used < 0)
		fatal("Parsing error");
	else if(used != filesize)
		fatal("Unexpected data after InterchangedObject");

	return EXIT_SUCCESS;
}

/*
 * verbose functions send output to stderr so error messages get interleaved correctly
 */

void
verbose(const char *fmt, ...)
{
	va_list ap;

	if(_verbose)
	{
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}

	return;
}

void
vverbose(const char *fmt, ...)
{
	va_list ap;

	if(_verbose > 1)
	{
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}

	return;
}

void
usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s [-vv] [-o <output_file>] <input_file>\n", prog_name);

	exit(EXIT_FAILURE);
}

