/*
 * der_encode.c
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

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "der_encode.h"
#include "utils.h"

void
der_encode_BOOLEAN(unsigned char **out, unsigned int *len, bool val)
{
	/* assert */
	if(*out != NULL || *len != 0)
		fatal("der_encode_BOOLEAN: length already %u", *len);

	*len = 1;
	*out = safe_malloc(1);
	(*out)[0] = val ? 0xff : 0;

	return;
}

void
der_encode_INTEGER(unsigned char **out, unsigned int *len, int val)
{
	unsigned int shifted;
	unsigned int i;
	unsigned int uval;

	/* assert */
	if(*out != NULL || *len != 0)
		fatal("der_encode_INTEGER: length already %u", *len);

	/* is it +ve or -ve */
	if(val >= 0)
	{
		/*
		 * work out how many bytes we need to store 'val'
		 * ints in DER are signed, so first byte we store must be <= 127
		 * we add a leading 0 if first byte is > 127
		 */
		shifted = val;
		*len = 1;
		while(shifted > 127)
		{
			(*len) ++;
			shifted >>= 8;
		}
		*out = safe_malloc(*len);
		/* big endian */
		for(i=1; i<=(*len); i++)
			(*out)[i - 1] = (val >> (((*len) - i) * 8)) & 0xff;
	}
	else
	{
		/* work with an unsigned value */
		uval = (unsigned int) val;
		/*
		 * use as few bytes as possible
		 * ie chop off leading 0xff's while the next byte has its top bit set
		 */
		*len = sizeof(unsigned int);
		while((*len) > 1
		   && ((uval >> (((*len) - 1) * 8)) & 0xff) == 0xff
		   && ((uval >> (((*len) - 2) * 8)) & 0x80) == 0x80)
		{
			(*len) --;
		}
		*out = safe_malloc(*len);
		/* big endian */
		for(i=1; i<=(*len); i++)
			(*out)[i - 1] = (uval >> (((*len) - i) * 8)) & 0xff;
	}

	return;
}

void
der_encode_OctetString(unsigned char **out, unsigned int *len, const unsigned char *str)
{
	/* assert */
	if(*out != NULL || *len != 0)
		fatal("der_encode_OctetString: length already %u", *len);

	/* convert the STRING, QPRINTABLE or BASE64 data to an OctetString */
	if(*str == '"')
		convert_STRING(out, len, str);
	else if(*str == '\'')
		convert_QPRINTABLE(out, len, str);
	else if(*str == '`')
		convert_BASE64(out, len, str);
	else
		fatal("der_encode_OctetString: str[0]=0x%02x", *str);

	return;
}

/*
 * string is enclosed in "
 * contains chars 0x20 to 0x7e
 * " and \ within the string are encoded as \" and \\
 */

void
convert_STRING(unsigned char **out, unsigned int *len, const unsigned char *str)
{
	const unsigned char *whole_str = str;
	unsigned char *p;
	bool err;

	/* max size it could be */
	*out = safe_malloc(strlen(str));

	/* skip the initial " */
	str ++;
	p = *out;
	err = false;
	while(!err && *str != '"')
	{
		if(*str < 0x20 || *str > 0x7e)
		{
			parse_error("Invalid character (0x%02x) in STRING: %s", *str, whole_str);
			err = true;
		}
		else if(*str != '\\')
		{
			*p = *str;
			p ++;
			str ++;
		}
		else if(*(str + 1) == '"')
		{
			*p = '"';
			p ++;
			str += 2;
		}
		else if(*(str + 1) == '\\')
		{
			*p = '\\';
			p ++;
			str += 2;
		}
		else
		{
			parse_error("Invalid escape sequence in STRING: %s", whole_str);
			err = true;
		}
	}

	/* check we got to the closing quote */
	if(!err && *(str + 1) != '\0')
	{
		parse_error("Unquoted \" in STRING: %s", whole_str);
		err = true;
	}

	if(!err)
	{
		/* return the length (note: no \0 terminator) */
		*len = (p - *out);
	}
	else
	{
		/* clean up */
		safe_free(*out);
		*out = NULL;
		*len = 0;
	}

	return;
}

/*
 * string is enclosed in '
 * encoded as specified in RFC 1521 (MIME)
 * in addition, ' is encoded as =27
 * and line breaks do not need to be converted to CRLF
 */

void
convert_QPRINTABLE(unsigned char **out, unsigned int *len, const unsigned char *str)
{
	const unsigned char *whole_str = str;
	unsigned char *p;
	bool err;

	/* max size it could be */
	*out = safe_malloc(strlen(str));

	/* skip the initial ' */
	str ++;
	p = *out;
	err = false;
	while(!err && *str != '\'')
	{
		if(*str != '=')
		{
			*p = *str;
			p ++;
			str ++;
		}
		else if(isxdigit(*(str + 1)) && isxdigit(*(str + 2)))
		{
			*p = char2hex(*(str + 1)) << 4 | char2hex(*(str + 2));
			p ++;
			str += 3;
		}
		else
		{
			parse_error("Invalid escape sequence in QPRINTABLE: %s", whole_str);
			err = true;
		}
	}

	/* check we got to the closing quote */
	if(!err && *(str + 1) != '\0')
	{
		parse_error("Unquoted ' in QPRINTABLE: %s", whole_str);
		err = true;
	}

	if(!err)
	{
		/* return the length (note: no \0 terminator) */
		*len = (p - *out);
	}
	else
	{
		/* clean up */
		safe_free(*out);
		*out = NULL;
		*len = 0;
	}

	return;
}

/*
 * enclosed in `
 * base 64 encoded as described in RFC 1521
 * line breaks in the input can be ignored
 */

unsigned char next_base64_char(unsigned char **);

void
convert_BASE64(unsigned char **out, unsigned int *len, const unsigned char *str)
{
	const unsigned char *whole_str = str;
	unsigned char *p;
	bool err;

	/* max size it could be */
	*out = safe_malloc(strlen(str));

	/* skip the initial ` */
	str ++;
	p = *out;
	err = false;
	while(!err && *str != '`')
	{
		/* need 4 chars */
		unsigned char v1 = next_base64_char((unsigned char **) &str);
		unsigned char v2 = next_base64_char((unsigned char **) &str);
		unsigned char v3 = next_base64_char((unsigned char **) &str);
		unsigned char v4 = next_base64_char((unsigned char **) &str);
		/* how many chars are valid */
		if(v1 < 64 && v2 < 64 && v3 < 64 && v4 < 64)
		{
			/* 4 chars => 24 bits */
			p[0] = (v1 << 2) | (v2 >> 4);
			p[1] = (v2 << 4) | (v3 >> 2);
			p[2] = (v3 << 6) | v4;
			p += 3;
		}
		else if(v1 < 64 && v2 < 64 && v3 < 64 && v4 == 64)
		{
			/* 3 chars => 16 bits */
			p[0] = (v1 << 2) | (v2 >> 4);
			p[1] = (v2 << 4) | (v3 >> 2);
			p += 2;
		}
		else if(v1 < 64 && v2 < 64 && v3 == 64 && v4 == 64)
		{
			/* 2 chars => 8 bits */
			p[0] = (v1 << 2) | (v2 >> 4);
			p += 1;
		}
		else
		{
			parse_error("Invalid BASE64 string: %s", whole_str);
			err = true;
		}
	}

	/* check we got to the closing quote */
	if(!err && *(str + 1) != '\0')
	{
		parse_error("Unexpected ` in BASE64: %s", whole_str);
		err = true;
	}

	if(!err)
	{
		/* return the length (note: no \0 terminator) */
		*len = (p - *out);
	}
	else
	{
		/* clean up */
		safe_free(*out);
		*out = NULL;
		*len = 0;
	}

	return;
}

/*
 * updates *in
 */

unsigned char
next_base64_char(unsigned char **in)
{
	/* would be faster with a 256 byte lookup table */
	unsigned char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	unsigned char *pos;
	unsigned char out;

	while(**in != '`')
	{
		out = **in;
		(*in) ++;
		if((pos = strchr(alphabet, out)) != NULL)
			return (pos - alphabet);
	}

	/* EOF, return (strchr(alphabet, '=') - alphabet) */
	return 64;
}

/*
 * recursively generate the DER tag/length header for the tree of nodes
 */

unsigned int
gen_der_header(struct node *n)
{
	unsigned int val_length;
	struct node *kid;

	/* if it is a constructed type, sum the length of its children */
	if(n->children)
	{
		/* assert */
		if(n->length != 0)
			fatal("Constructed type [%s %u] has a value", asn1class_name(n->asn1class), n->asn1tag);
		/* recursively generate the DER headers for our child nodes */
		val_length = 0;
		for(kid=n->children; kid; kid=kid->siblings)
			val_length += gen_der_header(kid);
	}
	else
	{
		val_length = n->length;
	}

	/* we don't need a header if we are a synthetic object */
	if(!is_synthetic(n->asn1tag))
	{
		/* allocate more than we will need */
		n->hdr_value = safe_malloc(MAX_DER_HDR_LENGTH);
		/*
		 * first DER type byte is:
		 * AABCCCCC
		 * AA = ASN1 class (UNIVERSAL, CONTEXT, etc)
		 * B = 1 if it is a constructed type (ie has children)
		 * CCCCC = tag value (0-30)
		 * if the tag is set to 31, then:
		 * while the next byte has its top-bit set,
		 * the big-endian tag value is encoded in the bottom 7-bits
		 */
		n->hdr_value[0] = n->asn1class;
		if(has_real_children(n))
			n->hdr_value[0] |= 0x20;
		if(n->asn1tag < 31)
		{
			n->hdr_value[0] |= n->asn1tag;
			n->hdr_length = 1;
		}
		else
		{
			n->hdr_value[0] |= 0x1f;
			/* cheat - we know it is < 0x3fff (16383) */
			if(n->asn1tag <= 0x7f)
			{
				n->hdr_value[1] = n->asn1tag & 0x7f;
				n->hdr_length = 2;
			}
			else if(n->asn1tag <= 0x3fff)
			{
				n->hdr_value[1] = 0x80 | ((n->asn1tag >> 7) & 0x7f);
				n->hdr_value[2] = n->asn1tag & 0x7f;
				n->hdr_length = 3;
			}
			else
			{
				/* assert */
				fatal("gen_der_header: tag=%u class=%s", n->asn1tag, asn1class_name(n->asn1class));
			}
		}
		/*
		 * if DER length is <128, then it is encoded as:
		 * 0LLLLLLL, where LLLLLLL is the 7-bit length
		 * if DER length is >=128, then it is encoded as:
		 * 1LLLLLLL, where LLLLLLL are the number of following bytes
		 * encoding the big-endian length
		 */
		if(val_length < 128)
		{
			n->hdr_value[n->hdr_length] = val_length;
			n->hdr_length ++;
		}
		else
		{
			unsigned int nlens = 1;
			unsigned int len = val_length;
			unsigned int i;
			while(len > 255)
			{
				len >>= 8;
				nlens ++;
			}
			/* assert */
			if(n->hdr_length + nlens > MAX_DER_HDR_LENGTH)
				fatal("gen_der_header: tag=%u class=%s nlens=%u", n->asn1tag, asn1class_name(n->asn1class), nlens);
			n->hdr_value[n->hdr_length] = 0x80 | nlens;
			n->hdr_length ++;
			for(i=nlens; i>0; i--)
			{
				n->hdr_value[n->hdr_length] = (val_length >> ((i - 1) * 8)) & 0xff;
				n->hdr_length ++;
			}
		}
		/*
		 * this header is followed by the DER encoded value
		 * the value of constructed types is one or more DER types
		 * ie one or more DER header/value pairs
		 */
	}

	return n->hdr_length + val_length;
}

void
write_der_object(FILE *out, struct node *n)
{
	struct node *kid;

	/* write our tag/length header */
	if(!is_synthetic(n->asn1tag))
		write_all(out, n->hdr_value, n->hdr_length);

	/* and our value */
	if(n->children)
	{
		for(kid=n->children; kid; kid=kid->siblings)
			write_der_object(out, kid);
	}
	else
	{
		write_all(out, n->value, n->length);
	}

	return;
}

