/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/reset.h>
#include <asm/arch/periph.h>


static void owl_reset_set(int rst_id, int iassert)
{
	unsigned long reg;
	unsigned int bit;

	reg  = CMU_DEVRST0 + (rst_id / 32) * 4;
	bit = rst_id % 32;

	if (iassert)
		clrsetbits_le32(reg, 1 << bit, 0);
	else
		clrsetbits_le32(reg, 1 << bit, 1 << bit);
}

void owl_reset_assert(int rst_id)
{
	owl_reset_set(rst_id, 1);
}

void owl_reset_deassert(int rst_id)
{
	owl_reset_set(rst_id, 0);
}

void owl_reset(int rst_id)
{
	owl_reset_assert(rst_id);
	udelay(1);
	owl_reset_deassert(rst_id);
}

static int owl_perip_to_reset_id(int perip_id)
{
	switch (perip_id) {
	case PERIPH_ID_UART0:
		return RESET_UART0;
	case PERIPH_ID_UART1:
		return RESET_UART1;
	case PERIPH_ID_UART2:
		return RESET_UART2;
	case PERIPH_ID_UART3:
		return RESET_UART3;
	case PERIPH_ID_UART4:
		return RESET_UART4;
	case PERIPH_ID_UART5:
		return RESET_UART5;
	case PERIPH_ID_UART6:
		return RESET_UART6;

	case PERIPH_ID_I2C0:
		return RESET_I2C0;
	case PERIPH_ID_I2C1:
		return RESET_I2C1;
	case PERIPH_ID_I2C2:
		return RESET_I2C2;
	case PERIPH_ID_I2C3:
		return RESET_I2C3;
#if defined(CONFIG_S900)
	case PERIPH_ID_I2C4:
		return RESET_I2C4;
	case PERIPH_ID_I2C5:
		return RESET_I2C5;
#endif


	case PERIPH_ID_SDMMC0:
		return RESET_SD0;
	case PERIPH_ID_SDMMC1:
		return RESET_SD1;
	case PERIPH_ID_SDMMC2:
		return RESET_SD2;
#if defined(CONFIG_S900)
	case PERIPH_ID_SDMMC3:
		return RESET_SD3;
#endif
	}
	return -1;
}

void owl_reset_assert_by_perip_id(int perip_id)
{
	int rst_id;
	rst_id = owl_perip_to_reset_id(perip_id);
	if (rst_id < 0)
		return;
	owl_reset_assert(rst_id);
}
void owl_reset_deassert_by_perip_id(int perip_id)
{
	int rst_id;
	rst_id = owl_perip_to_reset_id(perip_id);
	if (rst_id < 0)
		return;
	owl_reset_deassert(rst_id);
}
void owl_reset_by_perip_id(int perip_id)
{
	 owl_reset_assert_by_perip_id(perip_id);
	 udelay(1);
	 owl_reset_deassert_by_perip_id(perip_id);
}

