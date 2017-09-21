/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>
#include <asm/gpio.h>
#include <asm/arch/regs.h>
#include <asm/io.h>
#include "../common/board.h"
#include <asm/arch/pwm.h>
DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	int ret;

	owl_board_first_init();

	clrsetbits_le32(MFP_CTL1, (0x3 << 21), (0x2 << 21) ); //LVDS_O_PN, 切换到数字PIN，相应的GPIO 才能使用。
	clrsetbits_le32(MFP_CTL1, (0x3 << 5), (0x2 << 5) ); //LVDS_EE_PN, 切换到数字PIN，相应的GPIO 才能使用。

	owl_pwm_init(gd->fdt_blob);

	ret = owl_power_init();
	if (ret) {
		printf("%s power init failed!\n", __func__);
		return ret;
	}

#ifdef CONFIG_OWL_DMA
	owl_dma_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if 1
	int ret;

	ret = owl_board_late_init();
	if (ret)
		return ret;
#endif
	return 0;
}
#endif
