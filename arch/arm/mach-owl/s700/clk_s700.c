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

int owl_clk_init(void)
{
	unsigned int core_freq;
	unsigned int dev_freq;
	unsigned int display_freq;
	unsigned int ddr_spread;
	unsigned int nand_spread;
	unsigned int display_spread;
	unsigned int dsi_spread;
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
					     "actions,owl-clk");
	if (node < 0) {
		printf("cannot find 'actions,owl-clk' config\n");
		return -1;
	}

	core_freq = fdtdec_get_int(gd->fdt_blob, node, "core_pll", 792);
	dev_freq = fdtdec_get_int(gd->fdt_blob, node, "dev_pll", 396);
	display_freq = fdtdec_get_int(gd->fdt_blob, node, "display_pll", 480);
	printf("clk: core_pll %uMHz, dev_pll %uMHz, display_pll %uMHz\n",
	       core_freq, dev_freq, display_freq);

	ddr_spread = fdtdec_get_int(gd->fdt_blob, node, "ddr_pll_spread_spectrum", 0);
	nand_spread = fdtdec_get_int(gd->fdt_blob, node, "nand_pll_spread_spectrum", 0);
	display_spread = fdtdec_get_int(gd->fdt_blob, node, "display_pll_spread_spectrum", 0);
	dsi_spread = fdtdec_get_int(gd->fdt_blob, node, "dsi_pll_spread_spectrum", 0);
	printf("spread_spectrum: ddr %d, nand %d, display %d, dsi %d\n",
	       ddr_spread, nand_spread, display_spread, dsi_spread);

	/* dev_clk = hosc */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x0);

	/* PDBGDIV = core_pll/8(126), perdiv = core_pll/8(126),
	 *  noc0_clk = dev_clk/2 (300)
	 * core_clk = HOSC
	 */
	writel(0x1c710001, CMU_BUSCLK);

	/* core pll */
	owl_corepll_set_rate(core_freq * 1000ul * 1000ul);

	/* dev pll  */
	writel(0x100 | (dev_freq / 6), CMU_DEVPLL);

	/* display pll  */
	writel(0x100 | (display_freq / 6), CMU_DISPLAYPLL);

	udelay(200);

	/* dev_clk = dev_pll */
	clrsetbits_le32(CMU_DEVPLL, 0x1000, 0x1000);

	/* core_clk = core_pll */
	clrsetbits_le32(CMU_BUSCLK, 0x3, 0x2);

	/*noc1_div_clk= dev_clk/3 (200) , hclk_pclk_src=dev_clk/3 (200)*/
	/*HCLK=hclk_pclk_src/2 (100), PCLK = HCLK/2(50)*/
	/*noc1_clk = dev_clk/3 (200)*/
	writel(0x00026004, CMU_BUSCLK1);
	udelay(50);

	if (ddr_spread != 0)
		clrsetbits_le32(CMU_DDRPLLDEBUG0, 0xfffff800, 0x4BC18000);
	if (nand_spread != 0)
		clrsetbits_le32(CMU_NANDPLLDEBUG, 0xfffff800, 0x4BC18000);
	if (display_spread != 0)
		clrsetbits_le32(CMU_DISPLAYPLLDEBUG, 0xfffff800, 0x4BC18000);
	if (dsi_spread != 0)
		clrsetbits_le32(CMU_DSIPLLDEBUG1, 0xfffff800, 0x4BC18000);

	return 0;
}
