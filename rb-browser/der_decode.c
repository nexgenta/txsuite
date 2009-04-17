/*
 * der_decode.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "der_decode.h"

/* DER does not allow indefinite lengths */

int
der_decode_Tag(FILE *der, der_tag *tag)
{
	unsigned int type;
	unsigned int len;
	unsigned char byte;
	unsigned int longtype;
	int nlens;
	int nbytes = 0;

	/* type */
	if(der_read_file(der, 1, &byte) != 1)
		return der_error("DER tag");
	nbytes ++;
	type = byte;
	longtype = 0;
	if((type & 0x1f) == 0x1f)
	{
		/* multi byte type */
		do
		{
			if(der_read_file(der, 1, &byte) != 1)
				return der_error("DER tag");
			nbytes ++;
			longtype <<= 7;
			longtype += byte & 0x7f;
		}
		while((byte & 0x80) != 0);
	}
	tag->class = type & 0xc0;
	tag->number = ((type & 0x1f) == 0x1f) ? longtype : type & 0x1f;

	/* length */
	if(der_read_file(der, 1, &byte) != 1)
		return der_error("DER tag");
	nbytes ++;
	len = byte;
	if(len == 0 && type == 0)
	{
		return der_error("Found EOC; indefinite lengths not allowed in DER");
	}
	else if(len == 0x80)
	{
		return der_error("Indefinite lengths not allowed in DER");
	}
	else if((len & 0x80) == 0x80)
	{
		/* multibyte length field */
		nlens = len & 0x7f;
		len = 0;
		while(nlens > 0)
		{
			if(der_read_file(der, 1, &byte) != 1)
				return der_error("DER tag");
			nbytes ++;
			len <<= 8;
			len += byte;
			nlens --;
		}
	}
	tag->length = len;

	return nbytes;
}

/*
 * read the tag, but don't advance the FILE offset
 */

int
der_peek_Tag(FILE *der, der_tag *tag)
{
	long pretag = ftell(der);
	int length;

	length = der_decode_Tag(der, tag);

	fseek(der, pretag, SEEK_SET);

	return length;
}

int
der_decode_Null(FILE *der, Null *type, int length)
{
	if(length != 0)
		return der_error("Null: length=%d", length);

#ifdef DER_VERBOSE
	printf("<Null/>");
#endif

	return length;
}

int
der_decode_Boolean(FILE *der, bool *type, int length)
{
	unsigned char val;

	if(length != 1)
		return der_error("Boolean: length=%d", length);

	if(der_read_file(der, length, &val) < 0)
		return der_error("Boolean");

	*type = (val == 0) ? false : true;

#ifdef DER_VERBOSE
	printf("<Boolean value=\"%s\"/>\n", (*type) ? "true" : "false");
#endif

	return length;
}

int
der_decode_Integer(FILE *der, int *type, int length)
{
	unsigned char byte;
	unsigned int uval;
	bool negative;
	int i;

	if(length > sizeof(int))
		der_error("Integer: length=%d", length);

	/* is it -ve */
	if(der_read_file(der, 1, &byte) < 0)
		return der_error("Integer");
	negative = ((byte & 0x80) == 0x80);

	/* big endian */
	uval = byte;
	for(i=1; i<length; i++)
	{
		if(der_read_file(der, 1, &byte) < 0)
			return der_error("Integer");
		uval <<= 8;
		uval += byte;
	}

	/* sign extend if negative */
	if(negative)
	{
		/* byte order neutral */
		for(i=length; i<sizeof(int); i++)
			uval += (0xff << (i * 8));
	}

	*type = (int) uval;

#ifdef DER_VERBOSE
	printf("<Integer value=\"%d\"/>\n", *type);
#endif

	return length;
}

/* DER does not allow constructed OCTET-STRINGs */

int
der_decode_OctetString(FILE *der, OctetString *type, int length)
{
	bzero(type, sizeof(OctetString));

	/* don't alloc 0 length strings */
	if(length > 0)
	{
		if((type->data = der_alloc(length)) == NULL)
			return der_error("OctetString: out of memory");
	}

	/* only set the length after we are sure the alloc worked */
	type->size = length;

	if(der_read_file(der, length, type->data) < 0)
		return der_error("OctetString");

#ifdef DER_VERBOSE
	printf("<OctetString size=\"%d\">\n", type->size);
	hexdump(type->data, type->size);
	printf("</OctetString>\n");
#endif

	return length;
}

void
free_OctetString(OctetString *type)
{
	/* der_free(NULL) is okay */
	der_free(type->data);

	type->size = 0;
	type->data = NULL;

	return;
}

int
OctetString_cmp(OctetString *o1, OctetString *o2)
{
	if(o1->size != o2->size)
		return o1->size - o2->size;
	else
		return memcmp(o1->data, o2->data, o1->size);
}

/*
 * compare the OctetString with the given \0 terminated C string
 */

int
OctetString_strcmp(OctetString *oct, char *str)
{
	size_t len = strlen(str);

	if(oct->size != len)
		return oct->size - len;
	else
		return memcmp(oct->data, str, len);
}

/*
 * compare the first n characters of the OctetString with the given C string
 */

int
OctetString_strncmp(OctetString *oct, char *str, size_t n)
{
	if(oct->size < n)
		return oct->size - n;
	else
		return memcmp(oct->data, str, n);
}

/*
 * assumes its okay to call der_realloc(dst->data, x)
 * if dst is not initialised, you should use OctetString_dup instead
 * src can be NULL, but dst must be valid
 * returns false if no memory
 */

bool
OctetString_copy(OctetString *dst, OctetString *src)
{
	/* assert */
	if(dst == NULL)
		fatal("OctetString_copy: dst is NULL");

	/* special cases */
	if(src == NULL || src->size == 0)
	{
		safe_free(dst->data);
		dst->size = 0;
		dst->data = NULL;
		return true;
	}
	else if((dst->data = der_realloc(dst->data, src->size)) != NULL)
	{
		dst->size = src->size;
		memcpy(dst->data, src->data, src->size);
		return true;
	}
	else
	{
		dst->size = 0;
		return false;
	}
}

/*
 * makes a copy of src in dst
 * note: any existing dst->data will be lost, use OctetString_copy if dst already has content
 * src can be NULL, but dst must be valid
 */

void
OctetString_dup(OctetString *dst, OctetString *src)
{
	/* assert */
	if(dst == NULL)
		fatal("OctetString_dup: dst is NULL");

	if(src == NULL || src->size == 0)
	{
		dst->size = 0;
		dst->data = NULL;
	}
	else
	{
		dst->size = src->size;
		dst->data = safe_malloc(src->size);
		memcpy(dst->data, src->data, src->size);
	}

	return;
}

int
der_read_file(FILE *in, unsigned int nbytes, void *buf)
{
	int nread;

	if((nread = fread(buf, 1, nbytes, in)) != nbytes)
		return der_error("Unexpected EOF");

	return nread;
}

int
der_error(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	return -1;
}

/* number of bytes per line */
#define HEXDUMP_WIDTH	16

void
hexdump(unsigned char *data, size_t nbytes)
{
	size_t nout;
	int i;

	nout = 0;
	while(nout < nbytes)
	{
		/* byte offset at start of line */
		if((nout % HEXDUMP_WIDTH) == 0)
			printf("0x%.8x  ", nout);
		/* the byte value in hex */
		printf("%.2x ", data[nout]);
		/* the ascii equivalent at the end of the line */
		if((nout % HEXDUMP_WIDTH) == (HEXDUMP_WIDTH - 1))
		{
			printf(" ");
			for(i=HEXDUMP_WIDTH-1; i>=0; i--)
				printf("%c", isprint(data[nout - i]) ? data[nout - i] : '.');
			printf("\n");
		}
		nout ++;
	}

	/* the ascii equivalent if we haven't just done it */
	if((nout % HEXDUMP_WIDTH) != 0)
	{
		/* pad to the start of the ascii equivalent */
		for(i=(nout % HEXDUMP_WIDTH); i<HEXDUMP_WIDTH; i++)
			printf("   ");
		printf(" ");
		/* print the ascii equivalent */
		nout --;
		for(i=(nout % HEXDUMP_WIDTH); i>=0; i--)
			printf("%c", isprint(data[nout - i]) ? data[nout - i] : '.');
		printf("\n");
	}

	return;
}

