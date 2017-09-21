/*
 * OWL GHP(generic HDMI Panel)
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
#define pr_fmt(fmt) "owl_panel_gcp: " fmt

#include <fs.h>
#include <dss.h>

/*
 * using cvbs settings in /data/setting/setting_cvbs_*
 * to control CVBS's behavior
 */
#define USING_CVBS_SETTINGSX
#ifdef USING_CVBS_SETTINGS
const char *CVBS_SETTING_MODE_PATH = "setting/setting_cvbs_mode";
#endif

static struct owl_panel owl_panel_gcp = {
	.desc = {
		.name = "panel_gcp",
		.type = OWL_DISPLAY_TYPE_CVBS,
	},
};
#if 0
int gcp_cvbs_read_usr_cfg_file(const char *file_name, char *buf)
{
	char *devif, *bootdisk;
	char dev_part_str[16];

	int val = 0;
;
	loff_t actread;

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	if (!devif || !bootdisk) {
		error("no device\n");
		return -1;
	}

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, ANDROID_DATA_PART);

	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_EXT) != 0) {
		error("fs_set_blk_dev (%s %s) fialed\n", devif, dev_part_str);
		return -1;
	}

	fs_read(file_name, (unsigned long)buf, 0, 64, &actread);
	if (actread <= 0) {
		error("read %s failed\n", file_name);
		return -1;
	}

	val = simple_strtoul(buf, NULL, 0);

	debug("%s: %s = %d\n", __func__, file_name, val);

	return val;
}
#endif
int owl_panel_gcp_init(const void *blob)
{
	char *bootargs_add;
	char buf[256];

	int node;
	int ret = 0;

	debug("%s\n", __func__);
	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-gcp");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}

	/* no need care about return value */
	owl_panel_parse_panel_info(blob, node, &owl_panel_gcp);

#if 0
	if (!PANEL_IS_PRIMARY(&owl_panel_gcp) &&
	    gcp_cvbs_read_usr_cfg_file(CVBS_SETTING_MODE_PATH, buf) != 1) {
		debug("not enabled\n");
		return 0;
	}
#endif
	ret = owl_panel_register(&owl_panel_gcp);
	if (ret < 0) {
		/*
		 * debug, not error,
		 * because CVBS may fail if cable is not plugged
		 */
		debug("failed to register cvbs panel\n");
		return ret;
	}

#ifdef USING_HDMI_SETTINGS
#endif

	return 0;
}
