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
	owl_de_s900_init(blob);

	/*
	 * controllers init
	 */
	owl_dsic_init(blob);
	owl_edpc_init(blob);
	owl_hdmic_init(blob);
	owl_dummy_dispc_init(blob);

	/*
	 * panels init
	 */
	owl_panel_gmp_init(blob);
	owl_panel_gep_init(blob);
	owl_panel_ghp_init(blob);
	owl_panel_gdp_init(blob);
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
