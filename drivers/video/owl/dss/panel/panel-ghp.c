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
#define pr_fmt(fmt) "owl_panel_ghp: " fmt

#include <fs.h>

#include <dss.h>

/*
 * using hdmi settings in /data/setting/setting_hdmi_*
 * to control HDMI's behavior
 */
#define USING_HDMI_SETTINGS_X

#ifdef USING_HDMI_SETTINGS
#define SETTING_HDMI_ENABLE	"setting/setting_hdmi_enable"
#define SETTING_HDMI_SIZE	"setting/setting_hdmi_size"
#endif

static struct owl_panel owl_panel_ghp = {
	.desc = {
		.name = "panel_ghp",
		.type = OWL_DISPLAY_TYPE_HDMI,
	},
};

#ifdef USING_HDMI_SETTINGS
static int ghp_hdmi_setting_read_int(char *filename)
{
	char *devif, *bootdisk;
	char dev_part_str[16];
	char buf[64];

	int val = 0;

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

	fs_read(filename, (unsigned long)buf, 0, 64, &actread);
	if (actread <= 0) {
		error("read %s failed\n", filename);
		return -1;
	}

	val = simple_strtoul(buf, NULL, 0);

	debug("%s: %s = %d\n", __func__, filename, val);

	return val;
}

static int ghp_hdmi_setting_read_size(int *x, int *y)
{
	char *devif, *bootdisk;
	char dev_part_str[16];
	char buf[64];

	loff_t actread;

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	if (!devif || !bootdisk) {
		error("no device\n");
		return -1;
	}

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, ANDROID_DATA_PART);

	if (!file_exists(devif, dev_part_str, SETTING_HDMI_SIZE,
			 FS_TYPE_EXT)) {
		debug("%s is not exist\n", SETTING_HDMI_SIZE);
		return -1;
	}

	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_EXT) != 0) {
		error("fs_set_blk_dev (%s %s) fialed\n", devif, dev_part_str);
		return -1;
	}

	fs_read(SETTING_HDMI_SIZE, (unsigned long)buf, 0, 64, &actread);
	if (actread <= 0) {
		error("read %s failed\n", SETTING_HDMI_SIZE);
		return -1;
	}

	/* "%d %d\n" */
	*x = simple_strtoul(buf, NULL, 0);
	*y = simple_strtoul(strstr(buf, " ") + 1, NULL, 0);

	debug("%s: %s = %d %d\n", __func__, SETTING_HDMI_SIZE, *x, *y);

	return 0;
}
#endif

int owl_panel_ghp_init(const void *blob)
{
	char *bootargs_add;
	char buf[256];

	int node;
	int sizex, sizey;
	int ret = 0;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-ghp");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	/* no need care about return value */
	owl_panel_parse_panel_info(blob, node, &owl_panel_ghp);

#ifdef USING_HDMI_SETTINGS
	if (!PANEL_IS_PRIMARY(&owl_panel_ghp) &&
	    ghp_hdmi_setting_read_int(SETTING_HDMI_ENABLE) != 1) {
		debug("not enabled\n");
		return 0;
	}
#endif

	ret = owl_panel_register(&owl_panel_ghp);
	if (ret < 0) {
		/*
		 * debug, not error,
		 * because HDMI may fail if cable is not plugged
		 */
		debug("failed to register panel\n");
		return ret;
	}

#ifdef USING_HDMI_SETTINGS
	/* get HDMI size and pass it to kernel */
	if (ghp_hdmi_setting_read_size(&sizex, &sizey) == 0) {
		owl_panel_set_scale_factor(&owl_panel_ghp, sizex, sizey);

		bootargs_add = getenv("bootargs.add");
		if (bootargs_add == NULL)
			sprintf(buf, "actions.hdmi.size=%dx%d", sizex, sizey);
		else
			sprintf(buf, "%s actions.hdmi.size=%dx%d",
				bootargs_add, sizex, sizey);
		setenv("bootargs.add", buf);
	}
#endif

	return 0;
}
