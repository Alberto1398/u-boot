/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Lipeng<lipeng@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUGX
#define pr_fmt(fmt) "owl_pwm: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>

#include <asm/io.h>

#include <asm/arch/pwm.h>
#include <asm/arch/periph.h>
#include <asm/arch/clk.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/regs.h>

#define PWM_NUM_CHANNELS	6
#define PWM4	4
#define PWM5	5


#define NS_IN_HZ		(1000000000UL)

#define PWM_PARENT_HOSC		(0)
#define PWM_PARENT_LOSC		(1)

#ifndef DIV_ROUND
#define DIV_ROUND(x, y) (((x) + (y)-1) / (y))
#endif

#define PWM_CHIP_ID_S700	(1)
#define PWM_CHIP_ID_S900	(2)

struct owl_pwm_chip {
	int			chip_id;

	int			node;	/* device tree node */
	fdt_addr_t		base;	/* register address */
};

static struct owl_pwm_chip	chip;

static int pwm_clk_set(int hwpwm, uint32_t parent, int rate)
{
	uint32_t parent_rate;
	uint32_t div;
	uint32_t tmp;

	unsigned long cmu_pwmclk_reg;

	cmu_pwmclk_reg = CMU_PWM0CLK + hwpwm * 0x4;

	if (PWM4 == hwpwm)
		cmu_pwmclk_reg = CMU_PWM4CLK;
	if (PWM5 == hwpwm)
		cmu_pwmclk_reg = CMU_PWM5CLK;
	tmp = readl(cmu_pwmclk_reg);
	if (parent == PWM_PARENT_HOSC) {
		parent_rate = 24000000;
		tmp |= (1 << 12);
	} else {
		parent_rate = 32768;
		tmp &= (~(1 << 12));
	}

	div = DIV_ROUND(parent_rate, rate);
	div -= 1;
	tmp &= (~(0x3ff));
	tmp |= div;

	writel(tmp, cmu_pwmclk_reg);

	return 0;
}

int owl_pwm_enable(struct owl_pwm_device *pwm)
{
	owl_clk_enable_by_perip_id(pwm->periph_id);

	return 0;
}

void owl_pwm_disable(struct owl_pwm_device *pwm)
{
	owl_clk_disable_by_perip_id(pwm->periph_id);
}

int owl_pwm_config(struct owl_pwm_device *pwm, int duty_ns,
		   int period_ns, enum pwm_polarity polarity)
{
	uint32_t tmp = 0, val = 0;
	uint32_t rate;
	uint32_t parent;

	int polarity_bit;

	unsigned long pwm_ctl_reg;

	debug("request duty = %d\n", duty_ns);
	debug("request period_ns = %d\n", period_ns);

	if (period_ns > NS_IN_HZ || duty_ns > NS_IN_HZ)
		return -1;

	rate = NS_IN_HZ / period_ns;

	if (rate < 512) {
		parent = PWM_PARENT_LOSC;
	} else if (rate <= 375000) {
		parent = PWM_PARENT_HOSC;
	} else {
		rate = 375000;
		parent = PWM_PARENT_HOSC;
		debug("pwm freq will be 375kHZ at most!\n");
	}

	/*
	 * pwm clk has a pre dividor of 64
	 * so we should multi 64 first
	 */
	pwm_clk_set(pwm->hwpwm, parent, rate << 6);

	/* round up or down */
	val = DIV_ROUND(duty_ns*64, period_ns);

	pwm_ctl_reg = PWM_CTL0 + (pwm->hwpwm << 2);
	if (pwm->hwpwm == PWM4)
		pwm_ctl_reg = PWM_CTL4;
	if (pwm->hwpwm == PWM5)
		pwm_ctl_reg = PWM_CTL5;
	tmp = readl(pwm_ctl_reg);

	/*
	 * till now, val = duty * 64 / period,
	 * duty / preriod (T_active) = val / 64;
	 */

	if (chip.chip_id == PWM_CHIP_ID_S900) {
		/*
		 * S900, TODO, DIV is variable, FIXME
		 * T_active = (DUTY + 1) / DIV
		 * DIV = 64, DUTY = (val - 1)
		 * others, TODO
		 */
		val = (val > 64) ? 64 : val;
		//val = (val) ? (val - 1) : 0;

		tmp &= ~(0x1FF80000);	/* DUTY, 28:19 */
		tmp &= ~(0x7FE00);	/* DIV, 18:9 */
		tmp |= (val << 19) | (63U << 9);
	} else if (chip.chip_id == PWM_CHIP_ID_S700) {
		/*
		 * T_active = (DUTY + 1) / DIV
		 * DIV = 64, DUTY = (val - 1)
		 */
		val = (val > 64) ? 64 : val;
		//val = (val == 0) ? 0 : val - 1; /* to reg value */

		tmp &= ~((1U << 20) - 1U);
		tmp |= (val << 10) | 64U;

	} else {
		val = (val) ? (val - 1) : 0;
		tmp &= (~0x3f);
		tmp |= (val & 0x3f);
	}

	if (chip.chip_id == PWM_CHIP_ID_S900)
		polarity_bit = 8;
	else if (chip.chip_id == PWM_CHIP_ID_S700)
		polarity_bit = 20;
	else
		polarity_bit = 20;

	if (polarity == PWM_POLARITY_INVERSED) {
		debug("pwm inverse\n");
		tmp &= ~(1U << polarity_bit);
	} else {
		debug("pwm not inverse\n");
		tmp |= (1U << polarity_bit);
	}

	writel(tmp, pwm_ctl_reg);

	return 0;
}

static int fdtdec_pwm_parse(const void *blob, int node,
		const char *prop_name, struct owl_pwm_device *pwm)
{
	char pwm_subnode_name[10];
	int pwm_node;
	uint32_t data[4];

	/* data[4], for example, &pwm, 2, 50000, 0 */
	if (fdtdec_get_int_array(blob, node, prop_name, data, 4)) {
		error("Cannot decode property '%s'\n", prop_name);
		return -1;
	}

	pwm_node = fdt_node_offset_by_phandle(blob, data[0]);
	if (pwm_node != chip.node) {
		error("property '%s' phandle %d not recognised\n",
		      prop_name, data[0]);
		return -1;
	}
	debug("pwm_node = %x, chip.node = %x\n", pwm_node, chip.node);

	if (data[1] >= PWM_NUM_CHANNELS) {
		error("property '%s': invalid pwm channel %u\n",
		      prop_name, data[1]);
		return -1;
	}

	sprintf(pwm_subnode_name, "pwm%d", data[1]);
	pwm->node = fdt_subnode_offset(blob, pwm_node, pwm_subnode_name);
	if (pwm->node < 0) {
		error("cannot get pwm%d\n", data[1]);
		return -1;
	}

	pwm->hwpwm = fdtdec_get_int(blob, pwm->node, "id", 0);
	pwm->mfp_config = fdtdec_get_int(blob, pwm->node, "mfp", 0);

	pwm->periph_id = PERIPH_ID_PWM0 + pwm->hwpwm;
	pwm->period = data[2];
	pwm->polarity = ((data[3] == 0) ? PWM_POLARITY_NORMAL
					: PWM_POLARITY_INVERSED);

	debug("pwm device: node = %x\n", pwm->node);
	debug("pwm device: id = %d\n", pwm->hwpwm);
	debug("pwm device: periph_id = %d\n", pwm->periph_id);
	debug("pwm device: mfp_config = %d\n", pwm->mfp_config);
	debug("pwm device: period = %d\n", pwm->period);
	debug("pwm device: polarity = %d\n", pwm->polarity);

	return 0;
}

int fdtdec_pwm_get(const void *blob, int node,
		const char *prop_name, struct owl_pwm_device *pwm)
{
	int ret;

	debug("%s: %p, %d, %s, %d\n", __func__,
	      blob, node, prop_name, pwm->hwpwm);

	/* something like 'pwms = <&pwm 2 50000 0>;' */
	ret = fdtdec_pwm_parse(blob, node, prop_name, pwm);
	if (ret) {
		error("fdtdec_pwm_get failed\n");
		return -1;
	}

	pinmux_select(pwm->periph_id, pwm->mfp_config);

	return 0;
}

int owl_pwm_init(const void *blob)
{
	chip.node = fdt_node_offset_by_compatible(blob, 0, "actions,s900-pwm");
	if (chip.node < 0) {
		chip.node = fdt_node_offset_by_compatible(blob, 0,
							  "actions,s700-pwm");
		if (chip.node < 0) {
			error("no match in DTS\n");
			return -1;
		} else {
			chip.chip_id = PWM_CHIP_ID_S700;
		}
	} else {
		chip.chip_id = PWM_CHIP_ID_S900;
	}
	debug("%s\n", __func__);

	chip.base = fdtdec_get_addr(blob, chip.node, "reg");
	if (chip.base == FDT_ADDR_T_NONE) {
		error("Cannot find pwm reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, chip.base);

	return 0;
}
