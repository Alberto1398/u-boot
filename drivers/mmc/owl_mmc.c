/*
 * owlxx_mmc.c - OWL SD/MMC driver
 *
 * Copyright (C) 2012, Actions Semiconductor Co. LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <asm/arch/clocks.h>
#include <asm/arch/gmp.h>
#include <asm/arch/dma.h>
#include <asm/arch/owlxx_dma.h>

#include "owlxx_mmc.h"

#include "../../../../include/mbr_info.h"

#define DAM_ALIE_BYTE 32


static void acts_mmc_dev_init(struct mmc *mmc, struct owlxx_mmc_host *host ,\
								unsigned int bootdev,unsigned  int part_num);

#undef pr_debug
/* #define ACTS_MMC_DEBUG */

#ifdef ACTS_MMC_DEBUG
#define pr_debug(format, arg...)	\
	printf(format, ##arg)
#else
#define pr_debug(format, args...)
#endif

#define CONFIG_FPGA	1
#define DATA_TRANSFER_TIMEOUT		(3 * (100 * 1000))  /* 2 seconds */

#define MAX_PARTITION 12


/* max card devices, includes eMMC card */

static struct mmc sd_dev ;
static struct mmc mmc_dev[MAX_PARTITION] ;
static struct mmc tsd_dev[MAX_PARTITION] ;
/* support 3 SDC */
struct owlxx_mmc_host mmc_host[3];




/* start for misc process */
#define EMMC_SECTOR_SHIFT	9
#define EMMC_SECTOR_SIZE	(1 << EMMC_SECTOR_SHIFT)
#define EMMC_RESERVED		(4 * 1024 * 1024)

#define PART_FREE	0x55
#define PART_DUMMY	0xff
#define PART_READONLY	0x85
#define PART_WRITEONLY	0x86
#define PART_NO_ACCESS	0x87
#define PART_RESERVE	0x99

static void acts_dump_mfp(struct owlxx_mmc_host *host)
{
	printf("\tMFP_CTL0:0x%x\n", readl(MFP_CTL0));
	printf("\tMFP_CTL1:0x%x\n", readl(MFP_CTL1));
	printf("\tMFP_CTL2:0x%x\n", readl(MFP_CTL2));
	printf("\tMFP_CTL3:0x%x\n", readl(MFP_CTL3));
	printf("\tPAD_PULLCTL0:0x%x\n", readl(PAD_PULLCTL0));
	printf("\tPAD_PULLCTL1:0x%x\n", readl(PAD_PULLCTL1));
	printf("\tPAD_PULLCTL2:0x%x\n", readl(PAD_PULLCTL2));

	printf("\tPAD_DRV0:0x%x\n", readl(PAD_DRV0));
	printf("\tPAD_DRV1:0x%x\n", readl(PAD_DRV1));
	printf("\tPAD_DRV2:0x%x\n", readl(PAD_DRV2));

	printf("\tCMU_DEVCLKEN0:0x%x\n", readl(CMU_DEVCLKEN0));
	printf("\tCMU_DEVCLKEN1:0x%x\n", readl(CMU_DEVCLKEN1));

	printf("\tCMU_DEVPLL:0x%x\n", readl(CMU_DEVPLL));
	printf("\tCMU_NANDPLL:0x%x\n", readl(CMU_NANDPLL));

	printf("\tCMU_SD0CLK:0x%x\n", readl(CMU_SD0CLK));
	printf("\tCMU_SD1CLK:0x%x\n", readl(CMU_SD1CLK));
	printf("\tCMU_SD2CLK:0x%x\n", readl(CMU_SD2CLK));
}

static void acts_dump_sdc(struct owlxx_mmc_host *host)
{
	printf("\n\tSD_EN:0x%x\n", readl(HOST_EN(host)));
	printf("\tSD_CTL:0x%x\n", readl(HOST_CTL(host)));
	printf("\tSD_STATE:0x%x\n", readl(HOST_STATE(host)));
	printf("\tSD_CMD:0x%x\n", readl(HOST_CMD(host)));
	printf("\tSD_ARG:0x%x\n", readl(HOST_ARG(host)));
	printf("\tSD_RSPBUF0:0x%x\n", readl(HOST_RSPBUF0(host)));
	printf("\tSD_RSPBUF1:0x%x\n", readl(HOST_RSPBUF1(host)));
	printf("\tSD_RSPBUF2:0x%x\n", readl(HOST_RSPBUF2(host)));
	printf("\tSD_RSPBUF3:0x%x\n", readl(HOST_RSPBUF3(host)));
	printf("\tSD_RSPBUF4:0x%x\n", readl(HOST_RSPBUF4(host)));
	printf("\tSD_DAT:0x%x\n", readl(HOST_DAT(host)));
	printf("\tSD_BLK_SIZE:0x%x\n\n", readl(HOST_BLK_SIZE(host)));
	printf("\tSD_BLK_NUM:0x%x\n", readl(HOST_BLK_NUM(host)));
	printf("\tSD_BUF_SIZE:0x%x\n", readl(HOST_BUF_SIZE(host)));
}

static void acts_dump_dmac(struct owlxx_mmc_host *host)
{
	int dmanr;

	dmanr = host->dma_channel;
	printf("Current DMA Channel is: %d\n", dmanr);

	printf("\tDMA_MODE:0x%x\n", readl(dma_base[dmanr] + DMA_MODE_OFFSET));
	printf("\tDMA_SOURCE:0x%x\n", readl(dma_base[dmanr] +
			DMA_SOURCE_OFFSET));
	printf("\tDMA_DESTINATION:0x%x\n", readl(dma_base[dmanr] +
			DMA_DESTINATION_OFFSET));
	printf("\tDMA_FRAME_LEN:0x%x\n", readl(dma_base[dmanr] +
			DMA_FRAME_LEN_OFFSET));
	printf("\tDMA_FRAME_CNT:0x%x\n", readl(dma_base[dmanr] +
			DMA_FRAME_CNT_OFFSET));
	printf("\tDMA_REMAIN_FRAME_CNT:0x%x\n", readl(dma_base[dmanr] +
			DMA_REMAIN_FRAME_CNT_OFFSET));
	printf("\tDMA_REMAIN_CNT:0x%x\n", readl(dma_base[dmanr] +
			DMA_REMAIN_CNT_OFFSET));
	printf("\tDMA_SOURCE_STRIDE:0x%x\n", readl(dma_base[dmanr] +
			DMA_SOURCE_STRIDE_OFFSET));
	printf("\tDMA_DESTINATION_STRIDE:0x%x\n", readl(dma_base[dmanr] +
			DMA_DESTINATION_STRIDE_OFFSET));
	printf("\tDMA_START:0x%x\n", readl(dma_base[dmanr] +
			DMA_START_OFFSET));

	/* For advanced debug*/
#ifdef CONFIG_USE_ADVANCED_DMA
	printf("\tDMA_IRQ_PD0_BASE:0x%x\n", readl(DMA_IRQ_PD0_BASE));
	printf("\tDMA_IRQ_PD1_BASE:0x%x\n", readl(DMA_IRQ_PD1_BASE));
	printf("\tDMA_IRQ_PD2_BASE:0x%x\n", readl(DMA_IRQ_PD2_BASE));
	printf("\tDMA_IRQ_PD3_BASE:0x%x\n", readl(DMA_IRQ_PD3_BASE));

	printf("\tDMA_IRQ_EN0_BASE:0x%x\n", readl(DMA_IRQ_EN0_BASE));
	printf("\tDMA_IRQ_EN1_BASE:0x%x\n", readl(DMA_IRQ_EN1_BASE));
	printf("\tDMA_IRQ_EN2_BASE:0x%x\n", readl(DMA_IRQ_EN2_BASE));
	printf("\tDMA_IRQ_EN3_BASE:0x%x\n", readl(DMA_IRQ_EN3_BASE));


	printf("\tDMA_ACP_ATTRIBUTE:0x%x\n", readl(dma_base[dmanr] +
			DMA_ACP_ATTRIBUTE_OFFSET));
	printf("\tDMA_CHAINED_CTL:0x%x\n", readl(dma_base[dmanr] +
			DMA_CHAINED_CTL_OFFSET));
	printf("\tDMA_CONSTANT:0x%x\n", readl(dma_base[dmanr] +
			DMA_CONSTANT_OFFSET));
	printf("\tDMA_LINKLIST_CTL:0x%x\n", readl(dma_base[dmanr] +
			DMA_LINKLIST_CTL_OFFSET));
	printf("\tDMA_NEXT_DESCRIPTOR:0x%x\n", readl(dma_base[dmanr] +
			DMA_NEXT_DESCRIPTOR_OFFSET));
	printf("\tDMA_CURRENT_DESCRIPTOR_NUM:0x%x\n", readl(dma_base[dmanr] +
			DMA_CURRENT_DESCRIPTOR_NUM_OFFSET));
	printf("\tDMA_INT_CTL:0x%x\n", readl(dma_base[dmanr] +
			DMA_INT_CTL_OFFSET));

	printf("\tDMA_INT_STATUS:0x%x\n", readl(dma_base[dmanr] +
			DMA_INT_STATUS_OFFSET));
	printf("\tDMA_CURRENT_SOURCE_POINTER:0x%x\n", readl(dma_base[dmanr] +
			DMA_CURRENT_SOURCE_POINTER_OFFSET));
	printf("\tDMA_CURRENT_DESTINATION:0x%x\n", readl(dma_base[dmanr] +
			DMA_CURRENT_DESTINATION_POINTER_OFFSET));
#endif
}

static inline int acts_enable_clock(struct owlxx_mmc_host *host)
{
	int ret;

	if (!host->clk_on) {
		ret = module_clk_enable(host->module_id);
		if (ret) {
			printf("error: enable module clock error\n");
			return ret;
		}
		host->clk_on = 1;
	}
	return 0;
}

static inline int acts_disable_clock(struct owlxx_mmc_host *host)
{
	int ret;

	if (host->clk_on) {
		ret = module_clk_disable(host->module_id);
		if (ret) {
			printf("error: disable module clock error\n");
			return ret;
		}
		host->clk_on = 0;
	}
	return 0;
}


static int acts_mmc_send_init_clk(struct owlxx_mmc_host *host)
{
	u32 mode;
	int ret = 0;

	mode = SD_CTL_TS | SD_CTL_SCC | SD_CTL_TCN(5) | SD_CTL_TM(8);
	mode |= (readl(HOST_CTL(host)) & (0xff << 16));

	writel(mode, HOST_CTL(host));
	mdelay(10);
	if (readl(HOST_CTL(host)) & SD_CTL_TS) {
		printf("error: Memory card send init clock timeout error\n");
		ret = -1;
	}

	return ret;
}


static void acts_mmc_set_clk(struct owlxx_mmc_host *host, int rate)
{
	if (0 == rate)
		return;

	/*
	 * Set the RDELAY and WDELAY based on the sd clk.
	 */
	if (rate <= 1000000) {
		writel((readl(HOST_CTL(host)) & (~(0xff << 16))) |
			SD_CTL_RDELAY(host->rdelay.delay_lowclk) |
			SD_CTL_WDELAY(host->wdelay.delay_lowclk),
			HOST_CTL(host));
	} else if ((rate > 1000000) && (rate <= 26000000)) {
		writel((readl(HOST_CTL(host)) & (~(0xff << 16))) |
			SD_CTL_RDELAY(host->rdelay.delay_midclk) |
			SD_CTL_WDELAY(host->wdelay.delay_midclk),
			HOST_CTL(host));
	} else if ((rate > 26000000) && (rate <= 52000000)) {
		writel((readl(HOST_CTL(host)) & (~(0xff << 16))) |
			SD_CTL_RDELAY(host->rdelay.delay_highclk) |
			SD_CTL_WDELAY(host->wdelay.delay_highclk),
			HOST_CTL(host));
	} else {
		printf("error: SD2.0 max clock should < 50Mhz\n");
	}

	module_clk_set_rate(host->module_id, rate);
}

static void acts_mmc_power_up(struct owlxx_mmc_host *host)
{
	/* power on reset */
	module_reset(host->module_id);
	module_clk_enable(host->module_id);

	/* module function enable */
	writel(SD_ENABLE | SD_EN_RESE, HOST_EN(host));
}

DECLARE_GLOBAL_DATA_PTR;

static int acts_mmc_fdt_init(struct owlxx_mmc_host *host)
{
	const void *blob = gd->fdt_blob;
	int node_list[1], count;

	count = fdtdec_find_aliases_for_id(blob, "sdboot",
		COMPAT_ACTIONS_OWLXX_SDMMC, node_list, 1);
	if (count < 0 || node_list[0] < 0) {
		printf("error: %d, count of T30 sdhci nodes is %d\n",
			count, node_list[0]);
		return -1;
	}

	owlxx_device_fdtdec_set_pinctrl_default(node_list[0]);

	return 0;
}

static void acts_mmc_power_on(struct owlxx_mmc_host *host)
{
	u32 pad_drv_mask = 0, pad_drv_high = 0, pad_drv_high2 = 0;
	u32 pad_drv_tmp;
	u32 pad_sd1_mfp2;
	u32 pad_sd1_pull1;

	/* acts_mmc_fdt_init(host); */

	if (MOD_ID_SD0 == host->module_id) {
		/* mfp & pull up control */
		writel(readl(MFP_CTL2) & 0xfff0061f, MFP_CTL2);
		writel((readl(PAD_PULLCTL1) & 0xffffefff) | 0x3e000,
			PAD_PULLCTL1);

		printf("SDC0 cfg:(MFP_CTL2=0x%x\tPAD_PULLCTL1=0x%x)\n",
			readl(MFP_CTL2), readl(PAD_PULLCTL1));

		/* PAD drive capacity config */
		pad_drv_mask = SD0_DRV_HIGH_MASK & SD0_DRV_HIGH_MASK2;
		switch (host->pad_drv) {
		case PAD_DRV_LOW:
			pad_drv_high = SD0_DRV_HIGH_LOW;
			pad_drv_high2 = SD0_DRV_HIGH2_LOW;
			break;
		case PAD_DRV_MID:
			pad_drv_high = SD0_DRV_HIGH_MID;
			pad_drv_high2 = SD0_DRV_HIGH2_MID;
			break;
		case PAD_DRV_HIGH:
			pad_drv_high = SD0_DRV_HIGH_HIGH;
			pad_drv_high2 = SD0_DRV_HIGH2_HIGH;
			break;
		default:
			pad_drv_high = SD0_DRV_HIGH_HIGH;
			pad_drv_high2 = SD0_DRV_HIGH2_HIGH;
			printf("error: host->pad_drv %d\n", host->pad_drv);
		}
	} else if (MOD_ID_SD1 == host->module_id) {
		/* mfp & pull up control */
		//writel((readl(MFP_CTL2) & 0xffffffe7) | 0x600, MFP_CTL2);
		//writel(readl(PAD_PULLCTL1) | 0x878, PAD_PULLCTL1);
		pad_sd1_mfp2 = readl(MFP_CTL2);
		pad_sd1_mfp2 &=~((1<<4)|(1<<3)|(1<<10)|(1<<9));
		pad_sd1_mfp2 |=((1<<10)|(1<<9));
		writel(pad_sd1_mfp2 , MFP_CTL2);
		pad_sd1_pull1 = readl(PAD_PULLCTL1);
		pad_sd1_pull1 |=((1<<11)|(1<<6)|(1<<5)|(1<<4)|(1<<3));
		writel(pad_sd1_pull1 , PAD_PULLCTL1);

		printf("SDC1 cfg:(MFP_CTL2=0x%x\tPAD_PULLCTL1=0x%x)\n",
			readl(MFP_CTL2), readl(PAD_PULLCTL1));

		/* PAD drive capacity config */
		pad_drv_mask = SD1_DRV_HIGH_MASK & SD1_DRV_HIGH_MASK2;
		switch (host->pad_drv) {
		case PAD_DRV_LOW:
			pad_drv_high = SD1_DRV_HIGH_LOW;
			pad_drv_high2 = SD1_DRV_HIGH2_LOW;
			break;
		case PAD_DRV_MID:
			pad_drv_high = SD1_DRV_HIGH_MID;
			pad_drv_high2 = SD1_DRV_HIGH2_MID;
			break;
		case PAD_DRV_HIGH:
			pad_drv_high = SD1_DRV_HIGH_HIGH;
			pad_drv_high2 = SD1_DRV_HIGH2_HIGH;
			break;
		default:
			pad_drv_high = SD1_DRV_HIGH_HIGH;
			pad_drv_high2 = SD1_DRV_HIGH2_HIGH;
			printf("error: host->pad_drv %d\n", host->pad_drv);
		}
	} else if (MOD_ID_SD2 == host->module_id) {
		/* mfp & pull up control */
		writel(readl(MFP_CTL3) | (0xf << 24), MFP_CTL3);
		writel(readl(PAD_PULLCTL1) | (0x7 << 25), PAD_PULLCTL1);
		writel((readl(PAD_PULLCTL2) & 0xffffff87) | 0x48,
			PAD_PULLCTL2);

		printf("SDC2 cfg:(MFP_CTL3=0x%x\tPAD_PULLCTL1=0x%x\tPAD_PULLCTL2=0X%x)\n",
			readl(MFP_CTL3), readl(PAD_PULLCTL1), readl(PAD_PULLCTL2));

		/* PAD drive capacity config */
		/* SDC2 not supported PAD - DRV config */
	} else {
		printf("error: power on, MOD%d not supported\n",
			host->module_id);
		return;
	}

	pad_drv_tmp = readl(PAD_DRV1);
	pad_drv_tmp &= pad_drv_mask;
	pad_drv_tmp |= pad_drv_high;
	pad_drv_tmp |= pad_drv_high2;
	writel(pad_drv_tmp, PAD_DRV1);

	/* clocks is provided to eliminate power-up synchronization problems */
	acts_mmc_send_init_clk(host);
}

#if 1
static void acts_mmc_power_off(struct mmc *mmc)
{
	struct owlxx_mmc_host *host = mmc->priv;
	//acts_disable_clock(host);
	if(host->id == SDC0_SLOT){
		writel((readl(MFP_CTL2) & 0xfff03fff)|(0x90000), MFP_CTL2);
	}
}
#endif

static int acts_mmc_init_setup(struct mmc *mmc)
{
	struct owlxx_mmc_host *host = mmc->priv;

	acts_mmc_power_up(host);
	mdelay(10);
	acts_mmc_power_on(host);

	return 0;
}

static void acts_mmc_set_ios(struct mmc *mmc)
{
	struct owlxx_mmc_host *host = mmc->priv;
	u32 ctrl_reg;

	acts_enable_clock(host);

	pr_debug("acts_mmc_set_ios:\n");

	if (mmc->clock && mmc->clock != host->clock) {
		host->clock = mmc->clock;
		pr_debug("\tSet clock: %d\n", host->clock);
		acts_mmc_set_clk(host, mmc->clock);
	}

	ctrl_reg = readl(HOST_EN(host));
	if (mmc->bus_width != host->bus_width) {
		host->bus_width = mmc->bus_width;
		switch (mmc->bus_width) {
		case 8:
			ctrl_reg &= ~0x3;
			ctrl_reg |= 0x2;
			break;
		case 4:
			ctrl_reg &= ~0x3;
			ctrl_reg |= 0x1;
			break;
		case 1:
			ctrl_reg &= ~0x3;
			break;
		}
	}

	switch (ctrl_reg & 0x3) {
	case 0x2:
		pr_debug("\tMMC_BUS_WIDTH_8\n");
		break;
	case 0x1:
		pr_debug("\tMMC_BUS_WIDTH_4\n");
		break;
	case 0x0:
		pr_debug("\tMMC_BUS_WIDTH_1\n");
		break;
	default:
		pr_debug("\tMMC_BUS_WIDTH NOT known\n");
	}

	writel(ctrl_reg, HOST_EN(host));
}

static unsigned int _config_read_dma_mode(unsigned int sdc_id)
{
	unsigned int dma_mode = -1;

	switch (sdc_id) {
	case SDC0_SLOT:
		dma_mode = OWLXX_SDC0RD_DMAMODE;
		break;
	case SDC1_SLOT:
		dma_mode = OWLXX_SDC1RD_DMAMODE;
		break;
	case SDC2_SLOT:
		dma_mode = OWLXX_SDC2RD_DMAMODE;
		break;
	default:
		printf("error: MMC/SD slot %d not support\n", sdc_id);
		return -1;
	}

	return dma_mode;
}

static unsigned int _config_write_dma_mode(unsigned int sdc_id)
{
	unsigned int dma_mode;

	switch (sdc_id) {
	case SDC0_SLOT:
		dma_mode = OWLXX_SDC0WT_DMAMODE;
		break;
	case SDC1_SLOT:
		dma_mode = OWLXX_SDC1WT_DMAMODE;
		break;
	case SDC2_SLOT:
		dma_mode = OWLXX_SDC2WT_DMAMODE;
		break;
	default:
		printf("error: MMC/SD slot %d not support\n", sdc_id);
		return -1;
	}

	return dma_mode;
}


static int acts_mmc_prepare_data(struct owlxx_mmc_host *host,
	struct mmc_data *data)
{
	char * align_buf_wr = NULL ;
	int flags = data->flags;
	unsigned int dma_channel;

	dma_channel = host->dma_channel;
	data->temp_aline_buf = NULL;
	data->alin_buf = NULL;

	pr_debug("Acquire dma channel: %d\n", host->dma_channel);
	pr_debug("\tblocksize:0x%x\n\tblocks:%u\n", data->blocksize,
		data->blocks);

	writel(readl(HOST_EN(host)) | SD_EN_BSEL, HOST_EN(host));

	writel(data->blocksize, HOST_BLK_SIZE(host));
	writel(data->blocks, HOST_BLK_NUM(host));
	if (data->blocksize < 512)
		writel(data->blocksize, HOST_BUF_SIZE(host));
	else
		writel(512, HOST_BUF_SIZE(host));

	stop_dma(dma_channel);

	if (flags & MMC_DATA_READ) {

		if(((unsigned int)(data->dest))%DAM_ALIE_BYTE){
			data->temp_aline_buf = malloc((data->blocks* data->blocksize)+DAM_ALIE_BYTE-1);
			if(data->temp_aline_buf == NULL){
				printf("malloc read buf err:length:0x%08x\n",((data->blocks* data->blocksize)+63));
				return -1;
			}	
			data->alin_buf =  ((unsigned int )(data->temp_aline_buf+DAM_ALIE_BYTE-1))\
								&(~(DAM_ALIE_BYTE-1));			
		}
		set_dma_mode(dma_channel, _config_read_dma_mode(host->id));
		set_dma_src_addr(dma_channel, HOST_DAT(host));
		if(data->alin_buf){
			set_dma_dst_addr(dma_channel, (unsigned int)data->alin_buf);
		}else{
			set_dma_dst_addr(dma_channel, (unsigned int)data->dest);
		}
		set_dma_frame_len(dma_channel, data->blocks);
		set_dma_frame_count(dma_channel,  data->blocksize);
	} else {
		if(((unsigned int)(data->src))%DAM_ALIE_BYTE){
			data->temp_aline_buf = malloc((data->blocks* data->blocksize)+DAM_ALIE_BYTE-1);
			if(data->temp_aline_buf == NULL){
				printf("malloc read buf err:length:0x%08x\n",((data->blocks* data->blocksize)+63));
				return -1;
			}	
			data->alin_buf =  ((unsigned int )(data->temp_aline_buf+DAM_ALIE_BYTE-1))\
			&(~(DAM_ALIE_BYTE-1));			
		}
		set_dma_mode(dma_channel, _config_write_dma_mode(host->id));

		if(data->alin_buf){ // need aline
			memcpy(data->alin_buf,data->src,data->blocks* data->blocksize);
			
			set_dma_src_addr((unsigned int)dma_channel,
			(unsigned int)data->alin_buf);
			
			flush_dcache_range((u32)data->alin_buf,
			(u32)data->alin_buf + data->blocks* data->blocksize);

		}else{
			set_dma_src_addr((unsigned int)dma_channel,
				(unsigned int)data->src);

			flush_dcache_range((u32)data->src,
				(u32)data->src + data->blocks* data->blocksize);
		}
		set_dma_dst_addr((unsigned int)dma_channel, HOST_DAT(host));
		set_dma_frame_len(dma_channel, data->blocks);
		set_dma_frame_count(dma_channel,  data->blocksize);	


		}
			start_dma((unsigned int)dma_channel);
			
			return 0;
}

static void acts_mmc_finish_request(struct owlxx_mmc_host *host)
{
	/* release DMA, etc */
	stop_dma(host->dma_channel);
	while (readl(HOST_CTL(host)) & SD_CTL_TS) {
		writel(readl(HOST_CTL(host)) & (~SD_CTL_TS),
			HOST_CTL(host));
	}
}

static int acts_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
	struct mmc_data *data)
{
	struct owlxx_mmc_host *host;
	u32 mode;
	unsigned int cmd_rsp_mask = 0;
	int power_up_check = 0;
	int dat0_busy_check = 0;
	int timeout = DATA_TRANSFER_TIMEOUT;
	int ret = 0;

	host = mmc->priv;

	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		mode = SD_CTL_TM(0);
		mdelay(1);
		break;

	case MMC_RSP_R1:
		if (data) {
			if (data->flags & MMC_DATA_READ)
				mode = SD_CTL_TM(4);
			else
				mode = SD_CTL_TM(5);
		} else {
			mode = SD_CTL_TM(1);
		}
		cmd_rsp_mask = 0x11;
		break;

	case MMC_RSP_R1b:
		mode = SD_CTL_TM(3);
		cmd_rsp_mask = 0x11;
		dat0_busy_check = 1;
		break;

	case MMC_RSP_R2:
		mode = SD_CTL_TM(2);
		cmd_rsp_mask = 0x11;
		break;

	case MMC_RSP_R3:
		cmd->cmdarg = 0x40ff8000;

		mode = SD_CTL_TM(1);
		cmd_rsp_mask = 0x10;
		power_up_check = 1;
		break;

	default:
		printf("error: no math command RSP flag %x\n", cmd->cmdarg);
		return -1;
	}

	/* keep current RDELAY & WDELAY value */
	mode |= (readl(HOST_CTL(host)) & (0xff << 16));

	/*
	 * start to send corresponding command type
	 */
	writel(cmd->cmdarg, HOST_ARG(host));
	writel(cmd->cmdidx, HOST_CMD(host));

	if (data) {
		ret = acts_mmc_prepare_data(host, data);
		if (ret != 0) {
			printf("error: Prepare data error\n");
			acts_mmc_finish_request(host);
			if(data->temp_aline_buf){
				free(data->temp_aline_buf);
				data->temp_aline_buf= NULL;
				data->alin_buf= NULL;
			}
			return -1;
		}
		mode |= (SD_CTL_TS | 0xE4000000);
	} else {
		mode |= SD_CTL_TS;
	}

	pr_debug("Transfer mode:0x%x\n\tArg:0x%x\n\tCmd:%u\n",
		mode, cmd->cmdarg, cmd->cmdidx);

	writel(mode, HOST_CTL(host));	/* start transfer */

	/* wait SDC transfer complete */
	while ((readl(HOST_CTL(host)) & SD_CTL_TS)  && timeout--)
		udelay(20);

	if (timeout <= 0) {
		printf("error: SDC%d transfered data timeout\n",
			host->id);
		acts_dump_mfp(host);
		acts_dump_sdc(host);
		acts_dump_dmac(host);
		acts_mmc_finish_request(host);
			if(data->temp_aline_buf){
				free(data->temp_aline_buf);			
				data->temp_aline_buf= NULL;
				data->alin_buf= NULL;
			}	
		return -1;
	}


	if (data) {
		timeout = DATA_TRANSFER_TIMEOUT;

		/* waite for DMA transfer complete */
		while (dma_started(host->dma_channel) && timeout--)
			udelay(20);

		if (timeout <= 0) {
			printf("error: DMA%d transfered data timeout\n",
				host->dma_channel);
			acts_dump_mfp(host);
			acts_dump_sdc(host);
			acts_dump_dmac(host);
			acts_mmc_finish_request(host);
			if(data->temp_aline_buf){
				free(data->temp_aline_buf);
				data->temp_aline_buf= NULL;
				data->alin_buf= NULL;
			}		
			return -1;
		}

		acts_mmc_finish_request(host);
		if (data->flags & MMC_DATA_READ){
			if(data->alin_buf){
			invalidate_dcache_range((u32)data->alin_buf,
				(u32)data->alin_buf + data->blocks
				* data->blocksize);
			memcpy(data->dest,data->alin_buf,(data->blocks *data->blocksize ));
			}else{
			invalidate_dcache_range((u32)data->dest,
				(u32)data->dest + data->blocks
				* data->blocksize);
			}
		}
		pr_debug("transfer data finish\n");
		if(data->temp_aline_buf){
			free(data->temp_aline_buf);
			data->temp_aline_buf= NULL;
			data->alin_buf= NULL;
		}	
		return 0;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd_rsp_mask) {
			if (readl(HOST_STATE(host)) & SD_STATE_CLNR) {
				printf("error: SDC%d send CMD%d, No rsp\n",
					host->id, cmd->cmdidx);
#if 0
				acts_dump_mfp(host);
				acts_dump_sdc(host);
#endif
				return TIMEOUT;
			}

			if (readl(HOST_STATE(host)) & (cmd_rsp_mask &
					SD_STATE_CRC7ER)) {
				printf("error: SDC%d send CMD%d, CRC7 error\n",
					host->id, cmd->cmdidx);
				acts_dump_mfp(host);
				acts_dump_sdc(host);
				return -1;
			}
		}


		/* wait for DAT0 busy status over.
		 * when DAT0 is low level, busy
		 */
		if (dat0_busy_check) {
			while ((readl(HOST_STATE(host)) &
					SD_STATE_DAT0S) != SD_STATE_DAT0S)
				;
		}

		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[3] = readl(HOST_RSPBUF0(host));
			cmd->response[2] = readl(HOST_RSPBUF1(host));
			cmd->response[1] = readl(HOST_RSPBUF2(host));
			cmd->response[0] = readl(HOST_RSPBUF3(host));
		} else {
			u32 rsp[2];
			rsp[0] = readl(HOST_RSPBUF0(host));
			rsp[1] = readl(HOST_RSPBUF1(host));
			cmd->response[0] = rsp[1] << 24 | rsp[0] >> 8;
			cmd->response[1] = rsp[1] >> 8;
		}
	}
	return 0;
}

/* MBytes */
static unsigned int get_emmc_cap_offset(int part, partition_info_t *p_cap)
{
	int i;
	u32 part_cap = 0;

	for (i = 0; i < part; i++) {
		u32 cap;

		cap = get_unaligned_le32(&p_cap[i].partition_cap);
		if (cap != 0xFFFFFFFF) {
			part_cap += cap;
		}
	}

	return part_cap;
}




/* MBytes */
unsigned int emmc_get_logic_cap(struct mmc *mmc)
{
	u64 cap;

	cap = mmc->capacity;
	return (cap >> 20);
}

static int emmc_read_mbr(mbr_info_t *mbr_info, struct mmc *mmc)
{
	u32 mbr_storage_step = (3 * 1024) / EMMC_SECTOR_SIZE;
	u32 lba, excact_cap, cap_from_mbr;
	int mbr_index, mbr_count = 4;

	int mbr_size = sizeof(mbr_info_t);
	int  i;
	int has_mbr = 0;

	excact_cap = emmc_get_logic_cap(mmc);
	excact_cap -= (EMMC_RESERVED >> 20);

	for (lba = 2, mbr_index = 0;
		mbr_index < mbr_count;
		lba += mbr_storage_step, mbr_index++) {

		printf("read lba=%u   mbr_index=%d    sectors=%d\n",
			lba, mbr_index, (mbr_size >> EMMC_SECTOR_SHIFT));

		if ((mmc->block_dev.block_read(mmc->block_dev.dev, lba,
					mbr_size >> EMMC_SECTOR_SHIFT,
					mbr_info)) != (mbr_size >> EMMC_SECTOR_SHIFT)) {
			printf("read %d MBR from lba %u error.\n", mbr_index, lba);
			break;
		}

		cap_from_mbr = 0;
		for (i = 0; i < MAX_PARTITION; i++) {
			partition_info_t *part;
			u32 part_cap;

			part = &mbr_info->partition_info[i];
			part_cap = get_unaligned_le32(&part->partition_cap);
			cap_from_mbr += part_cap;
			if (cap_from_mbr < part_cap) {
				printf("partition too large, part: %d, cap : %u Mbyte.\n",
					part->partition_num, part_cap);
				break;
			}

			if (part->partition_num == 0xff)
				break;
		}

		if (cap_from_mbr != excact_cap) {
			printf("capacity from MBR erro, cap_from_mbr=%u excact_cap=%u\n",
				cap_from_mbr, excact_cap);
			continue;
		}
		init_emmc_part(mbr_info);
		has_mbr = 1;
		break;

	}

	if (has_mbr)
		return 0;

	return -1;

}

/*
mmc  partion 0 is physical for boot
*/
int  mmc_partitions_init(int dev_index, int bootdev,struct owlxx_mmc_host *host)
{
	int ret = 0;
	unsigned  int part_num = 0;
	unsigned int last_part_cap = 0;

	struct sd_mmc_partinfo * sd_mmc_partinfo = NULL;
	struct sd_mmc_partinfo * mmc_sd_part =NULL;
	partition_info_t*partition_info =NULL;
	struct mmc * last_mmc = NULL;
	struct mmc * mmc = NULL;



	printf("%s:dev_index:%d,bootdev:%d\n",__FUNCTION__,dev_index,bootdev);


	sd_mmc_partinfo = malloc(MAX_PARTITION*sizeof(struct sd_mmc_partinfo));
	if(sd_mmc_partinfo == NULL){
		printf("%s: Error malloc mmc_sd_part\n ",__FUNCTION__);
		return -1;
	}

	partition_info = malloc(MAX_PARTITION*sizeof( partition_info_t));
	if(partition_info == NULL){
		printf("%s: Error malloc partition_info\n ",__FUNCTION__);
		return -2;
	}

	memcpy(partition_info, afinfo->partition_info, \
			sizeof(partition_info_t) * MAX_PARTITION);

	for (part_num = 0; part_num < MAX_PARTITION; part_num++) {

		// for debug
		printf("slotid:%d part_num:%d mmc_ptn:%d partition_num:%d  partition_cap 0x%x.\n",
			    	host->id,
				part_num, 
				partition_info[part_num].flash_ptn,
				partition_info[part_num].partition_num,
				partition_info[part_num].partition_cap);
	}

	for (part_num = 0; part_num < MAX_PARTITION; part_num++) {	
			
		if ((int)partition_info[part_num].partition_cap == -1) {
			break;
		}

		if(TSD == bootdev){				
			mmc = &tsd_dev[part_num];
			if(0 == part_num){
				acts_mmc_dev_init(mmc, host,bootdev,part_num);
			}
			
			if(part_num != 0){
				memcpy(&tsd_dev[part_num],&tsd_dev[0],sizeof(struct mmc));
			}
			
			if(ret = mmc_register(mmc)<0){
				printf("Error:%d,mmc_register fail\n",ret);
				return ret;
			}
	
			if(0 == part_num ){
				if(ret = mmc_init(mmc)){
					printf("%s,slot:%d, err:%d\n",__FUNCTION__,host->id,ret);
					return ret;
				}
			}
		
			mmc->block_dev.priv = &sd_mmc_partinfo[part_num];
			mmc_sd_part =  &sd_mmc_partinfo[part_num];
			mmc->block_dev.lba = partition_info[part_num].partition_cap;//MByte
			mmc->block_dev.lba <<= (20 - 9); //sector:512byte
			printf("wrlen:%d  readlen:%d\n",mmc->write_bl_len,mmc->read_bl_len);

			if(part_num == 0){
				sd_mmc_partinfo[part_num].off= 0;
			}else{
				last_mmc = &tsd_dev[part_num -1];
				mmc->read_bl_len = last_mmc ->read_bl_len;
				mmc->write_bl_len= last_mmc ->write_bl_len;
				mmc->erase_grp_size = last_mmc->erase_grp_size;
				sd_mmc_partinfo[part_num].off = sd_mmc_partinfo[part_num-1].off \
											+ last_mmc->block_dev.lba;
			}

			mmc->block_dev.blksz = 512;		

			
		}else if(MMC == bootdev){
			mmc = &mmc_dev[part_num];
			if(0 == part_num){
				acts_mmc_dev_init(mmc, host,bootdev,part_num);
			}
			
			if(part_num != 0){
				memcpy(&mmc_dev[part_num],&mmc_dev[0],sizeof(struct mmc));
			}
			
			if(ret = mmc_register(mmc)<0){
				printf("Error:%d,mmc_register fail\n",ret);
				return ret;
			}
	
			if(0 == part_num ){
				if(ret = mmc_init(mmc)){
					printf("%s,slot:%d, err:%d\n",__FUNCTION__,host->id,ret);
					return ret;
				}
			}
			
			mmc->block_dev.priv = &sd_mmc_partinfo[part_num];
			mmc_sd_part =  &sd_mmc_partinfo[part_num];
			mmc->block_dev.lba = partition_info[part_num].partition_cap;//Mbyte
			mmc->block_dev.lba <<= (20 - 9); //sector:512byte
			if(part_num == 0){
				sd_mmc_partinfo[part_num].off= 0;
			}else{
				last_mmc = &mmc_dev[part_num -1];
				mmc->read_bl_len = last_mmc ->read_bl_len;
				mmc->write_bl_len= last_mmc ->write_bl_len;
				mmc->erase_grp_size = last_mmc->erase_grp_size;
				sd_mmc_partinfo[part_num].off = sd_mmc_partinfo[part_num-1].off \
										     + last_mmc->block_dev.lba;
			}
			mmc->block_dev.blksz = 512;	
		}else{
			printf("%s,Error host->id : %d\n",host->id,__FUNCTION__);
			return -5 ;
		}	
		printf(" %s:slotid:%d  part_num:%d, off: 0x%08x  cap_sectors:0x%08x\n",__FUNCTION__,host->id,\
			part_num, mmc_sd_part->off,(u32)mmc->block_dev.lba);					
	}

	return ret;
}

static void acts_host_init(int index, struct owlxx_mmc_host *host)
{
	memset((char*)host,0,sizeof(struct owlxx_mmc_host));
	
	host->clock = 0;
	host->dma_channel = ATS_MMC_DMA_CHAN_NUM;

	switch (index) {
	case 0:
		host->module_id = MOD_ID_SD1;
		host->id = SDC1_SLOT;
		host->iobase = OWLXX_SDC1_BASE;
		host->pad_drv = SDC1_PAD_DRV;
		host->wdelay.delay_lowclk = SDC1_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC1_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC1_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC1_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC1_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC1_RDELAY_HIGH_CLK;
		break;
	case 1:
		host->module_id = MOD_ID_SD1;
		host->id = SDC1_SLOT;
		host->iobase = OWLXX_SDC1_BASE;
		host->pad_drv = SDC1_PAD_DRV;
		host->wdelay.delay_lowclk = SDC1_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC1_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC1_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC1_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC1_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC1_RDELAY_HIGH_CLK;
		break;
	case 2:
		host->module_id = MOD_ID_SD2;
		host->id = SDC2_SLOT;
		host->iobase = OWLXX_SDC2_BASE;
		host->pad_drv = SDC2_PAD_DRV;
		host->wdelay.delay_lowclk = SDC2_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC2_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC2_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC2_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC2_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC2_RDELAY_HIGH_CLK;
		break;
	default:
		printf("error: SD host controller not supported: %d\n", index);
		return -1;
	}
}




static void acts_mmc_dev_init(struct mmc *mmc, struct owlxx_mmc_host *host ,\
							unsigned  int bootdev,unsigned  int part_num)
{


	mmc->priv = host;
	if(bootdev == SD){
		sprintf(mmc->name, "SD%d",part_num);
	}
	else if(bootdev == TSD){
		sprintf(mmc->name, "TSD%d",part_num);
	}else if(bootdev == MMC){
		sprintf(mmc->name, "MMC%d",part_num);
	}else{
		printf("check boot dev type,set to default SD\n");
		sprintf(mmc->name, "SD");
	}
		
	mmc->send_cmd = acts_mmc_send_cmd;
	mmc->set_ios = acts_mmc_set_ios;
	mmc->init = acts_mmc_init_setup;
	mmc->poweroff = acts_mmc_power_off;
	mmc->getcd = NULL;

	mmc->voltages = ACTS_MMC_OCR;
	mmc->host_caps = (MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS |
		MMC_MODE_HC);

	mmc->f_min = 187500;
	mmc->f_max = 52000000;

	mmc->b_max = 0;
}



int acts_sd_init(struct owlxx_mmc_host *host)
{
	struct sd_mmc_partinfo * sd_partinfo =NULL;
	int ret = 0;
	struct mmc *mmc = &sd_dev;
	unsigned int boot_dev = get_sd_mmc_type();
	
	acts_mmc_dev_init(mmc, host,boot_dev,0);
	
	if(mmc_init(mmc)){
		printf("slot:%d, error mmc_init\n",host->id);
		return -1;
	}
	printf("host->id:%d,mmc_init OK\n",host->id);
	if((sd_partinfo =  malloc( sizeof(struct sd_mmc_partinfo))) ==NULL) {
		printf("error malloc sd_partinfo:%s,%d\n ",__FUNCTION__,__LINE__);
		return -2;
	}

	mmc->block_dev.priv = sd_partinfo;
	sd_partinfo->off = 0;
	mmc->block_dev.lba= mmc->capacity;
	mmc->block_dev.blksz = 512;
	printf("\n sd block_dev.lba:0x%08x , blksz:%d\n", \
			mmc->block_dev.lba,mmc->block_dev.blksz);
	if(ret = mmc_register(mmc)){
		printf("Error:%d,mmc_register fail\n",ret);
		return ret;
	}
	return ret ;
		
}


int acts_mmc_init(int dev_index, uint host_caps_mask, uint f_max)
{
	struct owlxx_mmc_host *host;
	struct mmc *mmc;
	int part_num;
	int ret = 0;

	unsigned int boot_dev = get_sd_mmc_type();
	printf("%s  dev_index:%d ,boot_dev:%d\n",__FUNCTION__,\
			dev_index,boot_dev);

	host = &mmc_host[dev_index];
	
	acts_host_init(dev_index, host);

	if(boot_dev == SD && dev_index == SDC0_SLOT){
		printf("register the sd\n");
		if(ret = acts_sd_init(host)){
			printf("error:%d,acts_sd_init fail\n",ret);
			return ret;
		}
		
	}else if((boot_dev == TSD && dev_index == SDC0_SLOT)\
		||(boot_dev == MMC && dev_index == SDC2_SLOT)){

		/* register the  partitions for tsd and emmc*/
		printf("register the  partitions for tsd and emmc\n");
		if(ret = mmc_partitions_init(dev_index,boot_dev,host)){
			printf("error:%d,mmc_partitions_init fail\n",ret);
			return ret;
		}
		if(owlxx_mmc_init_miscinfo()){
			printf("%s:err\n",__FUNCTION__);
			return -1;
		}
	}else{
		printf("%s : Error :bootdev:%d  dev_index:%d\n",__FUNCTION__,boot_dev,dev_index);
		ret = -1;
	}

	return ret;
}
