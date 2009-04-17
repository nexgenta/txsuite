/*
 * cache.c
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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cache.h"
#include "table.h"
#include "utils.h"

/*
 * can't just cache tables in memory because the PMTs are mostly read when we
 * execute "avstream <service_id> ..." commands
 * each command is run in a child process, so the memory is lost when the
 * process ends
 * so we cache the tables in the file system
 * all cache items are MAX_TABLE_LEN bytes long
 */

#define CACHE_DIR	"cache"

bool
cache_init(void)
{
	if(mkdir(CACHE_DIR, 0755) < 0 && errno != EEXIST)
		fatal("Unable to create cache directory '%s': %s", CACHE_DIR, strerror(errno));

	cache_flush();

	return true;
}

/*
 * returns false if the item is not in the cache
 */

bool
cache_load(char *item, unsigned char *out)
{
	char filename[PATH_MAX];
	FILE *f;
	size_t nread;

	snprintf(filename, sizeof(filename), "%s/%s", CACHE_DIR, item);

	if((f = fopen(filename, "r")) == NULL)
		return false;

	nread = fread(out, 1, MAX_TABLE_LEN, f);

	fclose(f);

	return (nread == MAX_TABLE_LEN);
}

void
cache_save(char *item, unsigned char *data)
{
	char filename[PATH_MAX];
	FILE *f;

	snprintf(filename, sizeof(filename), "%s/%s", CACHE_DIR, item);

	if((f = fopen(filename, "w")) == NULL)
		return;

	/* if we don't write it all, we'll find out when we try to load it */
	fwrite(data, 1, MAX_TABLE_LEN, f);

	fclose(f);

	return;
}

void
cache_flush(void)
{
	DIR *d;
	struct dirent *item;
	char filename[PATH_MAX];

	if((d = opendir(CACHE_DIR)) == NULL)
		return;

	while((item = readdir(d)) != NULL)
	{
		/* skip . and .. */
		if(strcmp(item->d_name, ".") == 0
		|| strcmp(item->d_name, "..") == 0)
			continue;
		snprintf(filename, sizeof(filename), "%s/%s", CACHE_DIR, item->d_name);
		unlink(filename);
	}

	closedir(d);

	return;
}


