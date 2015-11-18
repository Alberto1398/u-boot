/*
 * Copyright (c) 2015 Actions Semi Co.,	Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <fs.h>
#include <linux/input.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_adckey.h>
#include <asm/arch/sys_proto.h>
#include "board.h"

int	owl_check_enter_shell(void)
{
	int	flag = 0;

	atc260x_pstore_get(ATC260X_PSTORE_TAG_BOOTLOADER, &flag);
	if (flag) {
		printf("enter shell");
		setenv("bootcmd", "help");
		atc260x_pstore_set(ATC260X_PSTORE_TAG_BOOTLOADER, 0);
	}

	return 0;
}
