/*
 * OWL GLP(generic LCD/LVDS Panel)
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/11/30: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define pr_fmt(fmt) "owl_panel_glp: " fmt

#include <asm/io.h>
#include <malloc.h>

#include <dss.h>

struct panel_glp_data {
	struct gpio_desc		power_gpio;
	struct gpio_desc		reset_gpio;
	/* Specific data can be added here */
};

static int panel_glp_power_on(struct owl_panel *panel)
{
	struct panel_glp_data *glp = panel->pdata;
	if (dm_gpio_is_valid(&glp->power_gpio))
		dm_gpio_set_value(&glp->power_gpio, 1);

	return 0;
}

static int panel_glp_power_off(struct owl_panel *panel)
{
	struct panel_glp_data *glp = panel->pdata;
	if (dm_gpio_is_valid(&glp->power_gpio))
		dm_gpio_set_value(&glp->power_gpio, 0);

	return 0;
}

struct owl_panel_ops owl_panel_glp_ops = {
	.power_on = panel_glp_power_on,
	.power_off = panel_glp_power_off,
};

static struct owl_panel owl_panel_glp = {
	.desc = {
		.name = "panel_glp",
		.type = OWL_DISPLAY_TYPE_LCD,
		.ops = &owl_panel_glp_ops,
	},
};

int owl_panel_glp_init(const void *blob)
{
	int node;

	int ret = 0;
	struct panel_glp_data *glp;

	/* 
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-glp");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	glp = malloc(sizeof(*glp));
	if (!glp) {
		error("malloc glp failed\n");
		return 0;
	}

	ret = owl_panel_parse_panel_info(blob, node, &owl_panel_glp);
	if (ret < 0) {
		error("failed to parse timings\n");
		return ret;
	}

	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power-gpio", 0,
				       &glp->power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power-gpio failed\n", __func__);

	ret = owl_panel_register(&owl_panel_glp);
	if (ret < 0) {
		error("failed to register panel\n");
		return ret;
	}

	owl_panel_glp.pdata = glp;

	return 0;
}
