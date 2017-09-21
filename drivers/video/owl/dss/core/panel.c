/*
 * Abstraction of OWL Display Panel
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
#define pr_fmt(fmt) "owl_panel: " fmt

#include <dss.h>

#define MAX_PANELS		(4)

static struct owl_panel		*owl_panel_array[MAX_PANELS];
static int			owl_panel_num;

/*
 * Update panel's timing info, include preline number
 *
 * Calculate preline numbers according to the recommended preline time
 * and panel's timings.
 *
 * Formula for preline number is:
 *	(recommended preline time(us) * 1000000)
 *	/ (x_res + hfp + hbp + hsw)) / pixel_clock - vfp)
 */
static void panel_update_timing_info(struct owl_panel *panel)
{
	int preline_num;
	int tmp;

	struct owl_videomode *mode;

	if (panel == NULL)
		return;

	mode = &panel->current_mode;

	tmp = (mode->xres + mode->hfp + mode->hbp + mode->hsw) * mode->pixclock;

	/* caculate preline number */
	preline_num = tmp;
	if (preline_num != 0)
		preline_num = (DSS_RECOMMENDED_PRELINE_TIME * 1000000
			+ preline_num / 2) / preline_num; /* round */

	preline_num -= mode->vfp;
	preline_num = (preline_num <= 0 ? 1 : preline_num);

	panel->desc.preline_num = preline_num;
}

int owl_panel_init(void)
{
	debug("%s\n", __func__);

	owl_panel_num = 0;

	return 0;
}

int owl_panel_register(struct owl_panel *panel)
{
	int i;

	debug("%s\n", __func__);

	if (panel == NULL)
		return -1;

	if (owl_panel_num > MAX_PANELS) {
		error("too many devices!\n");
		return -1;
	}

	for (i = 0; i < owl_panel_num; i++) {
		if (owl_panel_array[i]->desc.type == panel->desc.type) {
			error("same type device is already registered!\n");
			return -1;
		}
	}

	panel->desc.scale_factor_x = 100;
	panel->desc.scale_factor_y = 100;

	panel->state = OWL_DSS_STATE_DISABLED;

	if (owl_ctrl_add_panel(panel) < 0) {
		debug("add panel to ctrl failed!\n");
		return -1;
	}

	owl_panel_array[owl_panel_num] = panel;
	owl_panel_num++;

	return 0;
}

void owl_panel_unregister(struct owl_panel *panel)
{
	/* TODO */
}

/*
 * check panel's connect status, the rule is:
 *	if panel's .hpd_is_connected is not NULL, using it;
 *	or, if its controller's .hpd_is_panel_connected is not NULL, using it;
 *	or, return TRUE.
 */
bool owl_panel_hpd_is_connected(struct owl_panel *panel)
{
	struct owl_display_ctrl *ctrl = panel->ctrl;

	if (panel && panel->desc.ops && panel->desc.ops->hpd_is_connected)
		return panel->desc.ops->hpd_is_connected(panel);
	if (ctrl && ctrl->ops && ctrl->ops->hpd_is_panel_connected)
		return ctrl->ops->hpd_is_panel_connected(ctrl);
	else
		return true;
}

struct owl_panel *owl_panel_get_by_num(int num)
{
	if (num < 0 || num >= owl_panel_num)
		return NULL;
	else
		return owl_panel_array[num];
}

struct owl_panel *owl_panel_get_primary_panel(void)
{
	int i;

	struct owl_display_ctrl *ctrl;

	for (i = 0; i < owl_panel_num; i++) {
		if (owl_panel_array[i]->desc.is_primary) {
			if (owl_panel_hpd_is_connected(owl_panel_array[i]) == true)
				return owl_panel_array[i];
		}
	}

	/*
	 * for s700 OTT, if no primary panel is connected, hdmi panel by default
	 * */
	if (owl_panel_get_by_type(OWL_DISPLAY_TYPE_HDMI) != NULL) {
		debug("Not detected the primary panel!!! default HDMI\n");
		return owl_panel_get_by_type(OWL_DISPLAY_TYPE_HDMI);
	} else
		return NULL;
}

struct owl_panel *owl_panel_get_second_panel(void)
{
	int i;

	for (i = 0; i < owl_panel_num; i++) {
		if (!owl_panel_array[i]->desc.is_primary)
			return owl_panel_array[i];
	}

	return NULL;
}

struct owl_panel *owl_panel_get_by_name(const char *name)
{
	int i;

	for (i = 0; i < owl_panel_num; i++) {
		if (strcmp(owl_panel_array[i]->desc.name, name) == 0)
			return owl_panel_array[i];
	}

	return NULL;
}

struct owl_panel *owl_panel_get_by_type(enum owl_display_type type)
{
	int i;

	for (i = 0; i < owl_panel_num; i++) {
		if (owl_panel_array[i]->desc.type == type)
			return owl_panel_array[i];
	}

	return NULL;
}

void owl_panel_get_scale_factor(struct owl_panel *panel,
				uint16_t *x, uint16_t *y)
{
	*x = panel->desc.scale_factor_x;
	*y = panel->desc.scale_factor_y;
}

void owl_panel_set_scale_factor(struct owl_panel *panel,
				uint16_t x, uint16_t y)
{
	if (x > OWL_PANEL_SCALE_FACTOR_MAX)
		x = OWL_PANEL_SCALE_FACTOR_MAX;
	if (x < OWL_PANEL_SCALE_FACTOR_MIN)
		x = OWL_PANEL_SCALE_FACTOR_MIN;
	panel->desc.scale_factor_x = x;

	if (y > OWL_PANEL_SCALE_FACTOR_MAX)
		y = OWL_PANEL_SCALE_FACTOR_MAX;
	if (y < OWL_PANEL_SCALE_FACTOR_MIN)
		y = OWL_PANEL_SCALE_FACTOR_MIN;
	panel->desc.scale_factor_y = y;
}

void owl_panel_get_default_mode(struct owl_panel *panel,
				struct owl_videomode *mode)
{
	memcpy(mode, &panel->default_mode, sizeof(struct owl_videomode));
}

void owl_panel_get_mode(struct owl_panel *panel, struct owl_videomode *mode)
{
	memcpy(mode, &panel->current_mode, sizeof(struct owl_videomode));
}

void owl_panel_set_mode(struct owl_panel *panel, struct owl_videomode *mode)
{
	memcpy(&panel->current_mode, mode, sizeof(struct owl_videomode));
}

int owl_panel_enable(struct owl_panel *panel)
{
	struct owl_dss_panel_desc *desc = &panel->desc;

	debug("%s: power_on_delay %d, enable_delay %d\n", __func__,
	      desc->power_on_delay, desc->enable_delay);

	if (panel->state == OWL_DSS_STATE_ENABLED)
		return 0;

	/* panel power on */
	if (desc->ops && desc->ops->power_on) {
		desc->ops->power_on(panel);
		if (desc->power_on_delay > 0)
			mdelay(desc->power_on_delay);
	}
	/* controller power on */
	owl_ctrl_power_on(panel->ctrl);


	/* panel enable */
	if (desc->ops && desc->ops->enable)
		desc->ops->enable(panel);

	/* controller enable */
	owl_ctrl_enable(panel->ctrl);

	if (desc->enable_delay > 0)
		mdelay(desc->enable_delay);

	panel->state = OWL_DSS_STATE_ENABLED;

	return 0;
}

void owl_panel_disable(struct owl_panel *panel)
{
	struct owl_dss_panel_desc *desc = &panel->desc;

	debug("%s: power_off_delay %d, disable_delay %d\n", __func__,
	      desc->power_off_delay, desc->disable_delay);

	if (panel->state == OWL_DSS_STATE_DISABLED)
		return;

	owl_ctrl_disable(panel->ctrl);

	if (desc->disable_delay > 0)
		mdelay(desc->disable_delay);

	if (desc->ops && desc->ops->disable)
		desc->ops->disable(panel);

	if (desc->ops && desc->ops->power_off) {
		desc->ops->power_off(panel);
		if (desc->power_off_delay > 0)
			mdelay(desc->power_off_delay);
	}

	panel->state = OWL_DSS_STATE_DISABLED;
}

void owl_panel_get_resolution(struct owl_panel *panel, int *xres, int *yres)
{
	if (panel == NULL)
		return;

	*xres = panel->current_mode.xres;
	*yres = panel->current_mode.yres;
}

void owl_panel_get_gamma(struct owl_panel *panel, int *gamma_r_val,
				int *gamma_g_val, int *gamma_b_val)
{
	if (panel == NULL)
		return;
	*gamma_r_val = panel->desc.gamma_r_val;
	*gamma_g_val = panel->desc.gamma_g_val;
	*gamma_b_val = panel->desc.gamma_b_val;
}

uint32_t owl_panel_get_vmode(struct owl_panel *panel)
{
	if (panel == NULL)
		return;
	return panel->current_mode.vmode;
}
void owl_panel_get_draw_size(struct owl_panel *panel, int *xres, int *yres)
{
	if (panel == NULL)
		return;

	*xres = panel->draw_width;
	*yres = panel->draw_height;
}

int owl_panel_get_bpp(struct owl_panel *panel)
{
	return panel == NULL ? 0 : panel->desc.bpp;
}

enum owl_display_type owl_panel_get_type(struct owl_panel *panel)
{
	return panel == NULL ? OWL_DISPLAY_TYPE_NONE : panel->desc.type;
}

int owl_panel_get_preline_num(struct owl_panel *panel)
{
	return (panel == NULL ? 0 : panel->desc.preline_num);
}

/*
 * parse bootargs by display type which is like:
 *	video=owlfb0:dev=edp,primary=1
 * if found, get .is_primary, default mode etc.
 *	then, return 0
 * if no found, return error(-1)
 */
int owl_panel_parse_panel_info(const void *blob, int node,
			       struct owl_panel *panel)
{
	int entry, val;
	struct owl_videomode *mode = &panel->current_mode;
	struct owl_dss_panel_desc *desc = &panel->desc;

	debug("%s:\n", __func__);

	/* parse panel info ... */
	desc->bpp = fdtdec_get_int(blob, node, "bpp", 24);

	desc->power_on_delay = fdtdec_get_int(blob, node, "power_on_delay", 0);
	desc->power_off_delay = fdtdec_get_int(blob, node,
					       "power_off_delay", 0);
	desc->enable_delay = fdtdec_get_int(blob, node, "enable_delay", 0);
	desc->disable_delay = fdtdec_get_int(blob, node, "disable_delay", 0);

	/* parameter of gamma correction, default val is 100 */
	desc->gamma_r_val = fdtdec_get_int(blob, node, "gamma_r_val", 100);
	desc->gamma_g_val = fdtdec_get_int(blob, node, "gamma_g_val", 100);
	desc->gamma_b_val = fdtdec_get_int(blob, node, "gamma_b_val", 100);
	debug("gamma_r_val %d, gamma_g_val %d, gamma_b_val %d\n",
		desc->gamma_r_val, desc->gamma_g_val, desc->gamma_b_val);

	debug("bpp %d\n", desc->bpp);
	debug("power_on_delay %d, power_off_delay %d\n",
	      desc->power_on_delay, desc->power_off_delay);
	debug("enable_delay %d, disable_delay %d\n",
	      desc->enable_delay, desc->disable_delay);

	val = fdtdec_get_int(blob, node, "is_primary", 0);
	desc->is_primary = (val == 1 ? true : false);

	val = fdtdec_get_int(blob, node, "skip_edid", 0);
	desc->need_edid = (val == 0 ? true : false);

	debug("is_primary %d, need_edid %d\n",
	      desc->is_primary, desc->need_edid);

	panel->draw_width = fdtdec_get_int(blob, node, "draw_width", 0);
	panel->draw_height = fdtdec_get_int(blob, node, "draw_height", 0);

	/* parse video mode ... */
	entry = fdtdec_lookup_phandle(blob, node, "videomode-0");
	debug("entry = %d\n", entry);
	if (entry < 0) {
		debug("no etry for 'videomode-0'\n");
	} else {
		mode->refresh = fdtdec_get_int(blob, entry, "refresh_rate", 0);
		mode->xres = fdtdec_get_int(blob, entry, "xres", 0);
		mode->yres = fdtdec_get_int(blob, entry, "yres", 0);
		mode->pixclock = fdtdec_get_int(blob, entry, "pixel_clock", 0);
		mode->hsw = fdtdec_get_int(blob, entry, "hsw", 0);
		mode->hbp = fdtdec_get_int(blob, entry, "hbp", 0);
		mode->hfp = fdtdec_get_int(blob, entry, "hfp", 0);
		mode->vsw = fdtdec_get_int(blob, entry, "vsw", 0);
		mode->vbp = fdtdec_get_int(blob, entry, "vbp", 0);
		mode->vfp = fdtdec_get_int(blob, entry, "vfp", 0);
		mode->vmode = fdtdec_get_int(blob, entry, "vmode", 0);
	}

	debug("%dx%d, hsw/hfp/hbp: %d/%d/%d, vsw/vfp/vbp: %d/%d/%d\n",
	      mode->xres, mode->yres, mode->hsw, mode->hfp, mode->hbp,
	      mode->vsw, mode->vfp, mode->vbp);

	panel_update_timing_info(panel);

	if (panel->draw_width == 0 || panel->draw_height == 0) {
		if (mode->xres != 0 && mode->yres != 0) {
			debug("use 'mode' as draw size\n");
			panel->draw_width = mode->xres;
			panel->draw_height = mode->yres;
		} else {
			debug("use default size as draw size\n");
			panel->draw_width = 1920;
			panel->draw_height = 1080;
		}
	}


	debug("draw size %dx%d\n", panel->draw_width, panel->draw_height);

	/* set default_mode */
	if (mode->xres != 0 && mode->yres != 0 && mode->refresh != 0) {
		panel->default_mode.xres = mode->xres;
		panel->default_mode.yres = mode->yres;
		panel->default_mode.refresh = mode->refresh;
	} else {
		panel->default_mode.xres = 1280;
		panel->default_mode.yres = 720;
		panel->default_mode.refresh = 60;
	}

	return 0;
}
