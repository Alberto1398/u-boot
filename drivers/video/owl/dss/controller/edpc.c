/*
 * OWL eDP(Embedded Display Port) controller
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
#define pr_fmt(fmt) "owl_edpc: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>

#include <asm/io.h>

#include <asm/arch/regs_s900.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>

#include <dss.h>
#include "edpc.h"

struct edpc_config {
	uint32_t		link_rate;

	uint32_t		lane_count;
	uint32_t		lane_polarity;
	uint32_t		lane_mirror;

	uint32_t		mstream_polarity;
	uint32_t		user_sync_polarity;

	uint32_t		pclk_parent;
	uint32_t		pclk_rate;
};

struct edpc_data {
	fdt_addr_t		base;	/* register address */

	struct owl_display_ctrl	*ctrl;
	struct edpc_config	configs;
};

static struct edpc_data		owl_edpc_data;

#define edpc_writel(edpc, index, val) writel((val), edpc->base + (index))
#define edpc_readl(edpc, index) readl(edpc->base + (index))

static int edpc_parse_config(const void *blob, int node,
			     struct edpc_data *edpc)
{
	int entry;

	debug("%s:\n", __func__);

	/*
	 * parse video configs ...
	 */
	entry = fdtdec_lookup_phandle(blob, node, "panel_configs");
	if (entry < 0) {
		error("no etry for 'panel_configs'\n");
		return -1;
	}
	debug("entry = 0x%d\n", entry);

	edpc->configs.link_rate
		= fdtdec_get_int(blob, entry, "link_rate", 0);

	edpc->configs.lane_count
		= fdtdec_get_int(blob, entry, "lane_count", 0);
	edpc->configs.lane_polarity
		= fdtdec_get_int(blob, entry, "lane_polarity", 0);
	edpc->configs.lane_mirror
		= fdtdec_get_int(blob, entry, "lane_mirror", 0);

	edpc->configs.mstream_polarity
		= fdtdec_get_int(blob, entry, "mstream_polarity", 0);
	edpc->configs.user_sync_polarity
		= fdtdec_get_int(blob, entry, "user_sync_polarity", 0);

	edpc->configs.pclk_parent
		= fdtdec_get_int(blob, entry, "pclk_parent", 0);
	edpc->configs.pclk_rate
		= fdtdec_get_int(blob, entry, "pclk_rate", 0);
	if (edpc->configs.pclk_rate == 0) {
		error("pclk_rate is 0!!\n");
		return -1;
	}

	debug("%s:\n", __func__);
	debug("link_rate %d\n", edpc->configs.link_rate);
	debug("lane_count %d, lane_polarity %x, lane_mirror = %d\n",
	      edpc->configs.link_rate, edpc->configs.lane_count,
	      edpc->configs.lane_mirror);
	debug("mstream_polarity %x, user_sync_polarity = %x\n",
	      edpc->configs.mstream_polarity,
	      edpc->configs.user_sync_polarity);
	debug("pclk_parent %d pclk_rate %d\n",
	      edpc->configs.pclk_parent, edpc->configs.pclk_rate);

	return 0;
}

/*
 * edpclk divider table,
 * index is register value(0~7),
 * value is divider multiplied by 2(2/3/4/5/6/8/12/16)
 */
#define DIVIDER_TABLE_LEN	(8)
static unsigned int edpclk_divider_table[DIVIDER_TABLE_LEN] = {
	2, 3, 4, 5, 6, 8, 12, 16,
};

static unsigned int edpclk_get_divider(unsigned int parent_rate,
				       unsigned int target_rate)
{
	int i;
	unsigned int rate;

	for (i = 0; i < DIVIDER_TABLE_LEN; i++) {
		/* round */
		rate = (parent_rate * 2 + (edpclk_divider_table[i] >> 1))
			/ edpclk_divider_table[i];
		if (rate == target_rate)
			break;
	}
	BUG_ON(i == DIVIDER_TABLE_LEN);

	debug("%s: divider = %d, rate = %d\n",
	      __func__, i, parent_rate * 2 / edpclk_divider_table[i]);
	return i;
}

static void edpc_clk_enable(struct edpc_data *edpc)
{
	unsigned int tmp;

	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_EDP);

	/* enable edpclk from devpll */
	owl_clk_enable(CLOCK_EDP);

	/* Enable the ASSISTPLL, 500MHZ, TODO */
	setbits_le32(CMU_ASSISTPLL, 0x1 << 0);

	tmp = 0;
	/*
	 * 0, 1.62Gbps--810M
	 * 1, 2.7Gbps--1350M
	 * 2, 5.4Gbps--2700M
	 */
	tmp |= edpc->configs.link_rate;

	tmp |= (1 << 8);	/* 24M Diff Clock enable */
	tmp |= (1 << 9);	/* DP PLL enable */

	if (edpc->configs.pclk_parent == 0) {	/* ASSIST_PLL */
		tmp |= (0 << 19);
		tmp |= (edpclk_get_divider(500000000, edpc->configs.pclk_rate)
			<< 16);
	} else {
		tmp |= (1 << 19);		/* DISPLAY_PLL */
		tmp |= (edpclk_get_divider(1056000000, edpc->configs.pclk_rate)
			<< 16);
	}

	writel(tmp, CMU_EDPCLK);

	owl_reset_deassert(RESET_EDP);

	mdelay(1);
}

static void edpc_clk_disable(struct edpc_data *edpc)
{
	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_EDP);

	owl_clk_disable(CLOCK_EDP);

	writel(0, CMU_EDPCLK);
}

static void edpc_set_size(struct edpc_data *edpc,
			  uint16_t width, uint16_t height)
{
	BUG_ON((width > (1 << 14)) || (height > (1 << 14)));

	clrsetbits_le32(edpc->base + EDP_MSTREAM_HRES, 0x7FFF << 0,
			width << 0);
	clrsetbits_le32(edpc->base + EDP_MSTREAM_VRES, 0x7FFF << 0,
			height << 0);
}

static void edpc_set_mode(struct edpc_data *edpc,
			     uint16_t width, uint16_t height,
			     uint16_t hbp, uint16_t hfp,
			     uint16_t hsw, uint16_t vbp,
			     uint16_t vfp, uint16_t vsw)
{
	BUG_ON((hbp > (1 << 15)) || (hfp > (1 << 15)) || (hsw > (1 << 15)));

	BUG_ON((vbp > (1 << 15)) || (vfp > (1 << 15)) || (vsw > (1 << 15)));

	edpc_writel(edpc, EDP_MSTREAM_HSWIDTH, hsw);
	edpc_writel(edpc, EDP_MSTREAM_HSTART, hbp + hsw);
	edpc_writel(edpc, EDP_MSTREAM_HTOTAL, hsw + hbp + width + hfp);

	edpc_writel(edpc, EDP_MSTREAM_VSWIDTH, vsw);
	edpc_writel(edpc, EDP_MSTREAM_VSTART, vbp + vsw);
	edpc_writel(edpc, EDP_MSTREAM_VTOTAL, vsw + vbp + height + vfp);

	/* others, TODO */
	edpc_writel(edpc, EDP_MSTREAM_POLARITY,
		    edpc->configs.mstream_polarity);

	/* 64byte each micro packet */
	edpc_writel(edpc, EDP_MTRANSFER_UNIT, 64);

	edpc_writel(edpc, EDP_N_VID, 0x8000);
	edpc_writel(edpc, EDP_USER_DATA_COUNT, (width * 18 / 16 - 1));
	edpc_writel(edpc, EDP_USER_SYNC_POLARITY,
		    edpc->configs.user_sync_polarity);
}

static void edpc_set_default_color(struct edpc_data *edpc, uint32_t color)
{
	edpc_writel(edpc, EDP_RGB_COLOR, color);
}

static void edpc_set_single_format(struct edpc_data *edpc, uint8_t format)
{
	clrsetbits_le32(edpc->base + EDP_MSTREAM_MISC0, 0x3 << 1, format << 1);
}

static void edpc_set_preline(struct edpc_data *edpc)
{
	int preline;

	preline = PANEL_PRELINES(edpc->ctrl->panel);
	preline = (preline < 0 ? 0 : preline);
	preline = (preline > 31 ? 31 : preline);

	clrsetbits_le32(edpc->base + EDP_RGB_CTL, 0x1F << 4, preline << 4);
}

static void edpc_set_single_from(struct edpc_data *edpc, uint8_t single)
{
	clrsetbits_le32(edpc->base + EDP_RGB_CTL, 0x1 << 1, single << 1);
}

static void edpc_phy_config(struct edpc_data *edpc)
{
	edpc_writel(edpc, EDP_PHY_PREEM_L0, 0);
	edpc_writel(edpc, EDP_PHY_PREEM_L1, 0);
	edpc_writel(edpc, EDP_PHY_PREEM_L2, 0);
	edpc_writel(edpc, EDP_PHY_PREEM_L3, 0);

	edpc_writel(edpc, EDP_PHY_VSW_L0, 0);
	edpc_writel(edpc, EDP_PHY_VSW_L1, 0);
	edpc_writel(edpc, EDP_PHY_VSW_L2, 0);
	edpc_writel(edpc, EDP_PHY_VSW_L3, 0);

	/* set the aux channel voltage swing 400mV */
	edpc_writel(edpc, EDP_PHY_VSW_AUX, 0);

	/* set no Mirror,polarity not changed */
	edpc_writel(edpc, EDP_PHY_CTRL, 0);
}

static void edpc_tx_init(struct edpc_data *edpc)
{
	edpc_writel(edpc, EDP_CORE_TX_EN, 0x00);
	edpc_writel(edpc, EDP_PHY_PWR_DOWN, 0x00);
	edpc_writel(edpc, EDP_PHY_RESET, 0x00);
	mdelay(1);	/* wait for complete */
}

static void edpc_link_config(struct edpc_data *edpc)
{
	/* set aux clock 1MHz */
	edpc_writel(edpc, EDP_AUX_CLK_DIV, 24);

	edpc_writel(edpc, EDP_LNK_LANE_COUNT, edpc->configs.lane_count);

	/* enable enhanced frame */
	edpc_writel(edpc, EDP_LNK_ENHANCED, 0x01);

	/* eDP enable, use only for embedded application */
	edpc_writel(edpc, EDP_LNK_SCR_RST, 0x00);

	/* enable internal scramble */
	edpc_writel(edpc, EDP_LNK_SCR_CTRL, 0x00);
}

static void edpc_display_init_edp(struct edpc_data *edpc)
{
	struct owl_videomode *mode = &edpc->ctrl->panel->current_mode;

	edpc_set_size(edpc, mode->xres, mode->yres);
	edpc_set_mode(edpc, mode->xres, mode->yres, mode->hbp, mode->hfp,
		      mode->hsw, mode->vbp, mode->vfp, mode->vsw);

	edpc_set_default_color(edpc, 0xff);
	edpc_set_single_format(edpc, 0);

	edpc_set_preline(edpc);
	edpc_set_single_from(edpc, 0);	/* 0 DE, 1 default color */
	edpc_phy_config(edpc);
	edpc_tx_init(edpc);

	edpc_link_config(edpc);
}

static void edpc_single_enable(struct edpc_data *edpc, bool enable)
{
	uint32_t val;

	/* start calibrate */
	clrsetbits_le32(edpc->base + EDP_PHY_CAL_CTRL, 0x1 << 8, enable << 8);

	do {
		val = edpc_readl(edpc, EDP_PHY_CAL_CTRL);
	} while (val & 0x100);

	/* enable transmit */
	edpc_writel(edpc, EDP_CORE_TX_EN, enable);

	/* phy ctl */
	val = ((((1 << edpc->configs.lane_count) - 1) & 0xf) << 9);
	val |= (edpc->configs.lane_polarity << 4);
	val |= (edpc->configs.lane_mirror << 15);
	edpc_writel(edpc, EDP_PHY_CTRL, val);

	edpc_writel(edpc, EDP_CORE_FSCR_RST, 1);		/* TODO */

	/* Enable the main stream */
	clrsetbits_le32(edpc->base + EDP_CORE_MSTREAM_EN,
			0x1 << 0, enable << 0);

	/* enable RGB interface	*/
	clrsetbits_le32(edpc->base + EDP_RGB_CTL,
			0x1 << 0, enable << 0);
}

static int owl_edpc_enable(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	edpc_clk_enable(edpc);

	edpc_display_init_edp(edpc);

	edpc_single_enable(edpc, true);
	return 0;
}

static void owl_edpc_disable(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	edpc_single_enable(edpc, false);

	edpc_clk_disable(edpc);
}

struct owl_display_ctrl_ops owl_edp_ctrl_ops = {
	.enable = owl_edpc_enable,
	.disable = owl_edpc_disable,
};

static struct owl_display_ctrl owl_edp_ctrl = {
	.name = "edp_ctrl",
	.type = OWL_DISPLAY_TYPE_EDP,
	.ops = &owl_edp_ctrl_ops,
};

int owl_edpc_init(const void *blob)
{
	int ret = 0;
	int node;

	struct edpc_data *edpc;

	/* DTS match */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,s900-edp");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	edpc = &owl_edpc_data;

	edpc->base = fdtdec_get_addr(blob, node, "reg");
	if (edpc->base == FDT_ADDR_T_NONE) {
		error("Cannot find edpc reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, edpc->base);

	ret = edpc_parse_config(blob, node, edpc);
	if (ret < 0)
		goto err_parse_config;

	edpc->ctrl = &owl_edp_ctrl;
	owl_ctrl_set_drvdata(&owl_edp_ctrl, edpc);

	ret = owl_ctrl_register(&owl_edp_ctrl);
	if (ret < 0)
		goto err_ctrl_register;

	return 0;

err_ctrl_register:
err_parse_config:

	return ret;
}
