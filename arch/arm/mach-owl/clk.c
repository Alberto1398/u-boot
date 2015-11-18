/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/periph.h>


DECLARE_GLOBAL_DATA_PTR;

static void owl_clk_set(int clk_id, int enable)
{
	unsigned long reg;
	unsigned int bit;

	reg  = CMU_DEVCLKEN0 + (clk_id / 32) * 4;
	bit = clk_id % 32;

	if (enable)
		clrsetbits_le32(reg, 1 << bit, 1 << bit);
	else
		clrsetbits_le32(reg, 1 << bit, 0);
}

void owl_clk_enable(int clk_id)
{
	owl_clk_set(clk_id, 1);
}

void owl_clk_disable(int clk_id)
{
	owl_clk_set(clk_id, 0);
}

static int owl_perip_to_clk_id(int perip_id)
{
	switch (perip_id) {
	case PERIPH_ID_UART0:
		return CLOCK_UART0;
	case PERIPH_ID_UART1:
		return CLOCK_UART1;
	case PERIPH_ID_UART2:
		return CLOCK_UART2;
	case PERIPH_ID_UART3:
		return CLOCK_UART3;
	case PERIPH_ID_UART4:
		return CLOCK_UART4;
	case PERIPH_ID_UART5:
		return CLOCK_UART5;
	case PERIPH_ID_UART6:
		return CLOCK_UART6;

	case PERIPH_ID_I2C0:
		return CLOCK_I2C0;
	case PERIPH_ID_I2C1:
		return CLOCK_I2C1;
	case PERIPH_ID_I2C2:
		return CLOCK_I2C2;
	case PERIPH_ID_I2C3:
		return CLOCK_I2C3;
#if defined(CONFIG_S900)
	case PERIPH_ID_I2C4:
		return CLOCK_I2C4;
	case PERIPH_ID_I2C5:
		return CLOCK_I2C5;
#endif

	case PERIPH_ID_PWM0:
		return CLOCK_PWM0;
	case PERIPH_ID_PWM1:
		return CLOCK_PWM1;
	case PERIPH_ID_PWM2:
		return CLOCK_PWM2;
	case PERIPH_ID_PWM3:
		return CLOCK_PWM3;
	case PERIPH_ID_PWM4:
		return CLOCK_PWM4;
	case PERIPH_ID_PWM5:
		return CLOCK_PWM5;
	case PERIPH_ID_SDMMC0:
		return CLOCK_SD0;
	case PERIPH_ID_SDMMC1:
		return CLOCK_SD1;
	case PERIPH_ID_SDMMC2:
		return CLOCK_SD2;
#if defined(CONFIG_S900)
	case PERIPH_ID_SDMMC3:
		return CLOCK_SD3;
#endif

	}
	return -1;
}
void owl_clk_enable_by_perip_id(int perip_id)
{
	int clk_id;
	clk_id = owl_perip_to_clk_id(perip_id);
	if (clk_id < 0)
		return;
	owl_clk_enable(clk_id);
}

void owl_clk_disable_by_perip_id(int perip_id)
{
	int clk_id;
	clk_id = owl_perip_to_clk_id(perip_id);
	if (clk_id < 0)
		return;
	owl_clk_disable(clk_id);
}

