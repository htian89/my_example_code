/* insmod.c: insert a module into the kernel.
    Copyright (C) 2001  Rusty Russell.
    Copyright (C) 2002  Rusty Russell, IBM Corporation.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "fusb_main.h"

#define streq(a,b) (strcmp((a),(b)) == 0)

extern long init_module(void *, unsigned long, const char *);

static void *grab_file(const char *filename, unsigned long *size)
{
	unsigned int max = 16384;
	int ret, fd, err_save;
	void *buffer = malloc(max);
	if (!buffer)
		return NULL;

	if (streq(filename, "-"))
		fd = dup(STDIN_FILENO);
	else
		fd = open(filename, O_RDONLY, 0);

	if (fd < 0)
		return NULL;

	*size = 0;
	while ((ret = read(fd, buffer + *size, max - *size)) > 0) {
		*size += ret;
		if (*size == max) {
			void *p;

			p = realloc(buffer, max *= 2);
			if (!p)
				goto out_error;
			buffer = p;
		}
	}
	if (ret < 0)
		goto out_error;

	close(fd);
	return buffer;

out_error:
	err_save = errno;
	free(buffer);
	close(fd);
	errno = err_save;
	return NULL;
}

int insmod_module(char *module)
{
	long int ret;
	unsigned long len;
	void *file;
	char *filename,  *options = strdup("");

	filename = module;

	file = grab_file(filename, &len);
	if (!file) {
		LLOG_ERR("insmod: can't read '%s'\n",filename);
		return -1;
	}

	ret = init_module(file, len, options);
	if (ret != 0) {
		LLOG_ERR("insmod: error inserting '%s': %li \n",filename, ret);
		return -1;
	}
	return 0;
}
