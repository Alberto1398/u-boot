/*
 *
 * DSI contrllor driver.
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
#define pr_fmt(fmt) "owl_amoled: " fmt

#define DSS_SUBSYS_NAME "PANEL_MIPI"

#include <dss.h>
#include <malloc.h>
#include <asm/io.h>
#include <fdtdec.h>

static uint32_t gmc_init[] = {0x00110500, 0x00290500};
#define GMC_INIT_LENS	(ARRAY_SIZE(gmc_init))

static uint32_t gmc_disable[] = {0x00280500, 0x00100500};
#define GMC_DIS_LENS	(ARRAY_SIZE(gmc_disable))

struct dsi_init_cmd {
	uint32_t cmd_nums;
	uint32_t *mipi_cmd;
};
struct panel_amoled_data {
	struct gpio_desc		power_gpio;
	struct gpio_desc		power1_gpio;
	struct gpio_desc		reset_gpio;
	/* Specific data can be added here */
	struct dsi_init_cmd	*cmd;
};

static int panel_mipi_power_on(struct owl_panel *panel)
{
	struct panel_amoled_data *amoled = panel->pdata;

	debug("%s, ... ...\n", __func__);
	/* assert  */
	if (dm_gpio_is_valid(&amoled->reset_gpio))
		dm_gpio_set_value(&amoled->reset_gpio, 1);
	mdelay(10);

	/* power on */
	if (dm_gpio_is_valid(&amoled->power1_gpio))
		dm_gpio_set_value(&amoled->power1_gpio, 1);
	if (dm_gpio_is_valid(&amoled->power_gpio))
		dm_gpio_set_value(&amoled->power_gpio, 1);
	mdelay(10);

	/* deassert */
	if (dm_gpio_is_valid(&amoled->reset_gpio))
		dm_gpio_set_value(&amoled->reset_gpio, 0);
	return 0;
}

static int panel_mipi_power_off(struct owl_panel *panel)
{
	struct panel_amoled_data *amoled = panel->pdata;

	if (dm_gpio_is_valid(&amoled->power_gpio))
		dm_gpio_set_value(&amoled->power_gpio, 0);
	return 0;
}

static int panel_mipi_enable(struct owl_panel *panel)
{
	struct panel_amoled_data *amoled = panel->pdata;
	struct owl_display_ctrl *ctrl = panel->ctrl;
	int i;
	debug("%s\n", __func__);

	/* send mipi initail command */
	if (ctrl->ops && ctrl->ops->aux_write) {
		/* send mipi initail command */
		ctrl->ops->aux_write(ctrl, (char *)amoled->cmd->mipi_cmd,
				amoled->cmd->cmd_nums);

		/* send general mipi command TODO*/
		ctrl->ops->aux_write(ctrl, (char *)&gmc_init[0], 1);
		mdelay(200);
		ctrl->ops->aux_write(ctrl, (char *)&gmc_init[1], 1);
	}
	return 0;
}

static int panel_mipi_disable(struct owl_panel *panel)
{
	struct panel_amoled_data *amoled = panel->pdata;

	return 0;
}

struct owl_panel_ops owl_panel_amoled_ops = {
	.power_on = panel_mipi_power_on,
	.power_off = panel_mipi_power_off,

	.enable = panel_mipi_enable,
	.disable = panel_mipi_disable,
};

static struct owl_panel owl_panel_mipi = {
	.desc = {
		.name = "mipi_panel",
		.type = OWL_DISPLAY_TYPE_DSI,
		.ops = &owl_panel_amoled_ops,
	},
};

static int panel_parse_info(const void *blob, int node,
	struct owl_panel *panel, struct panel_amoled_data *amoled)
{
	int cmd_numbers = 0, ret;
	uint32_t *cmd, *prop, len;
	struct owl_dss_panel_desc *desc = &panel->desc;

	debug("%s\n", __func__);
	/*
	 * parse mipi initial command
	 * */
	amoled->cmd = malloc(sizeof(struct dsi_init_cmd));

	if (amoled->cmd == NULL) {
		error("%s Error: malloc in mipi init_cmd failed!\n",
		      __func__);
		return -1;
	}

	prop = fdt_getprop(blob, node, "mipi_cmd", &len);
	debug("cmd len  %d\n", len);

	if (len > 4) {
		amoled->cmd->mipi_cmd = calloc(len, sizeof(uint32_t));
		if (!amoled->cmd->mipi_cmd) {
			error("calloc mipi_cmd failed\n");
			return 0;
		} else {
			ret = fdtdec_get_int_array(blob, node, "mipi_cmd",
				amoled->cmd->mipi_cmd, len / sizeof(uint32_t));
			if (ret < 0) {
				error("parse mipi initail command failed!\n");
				return ret;
			}
			amoled->cmd->cmd_nums = len / sizeof(uint32_t);
		}
	} else {
		amoled->cmd->mipi_cmd == NULL;
		amoled->cmd->cmd_nums = 0;
		debug("%s: No mipi initail command!\n", __func__);
	}

	/*
	 * parse mipi panel power on gpio,  It is not necessary!!!
	 *
	 * */
	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power-gpio", 0,
				       &amoled->power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power-gpio failed\n", __func__);
	/* parse power1 gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power1-gpio", 0,
				       &amoled->power1_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power1-gpio failed\n", __func__);
	/* parse reset gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "reset-gpio", 0,
				       &amoled->reset_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_reset-gpio failed\n", __func__);

	return 0;
}
int owl_panel_amoled_init(const void *blob)
{
	int node;

	int ret = 0;
	struct panel_amoled_data *amoled;
	debug("%s\n", __func__);

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-amoled");
	if (node < 0) {
		debug("dsi amoled no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	amoled = malloc(sizeof(*amoled));
	if (!amoled) {
		error("malloc amoled failed\n");
		return 0;
	}
	/*
	 * parse private panel info
	 * */
	ret = panel_parse_info(blob, node, &owl_panel_mipi, amoled);

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

	owl_panel_mipi.pdata = amoled;

	return 0;
}
