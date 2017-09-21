/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Lipeng<lipeng@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUGX
#define pr_fmt(fmt) "de_s900: " fmt

#include <asm/io.h>

#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/arch/powergate.h>

#include <dss.h>

#include "de_s900.h"

static struct owl_de_device	owl_de_s9009;

#define de_readl(idx)		readl(owl_de_s9009.base + (idx))
#define de_writel(val, idx)	writel(val, owl_de_s9009.base + (idx))

#define SHARESRAM_BASE		0xE0240000
#define SHARESRAM_CTL		(SHARESRAM_BASE + 0x0004)

#define DMM_BASE		0xE0290000
#define DMM_AXI_DE_PRIORITY	(DMM_BASE + 0x0074)
#define DMM_AXI_NORMAL_PRIORITY	(DMM_BASE + 0x002C)

/*===================================================================
 *			S900 DE path
 *==================================================================*/

static int de_s900_path_enable(struct owl_de_path *path, bool enable)
{
	uint32_t val;

	debug("%s: path %d enable %d\n", __func__, path->id, enable);

	val = de_readl(DE_PATH_EN(path->id));
	val = REG_SET_VAL(val, enable, DE_PATH_ENABLE_BIT, DE_PATH_ENABLE_BIT);
	de_writel(val, DE_PATH_EN(path->id));

	return 0;
}

static void __path_display_type_set(struct owl_de_path *path,
				enum owl_display_type type)
{
	uint32_t val;
	uint32_t path2_clk_src;		/* only for ATM9009 */

	debug("%s: path %d type %d\n", __func__, path->id, type);

	if (type != OWL_DISPLAY_TYPE_HDMI) {
		val = de_readl(DE_OUTPUT_CON);

		path2_clk_src = readl(CMU_DECLK);
		path2_clk_src &= ~(0x3 << 16);

		switch (type) {
		case OWL_DISPLAY_TYPE_LCD:
			val = REG_SET_VAL(val, 0,
					  DE_OUTPUT_PATH2_DEVICE_END_BIT,
					  DE_OUTPUT_PATH2_DEVICE_BEGIN_BIT);
			path2_clk_src |= (0 << 16);
			break;
		case OWL_DISPLAY_TYPE_DSI:
			val = REG_SET_VAL(val, 1,
					  DE_OUTPUT_PATH2_DEVICE_END_BIT,
					  DE_OUTPUT_PATH2_DEVICE_BEGIN_BIT);
			path2_clk_src |= (1 << 16);
			break;
		case OWL_DISPLAY_TYPE_EDP:
			val = REG_SET_VAL(val, 2,
					  DE_OUTPUT_PATH2_DEVICE_END_BIT,
					  DE_OUTPUT_PATH2_DEVICE_BEGIN_BIT);
			path2_clk_src |= (2 << 16);
			break;
		default:
			BUG();
			break;
		}

		de_writel(val, DE_OUTPUT_CON);
		writel(path2_clk_src, CMU_DECLK);
	}
}

static void __path_size_set(struct owl_de_path *path,
			uint32_t width, uint32_t height)
{
	uint32_t val;

	debug("%s: path %d %dx%d\n", __func__, path->id, width, height);

	BUG_ON((width > DE_PATH_SIZE_WIDTH) || (height > DE_PATH_SIZE_HEIGHT));

	val = REG_VAL(height - 1, DE_PATH_SIZE_HEIGHT_END_BIT,
			  DE_PATH_SIZE_HEIGHT_BEGIN_BIT)
		| REG_VAL(width - 1, DE_PATH_SIZE_WIDTH_END_BIT,
			  DE_PATH_SIZE_WIDTH_BEGIN_BIT);

	de_writel(val, DE_PATH_SIZE(path->id));
}

static void __path_dither_enable(struct owl_de_path *path, bool enable)
{
	uint32_t val;

	debug("%s: path %d, enable %d\n", __func__, path->id, enable);

	/* only valid for LCD, TODO */
	if (path->id != 1)
		return;

	val = de_readl(DE_PATH_DITHER);
	val = REG_SET_VAL(val, enable, DE_PATH_DITHER_ENABLE_BIT,
			  DE_PATH_DITHER_ENABLE_BIT);
	de_writel(val, DE_PATH_DITHER);
}

static void __path_dither_set(struct owl_de_path *path,
			      enum owl_dither_mode mode)
{
	uint32_t val;

	debug("%s: path %d, mode %d\n", __func__, path->id, mode);

	if (path->id != 1)	/* TODO */
		return;

	if (mode == DITHER_DISABLE) {
		__path_dither_enable(path, false);
		return;
	}

	val = de_readl(DE_PATH_DITHER);

	switch (mode) {
	case DITHER_24_TO_18:
		val = REG_SET_VAL(val, 0, DE_PATH_DITHER_MODE_BIT,
				  DE_PATH_DITHER_MODE_BIT);
		break;

	case DITHER_24_TO_16:
		val = REG_SET_VAL(val, 1, DE_PATH_DITHER_MODE_BIT,
				  DE_PATH_DITHER_MODE_BIT);
		break;

	default:
		return;
	}

	de_writel(val, DE_PATH_DITHER);

	__path_dither_enable(path, true);
}

static void __path_default_color_set(struct owl_de_path *path, uint32_t color)
{
	debug("%s: path %d color %x\n", __func__, path->id, color);

	de_writel(color, DE_PATH_BK(path->id));
}

static void de_s900_path_apply_info(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;

	debug("%s: path%d\n", __func__, path->id);

	__path_display_type_set(path, info->type);

	__path_size_set(path, info->width, info->height);

	__path_dither_set(path, info->dither_mode);

	/* for test */
	__path_default_color_set(path, 0x0);
}

static void de_s900_path_set_go(struct owl_de_path *path)
{
	uint32_t val;

	debug("%s: path%d\n", __func__, path->id);

	val = de_readl(DE_PATH_FCR(path->id));
	val = REG_SET_VAL(val, 1, DE_PATH_FCR_BIT, DE_PATH_FCR_BIT);

	de_writel(val, DE_PATH_FCR(path->id));
}

static void __path_gamma_set(struct owl_de_path *path, uint32_t *gamma_val)
{
	bool is_busy;
	uint32_t idx, val;

	/* write operation mode(1 for write) */
	val = de_readl(DE_PATH_GAMMA_IDX(path->id));
	val = REG_SET_VAL(val, 1, DE_PATH_GAMMA_IDX_OP_SEL_END_BIT,
			DE_PATH_GAMMA_IDX_OP_SEL_BEGIN_BIT);

	for (idx = 0; idx < (256 * 3 / 4); idx++) {
		/* write index */
		val = REG_SET_VAL(val, idx, DE_PATH_GAMMA_IDX_INDEX_END_BIT,
				DE_PATH_GAMMA_IDX_INDEX_BEGIN_BIT);

		de_writel(val, DE_PATH_GAMMA_IDX(path->id));

		/* write ram */
		de_writel(gamma_val[idx], DE_PATH_GAMMA_RAM(path->id));

		/* wait for busy bit */
		do {
			val = de_readl(DE_PATH_GAMMA_IDX(path->id));
			is_busy = REG_GET_VAL(val, DE_PATH_GAMMA_IDX_BUSY_BIT,
					DE_PATH_GAMMA_IDX_BUSY_BIT);
		} while (is_busy);
	}

	/* write finish, clear write bit and index */
	val = de_readl(DE_PATH_GAMMA_IDX(path->id));
	val = REG_SET_VAL(val, 0, DE_PATH_GAMMA_IDX_INDEX_END_BIT,
					DE_PATH_GAMMA_IDX_INDEX_BEGIN_BIT);
	val = REG_SET_VAL(val, 0, DE_PATH_GAMMA_IDX_OP_SEL_END_BIT,
				DE_PATH_GAMMA_IDX_OP_SEL_BEGIN_BIT);

	de_writel(val, DE_PATH_GAMMA_IDX(path->id));
}
static void de_s900_path_set_gamma_table(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;
	uint8_t gamma_data[256 * 3];
	int i = 0;

	debug("%s, path%d\n", __func__, path->id);

	debug("gamma_r %d, gamma_g %d, gamma_b %d\n",
		info->gamma_r_val, info->gamma_g_val, info->gamma_b_val);

	/* only valid for LCD, TODO */
	if (path->id != 1)
		return;

	/* R */
	for (i = 0; i < 256; i++) {
		gamma_data[i] = i * info->gamma_r_val / 100;
	}

	/* G */
	for (i = 0; i < 256; i++) {
		gamma_data[i + 256] = i * info->gamma_g_val / 100;
	}

	/* B */
	for (i = 0; i < 256; i++) {
		gamma_data[i + 256 * 2] = i * info->gamma_b_val / 100;
	}

	__path_gamma_set(path, &gamma_data[0]);

	debug("%s, ok!\n", __func__);
}

static void de_s900_path_get_gamma_table(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;

	debug("%s, path%d\n", __func__, path->id);
}

static int de_s900_path_gamma_enable(struct owl_de_path *path, bool enable)
{
	uint32_t val;

	/* only valid for LCD, TODO */
	if (path->id != 1)
		return;

	debug("%s, enable %d\n", __func__, enable);

	val = de_readl(DE_PATH_GAMMA_ENABLE(path->id));

	val = REG_SET_VAL(val, enable, DE_PATH_GAMMA_ENABLE_BIT,
			DE_PATH_GAMMA_ENABLE_BIT);

	de_writel(val, DE_PATH_GAMMA_ENABLE(path->id));

}
static struct owl_de_path_ops de_s900_path_ops = {
	.enable = de_s900_path_enable,
	.apply_info = de_s900_path_apply_info,
	.set_go = de_s900_path_set_go,

	.set_gamma_table = de_s900_path_set_gamma_table,
	.get_gamma_table = de_s900_path_get_gamma_table,
	.gamma_enable = de_s900_path_gamma_enable,

};

static struct owl_de_path de_s900_paths[] = {
	{
		.id			= 0,
		.name			= "digit",
		.supported_displays	= OWL_DISPLAY_TYPE_HDMI,
		.ops			= &de_s900_path_ops,
	},
	{
		.id			= 1,
		.name			= "lcd",
		.supported_displays	= OWL_DISPLAY_TYPE_LCD
					| OWL_DISPLAY_TYPE_DSI
					| OWL_DISPLAY_TYPE_EDP
					| OWL_DISPLAY_TYPE_DUMMY,
		.ops			= &de_s900_path_ops,
	},
};


/*===================================================================
 *			S900 DE video layer
 *==================================================================*/
static int de_s900_video_enable(struct owl_de_video *video, bool enable)
{
	uint32_t val;

	debug("%s: %d\n", __func__, enable);

	if (video->path == NULL) {
		error("set a path before enable/disable\n");
		return -1;
	}

	if (video->path->info.type == OWL_DISPLAY_TYPE_DUMMY)
		return 0;

	val = de_readl(DE_PATH_CTL(video->path->id));
	val = REG_SET_VAL(val, enable,
			  DE_PATH_VIDEO_ENABLE_BEGIN_BIT + video->id,
			  DE_PATH_VIDEO_ENABLE_BEGIN_BIT + video->id);
	de_writel(val, DE_PATH_CTL(video->path->id));

	return 0;
}

static int __de_color_mode_to_hw_mode(enum owl_color_mode color_mode)
{
	int hw_format = 0;
	/*
	 * NOTE: R G B/Y U V is inversed between
	 * de color mode and datasheet!!
	 */
	switch (color_mode) {
	case OWL_DSS_COLOR_RGB16:
		hw_format = 0;
		break;
	case OWL_DSS_COLOR_BGR16:
		hw_format = 1;
		break;
	case OWL_DSS_COLOR_RGBA32:
		hw_format = 5;
		break;
	case OWL_DSS_COLOR_BGRA32:
		hw_format = 4;
		break;

	case OWL_DSS_COLOR_ABGR32:
		hw_format = 6;
		break;

	case OWL_DSS_COLOR_ARGB32:
		hw_format = 7;
		break;

	default:
		BUG();
		break;
	}

	return hw_format;
}

static void __video_format_set(struct owl_de_video *video,
			       enum owl_color_mode color_mode)
{
	uint32_t val;
	int hw_format = 0;

	hw_format = __de_color_mode_to_hw_mode(color_mode);
	debug("%s: color_mode = 0x%x, hw_format = 0x%x\n",
	      __func__, color_mode, hw_format);

	val = de_readl(DE_OVL_CFG(video->id));

	val = REG_SET_VAL(val, hw_format, DE_OVL_CFG_FMT_END_BIT,
			  DE_OVL_CFG_FMT_BEGIN_BIT);

	de_writel(val, DE_OVL_CFG(video->id));
}

static void __video_rotate_set(struct owl_de_video *video, int rotation)
{
	uint32_t val;

	debug("%s: video %d, rotation %d\n", __func__, video->id, rotation);

	val = de_readl(DE_OVL_CFG(video->id));

	val = REG_SET_VAL(val, rotation, 21, 20);
	de_writel(val, DE_OVL_CFG(video->id));
}

static void __video_isize_set(struct owl_de_video *video,
			      uint32_t width, uint32_t height)
{
	uint32_t val;

	debug("%s: video %d, %dx%d\n", __func__, video->id, width, height);

	BUG_ON((width > DE_PATH_SIZE_WIDTH) || (height > DE_PATH_SIZE_HEIGHT));

	val = REG_VAL(height - 1, DE_PATH_SIZE_HEIGHT_END_BIT,
		      DE_PATH_SIZE_HEIGHT_BEGIN_BIT)
		| REG_VAL(width - 1, DE_PATH_SIZE_WIDTH_END_BIT,
		      DE_PATH_SIZE_WIDTH_BEGIN_BIT);

	de_writel(val, DE_OVL_ISIZE(video->id));
}

static void __video_osize_set(struct owl_de_video *video,
			      uint32_t width, uint32_t height)
{
	uint32_t val;

	debug("%s: video %d, %dx%d\n", __func__, video->id, width, height);

	BUG_ON((width > DE_PATH_SIZE_WIDTH) || (height > DE_PATH_SIZE_HEIGHT));

	val = REG_VAL(height - 1, DE_PATH_SIZE_HEIGHT_END_BIT,
		      DE_PATH_SIZE_HEIGHT_BEGIN_BIT)
		| REG_VAL(width - 1, DE_PATH_SIZE_WIDTH_END_BIT,
		      DE_PATH_SIZE_WIDTH_BEGIN_BIT);

	de_writel(val, DE_OVL_OSIZE(video->id));
}

static void __video_position_set(struct owl_de_path *path,
				 struct owl_de_video *video,
				 uint32_t x_pos, uint32_t y_pos)
{
	uint32_t val;

	debug("%s: video %d, (%d, %d)\n", __func__, video->id, x_pos, y_pos);

	BUG_ON((x_pos > DE_PATH_SIZE_WIDTH) || (y_pos > DE_PATH_SIZE_HEIGHT));

	val = REG_VAL(y_pos, DE_PATH_SIZE_HEIGHT_END_BIT,
		      DE_PATH_SIZE_HEIGHT_BEGIN_BIT)
		| REG_VAL(x_pos, DE_PATH_SIZE_WIDTH_END_BIT,
		      DE_PATH_SIZE_WIDTH_BEGIN_BIT);

	de_writel(val, DE_OVL_COOR(path->id, video->id));
}

static void __video_set_scal_coef(struct owl_de_video *video,
				  uint8_t scale_mode)
{
	debug("%s: video %d, scale_mode %d\n", __func__,
	      video->id, scale_mode);

	switch (scale_mode) {
	case DE_SCLCOEF_ZOOMIN:
		de_writel(0x00004000, DE_OVL_SCOEF0(video->id));
		de_writel(0xFF073EFC, DE_OVL_SCOEF1(video->id));
		de_writel(0xFE1038FA, DE_OVL_SCOEF2(video->id));
		de_writel(0xFC1B30F9, DE_OVL_SCOEF3(video->id));
		de_writel(0xFA2626FA, DE_OVL_SCOEF4(video->id));
		de_writel(0xF9301BFC, DE_OVL_SCOEF5(video->id));
		de_writel(0xFA3810FE, DE_OVL_SCOEF6(video->id));
		de_writel(0xFC3E07FF, DE_OVL_SCOEF7(video->id));
		break;

	case DE_SCLCOEF_HALF_ZOOMOUT:
		de_writel(0x00004000, DE_OVL_SCOEF0(video->id));
		de_writel(0x00083800, DE_OVL_SCOEF1(video->id));
		de_writel(0x00103000, DE_OVL_SCOEF2(video->id));
		de_writel(0x00182800, DE_OVL_SCOEF3(video->id));
		de_writel(0x00202000, DE_OVL_SCOEF4(video->id));
		de_writel(0x00281800, DE_OVL_SCOEF5(video->id));
		de_writel(0x00301000, DE_OVL_SCOEF6(video->id));
		de_writel(0x00380800, DE_OVL_SCOEF7(video->id));
		break;

	case DE_SCLCOEF_SMALLER_ZOOMOUT:
		de_writel(0x00102010, DE_OVL_SCOEF0(video->id));
		de_writel(0x02121E0E, DE_OVL_SCOEF1(video->id));
		de_writel(0x04141C0C, DE_OVL_SCOEF2(video->id));
		de_writel(0x06161A0A, DE_OVL_SCOEF3(video->id));
		de_writel(0x08181808, DE_OVL_SCOEF4(video->id));
		de_writel(0x0A1A1606, DE_OVL_SCOEF5(video->id));
		de_writel(0x0C1C1404, DE_OVL_SCOEF6(video->id));
		de_writel(0x0E1E1202, DE_OVL_SCOEF7(video->id));
		break;

	default:
		BUG();
	}
}

static void __video_scaling_set(struct owl_de_video *video,
				uint32_t width, uint32_t height,
				uint32_t out_width, uint32_t out_height)
{
	uint8_t scale_mode;
	uint16_t w_factor, h_factor;
	uint16_t factor;
	uint32_t val = 0;

	debug("%s: video %d, %dx%d->%dx%d\n", __func__, video->id,
	      width, height, out_width, out_height);

	w_factor = (width * 8192 +  out_width - 1) / out_width;
	h_factor = (height * 8192 + out_height - 1) / out_height;

	val = REG_SET_VAL(val, h_factor, 31, 16);
	val |= REG_SET_VAL(val, w_factor, 15, 0);

	de_writel(val, DE_OVL_SR(video->id));

	factor = (width * height * 10) / (out_width * out_height);
	if (factor <= 10)
		scale_mode = DE_SCLCOEF_ZOOMIN;
	else if (factor <= 20)
		scale_mode = DE_SCLCOEF_HALF_ZOOMOUT;
	else if (factor > 20)
		scale_mode = DE_SCLCOEF_SMALLER_ZOOMOUT;

	__video_set_scal_coef(video, scale_mode);
}

static void __video_alpha_set(struct owl_de_path *path,
			      struct owl_de_video *video,
			      enum owl_blending_type blending, uint8_t alpha)
{
	/* TODO */
	de_writel(0x1ff, DE_OVL_ALPHA_CFG(path->id, video->id));

	debug("%s: DE_OVL_ALPHA_CFG %x\n",
	      __func__, de_readl(DE_OVL_ALPHA_CFG(path->id, video->id)));
}

static void de_s900_video_apply_info(struct owl_de_video *video)
{
	uint16_t outw, outh;
	struct owl_de_video_info *info = &video->info;

	debug("%s: video %d, dirty %d\n",
	      __func__, video->id, video->info.dirty);

	outw = (info->out_width == 0 ? info->width : info->out_width);
	outh = (info->out_height == 0 ? info->height : info->out_height);

	/* fb addr, obly fb0 is used */
	de_writel(info->addr[0], DE_OVL_BA0(video->id));

	/* stride set, only for RGB now */
	de_writel(info->pitch[0] / 8, DE_OVL_STR(video->id));

	__video_format_set(video, info->color_mode);
	__video_rotate_set(video, info->rotate);
	__video_isize_set(video, info->width, info->height);
	__video_osize_set(video, outw, outh);
	__video_scaling_set(video, info->width, info->height, outw, outh);
	__video_position_set(video->path, video, info->pos_x, info->pos_y);

	debug("%s: blending %d, alpha %d\n", __func__,
	      info->blending, info->alpha);
	__video_alpha_set(video->path, video, info->blending, info->alpha);
}

static struct owl_de_video_ops de_s900_video_ops = {
	.enable = de_s900_video_enable,
	.apply_info = de_s900_video_apply_info,
};


#define ATM9009_SUPPORTED_COLORS (OWL_DSS_COLOR_RGB16 | OWL_DSS_COLOR_BGR16 \
	| OWL_DSS_COLOR_ARGB32 | OWL_DSS_COLOR_ABGR32 \
	| OWL_DSS_COLOR_RGBA32 | OWL_DSS_COLOR_BGRA32 \
	| OWL_DSS_COLOR_YU12 | OWL_DSS_COLOR_NV12 | OWL_DSS_COLOR_NV21 \
	| OWL_DSS_COLOR_ARGB16 | OWL_DSS_COLOR_ABGR16 \
	| OWL_DSS_COLOR_RGBA16 | OWL_DSS_COLOR_BGRA16)

/* need 2 video layers at most in boot */
static struct owl_de_video de_s900_videos[] = {
	{
		.id			= 0,
		.name			= "video0",
		.supported_colors	= ATM9009_SUPPORTED_COLORS,
		.ops			= &de_s900_video_ops,
	},
	{
		.id			= 1,
		.name			= "video1",
		.supported_colors	= ATM9009_SUPPORTED_COLORS,
		.ops			= &de_s900_video_ops,
	},
};


/*===================================================================
 *			S900 DE video layer
 *==================================================================*/

static int de_s900_device_power_on(struct owl_de_device *de)
{
	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_DE);

	/* power on */
	owl_powergate_power_on(POWERGATE_DE);

	/*
	 * DECLK:
	 * source is ASSIST_PLL(500MHz),
	 * DE_CLK1/DE_CLK2/DE_CLK3, divider is 1, 500MHz
	 */
	writel(0x0, CMU_DECLK);

	/* enable declk from devpll */
	owl_clk_enable(CLOCK_DE);
	mdelay(1);

	/* de-assert reset */
	owl_reset_deassert(RESET_DE);
	mdelay(1);

	debug("%s: end\n", __func__);
	return 0;
}


static int de_s900_device_init(struct owl_de_device *de)
{
	uint32_t val;

	val = readl(SHARESRAM_CTL); /* share mem */
	val |= 0x3;  /* tshi, ebox hdmi use 1 bit, pad edp use 0 bit. */
	writel(val, SHARESRAM_CTL);

	/*
	 * some special init, must be same as kernel, which is located at
	 * "drivers/video/owl/dss/de_atm9009.c"
	 */
	de_writel(0x00001f1f, DE_MAX_OUTSTANDING);
	de_writel(0xcccc, DE_QOS);

	writel(0x0, DMM_AXI_DE_PRIORITY);
	val = readl(DMM_AXI_NORMAL_PRIORITY);
	val &= ~(1 << 4);
	writel(val, DMM_AXI_NORMAL_PRIORITY);
	val |= (1 << 4);
	writel(val, DMM_AXI_NORMAL_PRIORITY);

	return 0;
}

static void de_s900_device_dump_regs(struct owl_de_device *de)
{
	int i = 0;

#define DUMPREG(r) printf("%08x ~~ %08x ~~ %s\n", r, de_readl(r), #r)

	DUMPREG(DE_IRQSTATUS);
	DUMPREG(DE_IRQENABLE);
	DUMPREG(DE_MAX_OUTSTANDING);
	DUMPREG(DE_MMU_EN);
	DUMPREG(DE_MMU_BASE);
	DUMPREG(DE_OUTPUT_CON);
	DUMPREG(DE_OUTPUT_STAT);
	DUMPREG(DE_PATH_DITHER);

	for (i = 0 ; i < 2 ; i++) {
		printf("\npath %d ------------------>\n", i);
		DUMPREG(DE_PATH_CTL(i));
		DUMPREG(DE_PATH_FCR(i));
		DUMPREG(DE_PATH_EN(i));
		DUMPREG(DE_PATH_BK(i));
		DUMPREG(DE_PATH_SIZE(i));
		DUMPREG(DE_PATH_GAMMA_IDX(i));
		DUMPREG(DE_PATH_GAMMA_RAM(i));
	}
	for (i = 0 ; i < 4 ; i++) {
		printf("\nlayer %d ------------------>\n", i);
		DUMPREG(DE_OVL_CFG(i));
		DUMPREG(DE_OVL_ISIZE(i));
		DUMPREG(DE_OVL_OSIZE(i));
		DUMPREG(DE_OVL_SR(i));
		DUMPREG(DE_OVL_SCOEF0(i));
		DUMPREG(DE_OVL_SCOEF1(i));
		DUMPREG(DE_OVL_SCOEF2(i));
		DUMPREG(DE_OVL_SCOEF3(i));
		DUMPREG(DE_OVL_SCOEF4(i));
		DUMPREG(DE_OVL_SCOEF5(i));
		DUMPREG(DE_OVL_SCOEF6(i));
		DUMPREG(DE_OVL_SCOEF7(i));
		DUMPREG(DE_OVL_BA0(i));
		DUMPREG(DE_OVL_BA1UV(i));
		DUMPREG(DE_OVL_BA2V(i));
		DUMPREG(DE_OVL_3D_RIGHT_BA0(i));
		DUMPREG(DE_OVL_3D_RIGHT_BA1UV(i));
		DUMPREG(DE_OVL_3D_RIGHT_BA2V(i));

		DUMPREG(DE_OVL_STR(i));
		DUMPREG(DE_OVL_CRITICAL_CFG(i));
		DUMPREG(DE_OVL_REMAPPING(i));
		DUMPREG(DE_OVL_CSC(i));
		DUMPREG(DE_OVL_COOR(0, i));
		DUMPREG(DE_OVL_COOR(1, i));
		DUMPREG(DE_OVL_ALPHA_CFG(0, i));
		DUMPREG(DE_OVL_ALPHA_CFG(1, i));
	}
#undef DUMPREG
}

static struct owl_de_device_ops de_s900_device_ops = {
	.power_on = de_s900_device_power_on,
	.init = de_s900_device_init,
	.dump_regs = de_s900_device_dump_regs,
};

static struct owl_de_device owl_de_s9009 = {
	.hw_id			= DE_HW_ID_S900,

	.num_paths		= 2,
	.paths			= de_s900_paths,

	.num_videos		= 2,
	.videos			= de_s900_videos,

	.ops			= &de_s900_device_ops,
};


/*============================================================================
 *			register to DE core
 *==========================================================================*/

int owl_de_s900_init(const void *blob)
{
	int node;

	node = fdt_node_offset_by_compatible(blob, 0, "actions,s900-de");
	if (node < 0) {
		debug("%s: no match in DTS\n", __func__);
		return -1;
	}
	debug("%s\n", __func__);

	owl_de_s9009.blob = blob;
	owl_de_s9009.node = node;

	owl_de_register(&owl_de_s9009);

	return 0;
}
