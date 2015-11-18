/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
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

int owl_check_key(void)
{
	int key;

	key = atc260x_adckey_scan();
	printf("check_key: keycode=%d\n", key);

	/* if there is no any responding key pressing */
	if (-1 == key)
		return 0;

	switch (key) {
	case KEY_VOLUMEUP:
		printf("touch v+ key, enter adfu\n");
		run_command(getenv("owlboot"), 0);
		setenv("bootcmd", "owlboot");
		/*owl_reset_to_adfu();*/
		break;
	case KEY_VOLUMEDOWN:
#ifdef CONFIG_ANDROID_RECOVERY
		printf("touch v- key, enter recovery\n");
		setup_recovery_env();
#endif
		break;
	}

	return 0;
}
