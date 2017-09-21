/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Lipeng<lipeng@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUGX
#define pr_fmt(fmt) "de_s700: " fmt

#include <common.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/arch/powergate.h>

#include <dss.h>

#include "de_s700.h"

static struct owl_de_device	owl_de_s700;

#define de_readl(idx)		readl(owl_de_s700.base + (idx))
#define de_writel(val, idx)	writel(val, owl_de_s700.base + (idx))

/*===================================================================
 *			S700 DE path
 *==================================================================*/

static int de_s700_path_enable(struct owl_de_path *path, bool enable)
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

	debug("%s: path %d type %d\n", __func__, path->id, type);

	val = de_readl(DE_OUTPUT_CON);

	if (path->id == 1) {
		/* LCD */
		switch (type) {
		case OWL_DISPLAY_TYPE_LCD:
			val = REG_SET_VAL(val, 1,
					  DE_OUTPUT_PATH2_DEVICE_END_BIT,
					  DE_OUTPUT_PATH2_DEVICE_BEGIN_BIT);
			break;
		case OWL_DISPLAY_TYPE_DSI:
			val = REG_SET_VAL(val, 0,
					  DE_OUTPUT_PATH2_DEVICE_END_BIT,
					  DE_OUTPUT_PATH2_DEVICE_BEGIN_BIT);
			break;
		default:
			BUG();
			break;
		}
	} else {
		/* digit */
		switch (type) {
		case OWL_DISPLAY_TYPE_HDMI:
			val = REG_SET_VAL(val, 0,
					  DE_OUTPUT_PATH1_DEVICE_END_BIT,
					  DE_OUTPUT_PATH1_DEVICE_BEGIN_BIT);
			break;
		case OWL_DISPLAY_TYPE_CVBS:
			val = REG_SET_VAL(val, 1,
					  DE_OUTPUT_PATH1_DEVICE_END_BIT,
					  DE_OUTPUT_PATH1_DEVICE_BEGIN_BIT);
			break;
		default:
			BUG();
			break;
		}
	}

	de_writel(val, DE_OUTPUT_CON);
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
		val = REG_SET_VAL(val, 0, DE_PATH_DITHER_MODE_END_BIT,
				  DE_PATH_DITHER_MODE_BEGIN_BIT);
		break;

	case DITHER_24_TO_16:
		val = REG_SET_VAL(val, 1, DE_PATH_DITHER_MODE_END_BIT,
				  DE_PATH_DITHER_MODE_BEGIN_BIT);
		break;

	default:
		return;
	}

	de_writel(val, DE_PATH_DITHER);

	__path_dither_enable(path, true);
}
static void __path_output_format_set(struct owl_de_path *path, uint32_t is_yuv)
{
#define  format(x) (x == 1 ? "yuv" : "rgb")
	uint32_t val;
	debug("%s: path %d format %s\n", __func__, path->id, format(is_yuv));

	val = de_readl(DE_PATH_CTL(path->id));
	if (is_yuv == DE_OUTPUT_FORMAT_YUV)
		val = REG_SET_VAL(val, 1, DE_PATH_CTL_RGB_YUV_EN_BIT,
				  DE_PATH_CTL_RGB_YUV_EN_BIT);
	else
		val = REG_SET_VAL(val, 0, DE_PATH_CTL_RGB_YUV_EN_BIT,
				  DE_PATH_CTL_RGB_YUV_EN_BIT);
	de_writel(val, DE_PATH_CTL(path->id));

	val = de_readl(DE_PATH_CTL(path->id));
}

static void __path_vmode_set(struct owl_de_path *path, uint32_t vmode)
{
	uint32_t val;
	debug("%s: path %d vmode %d\n", __func__, path->id, vmode);

	val = de_readl(DE_PATH_CTL(path->id));
	if (vmode == DSS_VMODE_INTERLACED)
		val = REG_SET_VAL(val, 1, DE_PATH_CTL_ILACE_BIT,
				  DE_PATH_CTL_ILACE_BIT);
	else
		val = REG_SET_VAL(val, 0, DE_PATH_CTL_ILACE_BIT,
				  DE_PATH_CTL_ILACE_BIT);
	de_writel(val, DE_PATH_CTL(path->id));

	val = de_readl(DE_PATH_CTL(path->id));
}

static void __path_default_color_set(struct owl_de_path *path, uint32_t color)
{
	debug("%s: path %d color %x\n", __func__, path->id, color);

	de_writel(color, DE_PATH_BK(path->id));
}

static void de_s700_path_apply_info(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;

	debug("%s: path%d\n", __func__, path->id);

	__path_display_type_set(path, info->type);

	__path_size_set(path, info->width, info->height);

	__path_dither_set(path, info->dither_mode);

	__path_vmode_set(path, info->vmode);

	if (info->type == OWL_DISPLAY_TYPE_CVBS)
		__path_output_format_set(path, DE_OUTPUT_FORMAT_YUV);
	else
		__path_output_format_set(path, DE_OUTPUT_FORMAT_RGB);
	/* for test */
	__path_default_color_set(path, 0x0);
}

static void de_s700_path_set_go(struct owl_de_path *path)
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

	for (idx = 0; idx < (256 * 3 / 4); idx++) {
		/* write index */
		val = REG_SET_VAL(val, idx, DE_PATH_GAMMA_IDX_INDEX_END_BIT,
				DE_PATH_GAMMA_IDX_INDEX_BEGIN_BIT);
		de_writel(val, DE_PATH_GAMMA_IDX(path->id));

		/* write ram */
		de_writel(gamma_val[idx], DE_PATH_GAMMA_RAM(path->id));

	}

	/* write finish, clear write bit and index */
	val = de_readl(DE_PATH_GAMMA_IDX(path->id));
	val = REG_SET_VAL(val, 0, DE_PATH_GAMMA_IDX_INDEX_END_BIT,
					DE_PATH_GAMMA_IDX_INDEX_BEGIN_BIT);
	de_writel(val, DE_PATH_GAMMA_IDX(path->id));
}

static void de_s700_path_set_gamma_table(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;
	uint8_t gamma_data[256 * 3];
	int i = 0;

	debug("%s, path%d, gamma_r %d, gamma_g %d, gamma_b %d\n",
		__func__, path->id, info->gamma_r_val, info->gamma_g_val, info->gamma_b_val);

	if ((info->gamma_r_val < 0) ||
		(info->gamma_g_val < 0) || (info->gamma_b_val < 0))
		error("%s, unavailable gamma val!", __func__);

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

static void de_s700_path_get_gamma_table(struct owl_de_path *path)
{
	struct owl_de_path_info *info = &path->info;

	debug("%s, path%d\n", __func__, path->id);
}

static int de_s700_path_gamma_enable(struct owl_de_path *path, bool enable)
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

static struct owl_de_path_ops de_s700_path_ops = {
	.enable = de_s700_path_enable,
	.apply_info = de_s700_path_apply_info,
	.set_go = de_s700_path_set_go,

	.set_gamma_table = de_s700_path_set_gamma_table,
	.get_gamma_table = de_s700_path_get_gamma_table,
	.gamma_enable = de_s700_path_gamma_enable,
};

static struct owl_de_path de_s700_paths[] = {
	{
		.id			= 0,
		.name			= "digit",
		.supported_displays	= OWL_DISPLAY_TYPE_HDMI
					| OWL_DISPLAY_TYPE_CVBS,
		.ops			= &de_s700_path_ops,
	},
	{
		.id			= 1,
		.name			= "lcd",
		.supported_displays	= OWL_DISPLAY_TYPE_LCD
					| OWL_DISPLAY_TYPE_DSI
					| OWL_DISPLAY_TYPE_DUMMY,
		.ops			= &de_s700_path_ops,
	},
};


/*===================================================================
 *			S700 DE video layer
 *==================================================================*/
static int de_s700_video_enable(struct owl_de_video *video, bool enable)
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
			  DE_PATH_ML_EN_BEGIN_BIT + video->id,
			  DE_PATH_ML_EN_BEGIN_BIT + video->id);
	de_writel(val, DE_PATH_CTL(video->path->id));

	/* now only sublayer0 is used */
	val = de_readl(DE_ML_CFG(video->id));
	val = REG_SET_VAL(val, enable ? 1 : 0, 0, 0);
	de_writel(val, DE_ML_CFG(video->id));

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
		hw_format = 9;
		break;
	case OWL_DSS_COLOR_BGR16:
		hw_format = 8;
		break;
	case OWL_DSS_COLOR_RGBA32:
		hw_format = 1;
		break;
	case OWL_DSS_COLOR_BGRA32:
		hw_format = 0;
		break;

	case OWL_DSS_COLOR_ABGR32:
		hw_format = 2;
		break;

	case OWL_DSS_COLOR_ARGB32:
		hw_format = 3;
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

	val = de_readl(DE_SL_CFG(video->id, 0));

	val = REG_SET_VAL(val, hw_format, DE_SL_CFG_FMT_END_BIT,
			  DE_SL_CFG_FMT_BEGIN_BIT);

	de_writel(val, DE_SL_CFG(video->id, 0));
}

static void __video_rotate_set(struct owl_de_video *video, int rotation)
{
	uint32_t val;

	debug("%s: video %d, rotation %d\n", __func__, video->id, rotation);

	if (rotation != 0 && rotation != 3)
		return;

	val = de_readl(DE_ML_CFG(video->id));

	val = REG_SET_VAL(val, (rotation == 0 ? 0 : 1),
			  DE_ML_ROT180_BIT, DE_ML_ROT180_BIT);
	de_writel(val, DE_ML_CFG(video->id));
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

	de_writel(val, DE_ML_ISIZE(video->id));
	de_writel(val, DE_SL_CROPSIZE(video->id, 0));
}

static void __video_osize_set(struct owl_de_video *video,
			      uint32_t width, uint32_t height)
{
	/*
	 * nothing need do, S700's output size should set in scaler,
	 * please see __video_scaling_set
	 */
	debug("%s, video %d, %dx%d\n", __func__, video->id, width, height);
	BUG_ON((width > DE_PATH_SIZE_WIDTH) || (height > DE_PATH_SIZE_HEIGHT));
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

	de_writel(val, DE_PATH_COOR(path->id, video->id));
	de_writel(0, DE_SL_COOR(video->id, 0));
}

static void __video_set_scal_coef(uint8_t scaler_id, uint8_t scale_mode)
{
	debug("%s, scaler_id %d, scale_mode %d\n",
	      __func__, scaler_id, scale_mode);

	switch (scale_mode) {
	case DE_SCLCOEF_ZOOMIN:
		de_writel(0x00004000, DE_SCALER_SCOEF0(scaler_id));
		de_writel(0xFF073EFC, DE_SCALER_SCOEF1(scaler_id));
		de_writel(0xFE1038FA, DE_SCALER_SCOEF2(scaler_id));
		de_writel(0xFC1B30F9, DE_SCALER_SCOEF3(scaler_id));
		de_writel(0xFA2626FA, DE_SCALER_SCOEF4(scaler_id));
		de_writel(0xF9301BFC, DE_SCALER_SCOEF5(scaler_id));
		de_writel(0xFA3810FE, DE_SCALER_SCOEF6(scaler_id));
		de_writel(0xFC3E07FF, DE_SCALER_SCOEF7(scaler_id));
		break;

	case DE_SCLCOEF_HALF_ZOOMOUT:
		de_writel(0x00004000, DE_SCALER_SCOEF0(scaler_id));
		de_writel(0x00083800, DE_SCALER_SCOEF1(scaler_id));
		de_writel(0x00103000, DE_SCALER_SCOEF2(scaler_id));
		de_writel(0x00182800, DE_SCALER_SCOEF3(scaler_id));
		de_writel(0x00202000, DE_SCALER_SCOEF4(scaler_id));
		de_writel(0x00281800, DE_SCALER_SCOEF5(scaler_id));
		de_writel(0x00301000, DE_SCALER_SCOEF6(scaler_id));
		de_writel(0x00380800, DE_SCALER_SCOEF7(scaler_id));
		break;

	case DE_SCLCOEF_SMALLER_ZOOMOUT:
		de_writel(0x00102010, DE_SCALER_SCOEF0(scaler_id));
		de_writel(0x02121E0E, DE_SCALER_SCOEF1(scaler_id));
		de_writel(0x04141C0C, DE_SCALER_SCOEF2(scaler_id));
		de_writel(0x06161A0A, DE_SCALER_SCOEF3(scaler_id));
		de_writel(0x08181808, DE_SCALER_SCOEF4(scaler_id));
		de_writel(0x0A1A1606, DE_SCALER_SCOEF5(scaler_id));
		de_writel(0x0C1C1404, DE_SCALER_SCOEF6(scaler_id));
		de_writel(0x0E1E1202, DE_SCALER_SCOEF7(scaler_id));
		break;

	default:
		BUG();
	}
}

static void __video_scaling_set(struct owl_de_video *video,
				uint32_t width, uint32_t height,
				uint32_t out_width, uint32_t out_height)
{
	uint8_t scaler_id, ml_id;
	uint8_t scale_mode;
	uint16_t w_factor, h_factor;
	uint16_t factor;
	uint32_t val = 0;

	debug("%s: video %d, %dx%d->%dx%d\n", __func__, video->id,
	      width, height, out_width, out_height);

	/*
	 * scaler0 is for macro layer 0.
	 * S700 not OTT(LCD+HDMI), scaler1 is for macro layer 3
	 * others, scaler1 is for macro layer 1
	 */
#if defined(CONFIG_VIDEO_OWL_DE_S700) && !defined(CONFIG_VIDEO_OWL_DE_S700_OTT)
	if (video->id != 0 && video->id != 3)
#else
	if (video->id > 1)
#endif
		return;

	ml_id = video->id;		/* get macro layer ID */
	scaler_id = (video->id == 0 ? 0 : 1);

	w_factor = (width * 8192 +  out_width - 1) / out_width;
	h_factor = (height * 8192 + out_height - 1) / out_height;

	de_writel(w_factor, DE_SCALER_HSR(scaler_id));
	de_writel(h_factor, DE_SCALER_VSR(scaler_id));

	factor = (width * height * 10) / (out_width * out_height);
	if (factor <= 10)
		scale_mode = DE_SCLCOEF_ZOOMIN;
	else if (factor <= 20)
		scale_mode = DE_SCLCOEF_HALF_ZOOMOUT;
	else if (factor > 20)
		scale_mode = DE_SCLCOEF_SMALLER_ZOOMOUT;

	__video_set_scal_coef(scaler_id, scale_mode);

	val = de_readl(DE_SCALER_CFG(scaler_id));

	/* select Macro Layer */
	val = REG_SET_VAL(val, ml_id, DE_SCALER_CFG_SEL_END_BIT,
			  DE_SCALER_CFG_SEL_BEGIN_BIT);

	/* enable scaler */
	val = REG_SET_VAL(val, 1, DE_SCALER_CFG_ENABLE_BIT,
			  DE_SCALER_CFG_ENABLE_BIT);

	de_writel(val, DE_SCALER_CFG(scaler_id));

	/* set outpust size after scaler is enabled */
	val = REG_VAL(out_height - 1, DE_PATH_SIZE_HEIGHT_END_BIT,
		      DE_PATH_SIZE_HEIGHT_BEGIN_BIT)
		| REG_VAL(out_width - 1, DE_PATH_SIZE_WIDTH_END_BIT,
		      DE_PATH_SIZE_WIDTH_BEGIN_BIT);
	de_writel(val, DE_SCALER_OSZIE(scaler_id));
}

static void __video_alpha_set(struct owl_de_video *video,
			      enum owl_blending_type blending, uint8_t alpha)
{
	uint32_t val;

	if (blending == OWL_BLENDING_NONE)
		alpha = 0xff;

	val = de_readl(DE_SL_CFG(video->id, 0));

	/* set global alpha value */
	val = REG_SET_VAL(val, alpha, DE_SL_CFG_GLOBAL_ALPHA_END_BIT,
			  DE_SL_CFG_GLOBAL_ALPHA_BEGIN_BIT);

	if (blending == OWL_BLENDING_COVERAGE)
		val = REG_SET_VAL(val, 1, DE_SL_CFG_DATA_MODE_BIT_BIT,
				  DE_SL_CFG_DATA_MODE_BIT_BIT);
	else
		val = REG_SET_VAL(val, 0, DE_SL_CFG_DATA_MODE_BIT_BIT,
				  DE_SL_CFG_DATA_MODE_BIT_BIT);

	de_writel(val, DE_SL_CFG(video->id, 0));
}

static void de_s700_video_apply_info(struct owl_de_video *video)
{
	uint16_t outw, outh;
	struct owl_de_video_info *info = &video->info;

	debug("%s: video %d, dirty %d\n",
	      __func__, video->id, video->info.dirty);

	outw = (info->out_width == 0 ? info->width : info->out_width);
	outh = (info->out_height == 0 ? info->height : info->out_height);

	/* fb addr, only fb0 is used */
	de_writel(info->addr[0], DE_SL_FB(video->id, 0));

	/* stride set, only for RGB now */
	de_writel(info->pitch[0], DE_SL_STR(video->id, 0));

	__video_format_set(video, info->color_mode);
	__video_rotate_set(video, info->rotate);
	__video_isize_set(video, info->width, info->height);
	__video_osize_set(video, outw, outh);
	__video_scaling_set(video, info->width, info->height, outw, outh);
	__video_position_set(video->path, video, info->pos_x, info->pos_y);

	debug("%s: blending %d, alpha %d\n", __func__,
	      info->blending, info->alpha);
	__video_alpha_set(video, info->blending, info->alpha);
}

static struct owl_de_video_ops de_s700_video_ops = {
	.enable = de_s700_video_enable,
	.apply_info = de_s700_video_apply_info,
};


#define S700_SUPPORTED_COLORS (OWL_DSS_COLOR_RGB16 | OWL_DSS_COLOR_BGR16 \
	| OWL_DSS_COLOR_ARGB32 | OWL_DSS_COLOR_ABGR32 \
	| OWL_DSS_COLOR_RGBA32 | OWL_DSS_COLOR_BGRA32 \
	| OWL_DSS_COLOR_YU12 | OWL_DSS_COLOR_NV12 | OWL_DSS_COLOR_NV21 \
	| OWL_DSS_COLOR_ARGB16 | OWL_DSS_COLOR_ABGR16 \
	| OWL_DSS_COLOR_RGBA16 | OWL_DSS_COLOR_BGRA16)

/* need 2 video layers at most in boot */
static struct owl_de_video de_s700_videos[] = {
	{
		.id			= 0,
		.name			= "video0",
		.supported_colors	= S700_SUPPORTED_COLORS,
		.ops			= &de_s700_video_ops,
	},
	{
		.id			= 1,
		.name			= "video1",
		.supported_colors	= S700_SUPPORTED_COLORS,
		.ops			= &de_s700_video_ops,
	},
	{
		.id			= 2,
		.name			= "video2",
		.supported_colors	= S700_SUPPORTED_COLORS,
		.ops			= &de_s700_video_ops,
	},
	{
		.id			= 3,
		.name			= "video3",
		.supported_colors	= S700_SUPPORTED_COLORS,
		.ops			= &de_s700_video_ops,
	},
};


/*===================================================================
 *			S700 DE video layer
 *==================================================================*/

static int de_s700_device_power_on(struct owl_de_device *de)
{
	uint32_t tmp, i;

	debug("%s\n", __func__);

	/* assert reset */
	owl_reset_assert(RESET_DE);

	/* power on */
	owl_powergate_power_on(POWERGATE_DE);

	/*
	 * DECLK:
	 * source is DEV_CLK(600MHz),
	 * divider is 2, 300MHz
	 */
	writel(0x1002, CMU_DECLK);

	/* enable declk from devpll */
	owl_clk_enable(CLOCK_DE);
	mdelay(1);

	/* de-assert reset */
	owl_reset_deassert(RESET_DE);
	mdelay(1);

	debug("%s: end\n", __func__);
	return 0;
}


static int de_s700_device_init(struct owl_de_device *de)
{
	/*
	 * some special init, must be same as kernel
	 */

	de_writel(0x3f, DE_MAX_OUTSTANDING);
	de_writel(0x0f, DE_QOS);

	writel(0xf832, 0xe029000c);
	writel(0x100, 0xe0290068);

	writel(0x80000000, 0xe0290000);
	mdelay(1);
	writel(0x80000004, 0xe0290000);

	return 0;
}

static void de_s700_device_dump_regs(struct owl_de_device *de)
{
	int i, j;

#define DUMPREG(r) printf("%08x ~~ %08x ~~ %s\n", r, de_readl(r), #r)
	DUMPREG(DE_IRQENABLE);
	DUMPREG(DE_IRQSTATUS);
	DUMPREG(DE_MAX_OUTSTANDING);
	DUMPREG(DE_MMU_EN);
	DUMPREG(DE_MMU_BASE);
	DUMPREG(DE_OUTPUT_CON);
	DUMPREG(DE_OUTPUT_STAT);
	DUMPREG(DE_PATH_DITHER);

	for (i = 0; i < 2; i++) {
		printf("\npath %d ------------------>\n", i);
		DUMPREG(DE_PATH_CTL(i));
		DUMPREG(DE_PATH_FCR(i));
		DUMPREG(DE_PATH_EN(i));
		DUMPREG(DE_PATH_BK(i));
		DUMPREG(DE_PATH_SIZE(i));
		DUMPREG(DE_PATH_GAMMA_IDX(i));
	}

	for (i = 0; i < 4; i++) {
		printf("\nlayer %d ------------------>\n", i);
		DUMPREG(DE_ML_CFG(i));
		DUMPREG(DE_ML_ISIZE(i));
		DUMPREG(DE_ML_CSC(i));
		DUMPREG(DE_ML_BK(i));

		DUMPREG(DE_PATH_COOR(0, i));
		DUMPREG(DE_PATH_COOR(1, i));

		for (j = 0; j < 4; j++) {
			DUMPREG(DE_SL_CFG(i, j));
			DUMPREG(DE_SL_COOR(i, j));
			DUMPREG(DE_SL_FB(i, j));
			DUMPREG(DE_SL_FB_RIGHT(i, j));
			DUMPREG(DE_SL_STR(i, j));
			DUMPREG(DE_SL_CROPSIZE(i, j));
		}
	}

	for (i = 0; i < 2; i++) {
		printf("\nscaler %d ------------------>\n", i);
		DUMPREG(DE_SCALER_CFG(i));
		DUMPREG(DE_SCALER_OSZIE(i));
		DUMPREG(DE_SCALER_HSR(i));
		DUMPREG(DE_SCALER_VSR(i));
		DUMPREG(DE_SCALER_SCOEF0(i));
		DUMPREG(DE_SCALER_SCOEF1(i));
		DUMPREG(DE_SCALER_SCOEF2(i));
		DUMPREG(DE_SCALER_SCOEF3(i));
		DUMPREG(DE_SCALER_SCOEF4(i));
		DUMPREG(DE_SCALER_SCOEF5(i));
		DUMPREG(DE_SCALER_SCOEF6(i));
		DUMPREG(DE_SCALER_SCOEF7(i));
	}
#undef DUMPREG
}

static struct owl_de_device_ops de_s700_device_ops = {
	.power_on = de_s700_device_power_on,
	.init = de_s700_device_init,
	.dump_regs = de_s700_device_dump_regs,
};

static struct owl_de_device owl_de_s700 = {
	.hw_id			= DE_HW_ID_S700,

	.num_paths		= 2,
	.paths			= de_s700_paths,

	.num_videos		= ARRAY_SIZE(de_s700_videos),
	.videos			= de_s700_videos,

	.ops			= &de_s700_device_ops,
};


/*============================================================================
 *			register to DE core
 *==========================================================================*/

int owl_de_s700_init(const void *blob)
{
	int node;

	node = fdt_node_offset_by_compatible(blob, 0, "actions,s700-de");
	if (node < 0) {
		debug("%s: no match in DTS\n", __func__);
		return -1;
	}
	debug("%s\n", __func__);

	owl_de_s700.blob = blob;
	owl_de_s700.node = node;

	owl_de_register(&owl_de_s700);

	return 0;
}
