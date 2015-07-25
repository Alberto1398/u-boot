/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/owlxx_dma.h>

#define MYLEN			1024
#define DMA_IRQ_ID		89

static u8 *src;
static u8 *dst;
static u32 total;
static u32 irq_check;

void dma_test_irq(void *handle)
{
	if (!(readl(DMA0_INT_STATUS) & 0x1))
		printf("dma test irq status err\n");

	irq_check++;
	/* disable End of Block Interrupt */
	clrbits_le32(DMA0_INT_CTL, 0x1);

	/* clean all INT_STATUS */
	writel(0x7F, DMA0_INT_STATUS);

	/* clean all IRQxPD */
	writel(0xFFFF, DMA_IRQ_PD0);
	writel(0xFFFF, DMA_IRQ_PD1);
	writel(0xFFFF, DMA_IRQ_PD2);
	writel(0xFFFF, DMA_IRQ_PD3);
	printf("dma test irq exit\n");
#if 0
	/* GIC refered, Write GICC_EOIR */
	writel(intID, GICC_BASE + 0x10);
#endif
}

void dma_test_init(void)
{
	clrbits_le32(CMU_DEVRST0, 0x1);
	udelay(1);
	setbits_le32(CMU_DEVRST0, 0x1);
	/* Secure or Non-Secure */
	writel(0xFFFF, DMA_SECURE_ACCESS_CTL);

	irq_install_handler(DMA_IRQ_ID, dma_test_irq, NULL);

	writel(0xFFFF, DMA_IRQ_EN0);
	writel(0x0000, DMA_IRQ_EN1);
	writel(0x0000, DMA_IRQ_EN2);
	writel(0x0000, DMA_IRQ_EN3);

	/* enable end of block interrupt */
	writel(0x1, DMA0_INT_CTL);
	writel(0x7F, DMA0_INT_STATUS);
}

int dma_test_alloc(u32 count)
{
	u32 i;
	if (src || dst) {
		free(src);
		free(dst);
	}

	src = malloc(count);
	dst = malloc(count);

	printf("dma memory alloc: src(%p) dst(%p), count(%d)\n",
			src, dst, count);
	if (!src || !dst) {
		free(src);
		free(dst);
		printf("dma memory alloc fail\n");
		return 1;
	}

	for (i = 0; i < count; i++) {
		src[i] = i & 0xff;
		dst[i] = 0;
	}

	total = count;

	return 0;
}

void dma_test_transfer(void)
{
	u32 i, dma_channel = 0;

	stop_dma(dma_channel);

	set_dma_mode(dma_channel, 0x00050a00);	/* DDR -> DDR */
	set_dma_src_addr(dma_channel,  (u32) src);
	set_dma_dst_addr(dma_channel, (u32) dst);
	set_dma_frame_len(dma_channel, total);
	set_dma_frame_count(dma_channel,  1);

	start_dma(dma_channel);

	i = 10;
	while (dma_started(dma_channel) && i--)
		mdelay(100);

	if (i <= 0)
		printf("error: transfer data timeout\n");

}

int dma_test_check(void)
{
	u32 i;
	for (i = 0; i < total; i++) {
		if (src[i] != dst[i]) {
			printf("dma check fail (%d)\n", i);
			return 1;
		}
	}
	printf("dma check success (%d)\n", i);

	return 0;
}

void dma_test_show(void)
{
	u32 n, i;
	u8 *s = src, *d = dst;

	printf("dma show src addr(%p), dst addr(%p)\n", src, dst);
	for (i = 0; i < (total >> 3); i++) {
		n = i << 3;
		if (!memcmp(&s[n], &d[n], 8))
			printf("%d succ\n", n);
		else
			printf("%d fail\n", n);

		printf("   s: %x %x %x %x %x %x %x %x\n"
			 "   d: %x %x %x %x %x %x %x %x\n",
			s[n], s[n + 1], s[n + 2], s[n + 3],
			s[n + 4], s[n + 5], s[n + 6], s[n + 7],
			d[n], d[n + 1], d[n + 2], d[n + 3],
			d[n + 4], d[n + 5], d[n + 6], d[n + 7]);
	}
}

void dma_test_exit(void)
{
	printf("irq check (%d)\n", irq_check);

	irq_free_handler(DMA_IRQ_ID);
	free(src);
	free(dst);
	src   = NULL;
	dst   = NULL;
	total = 0;
}

static int do_dma(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret, count;
	char * const *av;

	if (argc < 2)
		return CMD_RET_USAGE;

	av = argv + 1;
	if (!strcmp(av[0], "all")) {
		if (argc < 3)
			return CMD_RET_USAGE;

		count = simple_strtoul(av[1], NULL, 10);
		dma_test_init();

		ret = dma_test_alloc(count);
		if (ret) {
			printf("dma alloc fail\n");
			return -1;
		}

		dma_test_transfer();
		mdelay(3000);
		ret = dma_test_check();
		if (ret) {
			dma_test_show();
			printf("dma trans check fail (%d)\n", ret);
		} else {
			printf("dma trans check success\n");
		}

		dma_test_exit();
	}

	if (!strcmp(av[0], "init")) {
		printf("dma init enter\n");
		dma_test_init();
	}

	if (!strcmp(av[0], "alloc")) {
		if (argc < 3)
			return CMD_RET_USAGE;

		count = simple_strtoul(av[1], NULL, 10);
		ret = dma_test_alloc(count);
		if (ret)
			printf("dma alloc fail\n");
	}

	if (!strcmp(av[0], "go")) {
		printf("dma tansfer data\n");
		dma_test_transfer();
	}

	if (!strcmp(av[0], "check")) {
		ret = dma_test_check();
		if (ret)
			printf("dma trans check fail (%d)\n", ret);
		else
			printf("dma trans check success\n");
	}

	if (!strcmp(av[0], "exit")) {
		printf("dma exit execute\n");
		dma_test_exit();
	}

	if (!strcmp(av[0], "show")) {
		printf("dma show content\n");
		dma_test_show();
	}

	return 0;
}

U_BOOT_CMD(dma, CONFIG_SYS_MAXARGS, 0, do_dma,
		"dma test utils",
		"command(all, init, alloc, go, check, exit, show) [buffer length]");
