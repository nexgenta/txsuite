/*
 * channels.h
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

#ifndef __CHANNELS_H__
#define __CHANNELS_H__

#include <stdint.h>
#include <stdbool.h>

char *zap_name(unsigned int);

bool init_channels_conf(char *, char *);

bool tune_service_id(unsigned int, unsigned int, uint16_t);

bool service_available(uint16_t);

#endif	/* __CHANNELS_H__ */

