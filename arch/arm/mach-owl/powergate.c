/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/arch/powergate.h>

#define PD_MAX_CLK_ID_NUM	2
#define PD_MAX_RST_ID_NUM	2

#define POWERGATE_TIMEOUT_US	2000

struct owl_powergate_info {
	char name[16];

	unsigned long ctrl_reg;
	unsigned long ack_reg;
	int ctrl_bit;
	int ack_bit;

	int clk_id[PD_MAX_CLK_ID_NUM];
	int rst_id[PD_MAX_RST_ID_NUM];
};

#if defined(CONFIG_S900)
struct owl_powergate_info owl_powergate[POWERGATE_MAX_ID] = {
	[POWERGATE_USB3]   = {"usb3", SPS_PG_CTL, SPS_PG_ACK, 8, 8,
			      {-1, -1,}, {RESET_USB3, -1,} },
	[POWERGATE_DE]     = {"de", SPS_PG_CTL, SPS_PG_ACK, 13, 11,
			      {CLOCK_DE, -1,}, {RESET_DE, -1,} },
	[POWERGATE_NAND]   = {"nand", SPS_PG_CTL, SPS_PG_ACK, 14, 12,
			      {CLOCK_NANDC0, CLOCK_NANDC1,},
			      {RESET_NANDC0, RESET_NANDC0,} },
};
#endif
#if defined(CONFIG_S700)
struct owl_powergate_info owl_powergate[POWERGATE_MAX_ID] = {
	[POWERGATE_USB3]   = {"usb3", SPS_PG_CTL, SPS_PG_ACK, 8, 8,
			      {-1, -1,}, {RESET_USB3, -1,} },
	[POWERGATE_DE]     = {"de", SPS_PG_CTL, SPS_PG_ACK, 9, 9,
			      {CLOCK_DE, -1,}, {RESET_DE, -1,} },
	[POWERGATE_NAND]   = {"nand", SPS_PG_CTL, SPS_PG_ACK, 14, 12,
			      {CLOCK_NANDC, -1,},
			      {RESET_NANDC, -1,} },
};
#endif

static int validate_pgid(int pg_id)
{
	if (pg_id > POWERGATE_MAX_ID)
		return false;

	/* no powergate config, skip it */
	if (!owl_powergate[pg_id].name[0])
		return false;

	return true;
}

/* open/close powergate without reset & clock operation */
int owl_powergate_power_raw(int pg_id, int on)
{
	unsigned long timeout_us = POWERGATE_TIMEOUT_US;
	struct owl_powergate_info *pg;
	unsigned int val;

	if (!validate_pgid(pg_id))
		return -EINVAL;

	pg = &owl_powergate[pg_id];

	val = readl(pg->ctrl_reg);
	if (on)
		val |= 1 << pg->ctrl_bit;
	else
		val &= ~(1 << pg->ctrl_bit);
	writel(val, pg->ctrl_reg);

	/* wait for power ACK */
	while (timeout_us) {
		val = readl(pg->ack_reg) & (1 << pg->ack_bit);
		if ((on && val) || (!on && !val))
			break;

		udelay(1);
		timeout_us--;
	}

	if (!timeout_us) {
		printf("%s: ERR: powergate '%s' (id: %d), on %d timeout!\n",
			__func__, pg->name, pg_id, on);

		return -ETIMEDOUT;
	}

	return 0;
}

static int owl_powergate_power(int pg_id, int on)
{
	struct owl_powergate_info *pg;
	int i, ret;

	if (!validate_pgid(pg_id))
		return -EINVAL;

	pg = &owl_powergate[pg_id];

	/* assert reset */
	for (i = 0; i < PD_MAX_RST_ID_NUM; i++) {
		if (pg->rst_id[i] < 0)
			break;

		owl_reset_assert(pg->rst_id[i]);
	}

	/* enable clock */
	for (i = 0; i < PD_MAX_CLK_ID_NUM; i++) {
		if (pg->clk_id[i] < 0)
			break;

		owl_clk_enable(pg->clk_id[i]);
	}

	/* enable clock */
	ret = owl_powergate_power_raw(pg_id, on);

	for (i = 0; i < PD_MAX_RST_ID_NUM; i++) {
		if (pg->rst_id[i] < 0)
			break;

		owl_reset_deassert(pg->rst_id[i]);
	}

	return ret;
}

int owl_powergate_is_powered(int pg_id)
{
	struct owl_powergate_info *pg;
	unsigned int val;

	if (!validate_pgid(pg_id))
		return -EINVAL;

	pg = &owl_powergate[pg_id];

	val = readl(pg->ack_reg);
	if ((val & (1 << pg->ack_bit)))
		return 1;

	return 0;
}

int owl_powergate_power_on(int pg_id)
{
	return owl_powergate_power(pg_id, 1);
}

int owl_powergate_power_off(int pg_id)
{
	return owl_powergate_power(pg_id, 0);
}
