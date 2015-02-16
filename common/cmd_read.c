/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <part.h>

/* 这里将原来的单个do_read()函数拆开3个函数, 其中的do_common_partn_read()可供外部调用. */

int do_common_partn_read2(const char *dev_ifname, int dev, int part,
		void *addr, uint blk, uint cnt)
{
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t part_info;
	ulong offset = 0u;
	ulong limit = 0u;

	dev_desc = get_dev(dev_ifname, dev);
	if (dev_desc == NULL) {
		printf("Block device %s %d not supported\n", dev_ifname, dev);
		return 1;
	}

	if (part != 0) {
		if (get_partition_info(dev_desc, part, &part_info)) {
			printf("Cannot find partition %d\n", part);
			return 1;
		}
		offset = part_info.start;
		limit = part_info.size;
	} else {
		/* Largest address not available in block_dev_desc_t. */
		limit = ~0;
	}

	if (cnt + blk > limit) {
		printf("Read out of range\n");
		return 1;
	}

	if (dev_desc->block_read(dev, offset + blk, cnt, addr) < 0) {
		printf("Error reading blocks\n");
		return 1;
	}

	return 0;
}

int do_common_partn_read(const char *dev_ifname, const char *dev_part_str,
		void *addr, uint blk, uint cnt)
{
	char *ep;
	int dev;
	int part = 0;

	dev = (int)simple_strtoul(dev_part_str, &ep, 16);
	if (*ep) {
		if (*ep != ':') {
			printf("Invalid block device %s\n", dev_part_str);
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	return do_common_partn_read2(dev_ifname, dev, part, addr, blk, cnt);
}

int do_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *dev_ifname;
	const char *dev_part_str;
	void *addr;
	uint blk;
	uint cnt;

	if (argc != 6) {
		cmd_usage(cmdtp);
		return 1;
	}

	dev_ifname = argv[1];
	dev_part_str = argv[2];

	addr = (void *)simple_strtoul(argv[3], NULL, 16);
	blk = simple_strtoul(argv[4], NULL, 16);
	cnt = simple_strtoul(argv[5], NULL, 16);

	return do_common_partn_read(dev_ifname, dev_part_str, addr, blk, cnt);
}

U_BOOT_CMD(
	read,	6,	0,	do_read,
	"Load binary data from a partition",
	"<interface> <dev[:part]> addr blk# cnt"
);
