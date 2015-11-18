/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/regs.h>

DECLARE_GLOBAL_DATA_PTR;

int owl_clk_init(void)
{
	unsigned int core_freq;
	unsigned int dev_freq;
	unsigned int display_freq;
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
					     "actions,owl-clk");
	if (node < 0) {
		printf("%s: cannot find 'actions,owl-clk' config\n");
		return -1;
	}

	core_freq = fdtdec_get_int(gd->fdt_blob, node, "core_pll", 1152);
	dev_freq = fdtdec_get_int(gd->fdt_blob, node, "dev_pll", 660);
	display_freq = fdtdec_get_int(gd->fdt_blob, node, "display_pll", 660);

	printf("clk: core_pll %uMHz, dev_pll %uMHz, display_pll %uMHz\n",
	       core_freq, dev_freq, display_freq);

	/* enable assist pll */
	clrsetbits_le32(CMU_ASSISTPLL, 0x1, 0x1);
	udelay(50);

	/* dev_clk = hosc */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x0);

	/* noc_clk = assistpll/1, noc_div_clk = noc_clk/2,
	 * hclk = noc_div_clk/2, pclk = hclk/2
	 * core_clk = HOSC
	 */
	writel(0x1c780991, CMU_BUSCLK);
	udelay(50);

	/* core pll */
	writel(0x300 | (core_freq / 24), CMU_COREPLL);

	/* dev pll */
	writel(0x100 | (dev_freq / 6), CMU_DEVPLL);

	/* display pll */
	writel(0x100 | (display_freq / 6), CMU_DISPLAYPLL);

	udelay(200);

	/* core_clk = core_pll */
	clrsetbits_le32(CMU_BUSCLK, 0x3, 0x2);

	/* dev_clk = dev_pll */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x1000);
	udelay(50);

	/* enable jtag backdoor for debug */
	clrsetbits_le32(CMU_ANALOGDEBUG, 0xb00, 0xa00);

	return 0;
}
