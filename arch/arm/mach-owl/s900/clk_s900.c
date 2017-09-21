/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#undef CPU_TMP_CLK_DEVCLK

int owl_clk_init(void)
{
	unsigned int core_freq;
	unsigned int dev_freq;
	unsigned int display_freq;
	unsigned int nand_spread;
	unsigned int ddr_spread;
	unsigned int dp_spread;
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

	ddr_spread = fdtdec_get_int(gd->fdt_blob, node, "ddr_pll_spread_spectrum", 0);
	nand_spread = fdtdec_get_int(gd->fdt_blob, node, "nand_pll_spread_spectrum", 0);
	dp_spread = fdtdec_get_int(gd->fdt_blob, node, "dp_pll_spread_spectrum", 0);
	printf("spread_spectrum: ddr 0x%x, nand 0x%x, dp 0x%x\n",
	       ddr_spread, nand_spread, dp_spread);

	/* enable assist pll */
	clrsetbits_le32(CMU_ASSISTPLL, 0x1, 0x1);
	udelay(50);

#ifndef CPU_TMP_CLK_DEVCLK
	/* dev_clk = hosc */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x0);
#endif

	/* noc_clk = assistpll/1, noc_div_clk = noc_clk/2,
	 * hclk = noc_div_clk/2, pclk = hclk/2
	 */
#ifdef CPU_TMP_CLK_DEVCLK
	/* cpu temp clock : dev_clk (vce_clk_before_gate) */
	writel(0x1c780993, CMU_BUSCLK);
#else
	/* cpu temp clock : hosc (24MHz) */
	writel(0x1c780991, CMU_BUSCLK);
#endif

	writel(0x1c780991, CMU_BUSCLK);
	udelay(50);

	/* core pll */
	owl_corepll_set_rate(core_freq * 1000ul * 1000ul);

#ifndef CPU_TMP_CLK_DEVCLK
	/* dev pll */
	writel(0x100 | (dev_freq / 6), CMU_DEVPLL);
#endif

	/* display pll */
	writel(0x100 | (display_freq / 6), CMU_DISPLAYPLL);

	udelay(200);

	/* core_clk = core_pll */
	clrsetbits_le32(CMU_BUSCLK, 0x3, 0x2);

#ifndef CPU_TMP_CLK_DEVCLK
	/* dev_clk = dev_pll */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x1000);
	udelay(50);
#endif

	if (ddr_spread != 0)
		clrsetbits_le32(CMU_DDRPLLDEBUG, 0xfffff800, 0x4BC18000);
	if (nand_spread != 0)
		clrsetbits_le32(CMU_NANDPLLDEBUG, 0xfffff800, 0x4BC18000);
	if (dp_spread != 0)
		clrsetbits_le32(CMU_DPPLLDEBUG, 0xfffff800, 0x4BC18000);

	/* enable jtag backdoor for debug */
	clrsetbits_le32(CMU_ANALOGDEBUG, 0xb00, 0xa00);

	/* choose 24M clock as system counter clock source */
	clrsetbits_le32(SYS_CNT_CTL, 1 << 31, 1 << 31);

	return 0;
}
