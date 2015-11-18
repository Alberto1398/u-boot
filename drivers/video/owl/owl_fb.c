/*
 * OWL Framebuffer
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
#define pr_fmt(fmt) "owl_fb: " fmt

#include <config.h>
#include <common.h>
#include <video_fb.h>

#include <asm-generic/errno.h>

#include <asm/arch/pwm.h>
#include <asm/arch/pwm_bl.h>

#include <owl_dss.h>

#define OWLFB_DISPLAY_ID	OWL_DISPLAY_LCD
#define OWLFB_COLOR_MODE	OWL_DSS_COLOR_BGRA32
#define OWLFB_BUF_ADDR		(0x19000000)

DECLARE_GLOBAL_DATA_PTR;

struct owl_fb {
	struct graphic_device	gd;

	struct owl_panel	*panel;
	struct owl_de_path	*path;
	struct owl_de_video	*video;

	/*
	 * we supported dual display device
	 * NOTE: we use 'second_panel' to record if there exists
	 *	secondary display device, so please let 'second_panel'
	 *	to be the real value, NULL or some panel.
	 */
	struct owl_panel	*second_panel;
	struct owl_de_path	*second_path;
	struct owl_de_video	*second_video;
};

static struct owl_fb		g_owl_fb;

static int owl_color_mode_to_gdf_mode(enum owl_color_mode color)
{
	int gdf_mode = GDF_32BIT_X888RGB;

	switch (color) {
	case OWL_DSS_COLOR_RGB16:
		gdf_mode = GDF_16BIT_565RGB;
		break;

	case OWL_DSS_COLOR_BGRA32:
		gdf_mode = GDF_32BIT_X888RGB;
		break;

	default:
		break;
	}

	return gdf_mode;
}

static void __owl_fb_init(struct owl_fb *fb, bool is_primary)
{
	struct owl_panel *panel;
	struct owl_de_path *path;
	struct owl_de_video *video;

	struct owl_de_path_info p_info;
	struct owl_de_video_info v_info;

	debug("%s: is_primary %d\n", __func__, is_primary);

	if (is_primary) {
		panel = fb->panel;
		path = fb->path;
		video = fb->video;
	} else {
		panel = fb->second_panel;
		path = fb->second_path;
		video = fb->second_video;
	}

	owl_de_video_set_path(video, path);

	/*
	 * init path info
	 */
	owl_de_path_get_info(path, &p_info);

	p_info.type = owl_panel_get_type(panel);
	owl_panel_get_resolution(panel, (int *)&p_info.width,
				 (int *)&p_info.height);
	switch (owl_panel_get_bpp(panel)) {
	case 16:
		p_info.dither_mode = DITHER_24_TO_16;
		break;

	case 18:
		p_info.dither_mode = DITHER_24_TO_18;
		break;

	default:
		p_info.dither_mode = DITHER_DISABLE;
		break;
	}

	owl_de_path_set_info(path, &p_info);

	/*
	 * init video info
	 */
	owl_de_video_get_info(video, &v_info);

	v_info.color_mode = OWLFB_COLOR_MODE;

	v_info.xoff = 0;
	v_info.yoff = 0;
	/* input size is equal to primary panel's draw size */
	owl_panel_get_draw_size(fb->panel, (int *)&v_info.width,
				(int *)&v_info.height);

	v_info.pos_x = 0;
	v_info.pos_y = 0;
	/* output size is equal to panel's resolution */
	owl_panel_get_resolution(panel, (int *)&v_info.out_width,
				 (int *)&v_info.out_height);

	v_info.addr[0] = OWLFB_BUF_ADDR;
	v_info.offset[0] = 0;
	v_info.pitch[0] = (owl_dss_get_color_bpp(v_info.color_mode) / 8)
			* v_info.width;

	owl_de_video_set_info(video, &v_info);
}

static int owl_fb_init(struct owl_fb *fb)
{
	debug("%s\n", __func__);

	/*
	 * primary panel
	 */
	fb->panel = owl_panel_get_primary_panel();
	if (fb->panel == NULL) {
		error("no primary panel\n");
		return -ENODEV;
	}
	debug("%s: primary panel type is %d\n", __func__,
	      owl_panel_get_type(fb->panel));

	fb->path = owl_de_path_get_by_type(owl_panel_get_type(fb->panel));
	if (fb->path == NULL) {
		error("can not get de path for primary panel\n");
		return -EINVAL;
	}

	fb->video = owl_de_video_get_by_id(0);
	if (fb->video == NULL) {
		error("can not get de video for primary panel\n");
		return -EINVAL;
	}

	__owl_fb_init(fb, true);

	/*
	 * second panel
	 */
	fb->second_panel = owl_panel_get_second_panel();
	fb->second_path
		= owl_de_path_get_by_type(owl_panel_get_type(fb->second_panel));
	fb->second_video = owl_de_video_get_by_id(1);

	if (fb->second_panel != NULL && fb->second_path != NULL &&
	    fb->second_video != NULL) {
		__owl_fb_init(fb, false);
	} else {
		debug("no valid second display device\n");
		fb->second_panel = NULL;
	}

	/*
	 * fill to graphic_device
	 */
	owl_panel_get_draw_size(fb->panel, (int *)&fb->gd.winSizeX,
				(int *)&fb->gd.winSizeY);
	fb->gd.gdfIndex = owl_color_mode_to_gdf_mode(OWLFB_COLOR_MODE);
	fb->gd.gdfBytesPP = owl_dss_get_color_bpp(OWLFB_COLOR_MODE) / 8;
	fb->gd.frameAdrs = OWLFB_BUF_ADDR;

	debug("%s: pGD info---\n", __func__);
	debug("%dx%d, gdfIndex %d, gdfBytesPP %d, frameAdrs %x\n",
	      fb->gd.winSizeX, fb->gd.winSizeY, fb->gd.gdfIndex,
	      fb->gd.gdfBytesPP, fb->gd.frameAdrs);

	return 0;
}

static void owl_fb_display_on(struct owl_fb *fb)
{
	debug("%s\n", __func__);

	debug("%s(primay panel)\n", __func__);
	owl_panel_enable(fb->panel);
	owl_de_video_enable(fb->video, true);
	owl_de_path_enable(fb->path, true);
	owl_de_path_set_go(fb->path);

	if (fb->second_panel != NULL) {
		debug("%s(secondary panel)\n", __func__);
		owl_panel_enable(fb->second_panel);
		owl_de_video_enable(fb->second_video, true);
		owl_de_path_enable(fb->second_path, true);
		owl_de_path_set_go(fb->second_path);
	}

	/*
	 * backlight on, using default brightness in DTS
	 * should move to other place, TODO
	 */
	owl_pwm_bl_init(gd->fdt_blob);
	owl_pwm_bl_on();
}

#ifdef CONFIG_VIDEO_OWL
void *video_hw_init(void)
{
	owl_pwm_init(gd->fdt_blob);
	owl_dss_init(gd->fdt_blob);

	if (owl_fb_init(&g_owl_fb) < 0)
		return NULL;

	owl_fb_display_on(&g_owl_fb);

	return &g_owl_fb.gd;
}
#endif
