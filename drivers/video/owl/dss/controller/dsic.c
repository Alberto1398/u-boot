/*
 * dss/dsic.h
 *
 * DSI Controller driver.
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
#define DSS_SUBSYS_NAME "DSIC"

#define DEBUG
#define pr_fmt(fmt) "owl_dsi: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/regs_s900.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <dss.h>

#include "dsic.h"

#define LONG_CMD_MODE		0x39
#define SHORT_CMD_MODE		0x05

#define POWER_MODE		0x01
#define DSIC_ID_S500 0
#define DSIC_ID_S700 1
#define DSIC_ID_S900 2

struct dsic_diffs {
	int id;
};
struct owl_dsi_config {

	uint32_t	lcd_mode;/* 0: command mode;1: video mode*/
	uint32_t	lane_count;/*00: 1,01:	1~2,10:	1~3,11:	1~4 */
	uint32_t	lane_polarity;/* 0: normal,1: reversed*/
	uint32_t	video_mode;/*00:sync mode,01:de mode,10:burst mode*/
	uint32_t	pclk_rate;
};

struct dsic_data {
	fdt_addr_t			base;    /*register address*/

	struct owl_display_ctrl		*ctrl;
	struct owl_dsi_config		configs;
	struct dsic_diffs		diffs;
};


#define dsic_writel(dsic, index, val) writel((val), dsic->base + (index))
#define dsic_readl(dsic, index) readl(dsic->base + (index))


static int dsic_parse_config(void *blob, int node, struct dsic_data *dsic)
{
	int entry;


	debug("%s:\n", __func__);

	/*
	 * parse hw configs ...
	 */
	entry = fdtdec_lookup_phandle(blob, node, "panel_configs");
	if (entry < 0) {
		error("no entry for 'panel_configs'\n");
		return -1;
	}
	debug("entry = 0x%d\n", entry);

	dsic->configs.lcd_mode = fdtdec_get_int(blob,
						entry, "lcd_mode", 0);

	dsic->configs.lane_count = fdtdec_get_int(blob,
						entry, "lane_count", 0);

	dsic->configs.lane_polarity = fdtdec_get_int(blob,
						entry, "lane_polarity", 0);

	dsic->configs.video_mode = fdtdec_get_int(blob,
						entry, "video_mode", 0);

	dsic->configs.pclk_rate = fdtdec_get_int(blob,
						entry, "pclk_rate", 0);


	if (0 == dsic->configs.pclk_rate) {
		error("pclk_rate is 0!!\n");
		return -1;
	}
	if (0 == dsic->configs.lane_count) {
		error("lane_count is 0!!\n");
		return -1;
	}

	debug("lcd_mode %d\n", dsic->configs.lcd_mode);
	debug("lane_count %d, lane_polarity %d\n",
			dsic->configs.lane_count, dsic->configs.lane_polarity);
	debug("video_mode %d\n", dsic->configs.video_mode);
	debug("pclk_rate %d\n", dsic->configs.pclk_rate);

	return 0;
}

static void dsic_send_short_packet(struct dsic_data *dsic, int data_type,
					int sp_data, int trans_mode)
{
	int tmp;

	debug("send short start\n");

	tmp = dsic_readl(dsic, DSI_CTRL);
	tmp &= 0xffffefff;
	dsic_writel(dsic, DSI_CTRL, tmp);

	dsic_writel(dsic, DSI_PACK_HEADER, sp_data);

	tmp = (data_type << 8) | (trans_mode << 14);
	dsic_writel(dsic, DSI_PACK_CFG, tmp);
	mdelay(1);

	tmp = dsic_readl(dsic, DSI_PACK_CFG);
	tmp |= 1;
	dsic_writel(dsic, DSI_PACK_CFG, tmp);
	do {
		tmp = dsic_readl(dsic, DSI_TR_STA);
	} while (!(tmp & 0x80000));
	dsic_writel(dsic, DSI_TR_STA, 0x80000);

	debug("send short end\n");
}

static void dsic_send_cmd(struct dsic_data *dsic)
{
	/* exit sleep mode */
	dsic_send_short_packet(dsic, SHORT_CMD_MODE, 0x11, POWER_MODE);
	mdelay(200);

	/* display on */
	dsic_send_short_packet(dsic, SHORT_CMD_MODE, 0x29, POWER_MODE);
	mdelay(200);

	dsic_send_short_packet(dsic, 0x32, 0, 1);
}

static uint32_t dsic_get_hsclk(struct dsic_data *dsic, uint16_t frame_rate,
				uint16_t lane_num, uint16_t xres, uint16_t hsw,
				uint16_t hfp, uint16_t hbp, uint16_t vtotal)
{
	uint16_t pixel2pro = 1;
	uint32_t h_bit = 0;/*horizontal bit*/
	uint32_t h_byte = 0;/*horizontal byte*/
	uint32_t per_lane_bps = 0;
	uint32_t htotal;
	uint32_t dsi_hsclk = 0;

	uint32_t bpp = owl_panel_get_bpp(dsic->ctrl->panel);
	uint16_t v_h = frame_rate;/*frequency vertial*/

	switch (bpp) {
	case 16:
		pixel2pro = 2;
	break;
	case 18:
		pixel2pro = 3;
	break;
	case 24:
		pixel2pro = 3;
	break;
	}
	debug("pixel2pro = %d\n", pixel2pro);
	switch (dsic->configs.video_mode) {
	case 0:/*sync mode*/
		h_byte = 4 + (hsw * pixel2pro + 6)
		+ (hbp * pixel2pro + 6) + (xres * pixel2pro + 6)
		+ (hfp * pixel2pro + 6);
	break;
	case 1:/*de mode*/
		h_byte = 4 + ((hsw + hbp)*pixel2pro + 6)
		+ ((xres * pixel2pro) + 6) + (hfp * pixel2pro + 6);
	break;
	case 2:/*burst mode*/
		h_byte = 4 + ((hsw + hbp)*pixel2pro + 6)
		+ ((xres * pixel2pro) + 6) + (1000 + 6)
		+ (hfp * pixel2pro + 6);/*bllp_reg = 1000*/
	break;
	}
	h_bit = h_byte * 8;
	htotal = h_bit / lane_num;
	per_lane_bps = htotal * vtotal * v_h;
	dsi_hsclk = (per_lane_bps / 2) / 1000000;/*Double edge transport*/

	if ((dsi_hsclk > 840) || (dsi_hsclk < 120)) {
		error("get_hsclk failed !\n");
		return -1;
	}

	debug("%s dsi_hsclk = %d\n", __func__, dsi_hsclk);

	return dsi_hsclk;
}

static unsigned int dsic_clk_get_divider(unsigned int parent_rate,
					unsigned int target_rate)
{
	unsigned int divider;

	divider = parent_rate / target_rate;

	debug("%s: parent_rate %d, target_rate %d, divider %d\n",
		__func__, parent_rate, target_rate, divider);
	return divider;
}
static int dsic_set_dsi_clk(struct dsic_data *dsic)
{
	struct owl_videomode *mode = &dsic->ctrl->panel->current_mode;
	uint32_t reg_val = 0;
	uint32_t hsclk_reg = 0;

	uint32_t cmu_dsiclk;
	uint32_t vtotal;
	uint16_t divider_reg = 0;
	int tmp;

	debug("%s\n", __func__);
	vtotal = mode->yres + mode->vsw + mode->vfp + mode->vbp;
	hsclk_reg = dsic_get_hsclk(dsic, 60, dsic->configs.lane_count,
			mode->xres, mode->hsw, mode->hfp, mode->hbp, vtotal);

	hsclk_reg = (hsclk_reg / 6) * 6;

	divider_reg = dsic_clk_get_divider(hsclk_reg,
				dsic->configs.pclk_rate) - 1;
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		cmu_dsiclk = 0x300 | (divider_reg << 16) | (hsclk_reg / 6);
		debug("cmu_dsiclk = %x\n", cmu_dsiclk);
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		cmu_dsiclk = (1 << 16) | (hsclk_reg / 6) << 8 | 2;
	}
	writel(cmu_dsiclk, CMU_DSICLK);
	debug("%s end.\n", __func__);
	return 0;
}

static void dsic_power_on(struct dsic_data *dsic)
{
	int temp;
	int ret = 0;
	/* assert reset */
	owl_reset_assert(RESET_DSI);
	temp = readl(SPS_LDO_CTL);
	temp |= (1 << 9);
	writel(temp, SPS_LDO_CTL);
	/*enable dsiclk from devpll*/
	owl_clk_enable(CLOCK_DSI);
	dsic_set_dsi_clk(dsic);
	mdelay(10);
	/* deassert reset*/
	owl_reset_deassert(RESET_DSI);
	debug("%s end.\n", __func__);
}

static void dsic_deinit(struct dsic_data *dsic)
{
	int temp;

	temp = dsic_readl(dsic, SPS_LDO_CTL);
	temp &= ~(1 << 9);
	dsic_writel(dsic, SPS_LDO_CTL, temp);

	/* reset */

	/* clk disable */
}

static void dsic_ctl_config(struct dsic_data *dsic)
{
	int val = 0;

	debug("%s\n", __func__);
	/*
	val = dsic_readl(dsic, DSI_CTRL);
	val |= (1 << 31);
	dsic_writel(dsic, DSI_CTRL, val);
	*/

	/*color from*/
	val = dsic_readl(dsic, DSI_CTRL);
	val = REG_SET_VAL(val, 0, 7, 7);/* 0 DE, 1 default color */
	dsic_writel(dsic, DSI_CTRL, val);

	val = 0;
	val = dsic_readl(dsic, DSI_CTRL);/* 0: command mode,1: video mode*/
	val = REG_SET_VAL(val, dsic->configs.lcd_mode, 12, 12);
	dsic_writel(dsic, DSI_CTRL, val);
	/*0: 1 data lane,1: 2 data lanes 2: 3 data lanes 3: 4 data lanes*/
	val = 0;
	val = dsic_readl(dsic, DSI_CTRL);
	val = REG_SET_VAL(val, (dsic->configs.lane_count - 1), 9, 8);
	dsic_writel(dsic, DSI_CTRL, val);


	/* others,to do*/
	val = 0;
	val = dsic_readl(dsic, DSI_CTRL);
	val = REG_SET_VAL(val, 1, 4, 4);/* EOTP enable*/
	dsic_writel(dsic, DSI_CTRL, val);
}


static void dsic_phy_config(struct dsic_data *dsic)
{
	int tmp = 0;

	debug("%s\n", __func__);
	tmp = dsic_readl(dsic, DSI_CTRL);
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		tmp |= (3 << 8);
		tmp |= (1 << 31);
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		tmp |= (3 << 8);
		tmp |= ((dsic->configs.lcd_mode) << 12);
	}
	dsic_writel(dsic, DSI_CTRL, tmp);
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		dsic_writel(dsic, DSI_PHY_T0, 0x1ba3);
		dsic_writel(dsic, DSI_PHY_T1, 0x1b1b);
		dsic_writel(dsic, DSI_PHY_T2, 0x2f06);
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		dsic_writel(dsic, DSI_PHY_T0, 0xa5a);
		dsic_writel(dsic, DSI_PHY_T1, 0x1b12);
		dsic_writel(dsic, DSI_PHY_T2, 0x2f05);
	}
	tmp = 0;
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		dsic_writel(dsic, DSI_PHY_CTRL, 0x7c600000);
		if (dsic->configs.lane_polarity) {
			tmp = dsic_readl(dsic, DSI_PHY_CTRL);
			tmp |= 0xb4083;/*swap N P,clk and data*/
			dsic_writel(dsic, DSI_PHY_CTRL, tmp);
		} else {
		 tmp = dsic_readl(dsic, DSI_PHY_CTRL);
		 tmp |= 0x800fb;/*normal*/
		 dsic_writel(dsic, DSI_PHY_CTRL, tmp);
		}
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		tmp = dsic_readl(dsic, DSI_PHY_CTRL);
		tmp |= (0xfc << 8);
		tmp |= (1 << 24);
		dsic_writel(dsic, DSI_PHY_CTRL, tmp);
	}
	/*dsic_writel(dsic, DSI_PIN_MAP, 0x688);*/
	/* cal */
	debug("cal start\n");
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		tmp = dsic_readl(dsic, DSI_CTRL);
		tmp |= (1 << 30);
		dsic_writel(dsic, DSI_CTRL, tmp);
		/* wait for cal done */
		do {
			tmp = dsic_readl(dsic, DSI_CTRL);
		} while ((tmp & (1 << 30)));
		debug("cal done\n");
		debug("phy set end~~~~\n");
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		tmp = dsic_readl(dsic, DSI_PHY_CTRL);
		tmp |= ((1 << 25) | (1 << 28));
		dsic_writel(dsic, DSI_PHY_CTRL, tmp);
		do {
			tmp = dsic_readl(dsic, DSI_PHY_CTRL);
		} while ((!(tmp & (1 << 31))));
		debug("cal done\n");
	}
	debug("wait line stop\n");
	/* data0 line ,clk line in stop state */
	do {
		tmp = dsic_readl(dsic, DSI_LANE_STA);
	} while (!(tmp & 0x1020));
	/* continue clock EN */
	tmp = dsic_readl(dsic, DSI_CTRL);
	tmp |= 0x40;
	dsic_writel(dsic, DSI_CTRL, tmp);
	mdelay(1);

	debug("%s end.\n", __func__);
}


static void dsic_set_size(struct dsic_data *dsic, int width, int height)
{
		int tmp = 0;

		tmp = dsic_readl(dsic, DSI_SIZE);
		tmp = REG_SET_VAL(tmp, height, 27, 16);
		dsic_writel(dsic, DSI_SIZE, tmp);
}


static void dsic_set_default_color(struct dsic_data *dsic, uint32_t color)
{
		debug("%s\n", __func__);
		dsic_writel(dsic, DSI_COLOR, color);
}

static void dsic_set_timings(struct dsic_data *dsic,
				uint16_t width, uint16_t height,
				uint16_t hbp, uint16_t hfp, uint16_t hsw,
				uint16_t vbp, uint16_t vfp, uint16_t vsw)
{
	uint32_t tmp = 0;
	uint32_t pixel2pro = 0;
	uint32_t bpp = owl_panel_get_bpp(dsic->ctrl->panel);
	uint32_t hsw_reg = 0, hbp_reg = 0, hfp_reg = 0, bllp_reg = 0;

	uint32_t rgbht0_reg = 0, rgbht1_reg = 0, rgbvt0_reg = 0, rgbvt1_reg = 0;
	uint32_t vtotal = vbp + vfp + vsw + height;

	debug("%s\n", __func__);
	switch (bpp) {
	case 16:
		pixel2pro = 2;
	break;
	case 18:
		pixel2pro = 3;
	break;
	case 24:
		pixel2pro = 3;
	break;
	}
	/**/
	switch (dsic->configs.video_mode) {
	case 0:/*sync mode*/
		hsw_reg  = hsw * pixel2pro;
		hbp_reg  = hbp * pixel2pro;
		hfp_reg  = hfp * pixel2pro;
		bllp_reg = 0;
	break;
	case 1:/*de mode*/
		hsw_reg  = 0;
		hbp_reg  = (hsw + hbp) * pixel2pro;
		hfp_reg  = hfp * pixel2pro;
		bllp_reg = 0;
	break;
	case 2:/*burst mode*/
		hsw_reg  = 0;
		hbp_reg  = (hsw + hbp) * pixel2pro;
		hfp_reg  = hfp * pixel2pro;
		bllp_reg = 1000;
	break;
	}

	rgbht0_reg = ((hsw_reg << 20) | (hfp_reg << 10) | (hbp_reg))
			& 0x3fffffff;
	rgbht1_reg = (bllp_reg) & 0xfff;
	/*preline | vsw | vtotal*/
	rgbvt0_reg = (0x01f00000 | ((vsw) << 13) | (vtotal)) & 0xfffffff;
	rgbvt1_reg = (vsw + vbp) & 0x1fffffff;

	debug("ht0 = %x\n", rgbht0_reg);
	debug("ht1 = %x\n", rgbht1_reg);
	debug("vt0 = %x\n", rgbvt0_reg);
	debug("vt1 = %x\n", rgbvt1_reg);

	dsic_writel(dsic, DSI_RGBHT0, rgbht0_reg);
	dsic_writel(dsic, DSI_RGBHT1, rgbht1_reg);
	dsic_writel(dsic, DSI_RGBVT0, rgbvt0_reg);
	dsic_writel(dsic, DSI_RGBVT1, rgbvt1_reg);

	/* others,to do*/
	dsic_writel(dsic, DSI_PACK_CFG, 0x0);
	if (dsic->diffs.id == DSIC_ID_S900) {
		debug("%s, s900\n", __func__);
		dsic_writel(dsic, DSI_PACK_HEADER, 0x900);
	} else if (dsic->diffs.id == DSIC_ID_S500) {
		debug("%s, s500\n", __func__);
		dsic_writel(dsic, DSI_PACK_HEADER, 0xf00);
	}
	tmp = dsic_readl(dsic, DSI_VIDEO_CFG);
	/*vidoe mode select*/
	tmp = REG_SET_VAL(tmp, dsic->configs.video_mode, 2, 1);
	dsic_writel(dsic, DSI_VIDEO_CFG, tmp);

	tmp = 0;
	tmp = dsic_readl(dsic, DSI_VIDEO_CFG);
	tmp = REG_SET_VAL(tmp, 1, 3, 3);
	dsic_writel(dsic, DSI_VIDEO_CFG, tmp);

	tmp = 0;
	tmp = dsic_readl(dsic, DSI_VIDEO_CFG);
	tmp = REG_SET_VAL(tmp, 0x3, 10, 8);/*RGB color format*/
	dsic_writel(dsic, DSI_VIDEO_CFG, tmp);

	debug("DSI_VIDEO_CFG = %x\n", tmp);
}


static void dsic_display_init(struct dsic_data *dsic)
{
	debug("%s\n", __func__);
	struct owl_videomode *mode = &dsic->ctrl->panel->current_mode;
	dsic_set_size(dsic, mode->xres, mode->yres);
	dsic_set_default_color(dsic, 0xff0000);

	dsic_set_timings(dsic, mode->xres, mode->yres,
	mode->hbp, mode->hfp, mode->hsw, mode->vbp, mode->vfp, mode->vsw);
}

static void dsic_single_enable(struct dsic_data *dsic, bool enable)
{
	int tmp;

	tmp = dsic_readl(dsic, DSI_VIDEO_CFG);

	if (enable)
		tmp |= 0x01;
	else
		tmp &= (~0x01);

	dsic_writel(dsic, DSI_VIDEO_CFG, tmp);
}
static void dsic_init(struct dsic_data *dsic)
{
	debug("%s\n", __func__);
	dsic_phy_config(dsic);
	dsic_send_cmd(dsic);
	dsic_ctl_config(dsic);
	dsic_display_init(dsic);
	dsic_single_enable(dsic, true);
}
int owl_dsic_enable(struct owl_display_ctrl *ctrl)
{
	debug("%s\n", __func__);
	struct dsic_data *dsic = owl_ctrl_get_drvdata(ctrl);
	dsic_power_on(dsic);
	dsic_init(dsic);

	return 0;
}

void owl_dsic_disable(struct owl_display_ctrl *ctrl)
{
	struct dsic_data *dsic = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	dsic_single_enable(dsic, false);
}

struct owl_display_ctrl_ops owl_dsi_ctrl_ops = {
	.enable = owl_dsic_enable,
	.disable = owl_dsic_disable,

};

static struct owl_display_ctrl owl_dsi_ctrl = {
	.name = "dsi_ctrl",
	.type = OWL_DISPLAY_TYPE_DSI,
	.ops = &owl_dsi_ctrl_ops,
};

static struct dsic_data	owl_dsic_data;

int owl_dsic_init(const void *blob)
{
	int node;

	int ret = 0;
	debug("%s\n", __func__);
	struct dsic_data *dsic = &owl_dsic_data;

	/* DTS match */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,s900-dsi");
	if (node > 0) {
		dsic->diffs.id = DSIC_ID_S900;
	} else {
		node = fdt_node_offset_by_compatible(blob, 0,
							"actions,s700-dsi");
		if (node > 0) {
			dsic->diffs.id = DSIC_ID_S700;
		} else {
			node = fdt_node_offset_by_compatible(blob, 0,
							"actions,s500-dsi");
			if (node > 0)
				dsic->diffs.id = DSIC_ID_S500;
			else {
				debug("no match in DTS\n");
				return 0;
			}
		}
	}
	debug("%s, ic_type = %d\n", __func__, dsic->diffs.id);

	dsic->base = fdtdec_get_addr(blob, node, "reg");
	if (dsic->base == FDT_ADDR_T_NONE) {
		error("Cannot find dsic reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, dsic->base);

	ret = dsic_parse_config(blob, node, dsic);
	if (ret < 0)
		goto err_parse_config;

	dsic->ctrl = &owl_dsi_ctrl;
	owl_ctrl_set_drvdata(&owl_dsi_ctrl, dsic);

	ret = owl_ctrl_register(&owl_dsi_ctrl);
	if (ret < 0)
		goto err_ctrl_register;

	return 0;

err_ctrl_register:
err_parse_config:
	return ret;
}
