/*
 * OWL HDMI core.
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/10: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define pr_fmt(fmt) "hdmic_core: " fmt

#include "hdmi.h"

/*===========================================================================
 *			macro definitions and data structures
 *=========================================================================*/

struct hdmi_property {
	int			channel_invert;
	int			bit_invert;
};

struct hdmi_data {
	struct hdmi_ip		*ip;
	struct owl_display_ctrl	*ctrl;

	struct hdmi_property	property;
	struct hdmi_edid	edid;	/* TODO */

	bool			cable_status;
};

/*===========================================================================
 *				static variables
 *=========================================================================*/

static struct hdmi_data		g_hdmi_data;

/*
 * Logic for the below structure :
 * user enters the CEA or VESA timings by specifying the HDMI code.
 * There is a correspondence between CEA/VESA timing and code, please
 * refer to section 6.3 in HDMI 1.3 specification for timing code.
 */

static const struct hdmi_config	cea_timings[] = {
	{
		VID720x480P_60_4VS3,
		{ 720, 480, 60, 27027, 16, 60, 9, 30, 62, 6, 0, 0 },
		false, 7, 0,
	},
	{
		VID720x576P_50_4VS3,
		{ 720, 576, 50, 27000, 12, 68, 5, 39, 64, 5, 0, 0 },
		false, 1, 0,
	},
	{
		VID1280x720P_50_16VS9,
		{ 1280, 720, 50, 74250, 440, 220, 5, 20, 40, 5,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID1280x720P_60_16VS9,
		{ 1280, 720, 60, 74250, 110, 220, 5, 20, 40, 5,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID1280x1024p_60,
		{ 1280, 1024, 60, 108000, 248, 48, 1, 38, 112, 3,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID1920x1080P_50_16VS9,
		{ 1920, 1080, 50, 148500, 528, 148, 4, 36, 44, 5,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID1920x1080P_60_16VS9,
		{ 1920, 1080, 60, 148500, 88, 148, 4, 36, 44, 5,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID2560x1024p_60,
		{ 2560, 1024, 60, 216000, 496, 96, 1, 38, 224, 3,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID2560x1024p_75,
		{ 2560, 1024, 75, 216000, 496, 96, 1, 38, 224, 3,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID3840x1080p_60,
		{ 3840, 1080, 60, 297000, 176, 296, 4, 36, 88, 5,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID3840x2160p_30,
		{ 3840, 2160, 30, 297000, 176, 296, 8, 72, 88, 10,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
	{
		VID4096x2160p_30,
		{ 4096, 2160, 30, 297000, 88, 128, 8, 72, 88, 10,
		DSS_SYNC_HOR_HIGH_ACT | DSS_SYNC_VERT_HIGH_ACT, 0 },
		false, 1, 0,
	},
};
#define CEA_TIMINGS_LEN		(ARRAY_SIZE(cea_timings))

/*===========================================================================
 *				internal interface
 *=========================================================================*/

/*
 * parse HDMI properties from DTS
 */
static int hdmc_parse_property(struct hdmi_data *hdmi)
{
	int node = hdmi->ip->node;
	const void *blob = hdmi->ip->blob;

	struct hdmi_property *property = &hdmi->property;

	debug("%s\n", __func__);

	property->channel_invert
		= fdtdec_get_int(blob, node, "channel_invert", 0);
	property->bit_invert
		= fdtdec_get_int(blob, node, "bit_invert", 0);

	debug("channel_invert %d, bit_invert %d\n",
	      property->channel_invert, property->bit_invert);

	return 0;
}

/*===========================================================================
 *			HDMI controller
 *=========================================================================*/

static void hdmic_update_videomode(struct hdmi_data *hdmi)
{
	char *bootargs_add;
	char buf[256];

	int i;

	struct hdmi_ip *ip = hdmi->ip;
	struct owl_panel *panel = hdmi->ctrl->panel;

	struct owl_videomode default_mode;

	const struct hdmi_config *cfg;

	if (PANEL_NEED_EDID(panel))
		hdmi_edid_parse(&hdmi->edid);

	/* search from cea_timings in inverted order */
	for (i = CEA_TIMINGS_LEN - 1; i >= 0; i--) {
		cfg = &cea_timings[i];

		if (cfg->vid < HDMI_EDID_MAX_VID &&
		    hdmi->edid.device_support_vic[cfg->vid] == 1)
			goto got_it;
	}

	/*
	 * try default vid provided by bootargs or DTS
	 */
	owl_panel_get_default_mode(panel, &default_mode);

	for (i = CEA_TIMINGS_LEN - 1; i >= 0; i--) {
		cfg = &cea_timings[i];
		if (cfg->mode.xres == default_mode.xres &&
		    cfg->mode.yres == default_mode.yres &&
		    cfg->mode.refresh == default_mode.refresh)
			goto got_it;
	}

	error("%s: cannot get valid vid!!\n", __func__);
	BUG();

got_it:
	owl_panel_set_mode(panel, &cfg->mode);

	ip->cfg = cfg;

	ip->settings.hdmi_mode = hdmi->edid.hdmi_mode;

	ip->settings.channel_invert = hdmi->property.channel_invert;
	ip->settings.bit_invert = hdmi->property.bit_invert;

	bootargs_add = getenv("bootargs.add");
	if (bootargs_add == NULL)
		sprintf(buf, "actions.hdmi.mode=%dx%d@%d",
			cfg->mode.xres, cfg->mode.yres, cfg->mode.refresh);
	else
		sprintf(buf, "%s actions.hdmi.mode=%dx%d@%d", bootargs_add,
			cfg->mode.xres, cfg->mode.yres, cfg->mode.refresh);
	setenv("bootargs.add", buf);

	debug("%s: vid = %d\n", __func__, cfg->vid);
}

static int owl_hdmic_enable(struct owl_display_ctrl *ctrl)
{
	struct hdmi_data *hdmi = owl_ctrl_get_drvdata(ctrl);
	struct hdmi_ip *ip = hdmi->ip;
	const struct hdmi_ip_ops *ip_ops = ip->ops;

	debug("%s\n", __func__);

	if (!ip_ops)
		return 0;

	if (ip_ops->is_power_on && ip_ops->is_power_on(ip))
		return 0;

	if (ip_ops->power_on)
		ip_ops->power_on(ip);
	mdelay(5);

	ip_ops->video_enable(ip);

	return 0;
}

static void owl_hdmic_disable(struct owl_display_ctrl *ctrl)
{
	struct hdmi_data *hdmi = owl_ctrl_get_drvdata(ctrl);
	struct hdmi_ip *ip = hdmi->ip;
	const struct hdmi_ip_ops *ip_ops = ip->ops;

	debug("%s\n", __func__);

	if (!ip_ops)
		return;

	if (ip_ops->is_power_on && !ip_ops->is_power_on(ip))
		return;

	ip_ops->video_disable(ip);

	if (ip_ops->power_off)
		ip_ops->power_off(ip);
}

static int owl_hdmic_add_panel(struct owl_display_ctrl *ctrl,
			       struct owl_panel *panel)
{
	bool status;

	struct hdmi_data *hdmi = owl_ctrl_get_drvdata(ctrl);
	struct hdmi_ip *ip = hdmi->ip;

	status = ip->ops->cable_status(ip);
	debug("%s: %d\n", __func__, status);

	if (!status)
		return -ENODEV;

	hdmic_update_videomode(hdmi);
	return 0;
}

static struct owl_display_ctrl_ops owl_hdmi_ctrl_ops = {
	.add_panel = owl_hdmic_add_panel,

	.enable = owl_hdmic_enable,
	.disable = owl_hdmic_disable,
};

static struct owl_display_ctrl owl_hdmi_ctrl = {
	.name = "hdmi_ctrl",
	.type = OWL_DISPLAY_TYPE_HDMI,
	.ops = &owl_hdmi_ctrl_ops,
};


/*===========================================================================
 *				HDMI IP
 *=========================================================================*/

static int hdmi_ip_get_resource(struct hdmi_ip *ip)
{
	debug("%s\n", __func__);

	ip->base = fdtdec_get_addr(ip->blob, ip->node, "reg");
	if (ip->base == FDT_ADDR_T_NONE) {
		error("Cannot find ip reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, ip->base);

	return 0;
}

int hdmi_ip_register(struct hdmi_ip *ip)
{
	int ret = 0;

	struct hdmi_data *hdmi = &g_hdmi_data;

	debug("%s\n", __func__);

	if (!ip || !ip->blob || !ip->node) {
		error("ip or ip->blob or ip->node is NULL!\n");
		return -1;
	}

	hdmi->ip = ip;
	ip->pdata = hdmi;

	ret = hdmi_ip_get_resource(ip);
	if (ret < 0)
		return ret;

	/*
	 * HDMI IP init
	 */
	if (ip->ops && ip->ops && ip->ops->init) {
		ret = ip->ops->init(ip);
		if (ret < 0)
			return ret;
	}

	/*
	 * HDMI controller
	 */
	hdmi->ctrl = &owl_hdmi_ctrl;
	owl_ctrl_set_drvdata(&owl_hdmi_ctrl, hdmi);

	ret = owl_ctrl_register(&owl_hdmi_ctrl);
	if (ret < 0) {
		error("register hdmi ctrl failed: %d\n", ret);
		return ret;
	}

	hdmc_parse_property(hdmi);

	hdmi_ddc_init(ip->blob);

	return 0;
}

void hdmi_ip_unregister(struct hdmi_ip *ip)
{
	debug("%s\n", __func__);
}


int owl_hdmic_init(const void *blob)
{
	hdmi_ip_sx00_init(blob);

	return 0;
}
