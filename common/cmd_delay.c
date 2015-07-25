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
#include <asm/io.h>

static int do_delay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int value, count, i;
	char * const *av;
	static struct owlxx_timer *t =
		(struct owlxx_timer *) TIMER_2HZ_BASE;

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;

	value = simple_strtoul(*av++, NULL, 10);
	count = simple_strtoul(*av++, NULL, 10);

	printf("delay value(%d), count(%d)\n", value, count);
	for (i = 0; i < count; i++) {
		printf("%d: time(0x%x), tick(%d)\n", i, readl(&t->t0_val),
				get_ticks());
		mdelay(value);
	}

	return 0;
}

U_BOOT_CMD(delay, CONFIG_SYS_MAXARGS, 0, do_delay,
		"delay test utils",
		"command [time] [counter]");
