/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <fs.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>
#include <asm/arch/sys_proto.h>
#include "board.h"

int board_video_skip(void)
{
	int power_status;

	power_status = owl_get_power_status();

	if (power_status == POWER_CRITICAL_LOW ||
	    power_status == POWER_CRITICAL_LOW_CHARGER)
		return 1;

	return 0;
}
#define DVFSLEVEL_MAGIC    0x47739582
#define OWL_GET_TYPE(dvfslevel)      (((((dvfslevel) ^ DVFSLEVEL_MAGIC) >> 10) ^ 0x3f) & 0x3f)

static char *splash_image_select(void)
{
	int power_status;
	unsigned int data = 0;
#ifdef CONFIG_ANDROID_RECOVERY
	if (owl_get_recovery_mode())
		return CONFIG_SYS_RECOVERY_LOGO_NAME;
#endif

	power_status = owl_get_power_status();

	if (power_status == POWER_NORMAL_CHARGER)
		return CONFIG_SYS_CHARGER_LOGO_NAME;
	else if (power_status == POWER_LOW)
		return CONFIG_SYS_BATTERY_LOW_NAME;
	else{
#ifdef CONFIG_USE_SHOW_CHIPID_BOOT_LOGO
			data = __read_data(0xc3000000);
			printf("splash_image_select---type = 0x%x---\n", OWL_GET_TYPE(data));
			if (0x2 == OWL_GET_TYPE (data)) {
				printf("----show chip id ok  ---\n");
				return CONFIG_SYS_VIDEO_LOGO_NAME_QC_SHOW_CHIPID_OK;
			} else{
				printf("----show chip id error  ---\n");
				return CONFIG_SYS_VIDEO_LOGO_NAME_QC_SHOW_CHIPID_ERROR;
			}

#endif
			return CONFIG_SYS_VIDEO_LOGO_NAME;

		}
}

int splash_screen_prepare(void)
{
	char *devif, *bootdisk, *bootpart;
	char *splashimage, *logo_name;
	u32 splashimage_addr;
	loff_t len_read;
	int ret;
	char dev_part_str[16];
	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	bootpart = getenv("bootpart");
	if (!devif || !bootdisk || !bootpart)
		return -ENOENT;

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, bootpart);

	splashimage = getenv("splashimage");
	if (splashimage == NULL)
		return -ENOENT;

	splashimage_addr = simple_strtoul(splashimage, 0, 16);
	if (splashimage_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		return -EFAULT;
	}

	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_FAT)) {
		printf("%s: fs_set_blk_dev (%s %s) ERROR\n",
		       __func__, devif, dev_part_str);
		return -1;
	}

	logo_name = splash_image_select();
	if (!logo_name)
		return -1;

	debug("%s: logo_name %s\n", __func__, logo_name);

	ret = fs_read(logo_name, splashimage_addr, 0, 0, &len_read);
	if (ret < 0) {
		printf("%s: fs_read ERROR\n", __func__);
		return -1;
	}

	return 0;
}
