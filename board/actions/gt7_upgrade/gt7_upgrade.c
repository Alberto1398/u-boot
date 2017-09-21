/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clk.h>

#include <asm/arch/pwm.h>
DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	int ret;

	owl_clk_init();
	owl_pwm_init(gd->fdt_blob);
	ret = owl_pmic_init();
	if (ret) {
		printf(" PMU Not Initilize\n ");
		return ret;
	}



	return 0;
}
