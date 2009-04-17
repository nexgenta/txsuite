/*
 * der_decode.h
 */

#ifndef __DER_DECODE_H__
#define __DER_DECODE_H__

#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

#define der_alloc(N)		safe_malloc(N)
#define der_realloc(P, N)	safe_realloc(P, N)
#define der_free(P)		safe_free(P)

struct der_tag
{
	unsigned char class;
	unsigned int number;
	unsigned int length;
};

#define DER_CLASS_UNIVERSAL	0x00
#define DER_CLASS_APPLICATION	0x40
#define DER_CLASS_CONTEXT	0x80
#define DER_CLASS_PRIVATE	0xc0

/* top 8 bits are class, bottom 24 are the tag number */
#define MATCH_TAGCLASS(CLASS, NUMBER, TAGCLASS)	((CLASS == ((TAGCLASS >> 24) & 0xff)) && (NUMBER == (TAGCLASS & 0xffffff)))

int der_decode_Tag(FILE *, struct der_tag *);

int der_decode_BOOLEAN(FILE *, FILE *, int);
int der_decode_INTEGER(FILE *, FILE *, int);
int der_decode_OctetString(FILE *, FILE *, int);
int der_decode_Null(FILE *, FILE *, int);
int der_decode_ENUMERATED(FILE *, FILE *, int, unsigned int, char **);

int get_der_int(FILE *, int);

int der_read_file(FILE *, unsigned int, void *);

int der_error(char *, ...);

#endif	/* __DER_DECODE_H__ */

