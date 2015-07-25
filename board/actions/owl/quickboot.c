/*
 * Copyright 2013 Actions Semi Inc.
 * Author: Actions Semi, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <u-boot/zlib.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/lzo.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/quickboot.h>

#define SWSUSP_SIG "S1SUSPEND"
#define __NEW_UTS_LEN 64
#define PAGE_SIZE 4096

typedef u64 sector_t;

#define MAP_PAGE_ENTRIES	(PAGE_SIZE / sizeof(sector_t) - 1)
#define MAP_PAGE_METAS	    (PAGE_SIZE / sizeof(unsigned int))

#define pgoff_t u32

#define STORAGE_SECTOR_SIZE 512
#define SECTORS_PER_BLOCK (PAGE_SIZE / STORAGE_SECTOR_SIZE)

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define lzo1x_worst_compress(x) ((x) + ((x) / 16) + 64 + 3)

#define LZO_HEADER	sizeof(size_t)

#define LZO_UNC_PAGES	32
#define LZO_UNC_SIZE	(LZO_UNC_PAGES * PAGE_SIZE)

#define LZO_CMP_PAGES	DIV_ROUND_UP(lzo1x_worst_compress(LZO_UNC_SIZE) + \
				LZO_HEADER, PAGE_SIZE)
#define LZO_CMP_SIZE	(LZO_CMP_PAGES * PAGE_SIZE)

/* #define USE_ZLIB */
#define zlib_worst_compress(s) (s + ((s + 7) >> 3) + ((s + 63) >> 6) + 11)
#define ZLIB_CMP_PAGES DIV_ROUND_UP(zlib_worst_compress(LZO_UNC_SIZE) + \
				LZO_HEADER, PAGE_SIZE)
#define ZLIB_CMP_SIZE	(ZLIB_CMP_PAGES * PAGE_SIZE)
#define DEFLATE_DEF_WINBITS		12

#ifdef USE_ZLIB
#define CMP_SIZE ZLIB_CMP_SIZE
#define worst_compress(x) zlib_worst_compress(x)
#else
#define CMP_SIZE LZO_CMP_SIZE
#define worst_compress(x) lzo1x_worst_compress(x)
#endif

#define SCU_CTRL		0x00
#define SCU_CONFIG		0x04
#define SCU_CPU_STATUS		0x08
#define SCU_INVALIDATE		0x0c
#define SCU_FPGA_REVISION	0x10

#define SCU_BASE 0xB0020000
#define CPU1_ADDR 0xB0168050
#define CPU1_FLAG 0xB016805C

#define NR_CPU 4
#define STACK_SIZE (400 * 1024)

#define CMU_CORECTL 0xB016009C
#define SPS_PG_CTL 0xb01b0100

#define OWLXX_DMA_IRQ_EN_0 0xb0260010
#define OWLXX_DMA_IRQ_EN_1 0xb0260014
#define OWLXX_DMA_IRQ_EN_2 0xb0260018
#define OWLXX_DMA_IRQ_EN_3 0xb026001C

/* #define TEST_SPEED_ALL */
#define TEST_SPEED_VERBOSE

#ifdef TEST_SPEED_VERBOSE
#define STAT_BEGIN	writel(0, T1_CTL); \
			writel(0, T1_VAL); \
			writel(4, T1_CTL);
#define STAT_END(x)	x += readl(T1_VAL);
#else
#define STAT_BEGIN
#define STAT_END(x)
#endif

#define CACHE_SIZE (592 * 1024)

DECLARE_GLOBAL_DATA_PTR;

struct swsusp_header {
	char reserved[PAGE_SIZE - 20 - sizeof(sector_t) - sizeof(int) -
			sizeof(u32)];
	u32	crc32;
	sector_t image;
	u32 flags;	/* Flags to pass to the "boot" kernel */
	char	orig_sig[10];
	char	sig[10];
} __packed;

struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

struct swsusp_info {
	struct new_utsname	uts;
	u32			version_code;
	u32			num_physpages;
	int			cpus;
	u32		image_pages;
	u32		pages;
	u32		size;
	u32		nosave_begin;
	u32		nosave_end;
	u32		swsusp_arch_resume_begin;
	u32		cpu_reset_begin;
} __aligned(PAGE_SIZE);

struct swap_map_page {
	sector_t entries[MAP_PAGE_ENTRIES];
	sector_t next_swap;
};

struct swap_map_handle {
	struct swap_map_page *cur;	/* PAGE_SIZE aligned */

	sector_t cur_swap;
	sector_t first_sector;
	u32 k;
	u32 meta_k;
	/* buffer for meta data, PAGE_SIZE aligned */
	u32 *meta_data;
	/* buffer for compressed data, PAGE_SIZE aligned */
	u8 *cmp[NR_CPU - 1];
	/* buffer for decompress output of copy page */
	u8 *copy_data[NR_CPU - 1];

	u32 crc32;
	u8 *cache;
	pgoff_t cache_sector[4];
	u32 *cmp_map[NR_CPU - 1];
	u32 cur_cache_pos;

#ifdef USE_ZLIB
	struct z_stream_s stream[NR_CPU - 1];
#endif
	struct {
		u32 cmp_len;        /* length of compressed data block */
		u32 start;          /* tell non-boot cpu to start decompress */
		u32 complete_all;   /* tell non-boot cpu whole image loaded */
		u32 meta_k;
	} __aligned(32) _start[NR_CPU - 1];

	struct {
		/* for non-boot cpu tell the boot cpu decompress finished */
		u32 finish;
		u32 ret;
		u32 out_len;
		u32 finish_all;
	} __aligned(32) _finish[NR_CPU - 1];

};

static struct swsusp_header *swsusp_header;
static struct swsusp_info *swsusp_info;

static struct swap_map_handle handle;
static u32 nr_copy_pages;
static u32 nr_meta_pages;
static u32 compressed_len;
static u32 compress_blk_nr;

static block_dev_desc_t *dev_desc;
static disk_partition_t info;
static u8 *p_stack_pointer;

/* Statistics */
static u32 stat_read_nand;
static u32 stat_wait;
static u32 read_4page_counts;
static u32 read_1page_counts;
static u32 stat_load_meta;
static u32 stat_copy_mem;
static u32 stat_clean_cache;
static u32 stat_whole;

static int checked;
static int snapshot_image_exist;

static void clean_range(u32 start, u32 stop)
{
	u32 mva;

	for (mva = start; mva < stop; mva = mva + 32) {
		/* DCCIMVAC - Clean & Invalidate data cache by MVA to PoC */
		asm volatile ("mcr p15, 0, %0, c7, c10, 1" : : "r" (mva));
	}
}

static void inv_range(u32 start, u32 stop)
{
	u32 mva;

	for (mva = start; mva < stop; mva = mva + 32) {
		/* DCIMVAC - Invalidate data cache by MVA to PoC */
		asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
	}
}


static void dump_ram(u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%02x ", *(buf + i));
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}

static void smp_enable(void)
{
	int i;
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
	u32 pg, start;
	u32 scu_ctrl;

	p_stack_pointer = malloc(STACK_SIZE * (NR_CPU - 1));
	for (i = 0; i < NR_CPU - 1; i++) {
		stack_pointer[i] = (u32)p_stack_pointer +
					(i + 1) * STACK_SIZE - 32;
		handle._start[i].start = 0;
		start = (u32)&handle._start[i];
		clean_range(start, start + 32);
	}
	pg = (u32) page_table;
	pg_dir = pg;

	scu_ctrl = readl(SCU_BASE + SCU_CTRL);
	scu_ctrl = scu_ctrl | 1;
	writel(scu_ctrl, SCU_BASE + SCU_CTRL);

	/*
	 * Ensure that the data accessed by CPU0 before the SCU was
	 * initialised is visible to the other CPUs.
	 */
	flush_dcache_all();

	/*
	 * Invalidate all instruction caches to PoU.
	 * Also flushes branch target cache.
	 */
	asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
	/* Invalidate entire branch predictor array */
	asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
	CP15DSB;
	CP15ISB;

	for (i = 0; i < NR_CPU - 1; i++) {
		writel((u32)secondary_startup, CPU1_ADDR + 4 * i);
		writel(0x55aa, CPU1_FLAG + 4 * i);
	}

	asm volatile("dsb\n"
		"sev\n"
		"nop\n"
	);
}


static void power_on_cpu(u32 nr)
{
	u32 status, shift, i;
	u32 v;

	if (nr != 2 && nr != 3) {
		printf("power_on_cpu invalid cpu nr %d\n", nr);
		return;
	}
	shift = (nr == 2) ? 21 : 22;
	status = readl(SPS_PG_CTL) & (1 << shift);
	printf("power_on_cpu %d status %x\n", nr, status);
	if (status)
		return;

	shift = (nr == 2) ? 2 : 3; /* clk en */
	v = readl(CMU_CORECTL);
	v &= ~(1 << shift);
	writel(v, CMU_CORECTL);

	shift = (nr == 2) ? 6 : 7; /* core reset */
	v = readl(CMU_CORECTL);
	v &= ~(1 << shift);
	writel(v, CMU_CORECTL);

	shift = (nr == 2) ? 5 : 6;
	v = readl(SPS_PG_CTL);
	v &= ~(1 << shift);
	writel(v, SPS_PG_CTL);

	shift = (nr == 2) ? 21 : 22;
	for (i = 0; i < 1000; i++) {
		udelay(50);

		status = readl(SPS_PG_CTL) & (1 << shift);
		if (status)
			break;
	}

	if (i >= 1000) {
		printf("Timeout power on cpu%d\n", nr);
		return;
	}
	shift = (nr == 2) ? 2 : 3;
	v = readl(CMU_CORECTL);
	v |= (1 << shift);
	writel(v, CMU_CORECTL); /* clk en */

	shift = (nr == 2) ? 6 : 7;
	v = readl(CMU_CORECTL);
	v |= (1 << shift);
	writel(v, CMU_CORECTL); /* core reset */
}


static void release_swap_reader(void)
{
	int i;
	if (handle.cur)
		free(handle.cur);

	if (handle.meta_data)
		free(handle.meta_data);

	for (i = 0; i < NR_CPU - 1; i++) {
		if (handle.copy_data[i])
			free(handle.copy_data[i]);
	}
}

static void swsusp_info_release(void)
{
	if (swsusp_info)
		free(swsusp_info);
}

static int swsusp_header_release(void)
{
	if (swsusp_header)
		free(swsusp_header);
	return 0;
}

static int swapdisk_read_page(pgoff_t page_off, void *addr, u32 len,
				u32 *cache_pos)
{
	u32 i;
	void *dst, *src;
	u32 read_len;

	if (len == 4 * PAGE_SIZE) {
		dst = addr;
		read_len = 4 * PAGE_SIZE;
		read_4page_counts++;
		if (cache_pos)
			*cache_pos = 0xffffffff;
	} else {
		for (i = 0; i < 4; i++) {
			if (handle.cache_sector[i] == page_off) {
				if (cache_pos == NULL) {
					STAT_BEGIN;
					src = handle.cache +
						(handle.cur_cache_pos + i)
							* PAGE_SIZE;
					memcpy(addr,
					       src,
					       PAGE_SIZE);
					STAT_END(stat_copy_mem)
				} else {
					*cache_pos = handle.cur_cache_pos + i;
				}
				return 0;
			}
		}

		if (page_off % 4) {
			dst = addr;
			read_len = PAGE_SIZE;
			read_1page_counts++;
			if (cache_pos)
				*cache_pos = 0xffffffff;
		} else {
			/* read 16k to cache */
			handle.cur_cache_pos += 4;
			if (handle.cur_cache_pos >= CACHE_SIZE / PAGE_SIZE)
				handle.cur_cache_pos = 0;
			dst = handle.cache + handle.cur_cache_pos * PAGE_SIZE;
			if (cache_pos)
				*cache_pos = handle.cur_cache_pos;
			read_len = 4 * PAGE_SIZE;
			read_4page_counts++;
		}
	}
	STAT_BEGIN
	dev_desc->block_read(dev_desc->dev,
			     page_off * (PAGE_SIZE / dev_desc->blksz),
			     (read_len / dev_desc->blksz),
			     dst);
	STAT_END(stat_read_nand)
	if (len == PAGE_SIZE && (page_off % 4 == 0)) {
		handle.cache_sector[0] = page_off;
		handle.cache_sector[1] = page_off + 1;
		handle.cache_sector[2] = page_off + 2;
		handle.cache_sector[3] = page_off + 3;
		if (cache_pos == NULL) {
			STAT_BEGIN;
			memcpy(addr, dst, PAGE_SIZE);
			STAT_END(stat_copy_mem);
		}
	}

	return 0;
}

static int swap_read_page(void *buf, u32 *plen, u32 *cache_pos)
{
	sector_t offset, offset_1, offset_2, offset_3;
	int error = 0;

	if (!handle.cur) {
		error = -EINVAL;
		printf("swap_read_page: handle->cur invalid\n");
		goto end;
	}

	if (handle.k <= 507 && *plen >= 4 * PAGE_SIZE) { /* 507,508,509,510 */
		offset = handle.cur->entries[handle.k];
		if (offset % 4 == 0) {
			offset_1 = handle.cur->entries[handle.k + 1];
			offset_2 = handle.cur->entries[handle.k + 2];
			offset_3 = handle.cur->entries[handle.k + 3];

			if ((offset_1 == offset + 1) &&
			    (offset_2 == offset_1 + 1) &&
			    (offset_3 == offset_2 + 1)) {
				swapdisk_read_page(offset, buf,
						   4 * PAGE_SIZE, cache_pos);
				handle.k = handle.k + 3;
				*plen = 4 * PAGE_SIZE;
				goto out;
			}
		}
	}

	*plen = PAGE_SIZE;
	offset = handle.cur->entries[handle.k];
	if (!offset) {
		error = -ENODATA;
		printf("swap_read_page: entry is invalid\n");
		goto end;
	}

	error = swapdisk_read_page(offset, buf, PAGE_SIZE, cache_pos);
	if (error) {
		printf("swap_read_page: swapdisk_read_page fail\n");
		goto end;
	}

out:	if (++handle.k >= MAP_PAGE_ENTRIES) {
		handle.k = 0;
		offset = handle.cur->next_swap;

		if (!offset)
			release_swap_reader();
		else if (!error)
			error = swapdisk_read_page(offset, handle.cur,
						   PAGE_SIZE, NULL);

	}

end:
	return error;
}

static int wait_cpu_done_decompress(u32 nr)
{
	u32 start, i;
	static u32 finish_counter[NR_CPU - 1] = {0,};

	for (i = 0; i < 1000000; i++) { /* timeout 1s */
		start = (u32)&handle._finish[nr];
		inv_range(start, start + 32);

		if (handle._finish[nr].finish != finish_counter[nr]) {
			finish_counter[nr] = handle._finish[nr].finish;
#ifdef USE_ZLIB
			if (handle._finish[nr].ret != Z_STREAM_END) {
#else
			if (handle._finish[nr].ret != 0) {
#endif
				printf("cpu%d decompress fail blk_nr %d\n",
				       nr + 1, compress_blk_nr);
				return handle._finish[nr].ret;
			}
			break;
		}
		udelay(1);
	}
	if (i >= 1000000) {
		printf("Timeout wait cpu%d decompress\n", nr);
		return -EIO;
	}
	return 0;
}

/* read a compressed data block from disk,
 * one data block is compressed data from 32 pages of original data
 * nr: cpu number
 */
static int read_one_compress_block(u32 nr, u32 meta)
{
	int ret = 0;
	u32 len;
	u32 out_len;
	u32 *map = handle.cmp_map[nr];
	u32 cache_pos = 0;
	u32 i = 0, j;
	u32 start;

	out_len = PAGE_SIZE;
	if (meta) {
		ret = swap_read_page(handle.cmp[nr], &out_len, NULL);
	} else {
		ret = swap_read_page(handle.cmp[nr], &out_len, &cache_pos);
		map[i++] = cache_pos;
		if (cache_pos != 0xffffffff)
			start = (u32)handle.cache + cache_pos * PAGE_SIZE;
		else
			start = (u32)handle.cmp[nr];

		STAT_BEGIN;
		clean_range(start, start + PAGE_SIZE);
		STAT_END(stat_clean_cache)
	}

	if (ret != 0) /* error or end of file */
		return ret;

	/* first dword stores the compressed data length */
	if (meta || cache_pos == 0xffffffff)
		handle._start[nr].cmp_len = *(u32 *)(handle.cmp[nr]);
	else
		handle._start[nr].cmp_len = *(u32 *)(handle.cache +
			cache_pos * PAGE_SIZE);
	compressed_len += handle._start[nr].cmp_len;

	if (unlikely((!handle._start[nr].cmp_len) ||
		     handle._start[nr].cmp_len >
		     worst_compress(LZO_UNC_SIZE))) {
		printf("read_ Invalid compressed len %d\n",
		       handle._start[nr].cmp_len);
		ret = -1;
		return ret;
	}
	len = PAGE_SIZE;
	while (len < handle._start[nr].cmp_len + 4) {
		out_len = handle._start[nr].cmp_len + 4 - len;
		if (meta)
			ret = swap_read_page(handle.cmp[nr] + len,
				&out_len, NULL);
		else {
			ret = swap_read_page(handle.cmp[nr] + len,
				&out_len, &cache_pos);
			if (out_len == 4 * PAGE_SIZE) {
				for (j = 0; j < 4; j++)
					map[i++] = cache_pos;
			} else {
				map[i++] = cache_pos;
			}
			if (cache_pos == 0xffffffff)
				start = (u32)handle.cmp[nr] + len;
			else
				start = (u32)handle.cache +
					cache_pos * PAGE_SIZE;
			STAT_BEGIN;
			clean_range(start, start + out_len);
			STAT_END(stat_clean_cache)
		}
		if (ret != 0) {
			printf("read_ Unexpected end of file\n");
			return ret;
		}
		len += out_len;
	}
	if (!meta) {
		map[i] = 0xfffffffe; /* indicate end */
		start = (u32)map;

		STAT_BEGIN;
		clean_range(start, start + (4 * i / 32 + 1) * 32);
		STAT_END(stat_clean_cache)
	}
	compress_blk_nr++;
	return ret;
}

static int swsusp_header_init(void)
{
	swsusp_header = (struct swsusp_header *)memalign(PAGE_SIZE, PAGE_SIZE);
	return 0;
}

static int swsusp_check(void)
{
	int error;

	error = swapdisk_read_page(0, (u8 *)swsusp_header, PAGE_SIZE, NULL);

	if (error)
		goto put;

	if (!memcmp(SWSUSP_SIG, swsusp_header->sig, 10)) {
		memcpy(swsusp_header->sig, swsusp_header->orig_sig, 10);

		/* Reset swap signature now */
		/* error = swapdisk_write_page(SWSUSP_RESUME_BLOCK,
				(unsigned int *)swsusp_header); */
	} else {
		dump_ram((u8 *)&swsusp_header->crc32, 36);
		error = -EINVAL;
	}

put:
	if (!error)
		printf("snapshot_boot: Signature found\n");
	else
		printf("snapshot_boot: Signature not found, error = %d\n",
		       error);

	return error;
}

static int get_swap_reader(u32 *flags_p)
{
	int error = 0;

	*flags_p = swsusp_header->flags;

	if (!swsusp_header->image) {
		error = -EINVAL;
		printf("get_swap_reader: swsusp_header image invalid\n");
		goto end;
	}

	handle.cur = (struct swap_map_page *)memalign(PAGE_SIZE, PAGE_SIZE);

	if (!handle.cur) {
		error = -ENOMEM;
		printf("get_swap_reader: malloc fail\n");
		goto end;
	}

	error = swapdisk_read_page(swsusp_header->image, handle.cur,
				   PAGE_SIZE, NULL);

	if (error) {
		release_swap_reader();
		printf("get_swap_reader: swapdisk_read_page fail\n");
		goto end;
	}
	handle.k = 0;

end:
	return error;
}

static int swsusp_info_init(void)
{
	int error = 0;
	u32 len = PAGE_SIZE;
	swsusp_info = (struct swsusp_info *)memalign(PAGE_SIZE, PAGE_SIZE);
	if (!error)
		error = swap_read_page(swsusp_info, &len, NULL);

	if (!error) {
		nr_copy_pages = swsusp_info->image_pages;
		nr_meta_pages = swsusp_info->pages
						- swsusp_info->image_pages - 1;
	}

	if (error)
		printf("swsusp_info_init: swap_read_page fail\n");
	return error;
}

static int load_image(void)
{
	int ret = 0, i, j;
	u32 nr_pages = 0;
	u32 phy_addr;
	u8 *src;

	u32 out_len = 0, total_len = 0, remain_len, len;
	u32 start;
	u32 progress = 0;
	u32 initial[NR_CPU - 1] = {0,};
	handle.crc32 = 0;

	STAT_BEGIN
	/* start to read and decompress all meta data */
	while (total_len < nr_meta_pages * PAGE_SIZE) {
		ret = read_one_compress_block(0, 1);
		if (ret != 0)
			return ret;

#ifdef USE_ZLIB
		inflateReset(&handle.stream[0]);
		handle.stream[0].next_in = handle.cmp[0] + LZO_HEADER;
		handle.stream[0].avail_in = handle._start[0].cmp_len;
		handle.stream[0].next_out = (u8 *)handle.meta_data + total_len;
		handle.stream[0].avail_out = LZO_UNC_SIZE;

		ret = inflate(&handle.stream[0], Z_FINISH);
		out_len = LZO_UNC_SIZE - handle.stream[0].avail_out;
#else
		out_len = LZO_UNC_SIZE;
		ret = lzo1x_decompress_safe(handle.cmp[0] + LZO_HEADER,
				handle._start[0].cmp_len,
				(u8 *)handle.meta_data + total_len,
				&out_len);
#endif

#ifdef USE_ZLIB
		if (ret != Z_STREAM_END) {
#else
		if (ret != 0) {
#endif
			printf("load_image decompress fail %d\n", ret);
			return ret;
		}
		handle.crc32 = crc32_le(handle.crc32,
				(u8 *)handle.meta_data + total_len,
				out_len);
		total_len += out_len;
	}

	handle.meta_k = 0;

	printf("snapshot_boot: Loading image data pages (%u pages) ...\n",
	       nr_copy_pages);

	remain_len = total_len - nr_meta_pages * PAGE_SIZE;
	if (remain_len > 0) {
		/* this data block contains both meta data and copy data,
		 * restore the remaining copy data to memory
		 */
		len = 0;
		while (len < remain_len) {
			phy_addr = handle.meta_data[handle.meta_k++];
			phy_addr = phy_addr * PAGE_SIZE;
			src = (u8 *)handle.meta_data +
				nr_meta_pages * PAGE_SIZE + len;
			memcpy((u8 *)phy_addr, src, PAGE_SIZE);
			len += PAGE_SIZE;
			nr_pages++;
		}
	}

	for (i = 0; i < NR_CPU - 1; i++) {
		handle._finish[i].finish = 0;
		handle._finish[i].finish_all = 0;
		start = (u32)&handle._finish[i];
		clean_range(start, start + 32);
	}
	STAT_END(stat_load_meta)

	smp_enable();

	while (nr_pages < nr_copy_pages) {
		/* fill each cpu's buffer with compressed data */
		for (i = 0; i < NR_CPU - 1; i++) {

			if (initial[i] > 0) {
				STAT_BEGIN
				ret = wait_cpu_done_decompress(i);
				STAT_END(stat_wait)
			}
			initial[i]++;
			if (ret != 0 || nr_pages >= nr_copy_pages)
				break;

			ret = read_one_compress_block(i, 0);
			nr_pages += 32;
			if (ret != 0)
				break;
			STAT_BEGIN;
			handle._start[i].start++;
			handle._start[i].meta_k = handle.meta_k;

			handle.meta_k += 32;
			start = (u32)&handle._start[i];
			clean_range(start, start + 32);
			STAT_END(stat_clean_cache)

			if (ret) {
				printf("nr_pages=%d, nr_copy_pages=%d\n",
				       nr_pages, nr_copy_pages);
				break;
			}

			if (progress != (nr_pages * 100 / nr_copy_pages)) {
				progress = nr_pages * 100 / nr_copy_pages;
				printf("\b\b\b\b%3d%%", progress);
			}
		}
		if (ret)
			break;

	}

	for (j = 0; j < i; j++)
		wait_cpu_done_decompress(j);

	if (ret && nr_pages >= nr_copy_pages)
		ret = 0;

	if (!ret)
		printf("\b\b\b\bdone\n");
	else
		printf("load_image failed, ret = %d\n", ret);

	return ret;
}

static int snapshot_restore(u32 *flags_p)
{
	int error;

	u32 i, start;
	compressed_len = 0;
	compress_blk_nr = 0;


	error = get_swap_reader(flags_p);
	if (error)
		goto end;

	error = swsusp_info_init();

	printf("num_physpages:%u image_pages:%u pages:%u\n",
	       swsusp_info->num_physpages,
	       swsusp_info->image_pages,
	       swsusp_info->pages);

	i = DIV_ROUND_UP(nr_meta_pages, LZO_UNC_PAGES) * LZO_UNC_SIZE;
	handle.meta_data = (u32 *)memalign(PAGE_SIZE, i);
	for (i = 0; i < NR_CPU - 1; i++) {
		handle.cmp[i] = (u8 *)(gd->ram_size + CACHE_SIZE +
			i * LZO_CMP_SIZE);
		handle.copy_data[i] = memalign(PAGE_SIZE, LZO_UNC_SIZE);
#ifdef USE_ZLIB
		handle.stream[i].zalloc = gzalloc;
		handle.stream[i].zfree = gzfree;
		inflateInit2(&handle.stream[i], -DEFLATE_DEF_WINBITS);
#endif
	}

	for (i = 0; i < NR_CPU - 1; i++) {
		handle._start[i].complete_all = 0;
		handle._start[i].start = 0;
		start = (u32)&handle._start[i];
		clean_range(start, start + 32);
	}

	if (!error)
		error = load_image();

end:
	for (i = 0; i < NR_CPU - 1; i++) {
		handle._start[i].complete_all = 1;
		start = (u32)&handle._start[i];
		clean_range(start, start + 32);
#ifdef USE_ZLIB
		inflateEnd(&handle.stream[i]);
#endif

	}
	release_swap_reader();

	if (!error)
		printf("Image successfully loaded len=%d\n", compressed_len);
	else
		printf("snapshot_boot: Error %d resuming\n", error);
	return error;
}

static void wait_for_secondary_boot(u32 nr)
{
	void (*owlxx_secondary_startup)(void);
	u32 v;

	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (v) : );
	v &= ~0x5; /* disable cache mmu */
	asm volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r" (v));

	while (1) {
		v = readl(CPU1_FLAG + (nr - 1) * 4);
		if (v == 0x55aa) {
			owlxx_secondary_startup =
			(void (*)(void))readl(CPU1_ADDR + (nr - 1) * 4);
			/* switch to owlxx_secondary_startup of kernel */
			owlxx_secondary_startup();
		}
	}
}

void decompress_fn(void)
{
	u32 nr, start;
	u32 len = 0;
	u32 phy_addr;
	u32 meta_k, start_counter[NR_CPU - 1];
	u32 i = 0, j;
	u32 *map;

	/* get cpu number */
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (nr) : );
	nr = nr & 0xF;
	nr = nr - 1;
	writel(0x44aa, CPU1_FLAG + nr * 4);
	start_counter[nr] = 0;
	while (1) {
		while (1) {
			start = (u32)&handle._start[nr];
			inv_range(start, start + 32);
			if (handle._start[nr].complete_all == 1) {
				flush_dcache_all();
				handle._finish[nr].finish_all = 1;
				start = (u32)&handle._finish[nr];
				clean_range(start, start + 32);
				wait_for_secondary_boot(nr + 1);
			}

			if (handle._start[nr].start != start_counter[nr]) {
				start_counter[nr] = handle._start[nr].start;
				break;
			}
		}
		i = 0;
		while (1) {
			map = handle.cmp_map[nr] + i * 8;
			start = (u32)map;
			inv_range(start, start + 32);
			for (j = 0; j < 8; j++) {
				if (map[j] == 0xffffffff) {
					start = (u32)handle.cmp[nr] +
						(i * 8 + j) * PAGE_SIZE;
					inv_range(start, start + PAGE_SIZE);
				} else if (map[j] == 0xfffffffe) {
					goto out;
				} else {
					start = (u32)handle.cache +
						map[j] * PAGE_SIZE;
					inv_range(start, start + PAGE_SIZE);
					memcpy(handle.cmp[nr] +
						(i * 8 + j) * PAGE_SIZE,
					       handle.cache +
						map[j] * PAGE_SIZE,
					       PAGE_SIZE);
				}
			}
			i++;
		}

out:
		meta_k = handle._start[nr].meta_k;

#ifdef USE_ZLIB
		inflateReset(&handle.stream[nr]);
		handle.stream[nr].next_in = handle.cmp[nr] + LZO_HEADER;
		handle.stream[nr].avail_in = handle._start[nr].cmp_len;
		handle.stream[nr].next_out = handle.copy_data[nr];
		handle.stream[nr].avail_out = LZO_UNC_SIZE;

		handle._finish[nr].ret = inflate(&handle.stream[nr], Z_FINISH);
		handle._finish[nr].out_len =
			LZO_UNC_SIZE - handle.stream[nr].avail_out;
#else
		handle._finish[nr].out_len = LZO_UNC_SIZE;
		handle._finish[nr].ret =
			lzo1x_decompress_safe(handle.cmp[nr] + LZO_HEADER,
					      handle._start[nr].cmp_len,
					      handle.copy_data[nr],
					      &handle._finish[nr].out_len);
#endif

		len = 0;
		while (len < handle._finish[nr].out_len) {
			phy_addr = handle.meta_data[meta_k++];
			phy_addr = phy_addr * PAGE_SIZE;
			memcpy((void *)phy_addr, handle.copy_data[nr] + len,
			       PAGE_SIZE);
			len += PAGE_SIZE;
		}
		handle._finish[nr].finish++;
		start = (u32)&handle._finish[nr];
		clean_range(start, start + 32);
	}
}

static void setup_peripherals(void)
{
	/* setup timer 0 */
	writel(0, T0_CTL);
	writel(0, T0_VAL);
	writel(0, T0_CMP);
	writel(4, T0_CTL);

	writel(0xffff, OWLXX_DMA_IRQ_EN_0);
	writel(0xffff, OWLXX_DMA_IRQ_EN_1);
	writel(0xffff, OWLXX_DMA_IRQ_EN_2);
	writel(0xffff, OWLXX_DMA_IRQ_EN_3);
}

void quickboot_init(void)
{
	checked = 0;
	snapshot_image_exist = 0;
}

int check_snapshot_image(void)
{
	int error, i;

	if (checked)
		return snapshot_image_exist;
#ifdef CONFIG_OWLXX_NAND
	get_device_and_partition("nand", "a:0", &dev_desc, &info, 1);
#else
	get_device_and_partition("mmc", "0:0", &dev_desc, &info, 1);
#endif

	memset(&handle, 0, sizeof(struct swap_map_handle));

	swsusp_header = NULL;
	swsusp_info = NULL;
	/* first 592k of 1M top ram for cache, total 148 pages */
	handle.cache = (u8 *)gd->ram_size;
	for (i = 0; i < 4; i++)
		handle.cache_sector[i] = 0xffffffff;
	for (i = 0; i < NR_CPU - 1; i++)
		handle.cmp_map[i] = memalign(32, 128);

	handle.cur_cache_pos = 0;
	swsusp_header_init();

	error = swsusp_check();
	checked = 1;
	if (error) {
		snapshot_image_exist = 0;
		for (i = 0; i < NR_CPU - 1; i++)
			free(handle.cmp_map[i]);
	} else {
		snapshot_image_exist = 1;
	}

	return snapshot_image_exist;
}

int do_bootsnapshot(void)
{
	int error = 0, i;
	u32 boot_flag;
	u32 start, stop;

	u32 stat_read_nand_ms;
	u32 stat_wait_ms;
	u32 stat_load_meta_ms;
	u32 stat_copy_mem_ms;
	u32 stat_clean_cache_ms;
	u32 stat_whole_ms;

	void (*target_cpu_reset)(u32 addr);
	void (*target_swsusp_arch_resume)(void);

#ifdef TEST_SPEED_ALL
	writel(0, T1_CTL);
	writel(0, T1_VAL);
	writel(4, T1_CTL);
#endif

	stat_read_nand = 0;
	stat_wait = 0;
	read_4page_counts = 0;
	read_1page_counts = 0;
	stat_load_meta = 0;
	stat_copy_mem = 0;
	stat_clean_cache = 0;
	stat_whole = 0;
	p_stack_pointer = 0;

	if (snapshot_image_exist == 0)
		goto out;

	printf("corepll %x devpll %x ddrpll %x\n",
	       readl(0xb0160000),
	       readl(0xb0160004),
	       readl(0xb0160008));

	for (i = 2; i < NR_CPU; i++)
		power_on_cpu(i);

	error = snapshot_restore(&boot_flag);
	if (error)
		goto out;

	setup_peripherals();

	/* clear up kernel nosave section */
	memset((void *)(swsusp_info->nosave_begin - 0xC0000000), 0,
	       swsusp_info->nosave_end - swsusp_info->nosave_begin);
	start = swsusp_info->nosave_begin - 0xC0000000;
	stop = start + swsusp_info->nosave_end - swsusp_info->nosave_begin;
	clean_range(start, stop);

	for (i = 0; i < NR_CPU - 1; i++) {
		while (1) {
			start = (u32)&handle._finish[i];
			inv_range(start, start + 32);
			if (handle._finish[i].finish_all == 1)
				break;
		}
	}

	v7_outer_cache_flush_all();

	/*
	 * Invalidate all instruction caches to PoU.
	 * Also flushes branch target cache.
	 */
	asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
	/* Invalidate entire branch predictor array */
	asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
	CP15DSB;
	CP15ISB;

	/* Invalidate entire unified TLB */
	asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	/* Invalidate entire data TLB */
	asm volatile ("mcr p15, 0, %0, c8, c6, 0" : : "r" (0));
	/* Invalidate entire instruction TLB */
	asm volatile ("mcr p15, 0, %0, c8, c5, 0" : : "r" (0));
	CP15DSB;
	CP15ISB;
#ifdef TEST_SPEED_ALL
	stat_whole += readl(T1_VAL);
#endif
	stat_read_nand_ms = stat_read_nand / 1024 * 1000 / 1024 / 24;
	stat_wait_ms = stat_wait / 1024 * 1000 / 1024 / 24;
	stat_whole_ms = stat_whole / 1024 * 1000 / 1024 / 24;
	stat_load_meta_ms = stat_load_meta / 1024 * 1000 / 1024 / 24;
	stat_copy_mem_ms = stat_copy_mem / 1024 * 1000 / 1024 / 24;
	stat_clean_cache_ms = stat_clean_cache / 1024 * 1000 / 1024 / 24;
	printf("16k-4k %d-%d nand %d.%03d wait %d.%03d\n",
	       read_4page_counts, read_1page_counts,
	       stat_read_nand_ms / 1000,
	       stat_read_nand_ms % 1000,
	       stat_wait_ms / 1000,
	       stat_wait_ms % 1000);

	printf("meta %d.%03d whole %d.%03d mem %d.%03d clean %d.%03d\n",
	       stat_load_meta_ms / 1000,
	       stat_load_meta_ms % 1000,
	       stat_whole_ms / 1000,
	       stat_whole_ms % 1000,
	       stat_copy_mem_ms / 1000,
	       stat_copy_mem_ms % 1000,
	       stat_clean_cache_ms / 1000,
	       stat_clean_cache_ms % 1000);

	target_cpu_reset =
			(void (*)(u32))swsusp_info->cpu_reset_begin;
	target_swsusp_arch_resume =
			(void (*)(void))swsusp_info->swsusp_arch_resume_begin;

	target_cpu_reset((u32)target_swsusp_arch_resume);

out:
	swsusp_header_release();
	swsusp_info_release();
	if (p_stack_pointer)
		free(p_stack_pointer);
	snapshot_image_exist = 0;
	return error;
}
