/*
 * OWL GEP(generic EDP Panel)
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUG
#define pr_fmt(fmt) "owl_panel_gep: " fmt

#include <asm/io.h>
#include <malloc.h>
#include <dss.h>
struct panel_gep_data {
	struct gpio_desc		power_gpio;
	struct gpio_desc		reset_gpio;
	/* Specific data can be added here */
};


static int panel_gep_power_on(struct owl_panel *panel)
{
	struct panel_gep_data *gep = panel->pdata;
	if (dm_gpio_is_valid(&gep->power_gpio))
		dm_gpio_set_value(&gep->power_gpio, 1);

	return 0;
}

static int panel_gep_power_off(struct owl_panel *panel)
{
	struct panel_gep_data *gep = panel->pdata;
	if (dm_gpio_is_valid(&gep->power_gpio))
		dm_gpio_set_value(&gep->power_gpio, 0);

	return 0;
}

struct owl_panel_ops owl_panel_gep_ops = {
	.power_on = panel_gep_power_on,
	.power_off = panel_gep_power_off,
};

static struct owl_panel owl_panel_gep = {
	.desc = {
		.name = "panel_gep",
		.type = OWL_DISPLAY_TYPE_EDP,
		.ops = &owl_panel_gep_ops,
	},
};

int owl_panel_gep_init(const void *blob)
{
	int node;

	int ret = 0;
	struct panel_gep_data *gep;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-gep");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	gep = malloc(sizeof(*gep));
	if (!gep) {
		error("malloc gep failed\n");
		return 0;
	}

	ret = owl_panel_parse_panel_info(blob, node, &owl_panel_gep);
	if (ret < 0) {
		error("failed to parse timings\n");
		return ret;
	}

	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power-gpio", 0,
				       &gep->power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power-gpio failed\n", __func__);

	ret = owl_panel_register(&owl_panel_gep);
	if (ret < 0) {
		error("failed to register panel\n");
		return ret;
	}

	owl_panel_gep.pdata = gep;

	return 0;
}
