/*
 * OWL PWM backlight support.
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Some idea is from
 * "kernel/drivers/video/backlight/pwm_bl.c"
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUGX
#define pr_fmt(fmt) "owl_pwm_bl: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>

#include <asm/io.h>
#include <asm-generic/gpio.h>

#include <asm/arch/pwm.h>
#include <asm/arch/pwm_bl.h>

struct owl_pwm_bl {
	struct owl_pwm_device	pwm;
	struct gpio_desc	power_gpio;

	int			total_steps;
	int			min_brightness;
	int			max_brightness;
	int			dft_brightness;

	int			delay_bf_pwm;
	int			delay_af_pwm;

	int			brightness;

	bool			is_avail;
};

struct owl_pwm_bl_data {
	/* read only */
	int			max_brightness;

	int			brightness;

	/* 0: power off, 1: power on */
	int			power;

	/* these guys only used by owl_pwm_bl driver */
	struct owl_pwm_device	pwm;
};

static struct owl_pwm_bl	pwm_bl;
static struct owl_pwm_bl_data	pwm_bl_data;

static int owl_pwm_bl_update_status(struct owl_pwm_bl_data *pd)
{
	int brightness;

	int total_steps = pwm_bl.total_steps;
	struct owl_pwm_device *pwm = &pwm_bl.pwm;

	if (!pwm_bl.is_avail)
		return -1;

	if (pd->brightness > pd->max_brightness)
		pd->brightness = pd->max_brightness;

	if (pd->brightness < 0)
		pd->brightness = 0;

	brightness = pd->brightness;

	if (pd->power == 0)
		brightness = 0;

	if (brightness > 0)
		brightness += pwm_bl.min_brightness;

	debug("%s: brightness %d, total_steps %d\n",
	      __func__, brightness, total_steps);

	if (brightness > total_steps)
		return -1;

	if (brightness == 0) {
		if (dm_gpio_is_valid(&pwm_bl.power_gpio))
			dm_gpio_set_value(&pwm_bl.power_gpio, 0);

		owl_pwm_config(pwm, pwm->period, pwm->period, !pwm->polarity);
		mdelay(10);
		owl_pwm_disable(pwm);
	} else {
		mdelay(pwm_bl.delay_bf_pwm);

		owl_pwm_config(pwm, brightness * pwm->period / total_steps,
			       pwm->period, pwm->polarity);
		owl_pwm_enable(pwm);

		mdelay(pwm_bl.delay_af_pwm);

		if (dm_gpio_is_valid(&pwm_bl.power_gpio))
			dm_gpio_set_value(&pwm_bl.power_gpio, 1);
	}

	return 0;
}

int owl_pwm_bl_init(const void *blob)
{
	int node;

	/* initialize */
	pwm_bl.is_avail = false;

	node = fdt_node_offset_by_compatible(blob, 0,
					     "actions,s900-pwm-backlight");
	if (node < 0) {
		node = fdt_node_offset_by_compatible(blob, 0,
						"actions,s700-pwm-backlight");
		if (node < 0) {
			debug("%s: no match in DTS\n", __func__);
			return -1;
		}
	}
	debug("%s\n", __func__);

	if (fdtdec_pwm_get(blob, node, "pwms", &pwm_bl.pwm)) {
		error("no 'pwms' subnode\n");
		return -1;
	}

	pwm_bl.total_steps = fdtdec_get_int(blob, node, "total_steps", 0);
	pwm_bl.min_brightness = fdtdec_get_int(blob, node,
					       "min_brightness", 0);
	pwm_bl.max_brightness = fdtdec_get_int(blob, node,
					       "max_brightness", 0);
	pwm_bl.dft_brightness = fdtdec_get_int(blob, node,
					       "dft_brightness", 0);
	debug("total_steps %d\n", pwm_bl.total_steps);
	debug("min_brightness %d\n", pwm_bl.min_brightness);
	debug("max_brightness %d\n", pwm_bl.max_brightness);
	debug("dft_brightness %d\n", pwm_bl.dft_brightness);

	pwm_bl.delay_bf_pwm = fdtdec_get_int(blob, node, "delay_bf_pwm", 0);
	pwm_bl.delay_af_pwm = fdtdec_get_int(blob, node, "delay_af_pwm", 0);
	debug("delay_bf_pwm %d, delay_af_pwm %d\n",
	      pwm_bl.delay_bf_pwm, pwm_bl.delay_af_pwm);

	if (pwm_bl.total_steps == 0 || pwm_bl.max_brightness == 0) {
		error("parameters error\n");
		return -1;
	}

	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "en-gpios", 0,
				       &pwm_bl.power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_gpio failed\n", __func__);

	pwm_bl.is_avail = true;

	/*
	 * update pwm backlight data
	 */
	pwm_bl_data.power = 0;
	pwm_bl_data.max_brightness = pwm_bl.max_brightness
					- pwm_bl.min_brightness;
	pwm_bl_data.brightness = pwm_bl.dft_brightness - pwm_bl.min_brightness;

	return 0;
}

int owl_pwm_bl_get_max_brightness(void)
{
	if (!pwm_bl.is_avail)
		return 0;

	return pwm_bl_data.max_brightness;
}

void owl_pwm_bl_on(void)
{
	if (pwm_bl_data.power == 1)
		return;

	pwm_bl_data.power = 1;

	owl_pwm_bl_update_status(&pwm_bl_data);
}

void owl_pwm_bl_off(void)
{
	if (pwm_bl_data.power == 0)
		return;

	pwm_bl_data.power = 0;

	owl_pwm_bl_update_status(&pwm_bl_data);
}

bool owl_pwm_bl_is_on(void)
{
	return pwm_bl_data.power == 1;
}

/*
 * owl_pwm_bl_set_brightness
 *
 * @brightness,
 *	0:	turn off.
 *	> 0:	turn on.
 *	> max_brightness: limit to max_brightness
 */
void owl_pwm_bl_set_brightness(int brightness)
{
	pwm_bl_data.brightness = brightness;
	owl_pwm_bl_update_status(&pwm_bl_data);
}

int owl_pwm_bl_get_brightness(void)
{
	return pwm_bl_data.brightness;
}
