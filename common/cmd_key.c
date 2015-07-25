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

static int do_showkey(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int result = 88;
	int i = 10;

	while (i-- > 0) {
		ftstc(0);
		result = fgetc(0);
		printf("get key value= %d", result);
	}

	return 0;
}

U_BOOT_CMD(showkey, CONFIG_SYS_MAXARGS, 0, do_showkey,
	   "Owlxx key test",
	   "Read : command [reg] " "Write : command [reg] [val]");
