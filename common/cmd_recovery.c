/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
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

#include <ansi.h>
#include <hush.h>
#include <malloc.h>
#include <linux/string.h>
#include <linux/input.h>

static int do_recovery(cmd_tbl_t *cmdtp,
	int flag, int argc, char * const argv[])
{
	debug("recovery checking start!\n");
	check_recovery_mode();
	return 0;
}

U_BOOT_CMD(recovery, CONFIG_SYS_MAXARGS, 0, do_recovery,
		"recovery check utils",
		"command [option]");

static int do_recovery_show(cmd_tbl_t *cmdtp,
	int flag, int argc, char * const argv[])
{
	printf("recovery show bmp start!\n");
	recovery_image_init();
	return 0;
}

U_BOOT_CMD(recovery_show, CONFIG_SYS_MAXARGS, 0, do_recovery_show,
		"recovery bmp show",
		"command [option]");

