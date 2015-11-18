/*
 * dss/edpc.h
 *
 * EDP contrllor driver.
 *
 * Copyright (C) 2015 Actions Corporation
 * Author: lipeng<lipeng@actions-semi.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define DEBUG
#define pr_fmt(fmt) "owl_panel_mipi: " fmt

#define DSS_SUBSYS_NAME "PANEL_MIPI"

#include <dss.h>
#include <asm/io.h>

static int panel_mipi_power_on(struct owl_panel *panel)
{
	debug("%s\n", __func__);
	/*dm_gpio_set_value(&panel->reset_gpio, 1);*/
	mdelay(10);
	dm_gpio_set_value(&panel->power_gpio, 1);

	/*dm_gpio_set_value(&panel->reset_gpio, 0);*/
	return 0;
}

static int panel_mipi_power_off(struct owl_panel *panel)
{
	dm_gpio_set_value(&panel->power_gpio, 0);
	return 0;
}

struct owl_panel_ops owl_panel_gmp_ops = {
	.power_on = panel_mipi_power_on,
	.power_off = panel_mipi_power_off,
};

static struct owl_panel owl_panel_mipi = {
	.desc = {
		.name = "mipi_panel",
		.type = OWL_DISPLAY_TYPE_DSI,
		.ops = &owl_panel_gmp_ops,
	},
};

int owl_panel_gmp_init(const void *blob)
{
	int node;

	int ret = 0;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-gmp");
	if (node < 0) {
		debug("dsi no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	ret = owl_panel_parse_panel_info(blob, node, &owl_panel_mipi);
	if (ret < 0) {
		error("failed to parse timings\n");
		return ret;
	}

	ret = owl_panel_register(&owl_panel_mipi);
	if (ret < 0) {
		error("failed to register panel\n");
		return ret;
	}

	return 0;
}
