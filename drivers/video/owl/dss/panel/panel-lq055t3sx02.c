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
#define DEBUGX
#define pr_fmt(fmt) "owl_panel_lq055t3sx02: " fmt

#define DSS_SUBSYS_NAME "PANEL_MIPI"

#include <dss.h>
#include <malloc.h>
#include <asm/io.h>
#include <fdtdec.h>
#include <power/regulator.h>

static uint32_t gmc_init[] = {0x00110500, 0x00290500};
#define GMC_INIT_LENS	(ARRAY_SIZE(gmc_init))

static uint32_t gmc_disable[] = {0x00280500, 0x00100500, 0x00FF0500};
#define GMC_DIS_LENS	(ARRAY_SIZE(gmc_disable))

struct dsi_init_cmd {
	uint32_t cmd_nums;
	uint32_t *mipi_cmd;
};
struct panel_lq055t3sx02_data {
	struct gpio_desc		power_gpio;
	struct gpio_desc		power1_gpio;
	struct gpio_desc		reset_gpio;
	/* Specific data can be added here */
	struct dsi_init_cmd	*cmd;
};

static int panel_mipi_power_on(struct owl_panel *panel)
{
	struct panel_lq055t3sx02_data *lq055t3sx02 = panel->pdata;

	debug("%s, ... ...\n", __func__);
	/* assert */
	if (dm_gpio_is_valid(&lq055t3sx02->reset_gpio))
		dm_gpio_set_value(&lq055t3sx02->reset_gpio, 1);
	mdelay(450);

	/* power supply VDDI */
	if (dm_gpio_is_valid(&lq055t3sx02->power_gpio))
		dm_gpio_set_value(&lq055t3sx02->power_gpio, 1);
	else {
		debug("%s, using ldo control vddio.\n", __func__);
		atc260x_reg_write(0x24, 0x6001);/* enable ldo7 and set 1.8v */
	}
	mdelay(10);

	/* deassert */
	if (dm_gpio_is_valid(&lq055t3sx02->reset_gpio))
		dm_gpio_set_value(&lq055t3sx02->reset_gpio, 0);
	mdelay(80);

	/* assert */
	if (dm_gpio_is_valid(&lq055t3sx02->reset_gpio))
		dm_gpio_set_value(&lq055t3sx02->reset_gpio, 1);
	mdelay(10);

	/* deassert */
	if (dm_gpio_is_valid(&lq055t3sx02->reset_gpio))
		dm_gpio_set_value(&lq055t3sx02->reset_gpio, 0);

	/* power supply VSP VSN */
	if (dm_gpio_is_valid(&lq055t3sx02->power1_gpio))
		dm_gpio_set_value(&lq055t3sx02->power1_gpio, 1);
	mdelay(10);

	return 0;
}

static int panel_mipi_power_off(struct owl_panel *panel)
{
	struct panel_lq055t3sx02_data *lq055t3sx02 = panel->pdata;

	if (dm_gpio_is_valid(&lq055t3sx02->power_gpio))
		dm_gpio_set_value(&lq055t3sx02->power_gpio, 0);
	return 0;
}

static int panel_mipi_enable(struct owl_panel *panel)
{
	struct panel_lq055t3sx02_data *lq055t3sx02 = panel->pdata;
	struct owl_display_ctrl *ctrl = panel->ctrl;
	int i;
	debug("%s\n", __func__);

	/* send mipi initail command */
	if (ctrl->ops && ctrl->ops->aux_write) {
		/* send mipi initail command */
		ctrl->ops->aux_write(ctrl, (char *)lq055t3sx02->cmd->mipi_cmd,
				lq055t3sx02->cmd->cmd_nums);
		/* send general mipi command TODO*/
		ctrl->ops->aux_write(ctrl, (char *)&gmc_init[0], 1);
		mdelay(200);
		ctrl->ops->aux_write(ctrl, (char *)&gmc_init[1], 1);
	}
	return 0;
}

static int panel_mipi_disable(struct owl_panel *panel)
{
	struct panel_lq055t3sx02_data *lq055t3sx02 = panel->pdata;

	return 0;
}

struct owl_panel_ops owl_panel_lq055t3sx02_ops = {
	.power_on = panel_mipi_power_on,
	.power_off = panel_mipi_power_off,

	.enable = panel_mipi_enable,
	.disable = panel_mipi_disable,
};

static struct owl_panel owl_panel_mipi = {
	.desc = {
		.name = "mipi_panel",
		.type = OWL_DISPLAY_TYPE_DSI,
		.ops = &owl_panel_lq055t3sx02_ops,
	},
};

static int panel_parse_info(const void *blob, int node,
	struct owl_panel *panel, struct panel_lq055t3sx02_data *lq055t3sx02)
{
	int cmd_numbers = 0, ret;
	uint32_t *cmd, *prop, len;
	struct owl_dss_panel_desc *desc = &panel->desc;

	debug("%s\n", __func__);
	/*
	 * parse mipi initial command
	 * */
	lq055t3sx02->cmd = malloc(sizeof(struct dsi_init_cmd));

	if (lq055t3sx02->cmd == NULL) {
		error("%s Error: malloc in mipi init_cmd failed!\n",
		      __func__);
		return -1;
	}

	prop = fdt_getprop(blob, node, "mipi_cmd", &len);
	debug("cmd len  %d\n", len);

	if (len > 4) {
		lq055t3sx02->cmd->mipi_cmd = calloc(len, sizeof(uint32_t));
		if (!lq055t3sx02->cmd->mipi_cmd) {
			error("calloc mipi_cmd failed\n");
			return 0;
		} else {
			ret = fdtdec_get_int_array(blob, node, "mipi_cmd",
				lq055t3sx02->cmd->mipi_cmd, len / sizeof(uint32_t));
			if (ret < 0) {
				error("parse mipi initail command failed!\n");
				return ret;
			}
			lq055t3sx02->cmd->cmd_nums = len / sizeof(uint32_t);
		}
	} else {
		lq055t3sx02->cmd->mipi_cmd == NULL;
		lq055t3sx02->cmd->cmd_nums = 0;
		debug("%s: No mipi initail command!\n", __func__);
	}

	/*
	 * parse mipi panel power on gpio,  It is not necessary!!!
	 *
	 * */
	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power-gpio", 0,
				       &lq055t3sx02->power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power-gpio failed\n", __func__);
	/* parse power1 gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power1-gpio", 0,
				       &lq055t3sx02->power1_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power1-gpio failed\n", __func__);
	/* parse reset gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "reset-gpio", 0,
				       &lq055t3sx02->reset_gpio, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE) < 0)
		debug("%s: fdtdec_decode_reset-gpio failed\n", __func__);

	return 0;
}
int owl_panel_lq055t3sx02_init(const void *blob)
{
	int node;

	int ret = 0;
	struct panel_lq055t3sx02_data *lq055t3sx02;
	debug("%s\n", __func__);

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-lq055t3sx02");
	if (node < 0) {
		debug("dsi lq055t3sx02 no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	lq055t3sx02 = malloc(sizeof(*lq055t3sx02));
	if (!lq055t3sx02) {
		error("malloc lq055t3sx02 failed\n");
		return 0;
	}
	/*
	 * parse private panel info
	 * */
	ret = panel_parse_info(blob, node, &owl_panel_mipi, lq055t3sx02);

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

	owl_panel_mipi.pdata = lq055t3sx02;

	return 0;
}
