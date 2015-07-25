/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
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

#include <errno.h>
#include <common.h>
#include <command.h>
#include <g_dnl.h>
#include <usb_mass_storage.h>

static void dump_buf(u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%02x ", *(buf + i));
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}

static int ums_read_sector(struct ums *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;
	
	if ( blkcnt == block_dev->block_read(dev_num, blkstart, blkcnt, buf) ) {
		//printf("%d,%d:\n",blkstart, blkcnt);
		//dump_buf((u8 *)buf, 64);
		return 0;
	}
	printf("read sector %d,%d\n", start, blkcnt);
	return -1;
}

static int ums_write_sector(struct ums *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	if ( blkcnt == block_dev->block_write(dev_num, blkstart, blkcnt, buf) )
		return 0;
	
	printf("read sector %d,%d\n", start, blkcnt);
	return -1;

}

static struct ums ums_dev = {
	.read_sector = ums_read_sector,
	.write_sector = ums_write_sector,
	.name = "ums",
};

struct ums *ums_init(const char *devtype, const char *devnum)
{
	block_dev_desc_t *block_dev;
	int ret;

	ret = get_device(devtype, devnum, &block_dev);
	if (ret < 0)
		return NULL;

	/* f_mass_storage.c assumes SECTOR_SIZE sectors */
	if (block_dev->blksz != SECTOR_SIZE)
		return NULL;

	ums_dev.block_dev = block_dev;
	ums_dev.start_sector = 0;
	ums_dev.num_sectors = block_dev->lba;

	printf("UMS-1: disk start sector: %#x, count: %#x\n",
	       ums_dev.start_sector, ums_dev.num_sectors);

	return &ums_dev;
}


int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag,
			       int argc, char * const argv[])
{

	const char *devtype;
	const char *devnum;
	struct ums *ums;

	int rc;
	if (argc != 3 )
		return CMD_RET_USAGE;

	dcache_disable();
	devtype = argv[1];
	devnum  = argv[2];

	ums = ums_init(devtype, devnum);
	if (!ums)
		return CMD_RET_FAILURE;

	rc = fsg_init(ums);
	if (rc) {
		error("fsg_init failed");
		return CMD_RET_FAILURE;
	}

	g_dnl_register(ums->name);
	
	while (1) {
		/* Handle control-c and timeouts */
		if (ctrlc()) {
			printf("The remote end did not respond in time.\n");
			goto exit;
		}
		usb_gadget_handle_interrupts();
		/* Check if USB cable has been detached */
		if (fsg_main_thread(NULL) == EIO)
			goto exit;
	}	
exit:
	dcache_enable();
	g_dnl_unregister();
	return 0;

fail:
	return -1;
}

U_BOOT_CMD(ums, 3, 1, do_usb_mass_storage,
	"Use the UMS [User Mass Storage]",
	"ums - User Mass Storage Gadget"
);
