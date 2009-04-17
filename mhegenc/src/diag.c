/* -*- c -*- ****************************************************************
 *
 *               Copyright 2003 Samsung Electronics (UK) Ltd.
 *
 *  System        : mhegenc
 *  Module        : mhegenc
 *
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision: 56 $
 *  Date          : $Date: 2005-04-28 03:57:10 +0100 (Thu, 28 Apr 2005) $
 *  Author        : $Author: pthomas $
 *  Created By    : Paul Thomas
 *  Create Date   : Mon Jun 9 2003
 *
 *  Description 
 *               diagnostic messages for mhegenc.
 *  Notes
 *
 *  History
 * 
 *      
 ****************************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 ****************************************************************************/
/* Diagnostics */
#include <stdio.h>
#include <assert.h>
#include "diag.h"
#include "lexer.h"

static const char *rcsid =
"@(#) $Id: diag.c 56 2005-04-28 02:57:10Z pthomas $";

static int m_arrMsgs[eSEVERITY_MAX];
static const char *filename;

/** sets the filename to be used for error message reports. */
void
diag_set_filename(const char *name)
{
    filename = name;
}

/** Reports a diagnostic message. */
void
diag_msg(Severity severity, const char *message, int lineno)
{
    char *sev[eSEVERITY_MAX] = {
        "error",
        "warning",
        "info"
    };
    assert(severity < eSEVERITY_MAX );
    assert(severity >= eSEVERITY_ERROR);
    m_arrMsgs[severity]++;
    
    fprintf(stderr, "%s:%d: %s: %s\n", filename, lineno, sev[severity],
            message);
//    fprintf(stderr, "%s\n", line_buf);
//    fprintf(stderr, "%*s\n", 1+ (token_start_offset), "^");
}

void
diag_report(const char *message)
{
    fprintf(stderr, "%s: %s\n", filename, message);
    fprintf(stderr, "%d Errors %d Warnings\n",  m_arrMsgs[eSEVERITY_ERROR],
             m_arrMsgs[eSEVERITY_WARNING]);

}

/** returns the total number of calls to error_msg with \c severity */
int
diag_count(Severity severity)
{
    assert(severity < eSEVERITY_MAX );
    assert(severity >= eSEVERITY_ERROR);
    return m_arrMsgs[severity];
}

