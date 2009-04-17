/* -*- c -*- ****************************************************************
 *
 *               Copyright 2003 Samsung Electronics (UK) Ltd.
 * 
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
 *               diagnostics definitions for mhegenc.
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

typedef enum _ESeverity{
    eSEVERITY_ERROR,
    eSEVERITY_WARNING,
    eSEVERITY_INFO,
    eSEVERITY_MAX
}Severity;

/** sets the filename to be used for error message reports. */
extern void
diag_set_filename(const char *name);

/** Reports a diagnostic message. */
extern void
diag_msg(Severity severity, const char *message, int lineno);

/** Reports exit status. */
extern void
diag_report(const char *message);

/** returns the total number of calls to error_msg with \c severity */
extern int
diag_count(Severity severity);

#define warn_range(field, line)
