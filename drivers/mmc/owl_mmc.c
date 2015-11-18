/*
 * owl_mmc.c - OWL SD/MMC driver
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
#include <owl_mmc.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clk.h>
#include <asm/arch/powergate.h>
#include <asm/arch/reset.h>
#include <asm/arch/dma.h>

#undef pr_debug
#ifdef OWL_MMC_DEBUG
#define pr_debug(format, arg...)	\
	printf(format, ##arg)
#else
#define pr_debug(format, args...)
#endif

/* 2 seconds */
#define DATA_TRANSFER_TIMEOUT		\
							(3 * (100 * 1000))

#define PHYMMC_BUF_SIZE (CONFIG_SYS_MMC_MAX_BLK_COUNT*256)
#define MAX_MMC_NUM 4
#define MAX_VIRMMC_NUM 2
extern unsigned int g_mmc_id;
struct owl_mmc_host mmc_host[MAX_MMC_NUM];
struct vir_mmc g_owl_vir_mmc[MAX_VIRMMC_NUM]={
	{
	.phy_mmca = NULL,
	.phy_mmcb = NULL,
	.dual_mmc_en = 0,
	},
	{
	.phy_mmca = NULL,
	.phy_mmcb = NULL,
	.dual_mmc_en = 0,
	}
};

void owl_dump_sdc(struct owl_mmc_host *host);
void owl_dump_mfp(struct owl_mmc_host *host);
static struct mmc_config *owl_virmmc_config_init(int id);

void owl_dump_debug(struct mmc *mmc)
{
	struct owl_mmc_host *host = (struct owl_mmc_host *)(mmc->priv);
	owl_dump_sdc(host);
	owl_dump_mfp(host);
}

void owl_dump_mfp(struct owl_mmc_host *host)
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
	printf("\tCD2CLK:0xMU_S%x\n", readl(CMU_SD2CLK));
	printf("\tCD3CLK:0xMU_S%x\n", readl(CMU_SD3CLK));
}

void owl_dump_sdc(struct owl_mmc_host *host)
{
	printf("\n\tSD_EN:0x%x\n", readl(HOST_EN(host)));
	printf("\n\tSD_CTL:0x%x\n", readl(HOST_CTL(host)));
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

static int owl_sd_clk_set_rate(enum sdctr_id id, unsigned long rate)
{

	unsigned long regv, div, div128;
	unsigned long parent_rate;

	if ((readl(CMU_DEVPLL) & (1 << 12)) && (readl(CMU_DEVPLL) & (1 << 8))) {
		parent_rate = readl(CMU_DEVPLL) & 0x7f;
		parent_rate *= 6000000;
		printf("MMC: source clk CMU_DEVPLL:%luHz\n", parent_rate);

	} else {
		printf("MMC: parent clock not used, CMU_DEVPLL:0x%x\n",
		       readl(CMU_DEVPLL));
		return -1;
	}

	rate *= 2;

	if (rate >= parent_rate)
		div = 0;
	else {
		div = parent_rate / rate;
		if (div >= 128) {
			div128 = div;
			div = div / 128;

			if (div128 % 128)
				div++;

			div--;
			div |= 0x100;
		} else {
			if (parent_rate % rate)
				div++;

			div--;
		}

		if (SDC3_SLOT == id) {
			regv = readl(CMU_SD3CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD3CLK);
		} else if (SDC2_SLOT == id) {
			regv = readl(CMU_SD2CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD2CLK);
		} else if (SDC0_SLOT == id) {
			regv = readl(CMU_SD0CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD0CLK);
		} else {
			printf("%s:%d:errid:%d:\n", __FUNCTION__, __LINE__,id);
			return -1;
		}
	}
	return 0;
}

static int owl_enable_clock(struct owl_mmc_host *host)
{

	if (!host->clk_on) {
		owl_clk_enable_by_perip_id(PERIPH_ID_SDMMC0 + host->id);
		host->clk_on = 1;
	}

	return 0;
}

static inline int owl_disable_clock(struct owl_mmc_host *host)
{
	return 0;
}

static void owl_mmc_set_clk(struct owl_mmc_host *host, int rate)
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

	owl_sd_clk_set_rate(host->id, rate);

}

static void owl_mmc_power_up(struct owl_mmc_host *host)
{
	owl_reset_by_perip_id(PERIPH_ID_SDMMC0 + host->id);
	owl_clk_enable_by_perip_id(PERIPH_ID_SDMMC0 + host->id);
	/* module function enable */
	writel(SD_ENABLE | SD_EN_RESE, HOST_EN(host));
}

DECLARE_GLOBAL_DATA_PTR;

static int owl_mmc_config_mfp(struct owl_mmc_host *host)
{
	unsigned int nand_alg0_ctr;
	unsigned int nand_alg1_ctr;
	pinmux_select((PERIPH_ID_SDMMC0 + host->id), 0);
	if (host->id == 2) {
		owl_powergate_power_on(POWERGATE_NAND);
		nand_alg0_ctr = readl(NAND_ALG0_CTR_IO);
		nand_alg0_ctr &= ~((1 << 21) | (1 << 20));
		nand_alg0_ctr |= (1 << 21);
		if (nand_alg0_ctr & EN_V18_R) {
			nand_alg0_ctr |= EN_V18_W;
		} else {
			nand_alg0_ctr &= ~EN_V18_W;
		}
		writel(nand_alg0_ctr, NAND_ALG0_CTR_IO);
	}else if(host->id == 3){
		nand_alg1_ctr = readl(NAND_ALG1_CTR_IO);
		nand_alg1_ctr &= ~((1 << 21) | (1 << 20));
		nand_alg1_ctr |= (1 << 21);
		if (nand_alg1_ctr & EN_V18_R) {
			nand_alg1_ctr |= EN_V18_W;
		} else {
			nand_alg1_ctr &= ~EN_V18_W;
		}
		writel(nand_alg1_ctr, NAND_ALG1_CTR_IO);
	}
	return 0;
}

static int owl_mmc_power_on(struct owl_mmc_host *host)
{
	int ret = 0;
	ret = owl_mmc_config_mfp(host);
	if (ret) {
		printf("err:%d:%s:%d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}

	return ret;

}

static int owl_mmc_init_setup(struct mmc *mmc)
{
	int ret = 0;
	struct owl_mmc_host *host = mmc->priv;

	owl_mmc_power_up(host);
	mdelay(10);
	ret = owl_mmc_power_on(host);

	return ret;
}

static void owl_mmc_set_ios(struct mmc *mmc)
{
	struct owl_mmc_host *host = mmc->priv;
	u32 ctrl_reg;

	owl_enable_clock(host);

	pr_debug("owl_mmc_set_ios:\n");

	if (mmc->clock && mmc->clock != host->clock) {
		host->clock = mmc->clock;
		pr_debug("\tSet clock: %d\n", host->clock);
		owl_mmc_set_clk(host, mmc->clock);
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

static int owl_mmc_prepare_data(struct owl_mmc_host *host,
				struct mmc_data *data)
{
	int ret = 0;
	int flags = data->flags;

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

	owl_dma_stop(host->dma_channel);

	flush_dcache_all();
	if (flags & MMC_DATA_READ) {
		ret = owl_dma_config(host->dma_channel,
				     host->dma_irq,
				     HOST_DAT(host),
				     (unsigned long)(data->dest),
				     (data->blocks * data->blocksize), 0);
		if (ret != 0) {
			printf("err:%d: read dma config err:config length:%x\n",
			       ret, (data->blocks * data->blocksize));

		}
	} else {
		ret = owl_dma_config(host->dma_channel,
				     host->dma_irq,
				     (unsigned long)(data->src), HOST_DAT(host),
				     (data->blocks * data->blocksize), 0);
		if (ret != 0) {
			printf
			    ("err:%d: write dma config err:config length:%x\n",
			     ret, (data->blocks * data->blocksize));

		}
	}
	owl_dma_start(host->dma_channel);

	return 0;
}

static void owl_mmc_finish_request(struct owl_mmc_host *host)
{
	/* release DMA, etc */

	owl_dma_stop(host->dma_channel);
	while (readl(HOST_CTL(host)) & SD_CTL_TS) {
		writel(readl(HOST_CTL(host)) & (~SD_CTL_TS), HOST_CTL(host));
	}

}

static int owl_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	struct owl_mmc_host *host;
	u32 mode;
	unsigned int cmd_rsp_mask = 0;
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
		ret = owl_mmc_prepare_data(host, data);
		/*set lbe to send clk after busy */
		if(host->id == 2 || host->id == 3){
			debug("emmc id:0x%x\n",g_mmc_id >> 24);
			if( (g_mmc_id >> 24) != FDI_EMMC_ID){
				mode |= SD_CTL_LBE ;
			}
		}else{
			/* not enable HW tiemout, use sw timeout */
			mode |=  SD_CTL_LBE ;
		}

		if (ret != 0) {
			printf("error: Prepare data error\n");
			owl_mmc_finish_request(host);
			return -1;
		}
		mode |= (SD_CTL_TS | 0xE4000000);
	} else {
		mode |= SD_CTL_TS;
	}

	debug("SDC%d:Transfer mode:0x%x\n\tArg:0x%x\n\tCmd:%u\n",
	      host->id, mode, cmd->cmdarg, cmd->cmdidx);

	writel(mode, HOST_CTL(host));	/* start transfer */

	/* wait SDC transfer complete */
	while ((readl(HOST_CTL(host)) & SD_CTL_TS) && timeout--)
		udelay(20);

	if (timeout <= 0) {
		printf("error: SDC%d transfered data timeout\n", host->id);
		owl_dump_mfp(host);
		owl_dump_sdc(host);
		owl_mmc_finish_request(host);
		return -1;
	}

	if (data) {
		if (owl_dma_wait_finished(host->dma_channel, 5000000)) {

			printf("SDC%d:dma transfer data 5s timeout\n",
			       host->id);
			owl_dump_mfp(host);
			owl_dump_sdc(host);
			owl_mmc_finish_request(host);
			return -1;
		}

		owl_mmc_finish_request(host);

		return 0;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd_rsp_mask) {
			if (readl(HOST_STATE(host)) & SD_STATE_CLNR) {
				printf("error: SDC%d send CMD%d, No rsp\n",
				       host->id, cmd->cmdidx);
#if 0
				owl_dump_mfp(host);
				owl_dump_sdc(host);

#endif
				return TIMEOUT;
			}

			if (readl(HOST_STATE(host)) & (cmd_rsp_mask &
						       SD_STATE_CRC7ER)) {
				printf("error: SDC%d send CMD%d, CRC7 error\n",
				       host->id, cmd->cmdidx);
				owl_dump_mfp(host);
				owl_dump_sdc(host);
				return -1;
			}
		}

		/* wait for DAT0 busy status over.
		 * when DAT0 is low level, busy
		 */
		if (dat0_busy_check) {
			while ((readl(HOST_STATE(host)) &
				SD_STATE_DAT0S) != SD_STATE_DAT0S){
				} ;
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

static int owl_host_init(int index, struct owl_mmc_host *host)
{

	memset(host, 0, sizeof(struct owl_mmc_host));

	host->dma_channel = owl_dma_request();
	if (!(host->dma_channel)) {
		printf("!!!err:owl_dma_request\n");
		return -1;
	}

	switch (index) {
	case 0:
		host->id = SDC0_SLOT;
		host->dma_irq = DMA_DRQ_SD0;
		host->iobase = OWL_SDC0_BASE;
		host->pad_drv = SDC0_PAD_DRV;
		host->wdelay.delay_lowclk = SDC0_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC0_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC0_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC0_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC0_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC0_RDELAY_HIGH_CLK;
		break;
	case 1:
		host->id = SDC1_SLOT;
		host->dma_irq = DMA_DRQ_SD1;
		host->iobase = OWL_SDC1_BASE;
		host->pad_drv = SDC1_PAD_DRV;
		host->wdelay.delay_lowclk = SDC1_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC1_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC1_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC1_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC1_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC1_RDELAY_HIGH_CLK;
		break;
	case 2:
		host->id = SDC2_SLOT;
		host->dma_irq = DMA_DRQ_SD2;
		host->iobase = OWL_SDC2_BASE;
		host->pad_drv = SDC2_PAD_DRV;
		host->wdelay.delay_lowclk = SDC2_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC2_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC2_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC2_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC2_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC2_RDELAY_HIGH_CLK;
		break;
	case 3:
		host->id = SDC3_SLOT;
		host->dma_irq = DMA_DRQ_SD3;
		host->iobase = OWL_SDC3_BASE;
		host->pad_drv = SDC3_PAD_DRV;
		host->wdelay.delay_lowclk = SDC3_WDELAY_LOW_CLK;
		host->wdelay.delay_midclk = SDC3_WDELAY_MID_CLK;
		host->wdelay.delay_highclk = SDC3_WDELAY_HIGH_CLK;
		host->rdelay.delay_lowclk = SDC3_RDELAY_LOW_CLK;
		host->rdelay.delay_midclk = SDC3_RDELAY_MID_CLK;
		host->rdelay.delay_highclk = SDC3_RDELAY_HIGH_CLK;
		break;
	default:
		printf("error: SD host controller not supported: %d\n", index);
		return -1;
	}
	return 0;
}

static struct mmc_config *owl_mmc_config_init(int dev_index)
{

	struct mmc_config *cfg = NULL;
	struct mmc_ops *mmc_ops = NULL;

	cfg = malloc(sizeof(struct mmc_config));
	if (cfg == NULL) {
		printf("err:%s:%d:malloc owl_mmc_config fail\n",
		       __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(cfg, 0, sizeof(struct mmc_config));

	mmc_ops = malloc(sizeof(struct mmc_ops));
	if (mmc_ops == NULL) {
		printf("err:%s:%d:malloc owl_mmc_ops fail\n",
		       __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(mmc_ops, 0, sizeof(struct mmc_ops));

	cfg->name = malloc(10);
	if (cfg->name == NULL) {
		printf("err:%s:%d:malloc owl_mmc_config->name fail\n",
		       __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(cfg->name, 0, 10);

	sprintf(cfg->name, "mmc%d", dev_index);

	cfg->part_type = PART_TYPE_EFI;
	cfg->ops = mmc_ops;

	mmc_ops->send_cmd = owl_mmc_send_cmd;
	mmc_ops->set_ios = owl_mmc_set_ios;
	mmc_ops->init = owl_mmc_init_setup;
	mmc_ops->getcd = NULL;

	if (dev_index == SDC0_SLOT) {
		cfg->host_caps =
		    (MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS);
	} else if ((dev_index == SDC2_SLOT) || (dev_index == SDC3_SLOT)) {
		cfg->host_caps =
		    (MMC_MODE_8BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS);
	} else {
		printf("%s:bad id:%d\n", __FUNCTION__, dev_index);
	}

	cfg->f_min = 187500;
	cfg->f_max = 52000000;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
	cfg->b_max = 512;
	cfg->voltages = OWL_MMC_OCR;

	return cfg;

}


static struct mmc_config *owl_virmmc_config_init(int id)
{

	struct mmc_config *cfg = NULL;

	cfg = malloc(sizeof(struct mmc_config));
	if (cfg == NULL) {
		printf("err:%s:%d:malloc owl_mmc_config fail\n",
		       __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(cfg, 0, sizeof(struct mmc_config));

	cfg->name = malloc(10);
	if (cfg->name == NULL) {
		printf("err:%s:%d:malloc owl_mmc_config->name fail\n",
		       __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(cfg->name, 0, 10);

	sprintf(cfg->name, "virmmc%d", id);


	return cfg;

}

int owl_mmc_init(int dev_index)
{
	struct mmc *phymmc_sd = NULL;
	struct mmc *phymmc_emmca = NULL;
	struct mmc *mmc = NULL;
	struct mmc_config *vircfg = NULL;
	struct mmc_config *phycfga = NULL;
	struct vir_mmc * vir_mmc = NULL;
	#ifdef DUAL_EMMC
	struct mmc_config *phycfgb = NULL;
	struct mmc *phymmc_emmcb = NULL;
	#endif
	int virmmc_id;
	int ret = 0;

	if(!(dev_index  == SLOT0 || dev_index  == SLOT2)  ){
		printf("%s:err dev_index:%d\n",__FUNCTION__,__LINE__);
		return -1;
	}

	ret = owl_host_init(dev_index, &mmc_host[dev_index]);
	if (ret) {
		printf("err:owl_host_init\n");
		return -1;
	}

	phycfga = owl_mmc_config_init(dev_index);
	if (!phycfga) {
		printf("err:owl_mmc_config_init\n");
		return -1;
	}

	if(dev_index){
		virmmc_id = 0;
	}else{
		virmmc_id = 1;
	}

	vircfg = owl_virmmc_config_init(virmmc_id);
	if (!vircfg) {
		printf("err:owl_mmc_config_init\n");
		return -1;
	}

	if(dev_index  == SLOT2){

#ifdef DUAL_EMMC
		ret = owl_host_init((SLOT3), &mmc_host[SLOT3]);
		if (ret) {
			printf("err:host% dowl_host_init\n",(dev_index+1));
			return -1;
		}
		phycfgb = owl_mmc_config_init(SLOT3);
		if (!phycfgb) {
			printf("err:owl_mmc_config_init\n");
			return -1;
		}
#endif
	}

	if(dev_index == SLOT0){

		phymmc_sd = mmc_create(phycfga, &mmc_host[dev_index]);
		if (!phymmc_sd) {
			printf("err:mmc_create phymmc_sd\n");
			return -1;
		}

		mmc = mmc_vir_create(vircfg,&g_owl_vir_mmc[virmmc_id]);
		if (!mmc) {
			printf("err:mmc_vir_create\n");
			return -1;
		}

		vir_mmc = (struct vir_mmc *)(mmc->priv);
		vir_mmc->phy_mmca = phymmc_sd;
		phymmc_sd->block_dev.dev = 0;
		ret = mmc_init(phymmc_sd);

		if (ret) {
			printf("host%d scan err\n", dev_index);
			printf("host0 checkout to uartpin\n");
			pinmux_select(PERIPH_ID_SDMMC0, 1);
			printf("ct2:0x%08x \n", readl(MFP_CTL2));
		}else{
			mmc->block_dev.lba = vir_mmc->phy_mmca->block_dev.lba;
			mmc->block_dev.part_type= phymmc_sd->block_dev.part_type ;
			printf("host%d scan ok\n", dev_index);
			return 0;
		}

	}else if(dev_index == SLOT2){

		mmc = mmc_vir_create(vircfg,&g_owl_vir_mmc[virmmc_id]);
		if (!mmc) {
			printf("err:mmc_vir_create\n");
			return -1;
		}
		vir_mmc = (struct vir_mmc *)(mmc->priv);
		phymmc_emmca = mmc_create(phycfga, &mmc_host[dev_index]);
		if (!phymmc_emmca) {
			printf("err:mmc_create phymmc_emmca\n");
			return -1;
		}
		vir_mmc->phy_mmca = phymmc_emmca;
		vir_mmc->phy_mmca->block_dev.dev = 1;

		if(mmc_init(vir_mmc->phy_mmca)){
			printf("phy_mmca init fail\n");
		}else{
			mmc->block_dev.part_type= phymmc_emmca->block_dev.part_type ;
			printf("phy_mmca init ok\n");
		}
#ifdef DUAL_EMMC
		vir_mmc->phy_mmca->card_trs_par.buf=malloc(PHYMMC_BUF_SIZE); // only init one time
		if(!(vir_mmc->phy_mmca->card_trs_par.buf)){
			printf("err:%s:malloc phy_mmca buf \n",__FUNCTION__);
			return -1;
		}

		phymmc_emmcb = mmc_create(phycfgb, &mmc_host[dev_index+1]);
		if (!phymmc_emmcb) {
			printf("err:mmc_create phymmc_emmcb\n");
			return -1;
		}
		vir_mmc->phy_mmcb = phymmc_emmcb;
		vir_mmc->phy_mmcb->card_trs_par.buf=malloc(PHYMMC_BUF_SIZE); // only init one time
		if(!(vir_mmc->phy_mmcb->card_trs_par.buf)){
			printf("err:%s:malloc phy_mmca buf \n",__FUNCTION__);
			return -1;
		}
		phymmc_emmcb->block_dev.dev = 1;
		if(mmc_init(vir_mmc->phy_mmcb)){
			printf("phy_mmcb init fail\n");
			return -1;
		}else{
			vir_mmc->dual_mmc_en = 1;
			printf("phy_mmcb init ok\n");
		 }
#endif
		 mmc_set_vir_cap(mmc);
	}


	return ret;
}
