/*
 * OWL LCD/LVDS controller
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
#define pr_fmt(fmt) "owl_lcdc: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>

#include <asm/io.h>

#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>

#include <dss.h>
#include "lcdc.h"

#define mfp0 0xe01b0040
#define mfp1 0xe01b0044
#define mfp2 0xe01b0048

enum lcd_port_type {
	LCD_PORT_TYPE_RGB = 0,
	LCD_PORT_TYPE_CPU,
	LCD_PORT_TYPE_LVDS,
	LCD_PORT_TYPE_EDP,
};

struct lcdc_config {
	uint32_t		port_type;

	uint32_t		vsync_inversion;
	uint32_t		hsync_inversion;
	uint32_t		dclk_inversion;
	uint32_t		lde_inversion;

	uint32_t		pclk_parent;
	uint32_t		pclk_rate;
};

struct lcdc_data {
	fdt_addr_t		base;	/* register address */

	struct owl_display_ctrl	*ctrl;
	struct lcdc_config	configs;
};

static struct lcdc_data		owl_lcdc_data;

#define lcdc_writel(lcdc, index, val) writel((val), lcdc->base + (index))
#define lcdc_readl(lcdc, index) readl(lcdc->base + (index))

static int lcdc_parse_config(const void *blob, int node,
			     struct lcdc_data *lcdc)
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

	lcdc->configs.port_type
		= fdtdec_get_int(blob, entry, "port_type", 0);

	lcdc->configs.vsync_inversion
		= fdtdec_get_int(blob, entry, "vsync_inversion", 0);
	lcdc->configs.hsync_inversion
		= fdtdec_get_int(blob, entry, "hsync_inversion", 0);
	lcdc->configs.dclk_inversion
		= fdtdec_get_int(blob, entry, "dclk_inversion", 0);
	lcdc->configs.lde_inversion
		= fdtdec_get_int(blob, entry, "lde_inversion", 0);

	lcdc->configs.pclk_parent
		= fdtdec_get_int(blob, entry, "pclk_parent", 0);
	lcdc->configs.pclk_rate
		= fdtdec_get_int(blob, entry, "pclk_rate", 0);
	if (lcdc->configs.pclk_rate == 0) {
		error("pclk_rate is 0!!\n");
		return -1;
	}

	debug("%s:\n", __func__);
	debug("port_type %d\n", lcdc->configs.port_type);
	debug("vsync_inversion %d, hsync_inversion %d\n",
	      lcdc->configs.vsync_inversion, lcdc->configs.hsync_inversion);
	debug("dclk_inversion %d, lde_inversion %d\n",
	      lcdc->configs.dclk_inversion, lcdc->configs.lde_inversion);

	debug("pclk_parent %d pclk_rate %d\n",
	      lcdc->configs.pclk_parent, lcdc->configs.pclk_rate);

	return 0;
}

static unsigned int lcdclk_get_divider(unsigned int parent_rate,
				       unsigned int target_rate)
{
	unsigned int divider;

	divider = parent_rate / target_rate;

	debug("%s: parent_rate %d, target_rate %d, divider %d\n",
		__func__, parent_rate, target_rate, divider);

	BUG_ON(divider * target_rate != parent_rate);
	BUG_ON(divider > 32 || divider == 0);

	return divider;
}
static unsigned int lcdc_get_display_pll(struct lcdc_data *lcdc)
{
	unsigned int val, tmp;

	tmp = readl(CMU_DISPLAYPLL);
	val = (tmp & 0xff) * 6;

	if (tmp & (1 << 8))
		return val;
	else
		error("get display pll failed!\n");
}
static void lcdc_clk_enable(struct lcdc_data *lcdc)
{
	unsigned int tmp, parent_clk;

	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_LCD0);

	/* enable lcdclk from devpll */
	owl_clk_enable(CLOCK_LCD0);

	parent_clk = lcdc_get_display_pll(lcdc);
	tmp = 0;
	if (lcdc->configs.pclk_parent == 0) {	/* DISPLAY_PLL */
		tmp |= (0 << 12);
		tmp |= (lcdclk_get_divider(parent_clk * 1000000,
				lcdc->configs.pclk_rate) - 1);
	} else {
		/* TODO */
		BUG();
	}
	writel(tmp, CMU_LCDCLK);

	owl_reset_deassert(RESET_LCD0);

	mdelay(1);
}

static void lcdc_clk_disable(struct lcdc_data *lcdc)
{
	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_LCD0);

	/* disable lcdclk from devpll */
	owl_clk_disable(CLOCK_LCD0);

	writel(0, CMU_LCDCLK);
}

static void lcdc_set_size(struct lcdc_data *lcdc,
			  uint16_t width, uint16_t height)
{
	uint32_t val;

	BUG_ON((width > (1 << 12)) || (height > (1 << 12)));

	val = REG_VAL(height - 1, 27, 16) | REG_VAL(width - 1, 11, 0);

	lcdc_writel(lcdc, LCDC_SIZE, val);
}

static void lcdc_set_mode(struct lcdc_data *lcdc,
			  uint16_t hbp, uint16_t hfp, uint16_t hsw,
			  uint16_t vbp, uint16_t vfp, uint16_t vsw)
{
	uint32_t val;

	BUG_ON((hbp > (1 << 9)) || (hfp > (1 << 9)) || (hsw > (1 << 9)));
	BUG_ON((vbp > (1 << 9)) || (vfp > (1 << 9)) || (vsw > (1 << 9)));

	val = REG_VAL(hsw - 1, 29, 20) | REG_VAL(hfp - 1, 19, 10)
		| REG_VAL(hbp - 1, 9, 0);
	lcdc_writel(lcdc, LCDC_TIM1, val);

	val = REG_VAL(vsw - 1, 29, 20) | REG_VAL(vfp - 1, 19, 10)
		| REG_VAL(vbp - 1, 9, 0);
	lcdc_writel(lcdc, LCDC_TIM2, val);
}

static void lcdc_set_default_color(struct lcdc_data *lcdc, uint32_t color)
{
	lcdc_writel(lcdc, LCDC_COLOR, color);
}

static void lcdc_set_single_format(struct lcdc_data *lcdc, uint8_t format)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_CTL);
	val = REG_SET_VAL(val, format, 18, 16);
	lcdc_writel(lcdc, LCDC_CTL, val);
}

static void lcdc_set_preline(struct lcdc_data *lcdc)
{
	uint32_t val;
	int preline;

	preline = owl_panel_get_preline_num(lcdc->ctrl->panel);
	preline -= 1;
	preline = (preline < 0 ? 0 : preline);
	preline = (preline > 31 ? 31 : preline);

	val = lcdc_readl(lcdc, LCDC_TIM0);
	val = REG_SET_VAL(0, preline, 12, 8);
	val = REG_SET_VAL(val, 1, 13, 13);
	lcdc_writel(lcdc, LCDC_TIM0, val);
}

static void lcdc_set_vsync_inv(struct lcdc_data *lcdc, uint8_t vsync_inv)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_TIM0);
	val = REG_SET_VAL(val, vsync_inv, 7, 7);
	lcdc_writel(lcdc, LCDC_TIM0, val);
}

static void lcdc_set_hsync_inv(struct lcdc_data *lcdc, uint8_t hsync_inv)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_TIM0);
	val = REG_SET_VAL(val, hsync_inv, 6, 6);
	lcdc_writel(lcdc, LCDC_TIM0, val);
}

static void lcdc_set_dclk_inv(struct lcdc_data *lcdc, uint8_t dclk_inv)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_TIM0);
	val = REG_SET_VAL(val, dclk_inv, 5, 5);
	lcdc_writel(lcdc, LCDC_TIM0, val);
}

static void lcdc_set_lde_inv(struct lcdc_data *lcdc, uint8_t led_inv)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_TIM0);
	val = REG_SET_VAL(val, led_inv, 4, 4);
	lcdc_writel(lcdc, LCDC_TIM0, val);
}

static void lcdc_set_rb_swap(struct lcdc_data *lcdc, bool rb_swap)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_CTL);
	val = REG_SET_VAL(val, rb_swap, 1, 1);
	lcdc_writel(lcdc, LCDC_CTL, val);
}

static void lcdc_set_single_from(struct lcdc_data *lcdc, uint8_t single)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_CTL);
	val = REG_SET_VAL(val, single, 7, 6);

	lcdc_writel(lcdc, LCDC_CTL, val);
}

static void lcdc_single_enable(struct lcdc_data *lcdc, bool enable)
{
	uint32_t val;

	val = lcdc_readl(lcdc, LCDC_CTL);
	val = REG_SET_VAL(val, enable, 0, 0);
	lcdc_writel(lcdc, LCDC_CTL, val);
}

static void lcdc_lvds_port_enable(struct lcdc_data *lcdc, bool enable)
{
	uint32_t val;
	if (enable) {
		val = lcdc_readl(lcdc, LCDC_LVDS_ALG_CTL0);
		val = REG_SET_VAL(val, 0, 30, 31);
		val = REG_SET_VAL(val, 0, 4, 5);
		lcdc_writel(lcdc, LCDC_LVDS_ALG_CTL0, 0xc141a030);

		val = lcdc_readl(lcdc, LCDC_LVDS_CTL);
		val = REG_SET_VAL(val, enable, 0, 0);
		lcdc_writel(lcdc, LCDC_LVDS_CTL, 0x000a9521);
	} else {
		val = lcdc_readl(lcdc, LCDC_LVDS_ALG_CTL0);
		val = REG_SET_VAL(val, 0, 30, 31);
		val = REG_SET_VAL(val, 0, 4, 5);
		lcdc_writel(lcdc, LCDC_LVDS_ALG_CTL0, val);

		val = lcdc_readl(lcdc, LCDC_LVDS_CTL);
		val = REG_SET_VAL(val, enable, 0, 0);
		lcdc_writel(lcdc, LCDC_LVDS_CTL, val);
	}
}

static void lcdc_display_init_lcdc(struct lcdc_data *lcdc)
{
	struct owl_videomode *mode = &lcdc->ctrl->panel->current_mode;

	debug("%s\n", __func__);

	lcdc_set_size(lcdc, mode->xres, mode->yres);
	lcdc_set_mode(lcdc, mode->hbp, mode->hfp, mode->hsw,
		      mode->vbp, mode->vfp, mode->vsw);

	lcdc_set_default_color(lcdc, 0xff);
	lcdc_set_single_format(lcdc, 0);

	lcdc_set_preline(lcdc);
	lcdc_set_single_from(lcdc, 0x02);

	lcdc_set_rb_swap(lcdc, 0);
	lcdc_set_vsync_inv(lcdc, lcdc->configs.vsync_inversion);
	lcdc_set_hsync_inv(lcdc, lcdc->configs.hsync_inversion);
	lcdc_set_dclk_inv(lcdc, lcdc->configs.dclk_inversion);
	lcdc_set_lde_inv(lcdc, lcdc->configs.lde_inversion);
}


static int owl_lcdc_enable(struct owl_display_ctrl *ctrl)
{
	struct lcdc_data *lcdc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	lcdc_clk_enable(lcdc);

	lcdc_display_init_lcdc(lcdc);

	if (lcdc->configs.port_type == LCD_PORT_TYPE_LVDS)
		lcdc_lvds_port_enable(lcdc, true);

	lcdc_single_enable(lcdc, true);

	return 0;
}

static void owl_lcdc_disable(struct owl_display_ctrl *ctrl)
{
	struct lcdc_data *lcdc = owl_ctrl_get_drvdata(ctrl);

	debug("%s\n", __func__);

	if (lcdc->configs.port_type == LCD_PORT_TYPE_LVDS)
		lcdc_lvds_port_enable(lcdc, false);

	lcdc_single_enable(lcdc, false);

	lcdc_clk_disable(lcdc);
}

static struct owl_display_ctrl_ops owl_lcd_ctrl_ops = {
	.enable = owl_lcdc_enable,
	.disable = owl_lcdc_disable,
};

static struct owl_display_ctrl owl_lcd_ctrl = {
	.name = "lcd_ctrl",
	.type = OWL_DISPLAY_TYPE_LCD,

	.ops = &owl_lcd_ctrl_ops,
};

int owl_lcdc_init(const void *blob)
{
	int ret = 0;
	int node;
	int tmp,tmp1,tmp2;

	struct lcdc_data *lcdc;
	

 	tmp = readl(mfp0);
 	tmp |=0x4<<23; 
	writel(tmp,mfp0);
	
	tmp1 = readl(mfp1);
 	tmp1 |=(0x4<<11)|(0x3<<12)|(0x3<<10)|(0x3<<5); 
	writel(tmp1,mfp1);
	
	tmp2 = readl(mfp2);
 	tmp2 |=(0x3<<29)|(0x3<<27); 
	writel(tmp2,mfp2);

	/* DTS match */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,s900-lcd");
	if (node < 0) {
		node = fdt_node_offset_by_compatible(blob, 0,
							"actions,s700-lcd");
		if (node < 0) {
			debug("no match in DTS\n");
			return 0;
		}
	}
	debug("%s\n", __func__);

	lcdc = &owl_lcdc_data;

	lcdc->base = fdtdec_get_addr(blob, node, "reg");
	if (lcdc->base == FDT_ADDR_T_NONE) {
		error("Cannot find lcdc reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, lcdc->base);

	ret = lcdc_parse_config(blob, node, lcdc);
	if (ret < 0)
		goto err_parse_config;

	lcdc->ctrl = &owl_lcd_ctrl;
	owl_ctrl_set_drvdata(&owl_lcd_ctrl, lcdc);

	ret = owl_ctrl_register(&owl_lcd_ctrl);
	if (ret < 0)
		goto err_ctrl_register;

	return 0;

err_ctrl_register:
err_parse_config:

	return ret;
}
