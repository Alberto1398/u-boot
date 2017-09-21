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
#include <dt-bindings/input/input.h>
#include <asm/arch/sys_proto.h>

#define CONFIG_ANDROID_RECOVERY_CMD_FILE	"recovery/command"
#define CONFIG_RECOVERYFILE_SIZE		1024

static int g_recovery_mode;

static int check_recovery_cmd_file(void)
{
	const char *devif, *bootdisk;
	char buf[CONFIG_RECOVERYFILE_SIZE];
	char dev_part_str[16];
	loff_t len_read;

	debug("check_recovery_cmd_file\n");

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	if (!devif || !bootdisk)
		return -ENOENT;

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, ANDROID_CACHE_PART);
	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_EXT) != 0) {
		debug("%s: fs_set_blk_dev error\n", __func__);
		return -1;
	}

	if (fs_read(CONFIG_ANDROID_RECOVERY_CMD_FILE, (unsigned long)buf,
		    0, 10, &len_read) < 0) {
		debug("not found recovery cmd file\n");
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

void setup_bootloader_env(void)
{
	printf("setup env for entering uboot cmdline\n");
	setenv("owlbootcmdline", SET_ENTER_BOOT_CMDLINE);
}

void setup_fastboot_env(void)
{
	run_command(getenv("owlboot"), 0);
	setenv("bootcmd", "owlboot");
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

	if (size < 1024 * 2)
		return -1;

	setup_recovery_env();

	return 0;
}

/*if you pressed irkey,check whether enter recovery*/
#if defined(CONFIG_IR_CONTROL_RECOVERY) && defined(CONFIG_ATC260X_IRKEY)
static int owl_check_irkey(int key)
{
	int ret = 0;
	ret = key;
	if (key == IR_KEY_ENTER_RECOVERY) {
		printf("press irkey KEY_SELECT, enter recovery\n");
		setup_recovery_env();
		ret = 0;
	}

	return ret;
}

/*scan U disk */
static int usb_start(void)
{
	if (usb_init() < 0)
		return -1;
	/* Driver model will probe the devices as they are found */
#ifndef CONFIG_DM_USB
#ifdef CONFIG_USB_STORAGE
	/*try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	if (usb_stor_curr_dev == -1)
		return -1;
#endif
#endif

	return 0;
}

static int check_updatefile_fromUDISK(void)
{
	loff_t size = 0;

	if (usb_start() != 0)
		return -1;

	if (fs_set_blk_dev("usb", "0", FS_TYPE_ANY))
		return -1;

	if (fs_size("update.zip", &size) < 0)
		return -1;

	if (size < 1024 * 2)
		return -1;

	setup_recovery_env();

	return 0;
}

int ir_check(void)
{
	int irkey = 0;
	int ret = 0;

	irkey = atc260x_irkey_scan();
	printf("irkey=%d\n", irkey);
	if (irkey == IR_KEY_UDISK_DETECT) {
		printf
		    ("Detection,Whether there is a update.zip file in the U disk\n");
		if (check_updatefile_fromUDISK() != 0) {
			printf("don't found update.zip in U disk!\n");
			if (usb_stop() != 0) {
				printf("usb stop failed!\n");
			}
		} else {
			printf("found update.zip in U disk!\n");
			return 0;
		}
		printf
		    ("Detected Whether  the presence of update.zip files in sd\n");
		if (check_update_file() != 0) {
			pinmux_select(PERIPH_ID_SDMMC0, 1);
		} else {
			printf("found update.zip in sdcard!\n");
			return 0;
		}
		printf("press ir KEY_BACK going recovery!\n");
		setup_recovery_env();
		return 0;
	} else {
		printf("you don't press ir KEY_BACK,so didn't check u disk\n");
	}
	ret = owl_check_irkey(irkey);
	if (ret != 0) {
		printf("you don't press KEY_SELECT\n");
	}
	return ret;
}
#endif
int check_recovery_mode(void)
{
	int ret;
	int irkey = 0;
	unsigned int recovery_mode;

	ret = atc260x_pstore_get(ATC260X_PSTORE_TAG_REBOOT_RECOVERY,
				 &recovery_mode);
	printf("check_recovery_mode ret = %d,recovery_mode = %d\n", ret,
	       recovery_mode);
	if (!ret) {
		if (recovery_mode == 1) {
			printf("PMU recovery flag founded!\n");
			atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY, 0);
			setup_recovery_env();
			return 0;
		} else if (recovery_mode == 2) {
			atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY, 0);
			setup_bootloader_env();
			return 0;
		} else if (recovery_mode == 3) {
			atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY, 0);
			setup_fastboot_env();
			return 0;
		}
	} else {
		printf("%s: failed to read pstore recovery flag\n", __func__);
	}

	if (check_recovery_cmd_file() == 0) {
		printf("Recovery command file founded!\n");
		setup_recovery_env();
	}
#ifdef CONFIG_BOOTDEV_AUTO
	if (owl_get_bootdev() == BOOTDEV_SD)  // boot from sd, not check update.zip
		return 0; 
#endif
#if defined(CONFIG_IR_CONTROL_RECOVERY) && defined(CONFIG_ATC260X_IRKEY)
	ir_check();
#else
	printf("Detected  the presence of update.zip files in sd\n");
	if (check_update_file() != 0) {
#ifndef CONFIG_OWL_CARD_BURN
		pinmux_select(PERIPH_ID_SDMMC0, 1);
		printf("change uart\n");
#endif
	} else {
		printf("found update.zip in sdcard!\n");
	}
#endif
	return 0;
}
