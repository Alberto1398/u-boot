/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <part.h>
#include <sparse_format.h>

#define MIN(x,y) (x < y ? x : y)
#define ROUNDUP(x, y)	(((x) + ((y) - 1)) & ~((y) - 1))

void fastboot_fail(const char *s);
void fastboot_okay(const char *s);

static inline int get_order(uint32_t size, uint32_t blksz)
{
	size--;
	return fls(size / blksz);
}

static inline int is_sparse_image(void *buf)
{
	sparse_header_t *s_header = (sparse_header_t *)buf;

	if ((le32_to_cpu(s_header->magic) == SPARSE_HEADER_MAGIC) &&
	    (le16_to_cpu(s_header->major_version) == 1))
		return 1;

	return 0;
}

void write_sparse_image(block_dev_desc_t *dev_desc,
		disk_partition_t *info, const char *part_name,
		void *data, unsigned sz);
