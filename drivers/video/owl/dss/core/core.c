/*
 * OWL Display Subsystem Core
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
#define pr_fmt(fmt) "owl_dss_core: " fmt

#include <asm/io.h>
#include <asm/arch/regs.h>

#include <dss.h>

void owl_dss_init(const void *blob)
{
	debug("%s\n", __func__);

	/*
	 * core init
	 */
	owl_ctrl_init();
	owl_panel_init();

	/*
	 * de_xxx init
	 */
#ifdef CONFIG_VIDEO_OWL_DE_S900
	owl_de_s900_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_DE_S700
	owl_de_s700_init(blob);
#endif

	/*
	 * controllers init
	 */
#ifdef CONFIG_VIDEO_OWL_LCD
	owl_lcdc_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_DSI
	owl_dsic_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_EDP
	owl_edpc_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_HDMI
	owl_hdmic_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_CVBS
	owl_cvbsc_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_DUMMY
	owl_dummy_dispc_init(blob);
#endif

	/*
	 * panels init
	 */
#ifdef CONFIG_VIDEO_OWL_LCD
	owl_panel_glp_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_DSI
	owl_panel_gmp_init(blob);
	owl_panel_lq055t3sx02_init(blob);
	owl_panel_amoled_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_EDP
	owl_panel_gep_init(blob);
	owl_panel_tc358860_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_HDMI
	owl_panel_ghp_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_CVBS
	owl_panel_gcp_init(blob);
#endif
#ifdef CONFIG_VIDEO_OWL_DUMMY
	owl_panel_gdp_init(blob);
#endif
}

int owl_dss_get_color_bpp(enum owl_color_mode color)
{
	int bpp;

	switch (color) {
	case OWL_DSS_COLOR_BGR16:
	case OWL_DSS_COLOR_RGB16:
		bpp = 16;
		break;
	default:
		bpp = 32;
		break;
	}

	return bpp;
}
