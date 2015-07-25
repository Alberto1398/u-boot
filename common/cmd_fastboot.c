/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <errno.h>
#include <common.h>
#include <command.h>
#include <g_dnl.h>

static int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
    dcache_disable();
	ret = g_dnl_register("fboot");
	if (ret)
		return ret;

	while (1) {
		if (ctrlc())
			break;
		//printf("enter usb_gadget_handle_interrupts\n");
		usb_gadget_handle_interrupts();
		//printf("exit usb_gadget_handle_interrupts\n");
	}

	g_dnl_unregister();
    dcache_enable();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	fastboot,	1,	1,	do_fastboot,
	"fastboot - enter USB Fastboot protocol",
	""
);
