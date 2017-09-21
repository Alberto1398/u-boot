/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define	DMM_INTERLEAVE_BASE		(0xe0290020)
#define	DMM_MASTER_READ_TO		(0xe0290068)

/* get ddr capacity in MB */
unsigned int owl_get_ddrcap(void)
{
	unsigned int val, cap;
	unsigned int ch_num;
	unsigned int channel;

	ch_num = (readl(DMM_INTERLEAVE_BASE) >> 24) & 0xf;
	channel = (ch_num >= 3) ? 1 : 0;
	val = (readl(DMM_MASTER_READ_TO)) & 0xf;
	
	cap = ((val + 1) << channel) * 256;

	printf("Total DDR capacity = %d MB\n",cap);

	return cap;
}

int dram_init(void)
{
#ifdef CONFIG_SYS_SDRAM_SIZE
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#else
	unsigned long ddrcap;

	ddrcap = owl_get_ddrcap();

	/* u-boot only support max 3.5GB by now */
	if (ddrcap > 3584)
		ddrcap = 3584;

	gd->ram_size = ddrcap * 1024 * 1024;
#endif

	return 0;
}

void dram_init_banksize(void)
{
#if defined(CONFIG_NR_DRAM_BANKS) && defined(CONFIG_SYS_SDRAM_BASE)
	if (gd->ram_size >= 0xe0000000ul) {
		gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0xe0000000ul;
#if (CONFIG_NR_DRAM_BANKS == 2)
		gd->bd->bi_dram[1].start = 0x1e0000000ul;
		gd->bd->bi_dram[1].size = 0x20000000ul;
#endif
	} else {
		gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = gd->ram_size;
#if (CONFIG_NR_DRAM_BANKS == 2)
		gd->bd->bi_dram[1].start = 0x0;
		gd->bd->bi_dram[1].size = 0x0;
#endif
	}
#endif
}
