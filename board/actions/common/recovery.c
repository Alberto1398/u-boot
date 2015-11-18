/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <fs.h>
#include <power/atc260x/owl_atc260x.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>


#define CONFIG_ANDROID_RECOVERY_CMD_FILE	"recovery/command"
#define CONFIG_RECOVERYFILE_SIZE		1024

static int g_recovery_mode;

static int check_recovery_cmd_file(void)
{
	const char *devif, *bootdisk;
	char buf[CONFIG_RECOVERYFILE_SIZE];
	char dev_part_str[16];
	loff_t len_read;

	printf("check_recovery_cmd_file\n");

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	if (!devif || !bootdisk)
		return -ENOENT;

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, ANDROID_CACHE_PART);
	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_EXT) != 0) {
		printf("%s: fs_set_blk_dev error\n", __func__);
		return -1;
	}

	if (fs_read(CONFIG_ANDROID_RECOVERY_CMD_FILE, (unsigned long)buf,
		    0, 10, &len_read) <= 0) {
		printf("not found recovery cmd file\n");
		return 1;
	}

	return 0;
}

void setup_recovery_env(void)
{
	printf("setup env for recovery..\n");
	setenv("bootpart", ANDROID_RECOVERY_PART);
	g_recovery_mode = 1;
}

int owl_get_recovery_mode(void)
{
	return g_recovery_mode;
}

static int check_update_file(void)
{
	loff_t size = 0;

	if (fs_set_blk_dev("mmc", "0", FS_TYPE_ANY))
		return -1;

	if (fs_size("update.zip", &size) < 0)
		return -1;

	if (size < 1024*2)
		return -1;

	setup_recovery_env();

	return 0;
}

int check_recovery_mode(void)
{
	int ret;
	unsigned int recovery_mode;

	printf("check recovery mode\n");

	ret = atc260x_pstore_get(ATC260X_PSTORE_TAG_REBOOT_RECOVERY,
				 &recovery_mode);
	if (!ret) {
		if (recovery_mode != 0) {
			printf("PMU recovery flag founded!\n");
			atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY, 0);
			setup_recovery_env();
			return 0;
		}
	} else {
		printf("%s: failed to read pstore recovery flag\n", __func__);
	}

	if (check_recovery_cmd_file() == 0) {
		printf("Recovery command file founded!\n");
		setup_recovery_env();
	}

	printf("===check update.zip in sdcard===\n");
	if (check_update_file() != 0) {
		pinmux_select(PERIPH_ID_SDMMC0, 1);
		printf("update.zip not exist\n");
	}

	return 0;
}
