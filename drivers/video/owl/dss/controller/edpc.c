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
#define DEBUGX
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
	      edpc->configs.lane_count, edpc->configs.lane_polarity,
	      edpc->configs.lane_mirror);
	debug("mstream_polarity %x, user_sync_polarity = %x\n",
	      edpc->configs.mstream_polarity,
	      edpc->configs.user_sync_polarity);
	debug("pclk_parent %d pclk_rate %d\n",
	      edpc->configs.pclk_parent, edpc->configs.pclk_rate);

	return 0;
}

unsigned short edp_auxread(struct edpc_data *edpc, unsigned short addr)
{
	unsigned int temp;
	edpc_writel(edpc, EDP_AUX_ADDR, addr);
	edpc_writel(edpc, EDP_AUX_COMD, 0x09<<8); //aux read, 1byte
	do{
		temp = edpc_readl(edpc, EDP_AUX_STATE);
	}while(!(temp&(1<<2)));		//if 1, reply is in progress

	temp = edpc_readl(edpc, EDP_AUX_RPLY_CODE);
	temp = edpc_readl(edpc, EDP_AUX_RPLY_COUNT );
	temp = edpc_readl(edpc, EDP_AUX_RPLY_DAT_CNT);
	temp = edpc_readl(edpc, EDP_AUX_RPLY_DAT);
	return(temp);
}

void edp_auxwrite(struct edpc_data *edpc, unsigned short addr,unsigned short Data)
{
	unsigned int temp;

	edpc_writel(edpc, EDP_AUX_ADDR, addr);
	edpc_writel(edpc, EDP_AUX_WR_FIFO, Data);
	edpc_writel(edpc, EDP_AUX_COMD, 0x08<<8); //aux write, 1byte
	do{
		temp = edpc_readl(edpc, EDP_AUX_STATE);
	}while(temp&(1<<1));		//if 1, request is in progress

	temp =edpc_readl(edpc, EDP_AUX_RPLY_CODE);
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
	uint32_t bpp = owl_panel_get_bpp(edpc->ctrl->panel);

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

	edpc_writel(edpc, EDP_M_VID, 0x7d28);/* vidio/1K TODO */
	/* 64byte each micro packet */
	edpc_writel(edpc, EDP_MTRANSFER_UNIT, 64);

	edpc_writel(edpc, EDP_N_VID, 0x8000);
	edpc_writel(edpc, EDP_USER_DATA_COUNT, (width * bpp / 16 - 1));
	edpc_writel(edpc, EDP_USER_SYNC_POLARITY,
		    edpc->configs.user_sync_polarity);
}

static void edpc_set_default_color(struct edpc_data *edpc, uint32_t color)
{
	edpc_writel(edpc, EDP_RGB_COLOR, color);
}

static void edpc_set_data_width(struct edpc_data *edpc)
{
	uint32_t bpp = owl_panel_get_bpp(edpc->ctrl->panel);
	uint8_t bit_depth = 1;

	switch(bpp) {
		case 18:
			bit_depth = 0;
			break;
		case 24:
			bit_depth = 1;
			break;
		case 30:
			bit_depth = 2;
			break;
		case 36:
			bit_depth = 3;
			break;
		case 48:
			bit_depth = 4;
			break;
		default:
			bit_depth = 0;
			break;
	}

	clrsetbits_le32(edpc->base + EDP_MSTREAM_MISC0,
				0x7 << 5, bit_depth << 5);
}

static void edpc_enable_internal_scramble(struct edpc_data *edpc, bool enable)
{
	bool val;
	val = !enable;

	edpc_writel(edpc, EDP_LNK_SCR_CTRL, val);
}

static void edpc_set_single_format(struct edpc_data *edpc, uint8_t format)
{
	clrsetbits_le32(edpc->base + EDP_MSTREAM_MISC0, 0x3 << 1, format << 1);
}

static void edpc_set_preline(struct edpc_data *edpc)
{
	int preline;

	preline = owl_panel_get_preline_num(edpc->ctrl->panel);
	preline -= 1;
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
	edpc_enable_internal_scramble(edpc, true);
}

static void edpc_display_init_edp(struct edpc_data *edpc)
{
	struct owl_videomode *mode = &edpc->ctrl->panel->current_mode;

	edpc_set_size(edpc, mode->xres, mode->yres);
	edpc_set_mode(edpc, mode->xres, mode->yres, mode->hbp, mode->hfp,
		      mode->hsw, mode->vbp, mode->vfp, mode->vsw);

	edpc_set_default_color(edpc, 0xff);
	edpc_set_single_format(edpc, 0);
	edpc_set_data_width(edpc);

	edpc_set_preline(edpc);
	edpc_set_single_from(edpc, 0);	/* 0 DE, 1 default color */
	edpc_phy_config(edpc);
	edpc_tx_init(edpc);

	edpc_link_config(edpc);
}

static void edpc_video_enable(struct edpc_data *edpc, bool enable)
{	

	edpc_writel(edpc, EDP_CORE_FSCR_RST, 1);		/* TODO */

	/* Enable the main stream */
	clrsetbits_le32(edpc->base + EDP_CORE_MSTREAM_EN,
			0x1 << 0, enable << 0);

	/* enable RGB interface	*/
	clrsetbits_le32(edpc->base + EDP_RGB_CTL, 0x1 << 0, enable << 0);
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
}

void edp_delay(unsigned int num)
{
	unsigned int i, k;

	for (i = 0; i < num; i++)
	{
		k= 0;
	}
	return;
}

void set_source_pre_emp(struct edpc_data *edpc, unsigned int level)
{
	//set source and sink to swing level0
	edpc_writel(edpc, EDP_PHY_PREEM_L0, level);
	edpc_writel(edpc, EDP_PHY_PREEM_L1, level);
	edpc_writel(edpc, EDP_PHY_PREEM_L2, level);
	edpc_writel(edpc, EDP_PHY_PREEM_L3, level);
}

void set_sink_speed(struct edpc_data *edpc, unsigned int speed)
{
	switch(speed)
	{
		case 0: edp_auxwrite(edpc, 0x100,0x06); break;	//1.62G
		case 1: edp_auxwrite(edpc, 0x100,0x0a); break;	//2.7G
		case 2: edp_auxwrite(edpc, 0x100,0x14); break;	//5.4G
		default: break; ;
	}
}

unsigned int link_training_pattern1(struct edpc_data *edpc)
{
	unsigned int val;

	mdelay(10);
	set_source_pre_emp(edpc, 2);

	edpc_writel(edpc, EDP_LNK_TRAP, 1);	/* set training pattern1 */

	set_sink_speed(edpc, 1);
	mdelay(10);
	edp_auxwrite(edpc, 0x0101, 0x84);

	val = edp_auxread(edpc, 0x0000);
	val = edp_auxread(edpc, 0x0001);
	val = edp_auxread(edpc, 0x0002);

	val = edp_auxread(edpc, 0x0003);
	edp_auxwrite(edpc, 0x0600, 0x01);

	val = edp_auxread(edpc, 0x0000);
	val = edp_auxread(edpc, 0x0100);
	val = edp_auxread(edpc, 0x0002);
	val = edp_auxread(edpc, 0x0101);
	val = edp_auxread(edpc, 0x0000);
	val = edp_auxread(edpc, 0x0102);

	edp_auxwrite(edpc, 0x0102, 0x21);	/* pattern1 training */

	edp_auxwrite(edpc, 0x0103, 0x00);
	edp_auxwrite(edpc, 0x0104, 0x00);
	edp_auxwrite(edpc, 0x0105, 0x00);
	edp_auxwrite(edpc, 0x0106, 0x00);
	mdelay(10);

	val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);	/* lane2\3 done */

	val = edp_auxread(edpc, 0x0101);

	val = edp_auxread(edpc, 0x0206);
	val = edp_auxread(edpc, 0x0207);

	edp_auxwrite(edpc, 0x0103, 0x38);/* swing= level 0; pre-emphasis =3 */
	edp_auxwrite(edpc, 0x0104, 0x38);
	edp_auxwrite(edpc, 0x0105, 0x38);
	edp_auxwrite(edpc, 0x0106, 0x38);

	mdelay(10);

	val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);	/* lane2\3 done */

	val = edp_auxread(edpc, 0x0000);
	val = edp_auxread(edpc, 0x0102);

	debug("wait for lane done\n");
	do {
		val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
		val = edp_auxread(edpc, 0x203);	/* lane2\3 done */
	} while((val & 0x11) != 0x11);

	return 0;

}

unsigned int link_training_pattern2(struct edpc_data *edpc)
{
	unsigned int val;

	edpc_writel(edpc,EDP_LNK_TRAP, 2);	/*set training partern 2*/

	edp_auxwrite(edpc, 0x0102, 0x22);	/* training pattern 2 */
	mdelay(10);

	val = edp_auxread(edpc, 0x202);		/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);		/* lane2\3 done */

	val = edp_auxread(edpc, 0x101);

	val = edp_auxread(edpc, 0x0206);
	val = edp_auxread(edpc, 0x0207);

	edp_auxwrite(edpc, 0x0103, 0x38);/*swing= level 0; pre-emphasis =3 */
	edp_auxwrite(edpc, 0x0104, 0x38);
	edp_auxwrite(edpc, 0x0105, 0x38);
	edp_auxwrite(edpc, 0x0106, 0x38);
	mdelay(10);

	val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);	/* lane2\3 done */

	val = edp_auxread(edpc, 0x101);

	val = edp_auxread(edpc, 0x0206);
	val = edp_auxread(edpc, 0x0207);

	edp_auxwrite(edpc, 0x0103, 0x10);/* swing= level 0; pre-emphasis =1 */
	edp_auxwrite(edpc, 0x0104, 0x10);
	edp_auxwrite(edpc, 0x0105, 0x10);
	edp_auxwrite(edpc, 0x0106, 0x10);
	mdelay(10);

	val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);	/* lane2\3 done */

	val = edp_auxread(edpc, 0x101);

	val = edp_auxread(edpc, 0x0206);
	val = edp_auxread(edpc, 0x0207);

	edp_auxwrite(edpc, 0x0103, 0x10);     /* swing= level 0; pre-emphasis = 1 */
	edp_auxwrite(edpc, 0x0104, 0x10);
	edp_auxwrite(edpc, 0x0105, 0x10);
	edp_auxwrite(edpc, 0x0106, 0x10);

	mdelay(10);

	val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	val = edp_auxread(edpc, 0x203);	/* lane2\3 done */

	do {
		val = edp_auxread(edpc, 0x202);	/* lane0\1 done */
	} while((val & 0x77) != 0x77);

	val = edp_auxread(edpc, 0x204);
	val = edp_auxread(edpc, 0x102);

	edp_auxwrite(edpc, 0x0102, 0x00);

}

void edpc_link_training(struct edpc_data *edpc)
{

	unsigned int temp;
	printf("%s, start!\n", __func__);

	temp = edp_auxread(edpc, 0x0000);
	temp = edp_auxread(edpc, 0x0001);
	temp = edp_auxread(edpc, 0x0002);

	edpc_enable_internal_scramble(edpc, false);	/* disable internal scramble */

	temp = link_training_pattern1(edpc);
	debug("%s, link_training_pattern1 end\n", __func__);
	mdelay(10);

	temp = edp_auxread(edpc, 0x000); 		/* REV	0x12 */
	temp = link_training_pattern2(edpc);
	mdelay(10);
	debug("%s, link_training_pattern2 end\n", __func__);

	edp_auxwrite(edpc, 0x102, 0);			/* set Sink training pattern3 */
	edp_delay(50);
	temp = edp_auxread(edpc, 0x102);

	edpc_writel(edpc,EDP_LNK_TRAP, 0x00);		/* training off */
	edpc_enable_internal_scramble(edpc, true);	/* enable internal scramble */
	printf("%s, end!\n", __func__);
}

static int owl_edpc_enable(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	edpc_video_enable(edpc, true);
	
	return 0;
}

static void owl_edpc_disable(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	edpc_single_enable(edpc, false);

	edpc_video_enable(edpc, false);

	edpc_clk_disable(edpc);
}

static int owl_edpc_power_on(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	/*
	 * enable clk and start phy calibrate
	 * */
	edpc_clk_enable(edpc);

	edpc_display_init_edp(edpc);

	edpc_single_enable(edpc, true);

	debug("%s, end\n", __func__);
}

static int owl_edpc_power_off(struct owl_display_ctrl *ctrl)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

}

int owl_edpc_aux_read(struct owl_display_ctrl *ctrl, char *buf, int count)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);
	

}
int owl_edpc_aux_write(struct owl_display_ctrl *ctrl, const char *buf,
			int count)
{
	struct edpc_data *edpc = owl_ctrl_get_drvdata(ctrl);
	unsigned int temp;

	edpc_link_training(edpc);

	debug("%s\n", __func__);

}

struct owl_display_ctrl_ops owl_edp_ctrl_ops = {
	.enable = owl_edpc_enable,
	.disable = owl_edpc_disable,

	.power_on = owl_edpc_power_on,
	.power_off = owl_edpc_power_off,

	.aux_read = owl_edpc_aux_read,
	.aux_write = owl_edpc_aux_write,
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
