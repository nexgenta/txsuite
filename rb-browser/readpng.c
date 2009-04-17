/*
 * readpng.c
 */

/* based on code with this copyright... */

/*---------------------------------------------------------------------------

   rpng - simple PNG display program                              readpng.c

  ---------------------------------------------------------------------------

      Copyright (c) 1998-2000 Greg Roelofs.  All rights reserved.

      This software is provided "as is," without warranty of any kind,
      express or implied.  In no event shall the author or contributors
      be held liable for any damages arising in any way from the use of
      this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute
      it freely, subject to the following restrictions:

      1. Redistributions of source code must retain the above copyright
         notice, disclaimer, and this list of conditions.
      2. Redistributions in binary form must reproduce the above copyright
         notice, disclaimer, and this list of conditions in the documenta-
         tion and/or other materials provided with the distribution.
      3. All advertising materials mentioning features or use of this
         software must display the following acknowledgment:

            This product includes software developed by Greg Roelofs
            and contributors for the book, "PNG: The Definitive Guide,"
            published by O'Reilly and Associates.

  ---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <png.h>

#include "utils.h"

/* internal */
static struct
{
	unsigned char *data;	/* base of PNG data in memory */
	unsigned int size;	/* size of PNG data */
	unsigned int used;	/* current offset into PNG data */
} read_mem_state;

static void read_mem(png_structp, png_bytep, png_size_t);

/*
 * returns an array of 32-bit pixels in RGBA format
 * pixel data will be in network byte order (ie MSB first)
 */

unsigned char *
readpng_get_image(unsigned char *png_data, unsigned int png_size, png_uint_32 *width, png_uint_32 *height)
{
	unsigned char *rgba = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	int bit_depth;
	int colour_type;
	png_uint_32 i, rowbytes;
	png_bytepp row_pointers = NULL;

	/* check the signature */
	if(!png_check_sig(png_data, 8))
		return NULL;

	/* init libpng data structures */
	if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
		return NULL;

	if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}

	/* setjmp() must be called in every function that calls a PNG-reading libpng function */
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		if(row_pointers != NULL)
			free(row_pointers);
		if(rgba != NULL)
			free(rgba);
		return NULL;
	}

	/* read from memory rather than a file */
	read_mem_state.data = png_data;
	read_mem_state.size = png_size;
	/* unlike file IO we don't need to seek passed the signature */
	read_mem_state.used = 0;
	png_set_read_fn(png_ptr, &read_mem_state, read_mem);

	/* read all PNG info up to image data */
	png_read_info(png_ptr, info_ptr);

	/* don't care about compression_type and filter_type => NULL */
	png_get_IHDR(png_ptr, info_ptr, width, height, &bit_depth, &colour_type, NULL, NULL, NULL);

	/* expand palette images to RGB */
	if((colour_type & PNG_COLOR_MASK_PALETTE) != 0)
		png_set_expand(png_ptr);
	/* expand low-bit-depth images to 8 bits */
	if(bit_depth < 8)
		png_set_expand(png_ptr);
	/* expand transparency chunks to full alpha channel */
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	/* strip 16-bit-per-sample images to 8-bits-per-sample */
	if(bit_depth == 16)
		png_set_strip_16(png_ptr);
	/* convert grayscale to RGB */
	if((colour_type & PNG_COLOR_MASK_COLOR) == 0)
		png_set_gray_to_rgb(png_ptr);
	/* add an opaque alpha channel if none exists in the PNG file */
	if((colour_type & PNG_COLOR_MASK_ALPHA) == 0)
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

	/* recalc rowbytes etc */
	png_read_update_info(png_ptr, info_ptr);

	/* assert */
	if(png_get_channels(png_ptr, info_ptr) != 4)
		fatal("readpng: didn't get 4 channels");

	/* alloc our RGBA array */
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	if((rgba = (unsigned char *) malloc(rowbytes * (*height))) == NULL)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}

	if((row_pointers = (png_bytepp) malloc((*height) * sizeof(png_bytep))) == NULL)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		free(rgba);
		return NULL;
	}

	/* set the individual row_pointers to point at the correct offsets */
	for(i=0; i<(*height); i++)
		row_pointers[i] = rgba + (i * rowbytes);

	/* now we can go ahead and just read the whole image */
	png_read_image(png_ptr, row_pointers);

	/* clean up */
	free(row_pointers);
	row_pointers = NULL;
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return rgba;
}

void
readpng_free_image(unsigned char *rgba)
{
	free(rgba);

	return;
}

/*
 * the default libpng reader reads from a file
 * this reads from a block of memory
 */

static void
read_mem(png_structp png_ptr, png_bytep data, png_size_t length)
{
	if(read_mem_state.used + length > read_mem_state.size)
	{
		png_error(png_ptr, "Unexpected end of PNG data");
		return;
	}

	memcpy(data, read_mem_state.data + read_mem_state.used, length);
	read_mem_state.used += length;

	return;
}

