/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <linux/input.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <ext4fs.h>
#include <mmc.h>

#include <asm/arch/pmu.h>


DECLARE_GLOBAL_DATA_PTR;

int check_key_pressing(void)
{
	int key_scan_flag = 0;

	debug("check_key_pressing\n");

	key_scan_flag = adckey_scan();

	if (key_scan_flag == KEY_VOLUMEDOWN) {
		printf("volume down key\n");
		return 1;
	}

	return 0;
}

int check_recovery_cmd_file(void)
{
	static disk_partition_t info;
	char buf[CONFIG_RECOVERYFILE_SIZE];
	int err;
	block_dev_desc_t *dev_desc = NULL;
	int dev = EXT4_CACHE_DEVICE;
	int part = EXT4_CACHE_PART;
	int filelen;
	const char *ifname;

	debug("check_recovery_cmd_file\n");
	ifname = getenv("devif");
	if ( ifname == NULL) {
		ifname = "nand";
		printf("get devif fail\n");
	}

#ifdef CONFIG_MMC
	if (strcmp(ifname, "mmc") == 0) {
		struct mmc *mmc = find_mmc_device(dev);

		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return 1;
		}

		mmc->has_init = 0;
		mmc_init(mmc);
	}
#endif  /* CONFIG_MMC */

	dev_desc = get_dev(ifname, dev);
	if (dev_desc == NULL) {
		printf("Failed to find %s%d\n", ifname, dev);
		return 1;
	}

	debug("part = %d\n", part);

	if (get_partition_info(dev_desc, part, &info)) {
		printf("** get_partition_info %s%d:%d\n",
				ifname, dev, part);

		if (part != 0) {
			printf("** Partition %d not valid on device %d **\n",
					part, dev_desc->dev);
			return -1;
		}

		info.start = 0;
		info.size = dev_desc->lba;
		info.blksz = dev_desc->blksz;
		info.name[0] = 0;
		info.type[0] = 0;
		info.bootable = 0;
#ifdef CONFIG_PARTITION_UUIDS
		info.uuid[0] = 0;
#endif
	}

	ext4fs_set_blk_dev(dev_desc, &info);

	debug("info.size = %d\n", (int) info.size);

	if (!ext4fs_mount(info.size)) {
		printf("Failed to mount %s%d:%d\n",
			ifname, dev, part);
		ext4fs_close();
		return 1;
	}

	filelen = ext4fs_open(CONFIG_ANDROID_RECOVERY_CMD_FILE);
	if (filelen < 0) {
		printf("** File not found %s\n",
			CONFIG_ANDROID_RECOVERY_CMD_FILE);
		ext4fs_close();
		return 1;
	}

	debug("filelen = %d\n", filelen);

	err = ext4fs_read(buf, CONFIG_RECOVERYFILE_SIZE);
	if (err < 0) {
		printf("** File read error:  %s\n",
			CONFIG_ANDROID_RECOVERY_CMD_FILE);
		ext4fs_close();
		return 1;
	}

	ext4fs_close();

	return 0;
}

void setup_recovery_env(void)
{
	printf("setup env for recovery..\n");
#ifdef CONFIG_OWLXX_MMC
	setenv("mmcpart", RECOVERY_MMC_DEV);
#endif

#ifdef CONFIG_OWLXX_EMMC
	setenv("emmcpart", RECOVERY_EMMC_DEV);
#endif

#ifdef CONFIG_OWLXX_NAND
	setenv("nandpart", RECOVERY_NAND_DEV);
#endif
	setenv("bootdelay", 0);

	gd->flags |= GD_FLG_RECOVERY;

}

void check_recovery_mode(void)
{
	if (atc260x_pstore_get_noerr(ATC260X_PSTORE_TAG_REBOOT_RECOVERY) != 0) {
		printf("PMU recovery flag founded!\n");
		atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY, 0);
		setup_recovery_env();
	}

	if (check_recovery_cmd_file() == 0) {
		printf("Recovery command file founded!\n");
		setup_recovery_env();
	}
}
