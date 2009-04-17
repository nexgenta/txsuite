/*
 * readpng.h
 */

#ifndef __READPNG_H__
#define __READPNG_H__

#include <png.h>

unsigned char *readpng_get_image(unsigned char *, unsigned int, png_uint_32 *, png_uint_32 *);

void readpng_free_image(unsigned char *);

#endif	/* __READPNG_H__ */

