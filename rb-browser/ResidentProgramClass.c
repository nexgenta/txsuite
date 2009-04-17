/*
 * ResidentProgramClass.c
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "MHEGEngine.h"
#include "ResidentProgramClass.h"
#include "RootClass.h"
#include "ExternalReference.h"
#include "Parameter.h"
#include "GenericBoolean.h"
#include "GenericInteger.h"
#include "GenericOctetString.h"
#include "GenericObjectReference.h"
#include "GenericContentReference.h"
#include "VariableClass.h"
#include "BooleanVariableClass.h"
#include "rtti.h"
#include "si.h"

/* internal functions */
bool run_program(ResidentProgramClass *, LIST_OF(Parameter) *, OctetString *, bool);
bool check_parameters(LIST_OF(Parameter) *, unsigned int, ...);
Parameter *get_parameter(LIST_OF(Parameter) *, unsigned int);

/* resident programs */
bool prog_GetCurrentDate(LIST_OF(Parameter) *, OctetString *);
bool prog_FormatDate(LIST_OF(Parameter) *, OctetString *);
bool prog_GetDayOfWeek(LIST_OF(Parameter) *, OctetString *);
bool prog_Random(LIST_OF(Parameter) *, OctetString *);
bool prog_CastToContentRef(LIST_OF(Parameter) *, OctetString *);
bool prog_CastToObjectRef(LIST_OF(Parameter) *, OctetString *);
bool prog_GetStringLength(LIST_OF(Parameter) *, OctetString *);
bool prog_GetSubString(LIST_OF(Parameter) *, OctetString *);
bool prog_SearchSubString(LIST_OF(Parameter) *, OctetString *);
bool prog_SearchAndExtractSubString(LIST_OF(Parameter) *, OctetString *);
bool prog_SI_GetServiceIndex(LIST_OF(Parameter) *, OctetString *);
bool prog_SI_TuneIndex(LIST_OF(Parameter) *, OctetString *);
bool prog_SI_TuneIndexInfo(LIST_OF(Parameter) *, OctetString *);
bool prog_SI_GetBasicSI(LIST_OF(Parameter) *, OctetString *);
bool prog_GetBootInfo(LIST_OF(Parameter) *, OctetString *);
bool prog_CheckContentRef(LIST_OF(Parameter) *, OctetString *);
bool prog_CheckGroupIDRef(LIST_OF(Parameter) *, OctetString *);
bool prog_VideoToGraphics(LIST_OF(Parameter) *, OctetString *);
bool prog_SetWidescreenAlignment(LIST_OF(Parameter) *, OctetString *);
bool prog_GetDisplayAspectRatio(LIST_OF(Parameter) *, OctetString *);
bool prog_CI_SendMessage(LIST_OF(Parameter) *, OctetString *);
bool prog_SetSubtitleMode(LIST_OF(Parameter) *, OctetString *);
bool prog_WhoAmI(LIST_OF(Parameter) *, OctetString *);
bool prog_Debug(LIST_OF(Parameter) *, OctetString *);

void
ResidentProgramClass_Preparation(ResidentProgramClass *t)
{
	verbose("ResidentProgramClass: %s; Preparation", ExternalReference_name(&t->rootClass.inst.ref));

	if(!RootClass_Preparation(&t->rootClass))
		return;

	t->inst.forked = false;

	return;
}

void
ResidentProgramClass_Activation(ResidentProgramClass *t)
{
	verbose("ResidentProgramClass: %s; Activation", ExternalReference_name(&t->rootClass.inst.ref));

	/* has it already been activated */
	if(!RootClass_Activation(&t->rootClass))
		return;

	/* set RunningStatus */
	t->rootClass.inst.RunningStatus = true;

	/* generate IsRunning event */
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_running, NULL);

	return;
}

void
ResidentProgramClass_Deactivation(ResidentProgramClass *t)
{
	verbose("ResidentProgramClass: %s; Deactivation", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it running */
	if(!t->rootClass.inst.RunningStatus)
		return;

	/* stop any forked program */
	if(t->inst.forked)
	{
/******************************************************************************************/
/* TODO */
printf("TODO: ResidentProgramClass_Deactivation stop forked program\n");
/******************************************************************************************/
		t->inst.forked = false;
	}

	/* base class Deactivation */
	RootClass_Deactivation(&t->rootClass);

	return;
}

void
ResidentProgramClass_Destruction(ResidentProgramClass *t)
{
	verbose("ResidentProgramClass: %s; Destruction", ExternalReference_name(&t->rootClass.inst.ref));

	/* is it already destroyed */
	if(!t->rootClass.inst.AvailabilityStatus)
		return;

	/* Deactivate it if it is running */
	if(t->rootClass.inst.RunningStatus)
	{
		/* generates an IsStopped event */
		ResidentProgramClass_Deactivation(t);
	}

	/* generate an IsDeleted event */
	t->rootClass.inst.AvailabilityStatus = false;
	MHEGEngine_generateEvent(&t->rootClass.inst.ref, EventType_is_deleted, NULL);

	return;
}

void
ResidentProgramClass_Clone(ResidentProgramClass *t, Clone *params, OctetString *caller_gid)
{
	verbose("ResidentProgramClass: %s; Clone", ExternalReference_name(&t->rootClass.inst.ref));

/* TODO */
printf("TODO: ResidentProgramClass_Clone not yet implemented\n");
	return;
}

void
ResidentProgramClass_Call(ResidentProgramClass *p, Call *params, OctetString *caller_gid)
{
	bool rc;
	VariableClass *var;

	/* has it been prepared yet */
	if(!p->rootClass.inst.AvailabilityStatus)
		ResidentProgramClass_Preparation(p);

	/* is it already active */
	if(p->rootClass.inst.RunningStatus)
		return;

	verbose("ResidentProgramClass: %s; Call '%.*s'", ExternalReference_name(&p->rootClass.inst.ref), p->name.size, p->name.data);

	/* run it and wait for it to complete */
	ResidentProgramClass_Activation(p);
	rc = run_program(p, params->parameters, caller_gid, false);

	/* store the return value */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->call_succeeded, caller_gid)) != NULL)
	{
		if(var->rootClass.inst.rtti == RTTI_VariableClass
		&& VariableClass_type(var) == OriginalValue_boolean)
		{
			BooleanVariableClass_setBoolean(var, rc);
		}
		else
		{
			error("Call: %s is not a BooleanVariableClass", ExternalReference_name(&var->rootClass.inst.ref));
		}
	}

	/* do Deactivation */
	ResidentProgramClass_Deactivation(p);

	return;
}

void
ResidentProgramClass_Fork(ResidentProgramClass *p, Fork *params, OctetString *caller_gid)
{
	bool rc;
	VariableClass *var;

	/* has it been prepared yet */
	if(!p->rootClass.inst.AvailabilityStatus)
		ResidentProgramClass_Preparation(p);

	/* is it already active */
	if(p->rootClass.inst.RunningStatus)
		return;

	verbose("ResidentProgramClass: %s; Fork '%.*s'", ExternalReference_name(&p->rootClass.inst.ref), p->name.size, p->name.data);

	/* run it in the background */
/******************************************************************************************/
/* TODO */
printf("TODO: ResidentProgramClass_Fork not yet implemented - running in foreground\n");
	ResidentProgramClass_Activation(p);
	rc = run_program(p, params->parameters, caller_gid, true);
/* return immediately */
/******************************************************************************************/

/******************************************************************************************/
/* when the program ends */
/******************************************************************************************/
	/* store the return value */
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->fork_succeeded, caller_gid)) != NULL)
	{
		if(var->rootClass.inst.rtti == RTTI_VariableClass
		&& VariableClass_type(var) == OriginalValue_boolean)
		{
			BooleanVariableClass_setBoolean(var, rc);
		}
		else
		{
			error("Fork: %s is not a BooleanVariableClass", ExternalReference_name(&var->rootClass.inst.ref));
		}
	}

	/* do Deactivation */
	ResidentProgramClass_Deactivation(p);

	MHEGEngine_generateAsyncEvent(&p->rootClass.inst.ref, EventType_asynch_stopped, NULL);

	return;
}

/*
 * run the given program (identified by the name field in the ResidentProgramClass)
 * returns true if the program succeeds
 * sets the parameters to the values described by the UK MHEG Profile
 * caller_gid is used to resolve Generic variables in the parameters
 * if forked is true, return immediately and run the program in the background
 */

struct
{
	char *short_name;
	char *long_name;
	bool (*func)(LIST_OF(Parameter) *, OctetString *);
} resident_progs[] =
{
	{ "GCD", "GetCurrentDate",		prog_GetCurrentDate },
	{ "FDa", "FormatDate",			prog_FormatDate },
	{ "GDW", "GetDayOfWeek",		prog_GetDayOfWeek },
	{ "Rnd", "Random",			prog_Random },
	{ "CTC", "CastToContentRef",		prog_CastToContentRef },
	{ "CTO", "CastToObjectRef",		prog_CastToObjectRef },
	{ "GSL", "GetStringLength",		prog_GetStringLength },
	{ "GSS", "GetSubString",		prog_GetSubString },
	{ "SSS", "SearchSubString",		prog_SearchSubString },
	{ "SES", "SearchAndExtractSubString",	prog_SearchAndExtractSubString },
	{ "GSI", "SI_GetServiceIndex",		prog_SI_GetServiceIndex },
	{ "TIn", "SI_TuneIndex",		prog_SI_TuneIndex },
	{ "TII", "SI_TuneIndexInfo",		prog_SI_TuneIndexInfo },
	{ "BSI", "SI_GetBasicSI",		prog_SI_GetBasicSI },
	{ "GBI", "GetBootInfo",			prog_GetBootInfo },
	{ "CCR", "CheckContentRef",		prog_CheckContentRef },
	{ "CGR", "CheckGroupIDRef",		prog_CheckGroupIDRef },
	{ "VTG", "VideoToGraphics",		prog_VideoToGraphics },
	{ "SWA", "SetWidescreenAlignment",	prog_SetWidescreenAlignment },
	{ "GDA", "GetDisplayAspectRatio",	prog_GetDisplayAspectRatio },
	{ "CIS", "CI_SendMessage",		prog_CI_SendMessage },
	{ "SSM", "SetSubtitleMode",		prog_SetSubtitleMode },
	{ "WAI", "WhoAmI",			prog_WhoAmI },
	{ "DBG", "Debug",			prog_Debug },
	{ "", "", NULL }
};

bool
run_program(ResidentProgramClass *p, LIST_OF(Parameter) *params, OctetString *caller_gid, bool forked)
{
	bool rc;
	unsigned int i;

	/* remember if we were forked or not */
	p->inst.forked = forked;

	/* find it */
	for(i=0; resident_progs[i].func!=NULL; i++)
	{
		if(OctetString_strcmp(&p->name, resident_progs[i].short_name) == 0
		|| OctetString_strcmp(&p->name, resident_progs[i].long_name) == 0)
			break;
	}

	/* run it */
	if(resident_progs[i].func)
	{
		rc = (*(resident_progs[i].func))(params, caller_gid);
	}
	else
	{
		error("Unknown ResidentProgram: '%.*s'", p->name.size, p->name.data);
		rc = false;
	}

	return rc;
}

/*
 * check the list of parameters is the correct length and match the given types
 * returns true if the parameters are valid
 */

bool
check_parameters(LIST_OF(Parameter) *params, unsigned int nparams, ...)
{
	va_list ap;

	va_start(ap, nparams);

	/* check each param in the list matches the vararg types */
	while(nparams != 0)
	{
		if(params == NULL
		|| params->item.choice != va_arg(ap, unsigned int))
			return false;
		/* move on */
		params = params->next;
		nparams --;
	}

	va_end(ap);

	/* make sure no params are left */
	if(params != NULL)
		return false;

	return true;
}

/*
 * returns the given Parameter from the list
 * counting starts at 1
 */

Parameter *
get_parameter(LIST_OF(Parameter) *params, unsigned int n)
{
	while(n > 1)
	{
		params = params->next;
		n --;
	}

	return &params->item;
}

/*
 * resident programs
 */

/* number of days between 17/11/1858 (UK MPEG epoch) and 1/1/1970 (UNIX epoch) */
#define MHEG_EPOCH_OFFSET	40587

bool
prog_GetCurrentDate(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericInteger *date_par;
	GenericInteger *time_par;
	unsigned int mheg_date;
	unsigned int mheg_time;
	unsigned int unix_time;
	struct timeval now;
	struct timezone zone;

	if(!check_parameters(params, 2, Parameter_new_generic_integer,	/* out: date */
					Parameter_new_generic_integer))	/* out: time */
	{
		error("ResidentProgram: GetCurrentDate (GCD): wrong number or type of parameters");
		return false;
	}

	date_par = &(get_parameter(params, 1)->u.new_generic_integer);
	time_par = &(get_parameter(params, 2)->u.new_generic_integer);

	/* need to return local time, so take timezone into account */
	gettimeofday(&now, &zone);
	unix_time = now.tv_sec - (zone.tz_minuteswest * 60);

	/* number of days since 00:00:00 1/1/1970 */
	mheg_date = unix_time / (60 * 60 * 24);
	/* number of days since 00:00:00 17/11/1858 */
	mheg_date += MHEG_EPOCH_OFFSET;

	/* number of seconds since 00:00:00 */
	mheg_time = unix_time % (60 * 60 * 24);

	GenericInteger_setInteger(date_par, caller_gid, mheg_date);
	GenericInteger_setInteger(time_par, caller_gid, mheg_time);

	verbose("ResidentProgram: GetCurrentDate(%u, %u)", mheg_date, mheg_time);

	return true;
}

bool
prog_FormatDate(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *dateFormat_par;
	GenericInteger *date_par;
	GenericInteger *time_par;
	GenericOctetString *dateString_par;
	OctetString *dateFormat;
	unsigned int mheg_date;
	unsigned int mheg_time;
	OctetString dateString;
	unsigned int npc;
	unsigned int i;
	time_t unix_time;
	struct tm *tm;
	unsigned int year, month, hour;

	if(!check_parameters(params, 4, Parameter_new_generic_octetstring,	/* in: dateFormat */
					Parameter_new_generic_integer,		/* in: date */
					Parameter_new_generic_integer,		/* in: time */
					Parameter_new_generic_octetstring))	/* out: dateString */
	{
		error("ResidentProgram: FormatDate (FDa): wrong number or type of parameters");
		return false;
	}

	dateFormat_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	date_par = &(get_parameter(params, 2)->u.new_generic_integer);
	time_par = &(get_parameter(params, 3)->u.new_generic_integer);
	dateString_par = &(get_parameter(params, 4)->u.new_generic_octetstring);

	dateFormat = GenericOctetString_getOctetString(dateFormat_par, caller_gid);
	mheg_date = GenericInteger_getInteger(date_par, caller_gid);
	mheg_time = GenericInteger_getInteger(time_par, caller_gid);

	/*
	 * max length of the output is format length + 2x the number of % chars
	 * max is %Y = 4 output chars, but we already have 2 for the %Y in the format string
	 */
	npc = 0;
	for(i=0; i<dateFormat->size; i++)
		npc += (dateFormat->data[i] == '%') ? 1 : 0;

	/* +1 for sprintf's \0 */
	dateString.data = safe_malloc(dateFormat->size + (npc * 2) + 1);

	/* convert to UNIX time */
	unix_time = ((mheg_date - MHEG_EPOCH_OFFSET) * (24 * 60 * 60)) + mheg_time;
	/*
	 * let libc do all the hard work of working out the year etc
	 * we are passed a date/time as returned by GetCurrentDate, ie local time
	 * as GCD has already taken care of the timezone, use gmtime() not localtime() here
	 */
	tm = gmtime(&unix_time);

	/* write it out */
	dateString.size = 0;
	for(i=0; i<dateFormat->size; i++)
	{
		if(dateFormat->data[i] != '%' || i == dateFormat->size - 1)
		{
			dateString.data[dateString.size ++] = dateFormat->data[i];
		}
		else
		{
			switch(dateFormat->data[i + 1])
			{
			case 'Y':
				/* 4 digit year */
				year = tm->tm_year + 1900;
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", year);
				break;

			case 'y':
				/* 2 digit year */
				year = (tm->tm_year + 1900) % 100;
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", year);
				break;

			case 'X':
				/* month 01-12 */
				month = tm->tm_mon + 1;
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", month);
				break;

			case 'x':
				/* month 1-12 */
				month = tm->tm_mon + 1;
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", month);
				break;

			case 'D':
				/* day 01-31 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", tm->tm_mday);
				break;

			case 'd':
				/* day 1-31 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", tm->tm_mday);
				break;

			case 'H':
				/* hour 00-23 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", tm->tm_hour);
				break;

			case 'h':
				/* hour 0-23 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", tm->tm_hour);
				break;

			case 'I':
				/* hour 01-12 */
				hour = tm->tm_hour % 12;
				if(hour == 0)
					hour = 12;
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", hour);
				break;

			case 'i':
				/* hour 1-12 */
				hour = tm->tm_hour % 12;
				if(hour == 0)
					hour = 12;
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", hour);
				break;

			case 'M':
				/* minutes 00-59 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", tm->tm_min);
				break;

			case 'm':
				/* minutes 0-59 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", tm->tm_min);
				break;

			case 'S':
				/* seconds 00-59 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%02u", tm->tm_sec);
				break;

			case 's':
				/* seconds 0-59 */
				dateString.size += sprintf(&dateString.data[dateString.size], "%u", tm->tm_sec);
				break;

			case 'A':
				/* AM/PM */
				if(tm->tm_hour < 12)
					dateString.size += sprintf(&dateString.data[dateString.size], "AM");
				else
					dateString.size += sprintf(&dateString.data[dateString.size], "PM");
				break;

			case 'a':
				/* am/pm */
				if(tm->tm_hour < 12)
					dateString.size += sprintf(&dateString.data[dateString.size], "am");
				else
					dateString.size += sprintf(&dateString.data[dateString.size], "pm");
				break;

			case '%':
				dateString.data[dateString.size ++] = '%';
				break;

			default:
				error("ResidentProgram: FormatDate: unknown format character '%c'", dateFormat->data[i + 1]);
				break;
			}
			i ++;
		}
	}

	GenericOctetString_setOctetString(dateString_par, caller_gid, &dateString);

	verbose("ResidentProgram: FormatDate(\"%.*s\", %u, %u, \"%.*s\")", dateFormat->size, dateFormat->data,
									   mheg_date, mheg_time,
									   dateString.size, dateString.data);

	/* GenericOctetString_setOctetString copies it */
	safe_free(dateString.data);

	return true;
}

bool
prog_GetDayOfWeek(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericInteger *date_par;
	GenericInteger *dayOfWeek_par;
	unsigned int mheg_date;
	unsigned int dayOfWeek;

	if(!check_parameters(params, 2, Parameter_new_generic_integer,	/* in: date */
					Parameter_new_generic_integer))	/* out: dayOfWeek */
	{
		error("ResidentProgram: GetDayOfWeek (GDW): wrong number or type of parameters");
		return false;
	}

	date_par = &(get_parameter(params, 1)->u.new_generic_integer);
	dayOfWeek_par = &(get_parameter(params, 2)->u.new_generic_integer);

	mheg_date = GenericInteger_getInteger(date_par, caller_gid);

	/* Julian day number modulo 7 (+3 cuz 17/11/1858 was a Wednesday) */
	dayOfWeek = (mheg_date + 3) % 7;
	GenericInteger_setInteger(dayOfWeek_par, caller_gid, dayOfWeek);

	verbose("ResidentProgram: GetDayOfWeek(%u, %u)", mheg_date, dayOfWeek);

	return true;
}

bool
prog_Random(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericInteger *num_par;
	GenericInteger *random_par;
	unsigned int num;
	unsigned int rnd;

	if(!check_parameters(params, 2, Parameter_new_generic_integer,	/* in: num */
					Parameter_new_generic_integer))	/* out: random */
	{
		error("ResidentProgram: Random (Rnd): wrong number or type of parameters");
		return false;
	}

	num_par = &(get_parameter(params, 1)->u.new_generic_integer);
	random_par = &(get_parameter(params, 2)->u.new_generic_integer);

	num = GenericInteger_getInteger(num_par, caller_gid);

	/* ITV like to get Random(0), returned value is supposed to be in the range 1..num */
	if(num == 0)
		rnd = 1;
	else	/* man page says low order bits are random too */
		rnd = 1 + (random() % num);
	GenericInteger_setInteger(random_par, caller_gid, rnd);

	verbose("ResidentProgram: Random(%u, %u)", num, rnd);

	return true;
}

bool
prog_CastToContentRef(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *string_par;
	GenericContentReference *contentRef_par;
	OctetString *string;
	ContentReference ref;

	if(!check_parameters(params, 2, Parameter_new_generic_octetstring,		/* in: string */
					Parameter_new_generic_content_reference))	/* out: contentRef */
	{
		error("ResidentProgram: CastToContentRef (CTC): wrong number or type of parameters");
		return false;
	}

	string_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	contentRef_par = &(get_parameter(params, 2)->u.new_generic_content_reference);

	string = GenericOctetString_getOctetString(string_par, caller_gid);

	ref.size = string->size;
	ref.data = string->data;
	GenericContentReference_setContentReference(contentRef_par, caller_gid, &ref);

	verbose("ResidentProgram: CastToContentRef(\"%.*s\")", string->size, string->data);

	return true;
}

bool
prog_CastToObjectRef(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *groupId_par;
	GenericInteger *objectId_par;
	GenericObjectReference *objectRef_par;
	OctetString *groupId;
	int objectId;
	ObjectReference ref;

	if(!check_parameters(params, 3, Parameter_new_generic_octetstring,		/* in: string (gid) */
					Parameter_new_generic_integer,			/* in: objectId */
					Parameter_new_generic_object_reference))	/* out: objectRef */
	{
		error("ResidentProgram: CastToObjectRef (CTO): wrong number or type of parameters");
		return false;
	}

	groupId_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	objectId_par = &(get_parameter(params, 2)->u.new_generic_integer);
	objectRef_par = &(get_parameter(params, 3)->u.new_generic_object_reference);

	groupId = GenericOctetString_getOctetString(groupId_par, caller_gid);
	objectId = GenericInteger_getInteger(objectId_par, caller_gid);

	/* UK MHEG Profile says we can only generate external references */
	ref.choice = ObjectReference_external_reference;
	ref.u.external_reference.group_identifier.size = groupId->size;
	ref.u.external_reference.group_identifier.data = groupId->data;
	ref.u.external_reference.object_number = objectId;

	GenericObjectReference_setObjectReference(objectRef_par, caller_gid, &ref);

	verbose("ResidentProgram: CastToObjectRef(\"%.*s\", %d)", groupId->size, groupId->data, objectId);

	return true;
}

bool
prog_GetStringLength(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *string_par;
	GenericInteger *length_par;
	OctetString *string;

	if(!check_parameters(params, 2, Parameter_new_generic_octetstring,	/* in: string */
					Parameter_new_generic_integer))		/* out: length */
	{
		error("ResidentProgram: GetStringLength (GSL): wrong number or type of parameters");
		return false;
	}

	string_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	length_par = &(get_parameter(params, 2)->u.new_generic_integer);

	string = GenericOctetString_getOctetString(string_par, caller_gid);

	GenericInteger_setInteger(length_par, caller_gid, string->size);

	verbose("ResidentProgram: GetStringLength(\"%.*s\", %u)", string->size, string->data, string->size);

	return true;
}

bool
prog_GetSubString(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *string_par;
	GenericInteger *beginExtract_par;
	GenericInteger *endExtract_par;
	GenericOctetString *stringResult_par;
	OctetString *string;
	int beginExtract;
	int endExtract;
	OctetString stringResult;

	if(!check_parameters(params, 4, Parameter_new_generic_octetstring,	/* in: string */
					Parameter_new_generic_integer,		/* in: beginExtract */
					Parameter_new_generic_integer,		/* in: endExtract */
					Parameter_new_generic_octetstring))	/* out: stringResult */
	{
		error("ResidentProgram: GetSubString (GSS): wrong number or type of parameters");
		return false;
	}

	string_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	beginExtract_par = &(get_parameter(params, 2)->u.new_generic_integer);
	endExtract_par = &(get_parameter(params, 3)->u.new_generic_integer);
	stringResult_par = &(get_parameter(params, 4)->u.new_generic_octetstring);

	string = GenericOctetString_getOctetString(string_par, caller_gid);
	beginExtract = GenericInteger_getInteger(beginExtract_par, caller_gid);
	endExtract = GenericInteger_getInteger(endExtract_par, caller_gid);

	/* range checks */
	beginExtract = MAX(beginExtract, 1);
	endExtract = MIN(endExtract, string->size);

	if(beginExtract > endExtract)
	{
		stringResult.size = 0;
		stringResult.data = NULL;
	}
	else
	{
		stringResult.size = (endExtract - beginExtract) + 1;	/* inclusive */
		stringResult.data = &string->data[beginExtract - 1];
	}
	GenericOctetString_setOctetString(stringResult_par, caller_gid, &stringResult);

	verbose("ResidentProgram: GetSubString(\"%.*s\", %d, %d, \"%.*s\")", string->size, string->data,
									     beginExtract, endExtract,
									     stringResult.size, stringResult.data);

	return true;
}

/*
 * does the searching for SearchSubString and SearchAndExtractSubString
 * start counts from 1
 */

int
search_substring(OctetString *string, int start, OctetString *search)
{
	/* assert */
	if(string->size != 0
	&& (start < 1 || start > string->size))
		fatal("search_substring: start=%d string->size=%d", start, string->size);

	/* simple cases */
	if(string->size == 0)
		return -1;
	if(search->size == 0)
		return start;

	while((start - 1) + search->size <= string->size)
	{
		if(memcmp(&string->data[start-1], search->data, search->size) == 0)
			return start;
		start ++;
	}

	/* not found */
	return -1;
}

bool
prog_SearchSubString(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *string_par;
	GenericInteger *startIndex_par;
	GenericOctetString *searchString_par;
	GenericInteger *stringPosition_par;
	OctetString *string;
	int startIndex;
	OctetString *searchString;
	int stringPosition;

	if(!check_parameters(params, 4, Parameter_new_generic_octetstring,	/* in: string */
					Parameter_new_generic_integer,		/* in: startIndex */
					Parameter_new_generic_octetstring,	/* in: searchString */
					Parameter_new_generic_integer))		/* out: stringPosition */
	{
		error("ResidentProgram: SearchSubString (SSS): wrong number or type of parameters");
		return false;
	}

	string_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	startIndex_par = &(get_parameter(params, 2)->u.new_generic_integer);
	searchString_par = &(get_parameter(params, 3)->u.new_generic_octetstring);
	stringPosition_par = &(get_parameter(params, 4)->u.new_generic_integer);

	string = GenericOctetString_getOctetString(string_par, caller_gid);
	startIndex = GenericInteger_getInteger(startIndex_par, caller_gid);
	searchString = GenericOctetString_getOctetString(searchString_par, caller_gid);

	/* range checks */
	startIndex = MAX(startIndex, 1);
	startIndex = MIN(startIndex, string->size);

	stringPosition = search_substring(string, startIndex, searchString);
	GenericInteger_setInteger(stringPosition_par, caller_gid, stringPosition);

	verbose("ResidentProgram: SearchSubString(\"%.*s\", %d, \"%.*s\", %d)", string->size, string->data, startIndex,
										searchString->size, searchString->data,
										stringPosition);

	return true;
}

bool
prog_SearchAndExtractSubString(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *string_par;
	GenericInteger *startIndex_par;
	GenericOctetString *searchString_par;
	GenericOctetString *stringResult_par;
	GenericInteger *stringPosition_par;
	OctetString *string;
	int startIndex;
	OctetString *searchString;
	OctetString stringResult;
	int stringPosition;
	int search_pos;

	if(!check_parameters(params, 5, Parameter_new_generic_octetstring,	/* in: string */
					Parameter_new_generic_integer,		/* in: startIndex */
					Parameter_new_generic_octetstring,	/* in: searchString */
					Parameter_new_generic_octetstring,	/* out: stringResult */
					Parameter_new_generic_integer))		/* out: stringPosition */
	{
		error("ResidentProgram: SearchAndExtractSubString (SES): wrong number or type of parameters");
		return false;
	}

	string_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	startIndex_par = &(get_parameter(params, 2)->u.new_generic_integer);
	searchString_par = &(get_parameter(params, 3)->u.new_generic_octetstring);
	stringResult_par = &(get_parameter(params, 4)->u.new_generic_octetstring);
	stringPosition_par = &(get_parameter(params, 5)->u.new_generic_integer);

	string = GenericOctetString_getOctetString(string_par, caller_gid);
	startIndex = GenericInteger_getInteger(startIndex_par, caller_gid);
	searchString = GenericOctetString_getOctetString(searchString_par, caller_gid);

	/* range checks */
	startIndex = MAX(startIndex, 1);
	startIndex = MIN(startIndex, string->size);

	if((search_pos = search_substring(string, startIndex, searchString)) != -1)
	{
		stringResult.size = search_pos - startIndex;
		stringResult.data = &string->data[startIndex - 1];
		stringPosition = search_pos + searchString->size;
	}
	else
	{
		stringResult.size = 0;
		stringResult.data = NULL;
		stringPosition = -1;
	}

	GenericOctetString_setOctetString(stringResult_par, caller_gid, &stringResult);
	GenericInteger_setInteger(stringPosition_par, caller_gid, stringPosition);

	verbose("ResidentProgram: SearchAndExtractSubString(\"%.*s\", %d, \"%.*s\", \"%.*s\", %d)",
								string->size, string->data,
								startIndex,
								searchString->size, searchString->data,
								stringResult.size, stringResult.data,
								stringPosition);

	return true;
}

bool
prog_SI_GetServiceIndex(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *serviceReference_par;
	GenericInteger *serviceIndex_par;
	OctetString *serviceReference;
	int serviceIndex;

	if(!check_parameters(params, 2, Parameter_new_generic_octetstring,	/* in: serviceReference */
					Parameter_new_generic_integer))		/* out: serviceIndex */
	{
		error("ResidentProgram: SI_GetServiceIndex (GSI): wrong number or type of parameters");
		return false;
	}

	serviceReference_par = &(get_parameter(params, 1)->u.new_generic_octetstring);
	serviceIndex_par = &(get_parameter(params, 2)->u.new_generic_integer);

	serviceReference = GenericOctetString_getOctetString(serviceReference_par, caller_gid);
	serviceIndex = si_get_index(serviceReference);

	GenericInteger_setInteger(serviceIndex_par, caller_gid, serviceIndex);

	verbose("ResidentProgram: SI_GetServiceIndex(\"%.*s\", %d)", serviceReference->size, serviceReference->data, serviceIndex);

	return true;
}

bool
prog_SI_TuneIndex(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericInteger *serviceIndex_par;
	int serviceIndex;

	if(!check_parameters(params, 1, Parameter_new_generic_integer))	/* in: serviceIndex */
	{
		error("ResidentProgram: SI_TuneIndex (TIn): wrong number or type of parameters");
		return false;
	}

	serviceIndex_par = &(get_parameter(params, 1)->u.new_generic_integer);
	serviceIndex = GenericInteger_getInteger(serviceIndex_par, caller_gid);

	(void) si_tune_index(serviceIndex);

	return true;
}

bool
prog_SI_TuneIndexInfo(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 1, Parameter_new_generic_integer))	/* in: tuneinfo */
	{
		error("ResidentProgram: SI_TuneIndexInfo (TII): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program TII SI_TuneIndexInfo\n");
	return true;
}

bool
prog_SI_GetBasicSI(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericInteger *serviceIndex_par;
	GenericInteger *networkId_par;
	GenericInteger *origNetworkId_par;
	GenericInteger *transportStreamId_par;
	GenericInteger *serviceId_par;
	int si;
	OctetString *url;
	unsigned int network_id;
	unsigned int transport_id;
	unsigned int service_id;

	if(!check_parameters(params, 5, Parameter_new_generic_integer,	/* in: serviceIndex */
					Parameter_new_generic_integer,	/* out: networkId */
					Parameter_new_generic_integer,	/* out: origNetworkId */
					Parameter_new_generic_integer,	/* out: transportStreamId */
					Parameter_new_generic_integer))	/* out: serviceId */
	{
		error("ResidentProgram: SI_GetBasicSI (BSI): wrong number or type of parameters");
		return false;
	}

	serviceIndex_par = &(get_parameter(params, 1)->u.new_generic_integer);
	networkId_par = &(get_parameter(params, 2)->u.new_generic_integer);
	origNetworkId_par = &(get_parameter(params, 3)->u.new_generic_integer);
	transportStreamId_par = &(get_parameter(params, 4)->u.new_generic_integer);
	serviceId_par = &(get_parameter(params, 5)->u.new_generic_integer);

	si = GenericInteger_getInteger(serviceIndex_par, caller_gid);
	url = si_get_url(si);

	network_id = si_get_network_id(url);
	transport_id = si_get_transport_id(url);
	service_id = si_get_service_id(url);

	/* not sure what the difference between the Network ID and the Original Network ID is */
	GenericInteger_setInteger(networkId_par, caller_gid, network_id);
	GenericInteger_setInteger(origNetworkId_par, caller_gid, network_id);
	GenericInteger_setInteger(transportStreamId_par, caller_gid, transport_id);
	GenericInteger_setInteger(serviceId_par, caller_gid, service_id);

	verbose("ResidentProgram: SI_GetBasicSI(%d, %u, %u, %u, %u)", si, network_id, network_id, transport_id, service_id);

	return true;
}

bool
prog_GetBootInfo(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 2, Parameter_new_generic_boolean,		/* out: infoResult */
					Parameter_new_generic_octetstring))	/* out: bootInfo */
	{
		error("ResidentProgram: GetBootInfo (GBI): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program GBI GetBootInfo\n");
	return true;
}

bool
prog_CheckContentRef(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericContentReference *refToCheck_par;
	GenericBoolean *refValid_par;
	GenericContentReference *refChecked_par;
	ContentReference *ref;
	bool valid;

	if(!check_parameters(params, 3, Parameter_new_generic_content_reference,	/* in: ref-to-check */
					Parameter_new_generic_boolean,			/* out: ref-valid-var */
					Parameter_new_generic_content_reference))	/* out: ref-checked-var */
	{
		error("ResidentProgram: CheckContentRef (CCR): wrong number or type of parameters");
		return false;
	}

	refToCheck_par = &(get_parameter(params, 1)->u.new_generic_content_reference);
	refValid_par = &(get_parameter(params, 2)->u.new_generic_boolean);
	refChecked_par = &(get_parameter(params, 3)->u.new_generic_content_reference);

	ref = GenericContentReference_getContentReference(refToCheck_par, caller_gid);

	valid = MHEGEngine_checkContentRef(ref);

	/* output values */
	GenericBoolean_setBoolean(refValid_par, caller_gid, valid);
	GenericContentReference_setContentReference(refChecked_par, caller_gid, ref);

	verbose("ResidentProgram: CheckContentRef(\"%.*s\", %s, \"%.*s\")", ref->size, ref->data, valid ? "true" : "false", ref->size, ref->data);

	return true;
}

bool
prog_CheckGroupIDRef(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 3, Parameter_new_generic_object_reference,		/* in: ref-to-check */
					Parameter_new_generic_boolean,			/* out: ref-valid-var */
					Parameter_new_generic_object_reference))	/* out: ref-checked-var */
	{
		error("ResidentProgram: CheckGroupIDRef (CGR): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program CGR CheckGroupIDRef\n");
	return true;
}

bool
prog_VideoToGraphics(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 4, Parameter_new_generic_integer,	/* in: videoX */
					Parameter_new_generic_integer,	/* in: videoY */
					Parameter_new_generic_integer,	/* out: graphicsX */
					Parameter_new_generic_integer))	/* out: graphicsY */
	{
		error("ResidentProgram: VideoToGraphics (VTG): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program VTG VideoToGraphics\n");
	return true;
}

bool
prog_SetWidescreenAlignment(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 1, Parameter_new_generic_integer))	/* in: mode */
	{
		error("ResidentProgram: SetWidescreenAlignment (SWA): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program SWA SetWidescreenAlignment\n");
	return true;
}

bool
prog_GetDisplayAspectRatio(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 1, Parameter_new_generic_integer))	/* out: aspectratio */
	{
		error("ResidentProgram: GetDisplayAspectRatio (GDA): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program GDA GetDisplayAspectRatio\n");
	return true;
}

bool
prog_CI_SendMessage(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 2, Parameter_new_generic_octetstring,	/* in: message */
					Parameter_new_generic_octetstring))	/* out: response */
	{
		error("ResidentProgram: CI_SendMessage (CIS): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program CIS CI_SendMessage\n");
	return true;
}

bool
prog_SetSubtitleMode(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	if(!check_parameters(params, 1, Parameter_new_generic_boolean))	/* in: on */
	{
		error("ResidentProgram: SetSubtitleMode (SSM): wrong number or type of parameters");
		return false;
	}
/* TODO */
printf("TODO: program SSM SetSubtitleMode\n");
	return true;
}

bool
prog_WhoAmI(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	GenericOctetString *ident_par;
	OctetString ident;
	char ident_str[] = MHEG_RECEIVER_ID " " MHEG_ENGINE_ID " " MHEG_DSMCC_ID;

	if(!check_parameters(params, 1, Parameter_new_generic_octetstring))	/* out: ident */
	{
		error("ResidentProgram: WhoAmI (WAI): wrong number or type of parameters");
		return false;
	}

	ident_par = &(get_parameter(params, 1)->u.new_generic_octetstring);

	ident.size = strlen(ident_str);
	ident.data = ident_str;

	GenericOctetString_setOctetString(ident_par, caller_gid, &ident);

	verbose("ResidentProgram: WhoAmI(\"%s\")", ident_str);

	return true;
}

bool
prog_Debug(LIST_OF(Parameter) *params, OctetString *caller_gid)
{
	printf("ResidentProgram: Debug message...\n");

	/* 0 or more params */
	while(params)
	{
		Parameter_print(&params->item, caller_gid);
		params = params->next;
	}

	return true;
}

