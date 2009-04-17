/*
 * carousel.h
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

#ifndef __CAROUSEL_H__
#define __CAROUSEL_H__

#include <stdint.h>

#include "module.h"
#include "dsmcc.h"

/* functions */
void load_carousel(struct carousel *);

void process_dii(struct carousel *, struct DownloadInfoIndication *, uint32_t);
void process_dsi(struct carousel *, struct DownloadServerInitiate *);
void process_ddb(struct carousel *, struct DownloadDataBlock *, uint32_t, uint32_t);

#endif	/* __CAROUSEL_H__ */

