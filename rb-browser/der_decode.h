/*
 * der_decode.h
 */

#ifndef __DER_DECODE_H__
#define __DER_DECODE_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

#define der_alloc(N)		safe_malloc(N)
#define der_realloc(P, N)	safe_realloc(P, N)
#define der_free(P)		safe_free(P)

typedef struct der_tag
{
	unsigned char class;
	unsigned int number;
	unsigned int length;
} der_tag;

#define DER_CLASS_UNIVERSAL	0x00
#define DER_CLASS_APPLICATION	0x40
#define DER_CLASS_CONTEXT	0x80
#define DER_CLASS_PRIVATE	0xc0

/* a NULL ASN.1 type really is nothing at all */
typedef char Null;

typedef struct OctetString
{
	unsigned int size;
	unsigned char *data;
} OctetString;

int der_decode_Tag(FILE *, der_tag *);
int der_peek_Tag(FILE *, der_tag *);

int der_decode_Boolean(FILE *, bool *, int);

int der_decode_Integer(FILE *, int *, int);

int der_decode_Null(FILE *, Null *, int);

int der_decode_OctetString(FILE *, OctetString *, int);
void free_OctetString(OctetString *);

int OctetString_cmp(OctetString *, OctetString *);
int OctetString_strcmp(OctetString *, char *);
int OctetString_strncmp(OctetString *, char *, size_t);

bool OctetString_copy(OctetString *, OctetString *);
void OctetString_dup(OctetString *, OctetString *);

int der_read_file(FILE *, unsigned int, void *);

int der_error(char *, ...);

void hexdump(unsigned char *, size_t);

#endif	/* __DER_DECODE_H__ */

