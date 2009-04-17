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
 *               lexer interface for mhegenc.
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
/* Interface to lexer. */
#include <stdio.h>

/* Exported by the lexer. */
/* ---------------------- */
extern FILE *yyin;              /* input file handle */


#define LINE_BUFFER_LENGTH 500  /* max line length for saving. */
extern char line_buf[];         /* saved current line. */
/* offsets into line_buf: */
extern int token_start_offset;  /* start of current token .*/
extern int token_end_offset;    /* end of current token. */
extern int yylineno;            /* current input line number. */

extern int yylex(void);         /* The scanner. */
