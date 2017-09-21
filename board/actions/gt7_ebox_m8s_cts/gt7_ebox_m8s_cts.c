/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>
#include "../common/board.h"
#include <asm/arch/pwm.h>
#include <asm/arch/sys_proto.h>
#include <power/atc260x/atc260x_power_leds.h>
DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	int ret;

#ifdef CONFIG_BOOTDEV_AUTO
	owl_bootdev_init();
#endif
	owl_clk_init();

	owl_pwm_init(gd->fdt_blob);

	ret = owl_power_init();
	if (ret) {
		printf("%s power init failed!\n", __func__);
		return ret;
	}
	gpio_led_init();
#ifdef CONFIG_OWL_DMA
	owl_dma_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if 0
	int ret;

	ret = owl_board_late_init();
	if (ret)
		return ret;
#endif
	return 0;
}
#endif
