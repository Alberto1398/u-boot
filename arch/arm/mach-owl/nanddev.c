/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <part.h>

DECLARE_GLOBAL_DATA_PTR;

#define NAND_MAX_DEV_NUM	1

extern int LDL_DeviceOpReadSectors(unsigned int start, unsigned int nsector,
		void *buf, int disk);
extern int LDL_DeviceOpWriteSectors(unsigned int start, unsigned int nsector,
		const void *buf, int disk);
extern int LDL_DeviceOperateInit(void);
extern void LDL_DeviceOperateRelease(void);
extern void LDL_DeviceOperateFreeze(void);
extern int LDL_GetPartitonMsize(int partition);

static int nand_initialized;

struct owl_block_dev {
	uint32_t off;
	int ptn;
	block_dev_desc_t blk_dev;   /* uboot/include/part.h */
};

static struct owl_block_dev nand_dev[NAND_MAX_DEV_NUM];

int nand_sync(void)
{
	return 0;
}

static unsigned long owl_nand_block_read(int dev,
			unsigned long start,
			lbaint_t blkcnt,
			void *buffer)
{
	struct owl_block_dev *blkdev = &nand_dev[dev];

	if (!nand_initialized) {
		printf("nand has not been initialized.\n");
		return -1;
	}

	if (dev >= NAND_MAX_DEV_NUM) {
		printf("%s partion %d not initialized.\n", __func__, dev);
		return -1;
	}

	if (blkdev->ptn <= 0) {
		printf("%s partion %d is physical.\n", __func__, dev);
		return -1;
	}

	if (blkcnt == 0)
		return 0;

	if (start >= blkdev->blk_dev.lba ||
	    (start + blkcnt) > blkdev->blk_dev.lba ||
	    (start + blkcnt) <= start ||
	    buffer == NULL) {
		printf("%s: invalid param, dev %d, start %lu, blkcnt: %u, "
		       "buffer: 0x%p, lba is %u\n",
		       __func__, dev, start, (unsigned)blkcnt, buffer,
		       (unsigned)blkdev->blk_dev.lba);
		return -1;
	}

	if (LDL_DeviceOpReadSectors(start + blkdev->off, blkcnt, buffer,
				    blkdev->ptn - 1) != 0) {
		printf("%s dev[%d] %d %d 0x%p ERROR\n",
		       __func__, dev, (int)start, (int)blkcnt, buffer);
		return -1;
	}

	return blkcnt;
}

static unsigned long owl_nand_block_write(int dev,
				      unsigned long start,
				      lbaint_t blkcnt,
				      const void *buffer)
{
	struct owl_block_dev *blkdev = &nand_dev[dev];

	if (!nand_initialized) {
		printf("nand has not been initialized.\n");
		return -1;
	}

	if (dev >= NAND_MAX_DEV_NUM) {
		printf("%s partion %d not initialized.\n", __func__, dev);
		return -1;
	}

	if (blkdev->ptn <= 0) {
		printf("%s partion %d is physical.\n", __func__, dev);
		return -1;
	}

	if (start >= blkdev->blk_dev.lba ||
	    (start + blkcnt) > blkdev->blk_dev.lba ||
	    (start + blkcnt) <= start ||
	    buffer == NULL) {
		printf("%s: invalid param, dev %d, start %lu, blkcnt: %u, "
		       "buffer: 0x%p, lba is %u\n",
		       __func__, dev, start, (unsigned)blkcnt, buffer,
		       (unsigned)blkdev->blk_dev.lba);
		return -1;
	}

	if (LDL_DeviceOpWriteSectors(start + blkdev->off, blkcnt, buffer,
				     blkdev->ptn - 1) != 0) {
		printf("%s dev[%d] %d %d 0x%p ERROR\n", __func__, dev,
		       (int)start, (int)blkcnt, buffer);
		return -1;
	}

	return blkcnt;
}

int RegisterOneDevice(void)
{
	unsigned int offset;
	struct owl_block_dev *blkdev;

	blkdev = &nand_dev[0];
	blkdev->blk_dev.if_type = -1;
	blkdev->blk_dev.dev = 0;
	blkdev->off = 0;
	offset = LDL_GetPartitonMsize(0);
	if (offset == -1) {
		printf("LDL_GetPartitonMsize err!\n");
		return -1;
	}
	printf("PartitionMsize[0] = %d MB\n", offset);

	blkdev->ptn = 1;
	blkdev->blk_dev.lba = offset;
	blkdev->blk_dev.lba <<= (20 - 9);
	printf("blkdev->blk_dev.lba = %d\n", blkdev->blk_dev.lba);
	blkdev->blk_dev.blksz = 512;
	blkdev->blk_dev.block_read = owl_nand_block_read;
	blkdev->blk_dev.block_write = owl_nand_block_write;
	blkdev->blk_dev.part_type = PART_TYPE_EFI;

	return 0;
}

int nand_init(void)
{
	printf("init owl NAND...");

	/* nand_dev array init */
	memset(&nand_dev, 0, sizeof(nand_dev));

	/* public init,physical init,run func:UTL_UnitCreate */
	if (LDL_DeviceOperateInit() != 0) {
		printf("init NAND err!.\n");
		return -1;
	}
	nand_initialized = 1;

	if (RegisterOneDevice() < 0)
		goto exit;

	return 0;
exit:
	printf("init nand error.\n");
	return -1;
}

void nand_uninit(void)
{
	if (nand_initialized) {
		printf("freeze owl NAND..\n");
		LDL_DeviceOperateFreeze();
	}
}

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *owl_nand_get_dev(int dev)
{
	struct owl_block_dev *blkdev;

	if (dev >= NAND_MAX_DEV_NUM) {
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
