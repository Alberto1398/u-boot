/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clk.h>

int board_init(void)
{
	int ret;

	ret = owl_pmic_init();
	if (ret) {
		printf(" PMU Not Initilize\n ");
		return ret;
	}

	owl_clk_init();

	return 0;
}
