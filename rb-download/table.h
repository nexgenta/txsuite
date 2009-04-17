/*
 * table.h
 */

/*
 * Copyright (C) 2005, Simon Kilvington
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

#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdint.h>
#include <stdbool.h>

#include "module.h"

/* max size of a DVB table */
#define MAX_TABLE_LEN   4096

bool read_pat(char *, unsigned int, unsigned char *);
bool read_pmt(char *, uint16_t, unsigned int, unsigned char *);
bool read_sdt(char *, unsigned int, unsigned char *);

bool read_table(char *, uint16_t, uint8_t, unsigned int, unsigned char *);

bool read_dsmcc_tables(struct carousel *, unsigned char *);

void add_dsmcc_pid(struct carousel *, uint16_t);

#endif	/* __TABLE_H__ */

