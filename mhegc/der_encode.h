/*
 * der_encode.h
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

#ifndef __DER_ENCODE_H__
#define __DER_ENCODE_H__

#include "parser.h"

#include <stdbool.h>

/*
 * max size a DER tag/length header could be
 * (this is not the theoretical max, but it is big enough for us)
 */
#define MAX_DER_HDR_LENGTH	16

void der_encode_BOOLEAN(unsigned char **, unsigned int *, bool);
void der_encode_INTEGER(unsigned char **, unsigned int *, int);
void der_encode_OctetString(unsigned char **, unsigned int *, const unsigned char *);

void convert_STRING(unsigned char **, unsigned int *, const unsigned char *);
void convert_QPRINTABLE(unsigned char **, unsigned int *, const unsigned char *);
void convert_BASE64(unsigned char **, unsigned int *, const unsigned char *);

unsigned int gen_der_header(struct node *);

void write_der_object(FILE *, struct node *);

#endif	/* __DER_ENCODE_H__ */

