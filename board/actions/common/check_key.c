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
#ifdef CONFIG_KEY_ENTER_BURN_SN
#include <adfu.h>
#endif

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
#ifdef CONFIG_KEY_ENTER_ADFU
		printf("touch v+ key, enter adfu\n");
		owl_reset_to_adfu();
#else
		printf("touch v+ key, enter fastboot\n");
		run_command(getenv("owlboot"), 0);
		setenv("bootcmd", "owlboot");
#endif
		break;
	case KEY_VOLUMEDOWN:
#ifdef CONFIG_ANDROID_RECOVERY
		printf("touch v- key, enter recovery\n");
		setup_recovery_env();
#endif
		break;
#ifdef CONFIG_KEY_ENTER_BURN_SN
	case KEY_BACK:
		printf("touch back key, enter u-boot adfu to burn SN\n");
		unsigned long time = timer_get_us();
		printf("\ntime start: %ld\n", time);

		do_adfu();
		time = timer_get_us();
		printf("time stop: %ld\n", time);
		break;
#endif
	}

	return 0;
}
