/*
 * der_decode.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "der_decode.h"
#include "asn1tag.h"

void verbose(char *, ...);
void vverbose(char *, ...);

/* DER does not allow indefinite lengths */

int
der_decode_Tag(FILE *der, struct der_tag *tag)
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

	vverbose("<TAG class=%s number=%d length=%d/>\n", asn1class_name(tag->class), tag->number, tag->length);

	return nbytes;
}

int
der_decode_BOOLEAN(FILE *der, FILE *out, int length)
{
	unsigned char val;

	if(length != 1)
		return der_error("Boolean: length=%d", length);

	if(der_read_file(der, length, &val) < 0)
		return der_error("Boolean");

	verbose("<Boolean value=\"%s\"/>\n", val ? "true" : "false");

	fprintf(out, " %s", val ? "true" : "false");

	return length;
}

int
der_decode_INTEGER(FILE *der, FILE *out, int length)
{
	int val = get_der_int(der, length);

	verbose("<Integer value=\"%d\"/>\n", val);

	fprintf(out, " %d", val);

	return length;
}

/* DER does not allow constructed OCTET-STRINGs */

int
der_decode_OctetString(FILE *der, FILE *out, int length)
{
	int left = length;
	unsigned char byte;

	verbose("<OctetString size=\"%d\">\n", length);

	/* output a QPRINTABLE string */
	fprintf(out, " '");
	while(left > 0)
	{
		if(der_read_file(der, 1, &byte) < 0)
			return der_error("OctetString");
		if(byte != '\'' && byte >= 0x20 && byte < 0x7f)
			fprintf(out, "%c", byte);
		else
			fprintf(out, "=%02x", byte);
		left --;
	}
	fprintf(out, "'");

	verbose("</OctetString>\n");

	return length;
}

int
der_decode_Null(FILE *der, FILE *out, int length)
{
	if(length != 0)
		return der_error("Null: length=%d", length);

	verbose("<Null/>");

	return length;
}

int
der_decode_ENUMERATED(FILE *der, FILE *out, int length, unsigned int max, char *names[])
{
	int val = get_der_int(der, length);

	if(val < 1 || val > max)
		return der_error("Enumerated: %d; not in range 1-%d", val, max);

	verbose("<Enumerated value=\"%d\"/>\n", val);

	fprintf(out, " %s", names[val - 1]);

	return length;
}

int
get_der_int(FILE *der, int length)
{
	unsigned char byte;
	unsigned int uval;
	int val;
	bool negative;
	int i;

	if(length > sizeof(int))
		fatal("Integer: length=%d", length);

	/* is it -ve */
	if(der_read_file(der, 1, &byte) < 0)
		fatal("Integer: EOF");
	negative = ((byte & 0x80) == 0x80);

	/* big endian */
	uval = byte;
	for(i=1; i<length; i++)
	{
		if(der_read_file(der, 1, &byte) < 0)
			fatal("Integer: EOF");
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

	val = (int) uval;

	return val;
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

