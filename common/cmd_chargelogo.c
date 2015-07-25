/*
 * (C) Copyright 2012 Actions Semi, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

#include <ansi.h>
#include <hush.h>
#include <malloc.h>
#include <linux/string.h>
#include <linux/input.h>

static int do_charge_animation_show(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	printf("show charge animation start!\n");
	charge_animation_display(0);
	return 0;
}

U_BOOT_CMD(charge_animation, CONFIG_SYS_MAXARGS, 0, do_charge_animation_show,
		"charge animation bmp show",
		"command [option]");

