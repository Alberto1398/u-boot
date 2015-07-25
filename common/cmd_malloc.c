/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>

#define MYLEN 1024
static int do_malloc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int value, count, i;
	char * const *av;
	char *buf[MYLEN];

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;

	value = simple_strtoul(*av++, NULL, 10);
	count = simple_strtoul(*av++, NULL, 10);
	printf("malloc value(%d), count(%d)\n", value, count);

	if (count > MYLEN)
		count = MYLEN;

	for (i = 0; i < count; i++) {
		buf[i] = malloc(value);
		if (buf[i]) {
			printf("%d: malloc success (%d), addr (%p)\n",
					i, value, buf[i]);
		} else {
			printf("%d: malloc faiill (%d)\n", i, value);
		}
	}

	for (i = 0; i < count; i++)
		free(buf[i]);

	return 0;
}

U_BOOT_CMD(malloc, CONFIG_SYS_MAXARGS, 0, do_malloc,
		"malloc test utils",
		"command [value] [count]");
