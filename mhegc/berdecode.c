/*
 * berdecode [<filename>]
 *
 * not a proper decoder, just a quick hack
 * if no filename is given, read from stdin
 *
 * Simon Kilvington, 2/10/2005
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/* how we know when we have got to the end of a BER type */
#define WANT_EOF	-2	/* continue until End-Of-File */
#define WANT_EOC	-1	/* continue until End-Of-Contents octets (00,00) */
/* 0 or +ve values are the number of bytes until the end of the type */

/* global buffer to read file contents into */
static char buf[64 * 1024];

int print_next(FILE *, int, int);
void print_indent(int);
void print_boolean(FILE *);
void print_integer(FILE *, unsigned int);
void print_oid(FILE *, unsigned int);

unsigned int read_file(FILE *, unsigned int, void *);

unsigned int type_number(unsigned int, unsigned int);
char *universal_typename(unsigned int);
void hexdump(unsigned char *, size_t);

void usage(char *);
void fatal(char *, ...);

int
main(int argc, char *argv[])
{
	FILE *in = NULL;
	int filelen;

	if(argc == 1)
	{
		in = stdin;
	}
	else if(argc == 2)
	{
		if((in = fopen(argv[1], "r")) == NULL)
			fatal("Unable to open file '%s': %s", argv[1], strerror(errno));
	}
	else
	{
		usage(argv[0]);
	}

	filelen = print_next(in, 0, WANT_EOF);

	printf("file length=%d\n", filelen);

	return EXIT_SUCCESS;
}

int
print_next(FILE *in, int indent, int wanted)
{
	unsigned int type;
	unsigned int len;
	unsigned char byte;
	unsigned int longtype;
	int nlens;
	int nbytes = 0;

	do
	{
		/* type */
		nbytes += read_file(in, 1, &byte);
		type = byte;
		longtype = 0;
		if((type & 0x1f) == 0x1f)
		{
			/* multi byte type */
			do
			{
				nbytes += read_file(in, 1, &byte);
				longtype <<= 7;
				longtype += byte & 0x7f;
			}
			while((byte & 0x80) != 0);
		}
		if(type != 0)
			print_indent(indent);
		switch(type & 0xc0)
		{
		case 0x00:
			//printf("UNIVERSAL");
			if(type != 0)
				printf("%s", universal_typename(type));
			break;

		case 0x40:
			printf("[APPLICATION %u]", type_number(type, longtype));
			break;

		case 0x80:
			printf("[CONTEXT %u]", type_number(type, longtype));
			break;

		case 0xc0:
			printf("[PRIVATE %u]", type_number(type, longtype));
			break;
		}

		/* length */
		nbytes += read_file(in, 1, &byte);
		len = byte;
		if(len == 0 && type == 0)
		{
			/* check we want EOC */
			if(wanted != WANT_EOC)
				fatal("Unexpected End-Of-Contents");
			return nbytes;
		}
		else if(len == 0x80)
		{
			/* indefinite length */
			len = WANT_EOC;
		}
		else if((len & 0x80) == 0x80)
		{
			/* multibyte length field */
			nlens = len & 0x7f;
			len = 0;
			while(nlens > 0)
			{
				nbytes += read_file(in, 1, &byte);
				len <<= 8;
				len += byte;
				nlens --;
			}
		}
		if(type == 0 && len != 0)
			fatal("type=0x%x (%u) len=0x%x (%u)", type, type, len, len);

		/* value */
		if((type & 0x20) == 0x20)
		{
			/* constructed */
			printf(" {\n");
			len = print_next(in, indent+1, len);
			print_indent(indent);
			printf("}\n");
		}
		else if(len == 0)
		{
			printf(" value=default\n");
		}
		else if(len > 0)
		{
			if(len > sizeof(buf))
				fatal("Value too long (%u)", len);
			/* BOOLEAN */
			if(type == 1 && len == 1)
				print_boolean(in);
			/* INTEGER */
			else if(type == 2 && len <= sizeof(int))
				print_integer(in, len);
			/* OBJECT IDENTIFIER */
			else if(type == 6)
				print_oid(in, len);
			/* ENUMERATED */
			else if(type == 10 && len <= sizeof(int))
				print_integer(in, len);
			/* default: hexdump the contents */
			else if(type != 0)
			{
				(void) read_file(in, len, buf);
				printf(" value=(%u bytes):\n", len);
				hexdump(buf, len);
			}
		}

		if(len > 0)
			nbytes += len;

		if(wanted >= 0 && nbytes > wanted)
			fatal("Unexpected EOF");
	}
	while(nbytes != wanted && !feof(in));

	return nbytes;
}

void
print_indent(int indent)
{
	indent *= 2;

	for(; indent>0; indent--)
		printf(" ");

	return;
}

void
print_boolean(FILE *in)
{
	unsigned char val;

	read_file(in, 1, &val);
	printf(" value=%s (%u)\n", (val == 0) ? "false" : "true", val);

	return;
}

void
print_integer(FILE *in, unsigned int len)
{
	int integer;
	unsigned char byte;
	unsigned int uval;
	int negative;
	int i;

	/* is it -ve */
	read_file(in, 1, &byte);
	negative = ((byte & 0x80) == 0x80);

	/* big endian */
	uval = byte;
	for(i=1; i<len; i++)
	{
		read_file(in, 1, &byte);
		uval <<= 8;
		uval += byte;
	}

	/* sign extend if negative */
	if(negative)
	{
		/* byte order neutral */
		for(i=len; i<sizeof(int); i++)
			uval += (0xff << (i * 8));
	}

	integer = (int) uval;

	printf(" value=%d\n", integer);

	return;
}

void
print_oid(FILE *in, unsigned int len)
{
	int oid;
	int oid1, oid2;
	int i;

	read_file(in, len, buf);

	if((buf[0] & 0x80) == 0x80)
		fatal("OID with top-bit set");
	oid1 = buf[0] / 40;
	oid2 = buf[0] % 40;
	printf(" value=%u.%u", oid1, oid2);

	i = 1;
	while(i < len)
	{
		if((buf[i] & 0x80) == 0)
		{
			oid = buf[i];
			i ++;
		}
		else
		{
			oid = 0;
			while(((buf[i] & 0x80) == 0x80) && i < len)
			{
				oid <<= 7;
				oid += buf[i] & 0x7f;
				i ++;
			}
			oid <<= 7;
			oid += buf[i];
			i ++;
		}
		printf(".%d", oid);
	}
	printf("\n");

	return;
}


unsigned int
read_file(FILE *in, unsigned int nbytes, void *buf)
{
	if(fread(buf, 1, nbytes, in) != nbytes)
		fatal("EOF");

	return nbytes;
}

unsigned int
type_number(unsigned int type, unsigned int longtype)
{
	return ((type & 0x1f) == 0x1f) ? longtype : type & 0x1f;
}

char *
universal_typename(unsigned int type)
{
	type &= 0x1f;

	if(type == 1)		return "BOOLEAN";
	else if(type == 2)	return "INTEGER";
	else if(type == 3)	return "BIT-STRING";
	else if(type == 4)	return "OCTET-STRING";
	else if(type == 5)	return "NULL";
	else if(type == 6)	return "OBJECT-IDENTIFIER";
	else if(type == 7)	return "ObjectDescriptor";
	else if(type == 8)	return "EXTERNAL";
	else if(type == 9)	return "REAL";
	else if(type == 10)	return "ENUMERATED";
	else if(type == 11)	return "EMBEDDED-PDV";
	else if(type == 12)	return "UTF8String";
	else if(type == 16)	return "SEQUENCE";
	else if(type == 17)	return "SET";
	else if(type == 18)	return "NumericString";
	else if(type == 19)	return "PrintableString";
	else if(type == 20)	return "T61String";
	else if(type == 21)	return "VideotexString";
	else if(type == 22)	return "IA5String";
	else if(type == 23)	return "UTCTime";
	else if(type == 24)	return "GeneralizedTime";
	else if(type == 25)	return "GraphicString";
	else if(type == 26)	return "VisibleString";
	else if(type == 27)	return "GeneralString";
	else if(type == 28)	return "UniversalString";
	else if(type == 30)	return "BMPString";

	fatal("Unknown UNIVERSAL type %u", type);

	return NULL;
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

void
usage(char *progname)
{
	fatal("Usage: %s [<file>]", progname);
}

void
fatal(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	va_end(args);

	exit(EXIT_FAILURE);
}
