/*
 * xsd2c [-v] [-c <output.c>] [-h <output.h>] <ASN1-types.xsd>
 *
 * convert XSD files produces by Objective Systems' ASN1C compiler into C source
 * obviously I could have just got ASN1C to produce the C, but the generated code needs Objective Systems' run time library
 *
 * -v is verbose/debug mode
 * -c gives the output C source file (default is <ASN1-types>.c)
 * -h gives the output header file (default is <ASN1-types>.h)
 *
 * Simon Kilvington, 25/8/2006
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <expat.h>
#include <sys/types.h>
#include <fcntl.h>

/* read .xsd file in chunks of this size (arbitrary) */
#define CHUNK_SIZE	(8 * 1024)

/* maximum depth we can parse (arbitrary) */
#define MAX_DEPTH	128

/* states we can be in during parsing the XSD file */
enum parse_state
{
	STATE_IGNORE,			/* ignoring a tag (and its children) we dont know about */
	STATE_WANT_SCHEMA,		/* looking for xsd:schema */
	STATE_SCHEMA,			/* found xsd:schema */
	STATE_SIMPLE,			/* found xsd:simpleType */
	STATE_RESTRICTION,		/* found xsd:restriction */
	STATE_ANNOTATION,		/* found xsd:annotation */
	STATE_APPINFO,			/* found xsd:appinfo */
	STATE_LIST,			/* found xsd:list */
	STATE_COMPLEX,			/* found xsd:complexType */
	STATE_CHOICE,			/* found xsd:choice */
	STATE_ALL,			/* found xsd:all */
	STATE_SEQUENCE,			/* found xsd:sequence */
	STATE_ELEMENT,			/* found xsd:element */
	STATE_UNION,			/* found xsd:union */
	STATE_COMPLEX_CONTENT		/* found xsd:complexContent */
};

/* BER tag data for a node or a leaf of the type tree */
struct taginfo
{
	char *class;					/* CTXT, APPL, UNIVERSAL */
	char *classnumber;				/* class ID number */
	char *tagtype;					/* IMPLICIT, EXPLICIT */
};

/*
 * data we store about each type we find
 * types are either:
 * - primitive types, type field is eg "xsd:integer", subtypes list is NULL
 * - compound types, type field is eg "xsd:choice", subtypes list contains the elements
 */
struct typeinfo
{
	char *name;					/* type name */
	char *type;					/* base type for simpleType; xsd:sequence, xsd:choice, etc for complexType */
	struct taginfo *taginfo;			/* BER tag data */
	bool optional;					/* OPTIONAL type */
	char *deflt;					/* DEFAULT value or NULL */
	char *value;					/* ENUMERATED value, only applies to xsd:enumeration type */
	bool output;					/* have we output this type to the C files yet */
	struct typeinfo *subtypes;			/* NULL terminated list of enum values, SET/SEQUENCE members etc */
	struct typeinfo *next;				/* linked list for subtypes */
};

/* data we pass to the parser callbacks */
struct userdata
{
	XML_Parser p;					/* so we can get the current line number for error messages */
	int verbose;					/* -v flag */
	char filename[PATH_MAX];			/* .xsd file we are parsing */
	enum parse_state state;				/* state we are currently in */
	enum parse_state state_stack[MAX_DEPTH];	/* stack of states we were in */
	unsigned int state_depth;			/* index to top of state_stack */
	struct typeinfo *type;				/* type we are currently building */
	struct typeinfo *type_stack[MAX_DEPTH];		/* stack of enclosing types we are currently parsing */
	unsigned int type_depth;			/* index to top of type_stack */
	struct typeinfo *types;				/* NULL terminated list of all the types we have found so far */
};

/* XML parser */
void tag_start(void *, const char *, const char **);
void tag_end(void *, const char *);

const char *get_attr(const char *, const char **);

void state_push(struct userdata *, enum parse_state);
void state_pop(struct userdata *);
void type_push(struct userdata *, struct typeinfo *);
void type_pop(struct userdata *);

/* handle typeinfo */
struct typeinfo *add_type(struct userdata *, const char *);
struct typeinfo *add_subtype(struct userdata *, struct typeinfo *, const char *);
struct typeinfo *find_type(char *, struct typeinfo *);
void set_type(struct userdata *, struct typeinfo *, const char *);
void set_taginfo(struct userdata *, struct typeinfo *, const char *, const char *, const char *);
void set_value(struct userdata *, struct typeinfo *, const char *);
void set_default(struct userdata *, struct typeinfo *, const char *);

struct typeinfo *new_typeinfo(const char *);
struct taginfo *new_taginfo(const char *, const char *, const char *);

/* output files */
void output_header(FILE *, FILE *, char *);

void output_typedef(FILE *, FILE *, struct typeinfo *, struct typeinfo *);
void output_subtype(FILE *, int, struct typeinfo *, struct typeinfo *);

void output_optional(FILE *, int, struct typeinfo *);

void output_first_choice(FILE *, int, struct typeinfo *, char *);
void output_choice(FILE *, int, struct typeinfo *, char *);
void output_last_choice(FILE *, int, struct typeinfo *, char *);

void output_decodefunc(FILE *, struct typeinfo *, struct typeinfo *);
void output_defaults(FILE *, struct typeinfo *);
void output_decode_tag(FILE *, char *, char *, int);
void output_decode_subtype(FILE *, char *, char *, char *, char *, int);
bool output_tagmatch(FILE *, struct typeinfo *, struct typeinfo *, int);
void output_extend_seq(FILE *, char *, char *, char *, int);

void output_freefunc(FILE *, struct typeinfo *);

char *output_typename(char *);
char *output_decodename(char *);
char *output_tagclass(char *);
unsigned int output_univnumber(char *);

/* verbose output */
void print_type(int, struct typeinfo *);
void print_indent(FILE *, int);

/* utils */
char *mybasename(char *);

char *safe_strdup(const char *);
void *safe_malloc(size_t);
void safe_free(void *);

void parse_info(struct userdata *, char *, ...);
void parse_warning(struct userdata *, char *, ...);
void parse_fatal(struct userdata *, char *, ...);

void usage(char *);
void fatal(char *, ...);

/* here we go... */
int
main(int argc, char *argv[])
{
	char *prog_name;
	char src_name[PATH_MAX];
	char hdr_name[PATH_MAX];
	char basename[PATH_MAX];
	struct userdata ud;
	int arg;
	int xsd_fd;
	char *buf;
	int len;
	struct typeinfo *t;
	FILE *src_file;
	FILE *hdr_file;

	prog_name = argv[0];
	bzero(&ud, sizeof(struct userdata));

	src_name[0] = '\0';
	hdr_name[0] = '\0';
	while((arg = getopt(argc, argv, "c:h:v")) != EOF)
	{
		switch(arg)
		{
		case 'c':
			strncpy(src_name, optarg, sizeof(src_name));
			/* just in case */
			src_name[sizeof(src_name) - 1] = '\0';
			break;

		case 'h':
			strncpy(hdr_name, optarg, sizeof(hdr_name));
			hdr_name[sizeof(hdr_name) - 1] = '\0';
			break;

		case 'v':
			ud.verbose ++;
			break;

		default:
			usage(prog_name);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if(argc != 1)
		usage(prog_name);

	strncpy(ud.filename, argv[0], sizeof(ud.filename));
	ud.filename[sizeof(ud.filename) - 1] = '\0';

	/* parse the XSD file to get all the type definitions */
	if((xsd_fd = open(ud.filename, O_RDONLY)) < 0)
		fatal("Unable to open file %s: %s", ud.filename, strerror(errno));

	if((ud.p = XML_ParserCreate(NULL)) == NULL)
		fatal("No memory to create parser");

	XML_SetUserData(ud.p, (void *) &ud);
	XML_SetElementHandler(ud.p, tag_start, tag_end);

	/* initial state */
	ud.state = STATE_WANT_SCHEMA;
	ud.state_depth = 0;
	/* no types found yet */
	ud.type = NULL;
	ud.type_depth = 0;
	ud.types = NULL;

	do
	{
		if((buf = XML_GetBuffer(ud.p, CHUNK_SIZE)) == NULL)
			fatal("No memory to parse file %s", ud.filename);
		if((len = read(xsd_fd, buf, CHUNK_SIZE)) < 0)
			fatal("Unable to read file %s", ud.filename);
		if(XML_ParseBuffer(ud.p, len, len==0) == 0)
			fatal("Parse error: %s line %d: %s", ud.filename, XML_GetCurrentLineNumber(ud.p), XML_ErrorString(XML_GetErrorCode(ud.p)));
	}
	while(len != 0);

	/* assert */
	if(ud.type_depth != 0)
		fatal("%s: type stack not empty (%d)", ud.filename, ud.type_depth);
	if(ud.state_depth != 0)
		fatal("%s: state stack not empty (%d)", ud.filename, ud.state_depth);

	XML_ParserFree(ud.p);
	close(xsd_fd);

	/* print the types we found */
	if(ud.verbose)
	{
		for(t=ud.types; t; t=t->next)
		{
			print_type(0, t);
			printf("\n");
		}
	}

	/* output the C files */
	if(hdr_name[0] == '\0')
	{
		strncpy(basename, ud.filename, sizeof(basename));
		basename[sizeof(basename) - 1] = '\0';
		snprintf(hdr_name, sizeof(hdr_name), "%s.h", mybasename(basename));
	}

	printf("Generating header file %s\n", hdr_name);

	if((hdr_file = fopen(hdr_name, "w")) == NULL)
		fatal("Unable to create output file '%s': %s", hdr_name, strerror(errno));

	if(src_name[0] == '\0')
	{
		strncpy(basename, ud.filename, sizeof(basename));
		basename[sizeof(basename) - 1] = '\0';
		snprintf(src_name, sizeof(src_name), "%s.c", mybasename(basename));
	}

	printf("Generating source file %s\n", src_name);

	if((src_file = fopen(src_name, "w")) == NULL)
		fatal("Unable to create output file '%s': %s", src_name, strerror(errno));

	/* standard header */
	output_header(src_file, hdr_file, ud.filename);

	/* .c includes */
	fprintf(src_file, "#include \"%s\"\n\n", hdr_name);

	/* .h guard against multiple inclusions */
	fprintf(hdr_file, "#ifndef __XSD2C_HEADER__\n");
	fprintf(hdr_file, "#define __XSD2C_HEADER__\n\n");

	/* .h includes */
	fprintf(hdr_file, "#include \"listof.h\"\n");
	fprintf(hdr_file, "#include \"der_decode.h\"\n\n");

	/* output each type */
	for(t=ud.types; t; t=t->next)
		output_typedef(src_file, hdr_file, t, ud.types);

	fprintf(hdr_file, "#endif\t/* __XSD2C_HEADER__ */\n\n");

	fclose(src_file);
	fclose(hdr_file);

	return EXIT_SUCCESS;
}

void
tag_start(void *data, const char *tag, const char **attr)
{
	struct userdata *ud = (struct userdata *) data;
	const char *name;
	const char *type;
	const char *class;
	const char *classnumber;
	const char *tagtype;
	const char *value;
	const char *optional;
	const char *deflt;
	struct typeinfo *t;

	switch(ud->state)
	{
	case STATE_IGNORE:
		parse_warning(ud, "Ignoring unknown tag %s", tag);
		/* ignore its children too */
		state_push(ud, STATE_IGNORE);
		break;

	case STATE_WANT_SCHEMA:
		if(strcmp(tag, "xsd:schema") == 0)
		{
			state_push(ud, STATE_SCHEMA);
		}
		else
		{
			parse_warning(ud, "Ignoring unknown tag %s", tag);
			state_push(ud, STATE_IGNORE);
		}
		break;

	case STATE_SCHEMA:
		/* assert - check we have no half completed types on the stack */
		if(ud->type_depth != 0)
			parse_fatal(ud, "tag_start: state=STATE_SCHEMA; type_depth=%d", ud->type_depth);
		/* is it a type definition */
		if(strcmp(tag, "xsd:simpleType") == 0)
		{
			name = get_attr("name", attr);
			type_push(ud, add_type(ud, name));
			state_push(ud, STATE_SIMPLE);
		}
		else if(strcmp(tag, "xsd:complexType") == 0)
		{
			name = get_attr("name", attr);
			type_push(ud, add_type(ud, name));
			state_push(ud, STATE_COMPLEX);
		}
		else if(strcmp(tag, "xsd:import") == 0)
		{
			/* ignore it and any sub-elements */
			state_push(ud, STATE_IGNORE);
		}
		else if(strcmp(tag, "xsd:element") == 0)
		{
			/*
			 * ignore top level PDU definitions like this:
			 * <xsd:element name="presentableClass" type="PresentableClass"/>
			 */
			state_push(ud, STATE_IGNORE);
		}
		else
		{
			parse_warning(ud, "Ignoring unknown tag %s", tag);
			state_push(ud, STATE_IGNORE);
		}
		break;

	case STATE_SIMPLE:
		if(strcmp(tag, "xsd:restriction") == 0)
		{
			type = get_attr("base", attr);
			/* base attr may not be present */
			if(type[0] != '\0')
				set_type(ud, ud->type, type);
			state_push(ud, STATE_RESTRICTION);
		}
		else if(strcmp(tag, "xsd:annotation") == 0)
		{
			state_push(ud, STATE_ANNOTATION);
		}
		else if(strcmp(tag, "xsd:list") == 0)
		{
			set_type(ud, ud->type, tag);
			/* create a subtype to hold the type of the list (ie x in SEQUENCE OF x) */
			type_push(ud, add_subtype(ud, ud->type, tag));
			/* see if the list tag defines the type of its items */
			type = get_attr("itemType", attr);
			if(type[0] != '\0')
				set_type(ud, ud->type, type);
			state_push(ud, STATE_LIST);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:simpleType", tag);
		}
		break;

	case STATE_RESTRICTION:
		if(strcmp(tag, "xsd:simpleType") == 0)
		{
			state_push(ud, STATE_SIMPLE);
		}
		else if(strcmp(tag, "xsd:enumeration") == 0)
		{
			name = get_attr("value", attr);
			value = get_attr("asn1:value", attr);
			t = add_subtype(ud, ud->type, name);
			set_type(ud, t, tag);
			set_value(ud, t, value);
			/* ignore any sub-elements */
			state_push(ud, STATE_IGNORE);
		}
		else if(strcmp(tag, "xsd:minLength") == 0 || strcmp(tag, "xsd:minInclusive") == 0)
		{
			/* we don't care about constraints */
			state_push(ud, STATE_IGNORE);
		}
		else if(strcmp(tag, "xsd:maxLength") == 0 || strcmp(tag, "xsd:maxInclusive") == 0)
		{
			/* we don't care about constraints */
			state_push(ud, STATE_IGNORE);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:restriction", tag);
		}
		break;

	case STATE_ANNOTATION:
		if(strcmp(tag, "xsd:appinfo") == 0)
		{
			state_push(ud, STATE_APPINFO);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:annotation", tag);
		}
		break;

	case STATE_APPINFO:
		if(strcmp(tag, "asn1:taginfo") == 0)
		{
			class = get_attr("class", attr);
			classnumber = get_attr("classnumber", attr);
			tagtype = get_attr("tagtype", attr);
			set_taginfo(ud, ud->type, class, classnumber, tagtype);
			/* ignore any sub-elements */
			state_push(ud, STATE_IGNORE);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:appinfo", tag);
		}
		break;

	case STATE_LIST:
		if(strcmp(tag, "xsd:simpleType") == 0)
		{
			state_push(ud, STATE_SIMPLE);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:list", tag);
		}
		break;

	case STATE_COMPLEX:
		if(strcmp(tag, "xsd:choice") == 0)
		{
			set_type(ud, ud->type, tag);
			state_push(ud, STATE_CHOICE);
		}
		else if(strcmp(tag, "xsd:all") == 0)
		{
			set_type(ud, ud->type, tag);
			state_push(ud, STATE_ALL);
		}
		else if(strcmp(tag, "xsd:sequence") == 0)
		{
			set_type(ud, ud->type, tag);
			state_push(ud, STATE_SEQUENCE);
		}
		else if(strcmp(tag, "xsd:complexContent") == 0)
		{
			state_push(ud, STATE_COMPLEX_CONTENT);
		}
		else if(strcmp(tag, "xsd:annotation") == 0)
		{
			state_push(ud, STATE_ANNOTATION);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:complexType", tag);
		}
		break;

	case STATE_CHOICE:
	case STATE_ALL:
	case STATE_SEQUENCE:
		if(strcmp(tag, "xsd:element") == 0)
		{
			name = get_attr("name", attr);
			type = get_attr("type", attr);
			optional = get_attr("minOccurs", attr);
			deflt = get_attr("default", attr);
			t = add_subtype(ud, ud->type, name);
			/* type may be defined by later tags or it may be named in this tag */
			if(type[0] != '\0')
				set_type(ud, t, type);
			/* minOccurs="0" means OPTIONAL */
			if(strcmp(optional, "0") == 0)
				t->optional = true;
			/* has it got a default value */
			if(deflt[0] != '\0')
				set_default(ud, t, deflt);
			/* all processing from now on is on this subtype */
			type_push(ud, t);
			state_push(ud, STATE_ELEMENT);
		}
		else
		{
			parse_fatal(ud, "Unexpected tag %s in xsd:complexType", tag);
		}
		break;

	case STATE_ELEMENT:
		if(strcmp(tag, "xsd:annotation") == 0)
		{
			state_push(ud, STATE_ANNOTATION);
		}
		else if(strcmp(tag, "xsd:simpleType") == 0)
		{
			state_push(ud, STATE_SIMPLE);
		}
		else if(strcmp(tag, "xsd:complexType") == 0)
		{
			state_push(ud, STATE_COMPLEX);
		}
		else if(strcmp(tag, "xsd:union") == 0)
		{
			set_type(ud, ud->type, tag);
			state_push(ud, STATE_UNION);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:element", tag);
		}
		break;

	case STATE_UNION:
		/* basically the same as STATE_SCHEMA, but we add the type we find as a subtype of ud->type */
		if(strcmp(tag, "xsd:simpleType") == 0)
		{
			type_push(ud, add_subtype(ud, ud->type, "xsd:union"));
			state_push(ud, STATE_SIMPLE);
		}
		else if(strcmp(tag, "xsd:complexType") == 0)
		{
			type_push(ud, add_subtype(ud, ud->type, "xsd:union"));
			state_push(ud, STATE_COMPLEX);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:union", tag);
		}
		break;

	case STATE_COMPLEX_CONTENT:
		if(strcmp(tag, "xsd:extension") == 0)
		{
			/* this is just an alias for another type */
			type = get_attr("base", attr);
			set_type(ud, ud->type, type);
			/* ignore any sub-elements */
			state_push(ud, STATE_IGNORE);
		}
		else
		{
			parse_fatal(ud, "Unknown tag %s in xsd:complexContent", tag);
		}
		break;

	default:
		parse_fatal(ud, "tag_start: Undefined state (%d)", ud->state);
		break;
	}

	return;
}

void
tag_end(void *data, const char *tag)
{
	struct userdata *ud = (struct userdata *) data;

	/* are we ignoring this tag */
	if(ud->state == STATE_IGNORE)
	{
		state_pop(ud);
		return;
	}

	/* find out what state we were in when we started this tag */
	state_pop(ud);

	/* any mismatched tag errors will be picked up for us by the XML parser */
	switch(ud->state)
	{
	case STATE_IGNORE:
		break;

	case STATE_WANT_SCHEMA:
		break;

	case STATE_SCHEMA:
		/* finished building this type now */
		type_pop(ud);
		break;

	case STATE_SIMPLE:
		/* if we were constructing a list, pop the typeinfo for the list items */
		if(strcmp(tag, "xsd:list") == 0)
			type_pop(ud);
		break;

	case STATE_RESTRICTION:
		break;

	case STATE_ANNOTATION:
		break;

	case STATE_APPINFO:
		break;

	case STATE_LIST:
		break;

	case STATE_COMPLEX:
		break;

	case STATE_CHOICE:
	case STATE_ALL:
	case STATE_SEQUENCE:
		/* finished building this subtype */
		type_pop(ud);
		break;

	case STATE_ELEMENT:
		break;

	case STATE_UNION:
		/* finished building this part of the union now */
		type_pop(ud);
		break;

	case STATE_COMPLEX_CONTENT:
		break;

	default:
		parse_fatal(ud, "tag_end: Undefined state (%d)", ud->state);
		break;
	}

	return;
}

static char _no_value[] = "";

const char *
get_attr(const char *name, const char **attr)
{
	const char *value = _no_value;
	int i;

	for(i=0; attr[i] && value==_no_value; i+=2)
	{
		if(strcmp(attr[i], name) == 0)
			value = attr[i+1];
	}

	return value;
}

void
state_push(struct userdata *ud, enum parse_state state)
{
	/* save current state on the stack */
	ud->state_stack[ud->state_depth] = ud->state;
	/* update current state to the new value */
	ud->state = state;

	ud->state_depth ++;

	if(ud->state_depth == MAX_DEPTH)
		parse_fatal(ud, "Maximum state depth (%d) reached: recompile with larger MAX_DEPTH", ud->state_depth);

	return;
}

void
state_pop(struct userdata *ud)
{
	if(ud->state_depth == 0)
		parse_fatal(ud, "Too many state_pop()'s!");

	ud->state_depth --;

	ud->state = ud->state_stack[ud->state_depth];

	return;
}

void
type_push(struct userdata *ud, struct typeinfo *type)
{
	/* save current type on the stack */
	ud->type_stack[ud->type_depth] = ud->type;
	/* update current type to the new value */
	ud->type = type;

	ud->type_depth ++;

	if(ud->type_depth == MAX_DEPTH)
		parse_fatal(ud, "Maximum type depth (%d) reached: recompile with larger MAX_DEPTH", ud->type_depth);

	return;
}

void
type_pop(struct userdata *ud)
{
	if(ud->type_depth == 0)
		parse_fatal(ud, "Too many type_pop()'s!");

	ud->type_depth --;

	ud->type = ud->type_stack[ud->type_depth];

	return;
}

/*
 * create a new type with the given name
 */

struct typeinfo *
add_type(struct userdata *ud, const char *name)
{
	struct typeinfo *t;
	struct typeinfo *list;

	t = new_typeinfo(name);

	/* add it to the list of types we have found so far */
	if(ud->types == NULL)
	{
		/* first one */
		ud->types = t;
	}
	else
	{
		/* add it to the end of the list */
		list = ud->types;
		while(list->next)
			list = list->next;
		list->next = t;
	}

	return t;
}

/*
 * add a subtype to an existing type
 */

struct typeinfo *
add_subtype(struct userdata *ud, struct typeinfo *parent, const char *name)
{
	struct typeinfo *st = new_typeinfo(name);
	struct typeinfo *list;

	if(parent->subtypes == NULL)
	{
		/* first one */
		parent->subtypes = st;
	}
	else
	{
		/* add it to the end of the list */
		list = parent->subtypes;
		while(list->next)
			list = list->next;
		list->next = st;
	}

	return st;
}

struct typeinfo *
find_type(char *name, struct typeinfo *types)
{
	while(types)
	{
		if(strcmp(types->name, name) == 0)
			return types;
		types = types->next;
	}

	fatal("Unknown type: %s", name);

	/* not reached */
	return NULL;
}

void
set_type(struct userdata *ud, struct typeinfo *t, const char *type)
{
	/* assert */
	if(t->type != NULL)
		parse_fatal(ud, "Trying to reassign type of %s (old=%s new=%s)", t->name, t->type, type);

	t->type = safe_strdup(type);

	return;
}

void
set_taginfo(struct userdata *ud, struct typeinfo *t, const char *class, const char *classnumber, const char *tagtype)
{
	/*
	 * the taginfo data is sometimes provided twice in the XSD
	 * eg standard_version in GroupClass
	 * if we already have some, check the new is the same as the old
	 */
	if(t->taginfo != NULL)
	{
		if(strcmp(t->taginfo->class, class) != 0)
			parse_fatal(ud, "Trying to redefine tag class from %s to %s", t->taginfo->class, class);
		if(strcmp(t->taginfo->classnumber, classnumber) != 0)
			parse_fatal(ud, "Trying to redefine tag number from %s to %s", t->taginfo->classnumber, classnumber);
		if(strcmp(t->taginfo->tagtype, tagtype) != 0)
			parse_fatal(ud, "Trying to redefine tag type from %s to %s", t->taginfo->tagtype, tagtype);
	}
	else
	{
		t->taginfo = new_taginfo(class, classnumber, tagtype);
	}

	return;
}

void
set_value(struct userdata *ud, struct typeinfo *t, const char *value)
{
	/* assert */
	if(t->value != NULL)
		parse_fatal(ud, "Trying to reassign value of %s (old=%s new=%s)", t->name, t->value, value);

	t->value = safe_strdup(value);

	return;
}

void
set_default(struct userdata *ud, struct typeinfo *t, const char *deflt)
{
	/* assert */
	if(t->deflt != NULL)
		parse_fatal(ud, "Trying to reassign value of %s (old=%s new=%s)", t->name, t->deflt, deflt);

	t->deflt = safe_strdup(deflt);

	return;
}

struct typeinfo *
new_typeinfo(const char *name)
{
	struct typeinfo *t = (struct typeinfo *) safe_malloc(sizeof(struct typeinfo));
	bzero(t, sizeof(struct typeinfo));

	t->name = safe_strdup(name);
	t->type = NULL;
	t->taginfo = NULL;
	t->optional = false;
	t->deflt = NULL;
	t->value = NULL;
	t->output = false;
	t->subtypes = NULL;
	t->next = NULL;

	return t;
}

struct taginfo *
new_taginfo(const char *class, const char *classnumber, const char *tagtype)
{
	struct taginfo *t = safe_malloc(sizeof(struct taginfo));
	bzero(t, sizeof(struct taginfo));

	t->class = safe_strdup(class);
	t->classnumber = safe_strdup(classnumber);
	t->tagtype = safe_strdup(tagtype);

	return t;
}

void
output_header(FILE *src, FILE *hdr, char *xsd_name)
{
	time_t now = time(NULL);
	FILE *out[2] = { src, hdr };
	int i;

	for(i=0; i<2; i++)
	{
		fprintf(out[i], "/*\n");
		fprintf(out[i], " * This file was automatically generated. Do not edit!\n");
		fprintf(out[i], " *\n");
		fprintf(out[i], " * Input: %s\n", xsd_name);
		fprintf(out[i], " * Date:  %s", ctime(&now));
		fprintf(out[i], " */\n\n");
	}

	return;
}

void
output_typedef(FILE *src, FILE *hdr, struct typeinfo *t, struct typeinfo *types)
{
	struct typeinfo *st;

	/* have we already output this type */
	if(t->output)
		return;

	/* is it a primitive or a compound type */
	if(t->subtypes == NULL)
	{
		/* if its not an xsd: type, make sure we have output its type already */
		if(strchr(t->type, ':') == NULL)
			output_typedef(src, hdr, find_type(t->type, types), types);
		fprintf(hdr, "/*\n * %s\n */\n", t->name);
		fprintf(hdr, "typedef %s %s;\n\n", output_typename(t->type), t->name);
	}
	else
	{
		/* make sure we have output all its subtypes first */
		for(st=t->subtypes; st; st=st->next)
		{
			/* special cases */
			if(strcmp(st->type, "xsd:sequence") == 0 && st->subtypes)
				output_typedef(src, hdr, find_type(st->subtypes->type, types), types);
			/* dont need to do xsd: types */
			else if(strchr(st->type, ':') == NULL)
				output_typedef(src, hdr, find_type(st->type, types), types);
		}
		/* now the type itself */
		fprintf(hdr, "/*\n * %s\n */\n", t->name);
		/* does it need to be a LIST_OF type */
		if(((strcmp(t->type, "xsd:list") == 0) || (strcmp(t->type, "xsd:sequence") == 0))
		&& t->subtypes && t->subtypes->next == NULL)
		{
			st = t->subtypes;
			fprintf(hdr, "typedef LIST_OF(%s) *%s;\n\n", output_typename(st->type), t->name);
		}
		/* normal type */
		else
		{
			if(strcmp(t->type, "xsd:token") == 0)
				fprintf(hdr, "typedef enum %s\n", t->name);
			else
				fprintf(hdr, "typedef struct %s\n", t->name);
			fprintf(hdr, "{\n");
			for(st=t->subtypes; st; st=st->next)
				output_subtype(hdr, 1, t, st);
			fprintf(hdr, "} %s;\n\n", t->name);
		}
	}

	/* define a LIST_OF type */
	fprintf(hdr, "DEFINE_LIST_OF(%s);\n\n", t->name);

	/* function prototypes */
	fprintf(hdr, "int der_decode_%s(FILE *, %s *, int);\n", t->name, t->name);
	fprintf(hdr, "/* only free's the contents, not the type itself */\n");
	fprintf(hdr, "void free_%s(%s *);\n\n", t->name, t->name);

	/* .c file */
	output_decodefunc(src, t, types);
	output_freefunc(src, t);

	/* don't output this type more than once */
	t->output = true;

	return;
}

void
output_subtype(FILE *out, int indent, struct typeinfo *parent, struct typeinfo *t)
{
	struct typeinfo *st;

	print_indent(out, indent);

	/* is it a primitive or a compound type */
	if(t->subtypes == NULL)
	{
		output_optional(out, indent, t);
		/* is it a SEQUENCE OF primitive-type */
		if(strcmp(parent->type, "xsd:list") == 0)
			fatal("xsd:list in subtype");
		/* is it a SEQUENCE OF complex-type (a sequence with only 1 subtype) */
		else if(strcmp(parent->type, "xsd:sequence") == 0 && parent->subtypes->next == NULL)
			fprintf(out, "LIST_OF(%s) *%s;\n", output_typename(t->type), parent->name);
		/* is it an ENUMERATED type */
		else if(strcmp(t->type, "xsd:enumeration") == 0)
			fprintf(out, "%s_%s = %s,\n", parent->name, t->name, t->value);
		/* is it the first CHOICE option (the first subtype of its parent) */
		else if(strcmp(parent->type, "xsd:choice") == 0 && parent->subtypes == t)
			output_first_choice(out, indent, t, parent->name);
		/* is it the last CHOICE option (the last subtype of its parent) */
		else if(strcmp(parent->type, "xsd:choice") == 0 && t->next == NULL)
			output_last_choice(out, indent, t, parent->name);
		/* a middle CHOICE option */
		else if(strcmp(parent->type, "xsd:choice") == 0)
			output_choice(out, indent, t, parent->name);
		/* normal type */
		else
		{
			fprintf(out, "%s %s;", output_typename(t->type), t->name);
			/* does it have a default value */
			if(t->deflt != NULL)
				fprintf(out, "\t/* DEFAULT %s */", t->deflt);
			fprintf(out, "\n");
		}
	}
	else
	{
		/*
		 * is it an INTEGER with named values
		 * typeinfo structure is:
		 * xsd:union {
		 *   xsd:token {
		 *     xsd:enumeration name1, xsd:enumeration name2, ...
		 *   }
		 *   xsd:integer
		 * }
		 */
		if(strcmp(t->type, "xsd:union") == 0
		&& t->subtypes
		&& strcmp(t->subtypes->type, "xsd:token") == 0
		&& t->subtypes->next
		&& strcmp(t->subtypes->next->type, "xsd:integer") == 0
		&& t->subtypes->next->next == NULL)
		{
			output_optional(out, indent, t);
			fprintf(out, "%s %s;", output_typename(t->subtypes->next->type), t->name);
			/* does it have a default value */
			if(t->deflt != NULL)
				fprintf(out, "\t/* DEFAULT %s */", t->deflt);
			fprintf(out, "\n");
			for(st=t->subtypes->subtypes; st; st=st->next)
				fprintf(out, "#define %s_%s %s\n", t->name, st->name, st->value);
		}
		else
		{
			/* check it is a SEQUENCE OF complex-type (a sequence with only 1 subtype) */
			if(strcmp(t->type, "xsd:sequence") != 0 || t->subtypes->next != NULL)
				fatal("Unexpected type %s in output_subtype", t->type);
			st = t->subtypes;
			fprintf(out, "LIST_OF(%s) *%s;", output_typename(st->type), t->name);
			if(t->optional)
				fprintf(out, "\t/* OPTIONAL */");
			fprintf(out, "\n");
		}
	}

	return;
}

void
output_optional(FILE *out, int indent, struct typeinfo *t)
{
	if(t->optional)
	{
		fprintf(out, "bool have_%s;\t/* OPTIONAL */\n", t->name);
		print_indent(out, indent);
	}

	return;
}

/*
 * CHOICE becomes:
 * unsigned int choice; union { type1 name1; type2 name2; ... } u;
 */

unsigned int _choice;

void
output_first_choice(FILE *out, int indent, struct typeinfo *t, char *parent)
{
	fprintf(out, "unsigned int choice;\n");

	print_indent(out, indent);
	fprintf(out, "union {\n");

	_choice = 1;

	print_indent(out, indent);
	output_choice(out, indent, t, parent);

	return;
}

void
output_choice(FILE *out, int indent, struct typeinfo *t, char *parent)
{
	fprintf(out, "\t%s %s;\t/* choice == %u */\n", output_typename(t->type), t->name, _choice);
	fprintf(out, "#define %s_%s %u\n", parent, t->name, _choice);

	_choice ++;

	return;
}

void
output_last_choice(FILE *out, int indent, struct typeinfo *t, char *parent)
{
	output_choice(out, indent, t, parent);

	print_indent(out, indent);
	fprintf(out, "} u;\n");

	return;
}

void
output_decodefunc(FILE *src, struct typeinfo *t, struct typeinfo *types)
{
	struct typeinfo *st;
	char ptr_str[1024];
	char len_str[1024];
	bool need_tag;
	int indent;

	fprintf(src, "int\n");
	fprintf(src, "der_decode_%s(FILE *der, %s *type, int length)\n", t->name, t->name);
	fprintf(src, "{\n");
	fprintf(src, "\tint left = length;\n");
	fprintf(src, "\tint sublen;\n");

	/* do we need this tag variable */
	if(t->subtypes != NULL && strcmp(t->type, "xsd:token") != 0)
		fprintf(src, "\tder_tag tag;\n");

	/* do we need this seqtag variable */
	if(strcmp(t->type, "xsd:sequence") == 0)
	{
		for(st=t->subtypes; st; st=st->next)
		{
			if(strcmp(st->type, "xsd:sequence") == 0 && st->subtypes && st->subtypes->next == NULL)
			{
				fprintf(src, "\tder_tag seqtag;\n");
				break;
			}
		}
	}

	/* do we need the ENUMERATED value variable */
	if(strcmp(t->type, "xsd:token") == 0)
		fprintf(src, "\tint value;\n");

	/* output the type name */
	fprintf(src, "\n#ifdef DER_VERBOSE\n");
	fprintf(src, "\tprintf(\"<%s>\\n\");\n", t->name);
	fprintf(src, "#endif\n\n");

	/* reset it all to 0 */
	fprintf(src, "\tbzero(type, sizeof(%s));\n\n", t->name);

	/* set any default values */
	output_defaults(src, t);

	/* is it a primitive type (or an alias for another complex type) */
	if(t->subtypes == NULL)
	{
		snprintf(ptr_str, sizeof(ptr_str), "(%s *) type", output_typename(t->type));
		output_decode_subtype(src, t->name, output_typename(t->type), ptr_str, "length", 1);
		fprintf(src, "\tleft -= sublen;\n\n");
	}
	/* is it a CHOICE */
	else if(strcmp(t->type, "xsd:choice") == 0)
	{
		fprintf(src, "\t/* CHOICE */\n");
		output_decode_tag(src, t->name, "&tag", 1);
		fprintf(src, "\t");
		for(st=t->subtypes; st; st=st->next)
		{
			/* if(tag.class == ... && tag.number == ...) */
			fprintf(src, "if(");
			need_tag = output_tagmatch(src, st, types, 1);
			fprintf(src, ")\n\t{\n");
			fprintf(src, "\t\t/* %s */\n", st->name);
			/* is the subtype also a CHOICE type => it needs the tag included */
			if(need_tag)
				fprintf(src, "\t\tfseek(der, -sublen, SEEK_CUR);\n");
			else
				fprintf(src, "\t\tleft -= sublen;\n");
			/* set choice value */
			fprintf(src, "\t\ttype->choice = %s_%s;\n", t->name, st->name);
			/* decode the type */
			snprintf(ptr_str, sizeof(ptr_str), "&type->u.%s", st->name);
			snprintf(len_str, sizeof(len_str), "%s", need_tag ? "length" : "tag.length");
			output_decode_subtype(src, t->name, output_decodename(st->type), ptr_str, len_str, 2);
			fprintf(src, "\t\tleft -= sublen;\n");
			fprintf(src, "\t}\n");
			/* get ready for the next choice */
			fprintf(src, "\telse ");
		}
		fprintf(src, "\n\t\treturn der_error(\"%s: invalid choice tag (%%u)\", tag.number);\n\n", t->name);
	}
	/* is it an ENUMERATED type */
	else if(strcmp(t->type, "xsd:token") == 0)
	{
		/* ENUMERATED is stored as an INTEGER */
		fprintf(src, "\t/* ENUMERATED */\n");
		output_decode_subtype(src, t->name, "Integer", "&value", "length", 1);
		fprintf(src, "\tleft -= sublen;\n\n");
		fprintf(src, "\t*type = (%s) value;\n\n", t->name);
	}
	/* is it a SEQUENCE OF primitive-type, or a SEQUENCE OF complex-type */
	else if((strcmp(t->type, "xsd:list") == 0 && t->subtypes && t->subtypes->next == NULL)
	     || (strcmp(t->type, "xsd:sequence") == 0 && t->subtypes && t->subtypes->next == NULL))
	{
		struct typeinfo *elem = t->subtypes;
		fprintf(src, "\t/* SEQUENCE OF %s */\n", output_typename(elem->type));
		fprintf(src, "\twhile(left > 0)\n");
		fprintf(src, "\t{\n");
		/* check the tag is what we expect */
		fprintf(src, "\t\tLIST_TYPE(%s) *new_item;\n", output_typename(elem->type));
		output_decode_tag(src, t->name, "&tag", 2);
		fprintf(src, "\t\tif(!(");
		need_tag = output_tagmatch(src, elem, types, 2);
		fprintf(src, "))\n");
		fprintf(src, "\t\t\treturn der_error(\"%s: unexpected tag %%u\", tag.number);\n", t->name);
		if(need_tag)
			fprintf(src, "\t\tfseek(der, -sublen, SEEK_CUR);\n");
		else
			fprintf(src, "\t\tleft -= sublen;\n");
		/* extend the elements array */
		output_extend_seq(src, t->name, "type", output_typename(elem->type), 2);
		/* decode the type */
		snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
		output_decode_subtype(src, t->name, output_decodename(elem->type), "&new_item->item", len_str, 2);
		fprintf(src, "\t\tleft -= sublen;\n");
		fprintf(src, "\t}\n\n");
	}
	/* is it a heterogeneous SEQUENCE */
	else if(strcmp(t->type, "xsd:sequence") == 0)
	{
		fprintf(src, "\t/* SEQUENCE */\n");
		/* for each subtype */
		for(st=t->subtypes; st; st=st->next)
		{
			fprintf(src, "\t/* %s", st->name);
			if(st->optional)
				fprintf(src, " OPTIONAL");
			if(st->deflt != NULL)
				fprintf(src, " DEFAULT %s", st->deflt);
			fprintf(src, " */\n");
			/* is it a SEQUENCE OF type */
			if(strcmp(st->type, "xsd:sequence") == 0 && st->subtypes && st->subtypes->next == NULL)
			{
				struct typeinfo *elem = st->subtypes;
				/* maybe OPTIONAL (having a DEFAULT value also means its optional) */
				indent = 1;
				if(st->optional || st->deflt != NULL)
				{
					if(st->next)
						fatal("%s.%s: SEQUENCE OF is not the last element", t->name, st->name);
					fprintf(src, "\tif(left > 0)\n");
					fprintf(src, "\t{\n");
					indent ++;
				}
				print_indent(src, indent);
				fprintf(src, "/* SEQUENCE OF %s */\n", output_typename(elem->type));
				/* get the length of the SEQUENCE */
				output_decode_tag(src, t->name, "&seqtag", indent);
				/* check it is a SEQUENCE tag */
				print_indent(src, indent);
				fprintf(src, "if(seqtag.class != DER_CLASS_UNIVERSAL || seqtag.number != 16)\n");
				print_indent(src, indent);
				fprintf(src, "\treturn der_error(\"%s: unexpected tag %%u\", seqtag.number);\n", t->name);
				/* get each element */
				print_indent(src, indent);
				fprintf(src, "left -= sublen + seqtag.length;\n");
				print_indent(src, indent);
				fprintf(src, "while(seqtag.length > 0)\n");
				print_indent(src, indent);
				fprintf(src, "{\n");
				print_indent(src, indent);
				fprintf(src, "\tLIST_TYPE(%s) *new_item;\n", output_typename(elem->type));
				/* check the tag is what we expect */
				output_decode_tag(src, t->name, "&tag", indent + 1);
				print_indent(src, indent + 1);
				fprintf(src, "if(!(");
				need_tag = output_tagmatch(src, elem, types, indent + 1);
				fprintf(src, "))\n");
				print_indent(src, indent + 1);
				fprintf(src, "\treturn der_error(\"%s: unexpected tag %%u\", tag.number);\n", t->name);
				/* does the subtype decoder need the tag */
				print_indent(src, indent + 1);
				if(need_tag)
					fprintf(src, "fseek(der, -sublen, SEEK_CUR);\n");
				else
					fprintf(src, "seqtag.length -= sublen;\n");
				/* extend the array */
				snprintf(ptr_str, sizeof(ptr_str), "&type->%s", st->name);
				output_extend_seq(src, t->name, ptr_str, output_typename(elem->type), indent + 1);
				/* decode the element */
				snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
				output_decode_subtype(src, t->name, output_decodename(elem->type), "&new_item->item", len_str, indent + 1);
				print_indent(src, indent + 1);
				fprintf(src, "if(sublen > seqtag.length)\n");
				print_indent(src, indent + 1);
				fprintf(src, "\treturn der_error(\"%s: %%u bytes left\", seqtag.length);\n", t->name);
				print_indent(src, indent + 1);
				fprintf(src, "seqtag.length -= sublen;\n");
				print_indent(src, indent);
				fprintf(src, "}\n");
				/* maybe OPTIONAL (having a DEFAULT value also means its optional) */
				if(st->optional || st->deflt != NULL)
					fprintf(src, "\t}\n");
				fprintf(src, "\n");
			}
			/* normal element */
			else
			{
				/* is it OPTIONAL (having a DEFAULT value also means its optional) */
				if(st->optional || st->deflt != NULL)
				{
					/* is there any data left */
					fprintf(src, "\tif(left > 0)\n");
					fprintf(src, "\t{\n");
					/* peek at the tag */
					fprintf(src, "\t\tsublen = der_peek_Tag(der, &tag);\n");
					fprintf(src, "\t\tif(sublen < 0)\n");
					fprintf(src, "\t\t\treturn der_error(\"%s\");\n", t->name);
					/* does it match what we want */
					fprintf(src, "\t\tif(");
					need_tag = output_tagmatch(src, st, types, 2);
					fprintf(src, ")\n");
					fprintf(src, "\t\t{\n");
					/* if the subtype decoder doesnt need the tag, skip over it */
					if(!need_tag)
					{
						fprintf(src, "\t\t\tfseek(der, sublen, SEEK_CUR);\n");
						fprintf(src, "\t\t\tleft -= sublen;\n");
					}
					/* set the have_ flag if is OPTIONAL (not DEFAULT) */
					if(st->optional)
						fprintf(src, "\t\t\ttype->have_%s = true;\n", st->name);
					/* decode the type */
					snprintf(ptr_str, sizeof(ptr_str), "&type->%s", st->name);
					snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
					output_decode_subtype(src, t->name, output_decodename(st->type), ptr_str, len_str, 3);
					fprintf(src, "\t\t\tleft -= sublen;\n");
					fprintf(src, "\t\t}\n");
					fprintf(src, "\t}\n\n");
				}
				else
				{
					/* check the tag is what we expect */
					output_decode_tag(src, t->name, "&tag", 1);
					fprintf(src, "\tif(!(");
					need_tag = output_tagmatch(src, st, types, 1);
					fprintf(src, "))\n");
					fprintf(src, "\t\treturn der_error(\"%s: unexpected tag %%u\", tag.number);\n", t->name);
					/* does the subtype decoder need the tag */
					if(need_tag)
						fprintf(src, "\tfseek(der, -sublen, SEEK_CUR);\n");
					else
						fprintf(src, "\tleft -= sublen;\n");
					/* decode the type */
					snprintf(ptr_str, sizeof(ptr_str), "&type->%s", st->name);
					snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
					output_decode_subtype(src, t->name, output_decodename(st->type), ptr_str, len_str, 1);
					fprintf(src, "\tleft -= sublen;\n\n");
				}
			}
		}
	}
	/* is it a SET */
	else if(strcmp(t->type, "xsd:all") == 0 && t->subtypes)
	{
		fprintf(src, "\t/* SET */\n");
		fprintf(src, "\twhile(left > 0)\n");
		fprintf(src, "\t{\n");
		/* get the next tag */
		output_decode_tag(src, t->name, "&tag", 2);
		fprintf(src, "\t\t");
		/* try to match it with each subtype */
		for(st=t->subtypes; st; st=st->next)
		{
			fprintf(src, "if(");
			need_tag = output_tagmatch(src, st, types, 2);
			fprintf(src, ")\n");
			fprintf(src, "\t\t{\n");
			fprintf(src, "\t\t\t/* %s", st->name);
			if(st->optional)
				fprintf(src, " OPTIONAL");
			if(st->deflt != NULL)
				fprintf(src, " DEFAULT %s", st->deflt);
			fprintf(src, " */\n");
			/* is it a SEQUENCE OF type */
			if(strcmp(st->type, "xsd:sequence") == 0 && st->subtypes && st->subtypes->next == NULL)
			{
				struct typeinfo *elem = st->subtypes;
				fprintf(src, "\t\t\t/* SEQUENCE OF %s */\n", output_typename(elem->type));
				/* get each element */
				fprintf(src, "\t\t\tint seqlen = tag.length;\n");
				fprintf(src, "\t\t\tleft -= sublen + seqlen;\n");
				fprintf(src, "\t\t\twhile(seqlen > 0)\n");
				fprintf(src, "\t\t\t{\n");
				fprintf(src, "\t\t\t\tder_tag tag;\n");
				fprintf(src, "\t\t\t\tLIST_TYPE(%s) *new_item;\n", output_typename(elem->type));
				/* check the tag is what we expect */
				output_decode_tag(src, t->name, "&tag", 4);
				fprintf(src, "\t\t\t\tif(!(");
				need_tag = output_tagmatch(src, elem, types, 4);
				fprintf(src, "))\n");
				fprintf(src, "\t\t\t\t\treturn der_error(\"%s: unexpected tag %%u\", tag.number);\n", t->name);
				/* does the subtype decoder need the tag */
				if(need_tag)
					fprintf(src, "\t\t\t\tfseek(der, -sublen, SEEK_CUR);\n");
				else
					fprintf(src, "\t\t\t\tseqlen -= sublen;\n");
				/* extend the array */
				snprintf(ptr_str, sizeof(ptr_str), "&type->%s", st->name);
				output_extend_seq(src, t->name, ptr_str, output_typename(elem->type), 4);
				/* decode the element */
				snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
				output_decode_subtype(src, t->name, output_decodename(elem->type), "&new_item->item", len_str, 4);
				fprintf(src, "\t\t\t\tif(sublen > seqlen)\n");
				fprintf(src, "\t\t\t\t\treturn der_error(\"%s: %%u bytes left\", seqlen);\n", t->name);
				fprintf(src, "\t\t\t\tseqlen -= sublen;\n");
				fprintf(src, "\t\t\t}\n");
			}
			/* normal element */
			else
			{
				/* does the subtype decoder need the tag */
				if(need_tag)
					fprintf(src, "\t\t\tfseek(der, -sublen, SEEK_CUR);\n");
				else
					fprintf(src, "\t\t\tleft -= sublen;\n");
				/* if its optional set the have_ flag */
				if(st->optional)
					fprintf(src, "\t\t\ttype->have_%s = true;\n", st->name);
				/* decode the type */
				snprintf(ptr_str, sizeof(ptr_str), "&type->%s", st->name);
				snprintf(len_str, sizeof(len_str), "%s", need_tag ? "sublen + tag.length" : "tag.length");
				output_decode_subtype(src, t->name, output_decodename(st->type), ptr_str, len_str, 3);
				fprintf(src, "\t\t\tleft -= sublen;\n");
			}
			fprintf(src, "\t\t}\n");
			/* get ready for the next possibility */
			fprintf(src, "\t\telse ");
		}
		fprintf(src, "\n\t\t\treturn der_error(\"%s: unexpected tag %%u\", tag.number);\n", t->name);
		fprintf(src, "\t}\n\n");
	}
	else
	{
		fprintf(src, "FIXME: type=%s\n\n", t->type);
	}

	/* check we used all the bytes */
	fprintf(src, "\tif(left != 0)\n");
	fprintf(src, "\t\treturn der_error(\"%s: %%d bytes left\", left);\n\n", t->name);

	fprintf(src, "#ifdef DER_VERBOSE\n");
	fprintf(src, "\tprintf(\"</%s>\\n\");\n", t->name);
	fprintf(src, "#endif\n\n");

	fprintf(src, "\treturn length;\n");
	fprintf(src, "}\n\n");

	return;
}

void
output_defaults(FILE *src, struct typeinfo *t)
{
	struct typeinfo *st;
	bool comment = false;
	char *p;

	for(st=t->subtypes; st; st=st->next)
	{
		if(st->deflt)
		{
			if(!comment)
			{
				fprintf(src, "\t/* DEFAULT values */\n");
				comment = true;
			}
			/* convert any - chars in the value to _ */
			for(p=st->deflt; *p; p++)
				*p = (*p == '-') ? '_' : *p;
			/* is it setting more than 1 value */
			if(st->deflt[0] != '{')
			{
				/* is it a named integer value */
				if(strcmp(st->type, "xsd:union") == 0 && !isdigit(st->deflt[0]))
					fprintf(src, "\ttype->%s = %s_%s;\n", st->name, st->name, st->deflt);
				else if(isdigit(st->deflt[0]) || strchr(st->type, ':') != NULL)
					fprintf(src, "\ttype->%s = %s;\n", st->name, st->deflt);
				else
					fprintf(src, "\ttype->%s = %s_%s;\n", st->name, st->type, st->deflt);
			}
			else
			{
				/* format is eg "{width 4, height 3}" */
				p = &st->deflt[1];
				while(*p && *p != '}')
				{
					while(*p && (*p == ' ' || *p == ','))
						p ++;
					fprintf(src, "\ttype->%s.", st->name);
					for(; *p && *p != ' '; p++)
						fputc(*p, src);
					fprintf(src, " =");
					for(; *p && *p != ',' && *p != '}'; p++)
						fputc(*p, src);
					fprintf(src, ";\n");
				}
			}
		}
	}

	/* did we output anything */
	if(comment)
		fprintf(src, "\n");

	return;
}

void
output_decode_tag(FILE *src, char *name, char *ptr, int indent)
{
	print_indent(src, indent);
	fprintf(src, "sublen = der_decode_Tag(der, %s);\n", ptr);
	print_indent(src, indent);
	fprintf(src, "if(sublen < 0)\n");
	print_indent(src, indent);
	fprintf(src, "\treturn der_error(\"%s\");\n", name);

	return;
}

void
output_decode_subtype(FILE *src, char *name, char *type, char *ptr, char *len, int indent)
{
	print_indent(src, indent);
	fprintf(src, "sublen = der_decode_%s(der, %s, %s);\n", type, ptr, len);
	print_indent(src, indent);
	fprintf(src, "if(sublen < 0)\n");
	print_indent(src, indent);
	fprintf(src, "\treturn der_error(\"%s\");\n", name);

	return;
}

bool
output_tagmatch(FILE *src, struct typeinfo *t, struct typeinfo *types, int indent)
{
	bool need_tag = false;

	/* do we have taginfo for this type */
	if(t->taginfo != NULL)
	{
		fprintf(src, "tag.class == %s && tag.number == %s", output_tagclass(t->taginfo->class), t->taginfo->classnumber);
	}
	else
	{
		/*
		 * special case for named integers
		 * should be an xsd:union with 2 subtypes: xsd:token with a tag; xsd:integer
		 */
		if(strcmp(t->type, "xsd:union") == 0
		&& t->subtypes != NULL
		&& strcmp(t->subtypes->type, "xsd:token") == 0
		&& t->subtypes->taginfo != NULL
		&& t->subtypes->next != NULL
		&& t->subtypes->next->next == NULL)
		{
			struct taginfo *tag = t->subtypes->taginfo;
			fprintf(src, "tag.class == %s && tag.number == %s", output_tagclass(tag->class), tag->classnumber);
		}
		/*  is it a primitive type */
		else if(strchr(t->type, ':') != NULL)
		{
			fprintf(src, "tag.class == DER_CLASS_UNIVERSAL && tag.number == %u", output_univnumber(t->type));
		}
		else
		{
			/* does the complex type have a tag */
			struct typeinfo *c = find_type(t->type, types);
			if(c->taginfo != NULL)
			{
				fprintf(src, "tag.class == %s && tag.number == %s", output_tagclass(c->taginfo->class), c->taginfo->classnumber);
			}
			else
			{
				/* have we still got a complex type */
				if(strchr(c->type, ':') == NULL)
				{
					need_tag = output_tagmatch(src, c, types, indent);
				}
				/* choice within a choice */
				else if(strcmp(c->type, "xsd:choice") == 0)
				{
					struct typeinfo *st;
					for(st=c->subtypes; st; st=st->next)
					{
						fprintf(src, "(");
						(void) output_tagmatch(src, st, types, indent);
						fprintf(src, ")");
						if(st->next)
						{
							fprintf(src, "\n");
							print_indent(src, indent);
							fprintf(src, "|| ");
						}
					}
					need_tag = true;
				}
				/* primitive type */
				else
				{
					fprintf(src, "tag.class == DER_CLASS_UNIVERSAL && tag.number == %u", output_univnumber(c->type));
				}
			}
		}
	}

	return need_tag;
}

void
output_extend_seq(FILE *src, char *name, char *seq, char *type, int indent)
{
	print_indent(src, indent);
	fprintf(src, "if((new_item = der_alloc(sizeof(LIST_TYPE(%s)))) == NULL)\n", type);
	print_indent(src, indent);
	fprintf(src, "\treturn der_error(\"%s: out of memory\");\n", name);
	print_indent(src, indent);
	fprintf(src, "LIST_APPEND(%s, new_item);\n", seq);

	return;
}

void
output_freefunc(FILE *src, struct typeinfo *t)
{
	struct typeinfo *st;

	fprintf(src, "void\n");
	fprintf(src, "free_%s(%s *type)\n", t->name, t->name);
	fprintf(src, "{\n");

	/* is it an alias for another complex type */
	if(t->subtypes == NULL
	&& (strcmp(t->type, "xsd:hexBinary") == 0 || strchr(t->type, ':') == NULL))
	{
		fprintf(src, "\tfree_%s((%s *) type);\n\n", output_typename(t->type), output_typename(t->type));
	}
	/* is it a CHOICE */
	else if(strcmp(t->type, "xsd:choice") == 0)
	{
		fprintf(src, "\tswitch(type->choice)\n");
		fprintf(src, "\t{\n");
		for(st=t->subtypes; st; st=st->next)
		{
			fprintf(src, "\tcase %s_%s:\n", t->name, st->name);
			if(strcmp(st->type, "xsd:hexBinary") == 0
			|| strchr(st->type, ':') == NULL)
				fprintf(src, "\t\tfree_%s(&type->u.%s);\n", output_typename(st->type), st->name);
			fprintf(src, "\t\tbreak;\n\n");
		}
		fprintf(src, "\tdefault:\n");
		fprintf(src, "\t\tder_error(\"free_%s: invalid choice (%%u)\", type->choice);\n", t->name);
		fprintf(src, "\t\tbreak;\n");
		fprintf(src, "\t}\n\n");
	}
	/* is it a SEQUENCE OF primitive-type */
	else if(strcmp(t->type, "xsd:list") == 0 && t->subtypes && t->subtypes->next == NULL)
	{
		/* free the list */
		fprintf(src, "\tLIST_FREE(type, %s, der_free);\n\n", output_typename(t->subtypes->type));
	}
	/* is it a SEQUENCE OF complex-type */
	else if(strcmp(t->type, "xsd:sequence") == 0 && t->subtypes && t->subtypes->next == NULL)
	{
		/* free the list and the elements */
		fprintf(src, "\tLIST_FREE_ITEMS(type, %s, free_%s, der_free);\n\n", output_typename(t->subtypes->type), output_typename(t->subtypes->type));
	}
	/* normal complex type */
	else
	{
		/* call each subtype's free function */
		for(st=t->subtypes; st; st=st->next)
		{
			/* free lists */
			if(strcmp(st->type, "xsd:sequence") == 0 && st->subtypes)
			{
				/* if its a primitive type, just free the list, for complex types free the elements too */
				if(strcmp(st->subtypes->type, "xsd:hexBinary") == 0
				|| strchr(st->subtypes->type, ':') == NULL)
					fprintf(src, "\tLIST_FREE_ITEMS(&type->%s, %s, free_%s, der_free);\n\n", st->name, output_typename(st->subtypes->type), output_typename(st->subtypes->type));
				else
					fprintf(src, "\tLIST_FREE(&type->%s, %s, der_free);\n\n", st->name, output_typename(st->subtypes->type));
			}
			/* don't need to call free for primitive types */
			else if(strcmp(st->type, "xsd:hexBinary") == 0
			     || strchr(st->type, ':') == NULL)
			{
				if(st->optional)
					fprintf(src, "\tif(type->have_%s)\n\t", st->name);
				fprintf(src, "\tfree_%s(&type->%s);\n\n", output_typename(st->type), st->name);
			}
		}
	}

	fprintf(src, "\treturn;\n");
	fprintf(src, "}\n\n");

	return;
}

char *
output_typename(char *xsd_type)
{
	if(strchr(xsd_type, ':') != NULL)
	{
		if(strcmp(xsd_type, "asn1:NULL") == 0)			return "Null";
		else if(strcmp(xsd_type, "xsd:boolean") == 0)		return "bool";
		else if(strcmp(xsd_type, "xsd:unsignedByte") == 0)	return "unsigned int";
		else if(strcmp(xsd_type, "xsd:integer") == 0)		return "int";
		else if(strcmp(xsd_type, "xsd:unsignedInt") == 0)	return "unsigned int";
		else if(strcmp(xsd_type, "xsd:hexBinary") == 0)		return "OctetString";
	}
	else
	{
		return xsd_type;
	}

	fatal("Unknown xsd type: %s", xsd_type);

	/* not reached */
	return NULL;
}

char *
output_decodename(char *xsd_type)
{
	if(strchr(xsd_type, ':') != NULL)
	{
		if(strcmp(xsd_type, "asn1:NULL") == 0)			return "Null";
		else if(strcmp(xsd_type, "xsd:boolean") == 0)		return "Boolean";
		else if(strcmp(xsd_type, "xsd:unsignedByte") == 0)	return "Integer";
		else if(strcmp(xsd_type, "xsd:integer") == 0)		return "Integer";
		else if(strcmp(xsd_type, "xsd:unsignedInt") == 0)	return "Integer";
		else if(strcmp(xsd_type, "xsd:union") == 0)		return "Integer";
		else if(strcmp(xsd_type, "xsd:hexBinary") == 0)		return "OctetString";
	}
	else
	{
		return xsd_type;
	}

	fatal("Unknown decode type: %s", xsd_type);

	/* not reached */
	return NULL;
}

char *
output_tagclass(char *class)
{
	if(strcmp(class, "UNIVERSAL") == 0)		return "DER_CLASS_UNIVERSAL";
	else if(strcmp(class, "APPL") == 0)		return "DER_CLASS_APPLICATION";
	else if(strcmp(class, "CTXT") == 0)		return "DER_CLASS_CONTEXT";
	else if(strcmp(class, "PRIV") == 0)		return "DER_CLASS_PRIVATE";

	fatal("Unknown tag class: %s", class);

	/* not reached */
	return NULL;
}

unsigned int
output_univnumber(char *xsd_type)
{
	if(strcmp(xsd_type, "xsd:boolean") == 0)		return 1;	/* BOOLEAN */
	else if(strcmp(xsd_type, "xsd:unsignedByte") == 0)	return 2;	/* INTEGER */
	else if(strcmp(xsd_type, "xsd:integer") == 0)		return 2;	/* INTEGER */
	else if(strcmp(xsd_type, "xsd:unsignedInt") == 0)	return 2;	/* INTEGER */
	else if(strcmp(xsd_type, "xsd:hexBinary") == 0)		return 4;	/* OCTET-STRING */
	else if(strcmp(xsd_type, "asn1:NULL") == 0)		return 5;	/* NULL */
	else if(strcmp(xsd_type, "xsd:token") == 0)		return 10;	/* ENUMERATED */
	else if(strcmp(xsd_type, "xsd:sequence") == 0)		return 16;	/* SEQUENCE */
	else if(strcmp(xsd_type, "xsd:list") == 0)		return 16;	/* SEQUENCE */

	fatal("Unknown UNIVERSAL type: %s", xsd_type);

	/* not reached */
	return 0;
}

void
print_type(int indent, struct typeinfo *t)
{
	struct typeinfo *st;

	print_indent(stdout, indent);

	if(t->taginfo)
		printf("[%s %s %s] ", t->taginfo->class, t->taginfo->classnumber, t->taginfo->tagtype);

	printf("%s %s", t->type, t->name);

	if(t->optional)
		printf(" OPTIONAL");
	if(t->deflt)
		printf(" DEFAULT %s", t->deflt);

	if(t->value)
		printf(" (%s)\n", t->value);
	else
		printf("\n");

	if(t->subtypes)
	{
		print_indent(stdout, indent);
		printf("{\n");
		for(st=t->subtypes; st; st=st->next)
			print_type(indent + 1, st);
		print_indent(stdout, indent);
		printf("}\n");
	}

	return;
}

void
print_indent(FILE *out, int indent)
{
	for(; indent>0; indent--)
		fprintf(out, "\t");

	return;
}

/*
 * strips off any trailing file suffix from the filename
 */

char *
mybasename(char *filename)
{
	char *p;

	if((p = strrchr(filename, '.')) != NULL)
		*p = '\0';

	return filename;
}

/*
 * safe_strdup(NULL) == NULL
 */

char *
safe_strdup(const char *src)
{
	char *dst;

	if(src == NULL)
		return NULL;

	dst = (char *) safe_malloc(strlen(src) + 1);
	strcpy(dst, src);

	return dst;
}

void *
safe_malloc(size_t nbytes)
{
	void *buf;

	if((buf = malloc(nbytes)) == NULL)
		fatal("Out of memory");

	return buf;
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
parse_info(struct userdata *ud, char *fmt, ...)
{
	va_list args;

	if(ud->verbose)
	{
		fprintf(stdout, "%s line %d: ", ud->filename, XML_GetCurrentLineNumber(ud->p));
		va_start(args, fmt);
		vfprintf(stdout, fmt, args);
		fprintf(stdout, "\n");
		va_end(args);
	}

	return;
}

void
parse_warning(struct userdata *ud, char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Warning: %s line %d: ", ud->filename, XML_GetCurrentLineNumber(ud->p));

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	return;
}

void
parse_fatal(struct userdata *ud, char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Error: %s line %d: ", ud->filename, XML_GetCurrentLineNumber(ud->p));

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	exit(EXIT_FAILURE);
}

void
usage(char *prog_name)
{
	fatal("Usage: %s [-v] [-c <output.c>] [-h <output.h>] <ASN1-types.xsd>", prog_name);
}

void
fatal(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	exit(EXIT_FAILURE);
}
