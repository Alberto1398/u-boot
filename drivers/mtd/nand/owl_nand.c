
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <part.h>
#include <fdtdec.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>

#include "mbr_info.h"
#include "afinfo.h"

DECLARE_GLOBAL_DATA_PTR;

extern int LDL_DeviceOpReadSectors(unsigned int start, unsigned int nsector, void *buf, int diskNo);
extern int LDL_DeviceOpWriteSectors(unsigned int start, unsigned int nsector, void *buf, int diskNo);
extern int LDL_DeviceOperateInit(void);
extern void LDL_DeviceOperateRelease(void);
extern void LDL_DeviceOperateFreeze(void);

static partition_info_t nand_partition_info[MAX_PARTITION];
static int nand_initialized = 0;
static int initialized_partions = 0;

struct owlxx_block_dev {
	uint32_t off;
	int ptn;
	block_dev_desc_t blk_dev;
};

static struct owlxx_block_dev nand_dev[MAX_PARTITION];

int nand_sync(void)
{

	return 0;
}

static unsigned long owlxx_nand_block_read(int dev,
				      unsigned long start,
				      lbaint_t blkcnt,
				      void *buffer)
{
	struct owlxx_block_dev *blkdev = &nand_dev[dev];

	if (!nand_initialized) {
		printf("nand has not been initialized.\n");
		return -1;
	}

	if (dev >= initialized_partions) {
		printf("%s partion %d not initialized.\n", __FUNCTION__, dev);
		return -1;
	}

	if (blkdev->ptn <= 0) {
		printf("%s partion %d is physical.\n", __FUNCTION__, dev);
		return -1;
	}

	if (start >= blkdev->blk_dev.lba 
		|| (start + blkcnt) >= blkdev->blk_dev.lba
		|| (start + blkcnt) <= start
		|| buffer == NULL) {
		printf("%s: invalid param, dev %d, start %lu, blkcnt: %u, buffer: 0x%p, lba is %u\n", 
			__FUNCTION__, dev, start, (unsigned)blkcnt, buffer, (unsigned)blkdev->blk_dev.lba);
		return -1;
	}

	if (LDL_DeviceOpReadSectors(start + blkdev->off, blkcnt, buffer, blkdev->ptn - 1) != 0) {
		printf("%s dev[%d] %d %d 0x%p ERROR\n", __FUNCTION__, dev, (int)start, (int)blkcnt, buffer);
		return -1;
	}

	return blkcnt;
}

static unsigned long owlxx_nand_block_write(int dev,
				      unsigned long start,
				      lbaint_t blkcnt,
				      const void *buffer)
{
	struct owlxx_block_dev *blkdev = &nand_dev[dev];

	if (!nand_initialized) {
		printf("nand has not been initialized.\n");
		return -1;
	}

	if (dev >= initialized_partions) {
		printf("%s partion %d not initialized.\n", __FUNCTION__, dev);
		return -1;
	}

	if (blkdev->ptn <= 0) {
		printf("%s partion %d is physical.\n", __FUNCTION__, dev);
		return -1;
	}

	if (start >= blkdev->blk_dev.lba 
		|| (start + blkcnt) > blkdev->blk_dev.lba
		|| (start + blkcnt) <= start
		|| buffer == NULL) {
		printf("%s: invalid param, dev %d, start %lu, blkcnt: %u, buffer: 0x%p, lba is %u\n", 
			__FUNCTION__, dev, start, (unsigned)blkcnt, buffer, (unsigned)blkdev->blk_dev.lba);
		return -1;
	}

	if (LDL_DeviceOpWriteSectors(start + blkdev->off, blkcnt, buffer, blkdev->ptn - 1) != 0) {
		printf("%s dev[%d] %d %d 0x%p ERROR\n", __FUNCTION__, dev, (int)start, (int)blkcnt, buffer);
		return -1;
	}

	return blkcnt;
}

int nand_init(void)
{
	struct owlxx_block_dev *blkdev;
	int i;
	void *blob = gd->fdt_blob;
	unsigned int offset;
	int last_ptn = -1;

	printf("init owl NAND..\n");

	memset(&nand_dev, 0, sizeof(nand_dev));

	if (LDL_DeviceOperateInit() != 0) {
		printf("init NAND err!.\n");
		return -1;
	}

	nand_initialized = 1;
	memcpy(&nand_partition_info, afinfo->partition_info, 
			sizeof(partition_info_t) * MAX_PARTITION);

	for (i = 0; i < MAX_PARTITION; i++) {
		debug("%d: flash_ptn %d: partition_num %d, partition_cap 0x%x.\n",
			i, 
			nand_partition_info[i].flash_ptn,
			nand_partition_info[i].partition_num,
			nand_partition_info[i].partition_cap);
	}

	offset = 0;
	for (i = 0; i < MAX_PARTITION; i++) {
		blkdev = &nand_dev[i];

		blkdev->blk_dev.if_type = -1;
		blkdev->blk_dev.dev = i;

		if ((int)nand_partition_info[i].partition_cap == -1) {
			break;
		}

		if (last_ptn != nand_partition_info[i].flash_ptn) {
			blkdev->off = 0;
			offset = nand_partition_info[i].partition_cap;
			last_ptn = nand_partition_info[i].flash_ptn;
		}
		else {
			blkdev->off = offset << (20 - 9);
			offset += nand_partition_info[i].partition_cap;
		}

		initialized_partions++;

		blkdev->ptn = nand_partition_info[i].flash_ptn;
		blkdev->blk_dev.lba = nand_partition_info[i].partition_cap;
		blkdev->blk_dev.lba <<= (20 - 9);

		blkdev->blk_dev.blksz = 512;
#ifdef CONFIG_LBA48
		blkdev->blk_dev.lba48 = 1;
#endif
		blkdev->blk_dev.block_read = owlxx_nand_block_read;
		blkdev->blk_dev.block_write = owlxx_nand_block_write;

		init_part(&blkdev->blk_dev);
		debug("init nand dev: %d, off: %u, n_sectors: %u.\n", i, 
				blkdev->off, (u32)blkdev->blk_dev.lba);
	}

	return 0;
exit:
	printf("init nand error.\n");
	return -1;
}

#ifdef CONFIG_PARTITIONS

block_dev_desc_t *owlxx_nand_get_dev(int dev)
{
	struct owlxx_block_dev *blkdev;

	if (dev >= initialized_partions) {
		printf("ERROR dev num %d.\n", dev);
		return NULL;
	}

	blkdev = &nand_dev[dev];
	if (blkdev->blk_dev.dev != dev) {
		printf("dev %d not init.\n", dev);
		return NULL;
	}
	return &blkdev->blk_dev;
}

#endif

static int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	ulong off;
	ulong size;
	char *cmd;
	struct owlxx_block_dev *blkdev;

	if (!nand_initialized) {
		printf("nand has not been initialized.\n");
		return -1;
	}

	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (!strncmp(cmd, "info", 4)) {
		int i;

		for (i = 0; i < MAX_PARTITION; i++) {
			printf("%d: flash_ptn %d: partition_num %d, partition_cap 0x%x.\n",
				i, 
				nand_partition_info[i].flash_ptn,
				nand_partition_info[i].partition_num,
				nand_partition_info[i].partition_cap);
		}

		for (i = 0; i < initialized_partions; i++) {
			blkdev = &nand_dev[i];
			printf("nand dev: %d, off: %u, n_sectors: %u.\n", 
					i, blkdev->off, (u32)blkdev->blk_dev.lba);
		}
	}

usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"NAND sub-system",
	"\n"
	"read - mem nand_lba sector_count, sector size 512\n"
	"write - mem nand_lba sector_count, sector size 512"
);

