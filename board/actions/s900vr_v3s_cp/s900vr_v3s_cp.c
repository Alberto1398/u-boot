/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>
#include "../common/board.h"

int board_init(void)
{
	int ret;

	owl_board_first_init();

	ret = owl_power_init();
	if (ret) {
		printf("%s power init failed!\n", __func__);
		return ret;
	}



#ifdef CONFIG_OWL_DMA
	owl_dma_init();
#endif
#ifdef CONFIG_STATUS_LED
	status_led_set(0, STATUS_LED_ON);
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;

	ret = owl_board_late_init();
	if (ret)
		return ret;

	return 0;
}
#endif
