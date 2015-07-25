#ifndef _OWLXX_MMC_H_
#define _OWLXX_MMC_H_

#include <asm/arch/clocks.h>



#define MMC_CMD_COMPLETE	1
#define ATS_MMC_DMA_CHAN_NUM	0

/*
 * command response code
 */
#define CMD_RSP_OK		0
#define CMD_RSP_ERR		1
#define CMD_RSP_BUSY		2
#define CMD_RSP_CRC_ERROR	3

#define SDC0_SLOT		0
#define SDC1_SLOT		1
#define SDC2_SLOT		2

#define OWLXX_SDC0_BASE	0xB0230000
#define OWLXX_SDC1_BASE	0xB0234000
#define OWLXX_SDC2_BASE	0xB0238000

#define ACTS_MMC_OCR (MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30  | \
	MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33  | \
	MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36)

struct mmc_con_delay {
	unsigned char delay_lowclk;
	unsigned char delay_midclk;
	unsigned char delay_highclk;
};

struct owlxx_mmc_host {
	u32 id;			/* SD Controller number */
	u32 iobase;
	u32 module_id;		/* module ID */

	u32 clock;		/* current clock frequency */
	int bus_width;		/* data bus width */
	u32 clk_on;		/* card module clock status */
	int dma_channel;

	u16 dma_dir;

	struct mmc_con_delay wdelay;
	struct mmc_con_delay rdelay;
	unsigned char pad_drv;
};

/*
 * PAD Drive Capacity config
 */
#define PAD_DRV_LOW		(0)
#define PAD_DRV_MID		(1)
#define PAD_DRV_HIGH		(3)

#define SDC0_WDELAY_LOW_CLK	(0xf)
#define SDC0_WDELAY_MID_CLK	(0xa)
#define SDC0_WDELAY_HIGH_CLK	(0x9)

#define SDC0_RDELAY_LOW_CLK	(0xf)
#define SDC0_RDELAY_MID_CLK	(0xa)
#define SDC0_RDELAY_HIGH_CLK	(0x8)

#define SDC0_PAD_DRV		PAD_DRV_HIGH

#define SDC1_WDELAY_LOW_CLK	(0xf)
#define SDC1_WDELAY_MID_CLK	(0xa)
#define SDC1_WDELAY_HIGH_CLK	(0x8)

#define SDC1_RDELAY_LOW_CLK	(0xf)
#define SDC1_RDELAY_MID_CLK	(0xa)
#define SDC1_RDELAY_HIGH_CLK	(0x8)

#define SDC1_PAD_DRV		PAD_DRV_MID

#define SDC2_WDELAY_LOW_CLK	(0xf)
#define SDC2_WDELAY_MID_CLK	(0xa)
#define SDC2_WDELAY_HIGH_CLK	(0x8)

#define SDC2_RDELAY_LOW_CLK	(0xf)
#define SDC2_RDELAY_MID_CLK	(0xa)
#define SDC2_RDELAY_HIGH_CLK	(0x8)

#define SDC2_PAD_DRV		PAD_DRV_MID


/*
 * SDC registers
 */
#define SD_EN_OFFSET			0x0000
#define SD_CTL_OFFSET			0x0004
#define SD_STATE_OFFSET			0x0008
#define SD_CMD_OFFSET			0x000c
#define SD_ARG_OFFSET			0x0010
#define SD_RSPBUF0_OFFSET		0x0014
#define SD_RSPBUF1_OFFSET		0x0018
#define SD_RSPBUF2_OFFSET		0x001c
#define SD_RSPBUF3_OFFSET		0x0020
#define SD_RSPBUF4_OFFSET		0x0024
#define SD_DAT_OFFSET			0x0028
#define SD_BLK_SIZE_OFFSET		0x002c
#define SD_BLK_NUM_OFFSET		0x0030
#define SD_BUF_SIZE_OFFSET		0x0034

#define HOST_EN(h)			((h)->iobase + SD_EN_OFFSET)
#define HOST_CTL(h)			((h)->iobase + SD_CTL_OFFSET)
#define HOST_STATE(h)			((h)->iobase + SD_STATE_OFFSET)
#define HOST_CMD(h)			((h)->iobase + SD_CMD_OFFSET)
#define HOST_ARG(h)			((h)->iobase + SD_ARG_OFFSET)
#define HOST_RSPBUF0(h)			((h)->iobase + SD_RSPBUF0_OFFSET)
#define HOST_RSPBUF1(h)			((h)->iobase + SD_RSPBUF1_OFFSET)
#define HOST_RSPBUF2(h)			((h)->iobase + SD_RSPBUF2_OFFSET)
#define HOST_RSPBUF3(h)			((h)->iobase + SD_RSPBUF3_OFFSET)
#define HOST_RSPBUF4(h)			((h)->iobase + SD_RSPBUF4_OFFSET)
#define HOST_DAT(h)			((h)->iobase + SD_DAT_OFFSET)
#define HOST_BLK_SIZE(h)		((h)->iobase + SD_BLK_SIZE_OFFSET)
#define HOST_BLK_NUM(h)			((h)->iobase + SD_BLK_NUM_OFFSET)
#define HOST_BUF_SIZE(h)		((h)->iobase + SD_BUF_SIZE_OFFSET)

/*
 * Register Bit defines
 */

/*
 * Register SD_EN
 */
#define SD_EN_RANE			(1 << 31)
/* bit 30 reserved */
#define SD_EN_RAN_SEED(x)		(((x) & 0x3f) << 24)
/* bit 23~13 reserved */
#define SD_EN_S18EN			(1 << 12)
/* bit 11 reserved */
#define SD_EN_RESE			(1 << 10)
#define SD_EN_DAT1_S			(1 << 9)
#define SD_EN_CLK_S			(1 << 8)
#define SD_ENABLE			(1 << 7)
#define SD_EN_BSEL			(1 << 6)
/* bit 5~4 reserved */
#define SD_EN_SDIOEN			(1 << 3)
#define SD_EN_DDREN			(1 << 2)
#define SD_EN_DATAWID(x)		(((x) & 0x3) << 0)

/*
 * Register SD_CTL
 */
#define SD_CTL_TOUTEN			(1 << 31)
#define SD_CTL_TOUTCNT(x)		(((x) & 0x7f) << 24)
#define SD_CTL_RDELAY(x)		(((x) & 0xf) << 20)
#define SD_CTL_WDELAY(x)		(((x) & 0xf) << 16)
/* bit 15~14 reserved */
#define SD_CTL_CMDLEN			(1 << 13)
#define SD_CTL_SCC			(1 << 12)
#define SD_CTL_TCN(x)			(((x) & 0xf) << 8)
#define SD_CTL_TS			(1 << 7)
#define SD_CTL_LBE			(1 << 6)
#define SD_CTL_C7EN			(1 << 5)
/* bit 4 reserved */
#define SD_CTL_TM(x)			(((x) & 0xf) << 0)

/*
 * Register SD_STATE
 */
/* bit 31~19 reserved */
#define SD_STATE_DAT1BS			(1 << 18)
#define SD_STATE_SDIOB_P		(1 << 17)
#define SD_STATE_SDIOB_EN		(1 << 16)
#define SD_STATE_TOUTE			(1 << 15)
#define SD_STATE_BAEP			(1 << 14)
/* bit 13 reserved */
#define SD_STATE_MEMRDY			(1 << 12)
#define SD_STATE_CMDS			(1 << 11)
#define SD_STATE_DAT1AS			(1 << 10)
#define SD_STATE_SDIOA_P		(1 << 9)
#define SD_STATE_SDIOA_EN		(1 << 8)
#define SD_STATE_DAT0S			(1 << 7)
#define SD_STATE_TEIE			(1 << 6)
#define SD_STATE_TEI			(1 << 5)
#define SD_STATE_CLNR			(1 << 4)
#define SD_STATE_CLC			(1 << 3)
#define SD_STATE_WC16ER			(1 << 2)
#define SD_STATE_RC16ER			(1 << 1)
#define SD_STATE_CRC7ER			(1 << 0)

/*
 * DMA mode config
 */

#if 0  /* save for SRAM */
#define OWLXX_SDC0WT_DMAMODE		(0x00010302)	/* SRAM->FIFO */
#define OWLXX_SDC1WT_DMAMODE		(0x00010303)
#define OWLXX_SDC2WT_DMAMODE		(0x00010304)

#define OWLXX_SDC0RD_DMAMODE		(0x00040C02)	/* FIFO->SRAM */
#define OWLXX_SDC1RD_DMAMODE		(0x00040C03)
#define OWLXX_SDC2RD_DMAMODE		(0x00040C04)
#endif

#if 0 /* Normal mode */
#define OWLXX_SDC0WT_DMAMODE		(0x00010202)	/* DDR->FIFO */
#define OWLXX_SDC1WT_DMAMODE		(0x00010203)
#define OWLXX_SDC2WT_DMAMODE		(0x00010204)

#define OWLXX_SDC0RD_DMAMODE		(0x00040802)	/* FIFO->DDR */
#define OWLXX_SDC1RD_DMAMODE		(0x00040803)
#define OWLXX_SDC2RD_DMAMODE		(0x00040804)
#endif

/* stride mode */
#define OWLXX_SDC0WT_DMAMODE		(0x00020202)	/* DDR->FIFO */
#define OWLXX_SDC1WT_DMAMODE		(0x00020203)
#define OWLXX_SDC2WT_DMAMODE		(0x00020204)

#define OWLXX_SDC0RD_DMAMODE		(0x00080802)	/* FIFO->DDR */
#define OWLXX_SDC1RD_DMAMODE		(0x00080803)
#define OWLXX_SDC2RD_DMAMODE		(0x00080804)


/*
 * PAD drive capacity config
 */
#define SD1_DRV_HIGH_MASK		(~(0x3 << 20))	/* SD1 DATA */
#define SD1_DRV_HIGH_LOW		(0x0 << 20)
#define SD1_DRV_HIGH_MID		(0x1 << 20)
#define SD1_DRV_HIGH_HIGH		(0x2 << 20)

#define SD1_DRV_HIGH_MASK2		(~(0xF << 12))	/* SD1 CMD, CLK */
#define SD1_DRV_HIGH2_LOW		(0x0 << 12)
#define SD1_DRV_HIGH2_MID		(0x5 << 12)
#define SD1_DRV_HIGH2_HIGH		(0xa << 12)

#define SD0_DRV_HIGH_MASK		(~(0x3 << 22))	/* SD0 DATA */
#define SD0_DRV_HIGH_LOW		(0x0 << 22)
#define SD0_DRV_HIGH_MID		(0x1 << 22)
#define SD0_DRV_HIGH_HIGH		(0x3 << 22)

#define SD0_DRV_HIGH_MASK2		(~(0xF << 16))	/* SD0 CMD, CLK */
#define SD0_DRV_HIGH2_LOW		(0x0 << 16)
#define SD0_DRV_HIGH2_MID		(0x5 << 16)
#define SD0_DRV_HIGH2_HIGH		(0xf << 16)


#define DMA_ALIGN_NUM  			32

#define SDNUM					1



/* eMMC card */
struct sd_mmc_partinfo {
	unsigned long partsize;
	unsigned long off;
	unsigned char type;
};

#if 0
enum boot_type {
	SD  = 0 ,
	TSD ,
	MMC ,		
};
#endif

/* Other FUNCs */
enum module_id {
	MOD_ID_SD0,
	MOD_ID_SD1,
	MOD_ID_SD2,
};

static int module_reset(enum module_id mod_id)
{
	u32 regv, offset;
	u32 timeout;

	switch (mod_id) {
	case MOD_ID_SD0:
		offset = 1 << 4;
		break;
	case MOD_ID_SD1:
		offset = 1 << 5;
		break;
	case MOD_ID_SD2:
		offset = 1 << 9;
		break;
	default:
		printf("error: RST, Mod not supported\n");
		return -1;
	}

	/* clear */
	timeout = 0;
	regv = readl(CMU_DEVRST0);
	regv &= ~offset;
	writel(regv, CMU_DEVRST0);
	do {
		regv = readl(CMU_DEVRST0);
		regv &= offset;
		timeout++;
	} while ((0 != regv) && (timeout < 0x100));

	/* set */
	timeout = 0;
	regv = readl(CMU_DEVRST0);
	regv |= offset;
	writel(regv, CMU_DEVRST0);
	do {
		regv = readl(CMU_DEVRST0);
		regv &= offset;
		timeout++;
	} while ((0 == regv) && (timeout < 0x100));

	return 0;
}

static void module_clk_set(enum module_id mod_id, int enable)
{
	u32 regv, offset;

	switch (mod_id) {
	case MOD_ID_SD0:
		offset = 1 << 5;
		break;
	case MOD_ID_SD1:
		offset = 1 << 6;
		break;
	case MOD_ID_SD2:
		offset = 1 << 7;
		break;
	default:
		printf("error: CLK, Mod not supported\n");
		return;
	}

	if (enable)
		setbits_le32(CMU_DEVCLKEN0, offset);
	else
		clrbits_le32(CMU_DEVCLKEN0, offset);

	readl(CMU_DEVCLKEN0);
}

static int module_clk_enable(enum module_id mod_id)
{
	module_clk_set(mod_id, 1);
	return 0;
}

static int module_clk_disable(enum module_id mod_id)
{
	module_clk_set(mod_id, 0);
	return 0;
}

static int module_clk_set_rate(enum module_id mod_id, unsigned long rate)
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

		if(MOD_ID_SD2 == mod_id){
			regv  = readl(CMU_SD2CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD2CLK);
		}else if(MOD_ID_SD0 == mod_id){
			regv  = readl(CMU_SD0CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD0CLK);
		}else if(MOD_ID_SD1 == mod_id){
			regv  = readl(CMU_SD1CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD1CLK);
		}else{
			printf("%s:%d:err\n",__FUNCTION__,__LINE__);
			return -1;
		}
	}
	return 0;
}

#endif /* end of _OWLXX_MMC_H_ */
