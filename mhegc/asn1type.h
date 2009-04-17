/*
 * asn1type.h
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

#ifndef __ASN1TYPE_H__
#define __ASN1TYPE_H__

enum asn1type
{
	ASN1TYPE_UNKNOWN,
	ASN1TYPE_CHOICE,
	ASN1TYPE_ENUMERATED,
	ASN1TYPE_SET,
	ASN1TYPE_SEQUENCE
};

enum asn1type asn1type(const char *);

#endif	/* __ASN1TYPE_H__ */
