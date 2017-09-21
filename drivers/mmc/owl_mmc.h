#ifndef _OWL_MMC_H_
#define _OWL_MMC_H_

#if defined(CONFIG_S900)
#include <owl_mmc_s900.h>
#elif defined(CONFIG_S700)
#include <owl_mmc_s700.h>
#else
#error Unknown SoC type
#endif

#define SLOT0  0
#define SLOT2  2
#define SLOT3  3

#define MMC_CMD_COMPLETE	1
/*
 * command response code
 */
#define CMD_RSP_OK				0
#define CMD_RSP_ERR			1
#define CMD_RSP_BUSY			2
#define CMD_RSP_CRC_ERROR	3
#define OWL_MMC_OCR 	(MMC_VDD_27_28 | MMC_VDD_28_29 |\
						MMC_VDD_29_30 | MMC_VDD_30_31 |\
						MMC_VDD_31_32 | MMC_VDD_32_33 |\
						MMC_VDD_33_34 | MMC_VDD_34_35 |\
						MMC_VDD_35_36)
/*
 * PAD Drive Capacity config
 */
#define PAD_DRV_LOW			(0)
#define PAD_DRV_MID			(1)
#define PAD_DRV_HIGH			(3)
#define SDC0_WDELAY_LOW_CLK	(0xf)
#define SDC0_WDELAY_MID_CLK	(0xa)
#define SDC0_WDELAY_HIGH_CLK	(0x9)
#define SDC0_RDELAY_LOW_CLK	(0xf)
#define SDC0_RDELAY_MID_CLK	(0xa)
#define SDC0_RDELAY_HIGH_CLK	(0x8)
#define SDC0_PAD_DRV			PAD_DRV_HIGH
#define SDC1_WDELAY_LOW_CLK	(0xf)
#define SDC1_WDELAY_MID_CLK	(0xa)
#define SDC1_WDELAY_HIGH_CLK	(0x8)
#define SDC1_RDELAY_LOW_CLK	(0xf)
#define SDC1_RDELAY_MID_CLK	(0xa)
#define SDC1_RDELAY_HIGH_CLK	(0x8)
#define SDC1_PAD_DRV			PAD_DRV_MID
#define SDC2_WDELAY_LOW_CLK	(0xf)
#define SDC2_WDELAY_MID_CLK	(0xa)
#define SDC2_WDELAY_HIGH_CLK	(0x8)
#define SDC2_RDELAY_LOW_CLK	(0xf)
#define SDC2_RDELAY_MID_CLK	(0xa)
#define SDC2_RDELAY_HIGH_CLK	(0x8)
#define SDC2_PAD_DRV			PAD_DRV_MID
#define SDC3_WDELAY_LOW_CLK	(0xf)
#define SDC3_WDELAY_MID_CLK	(0xa)
#define SDC3_WDELAY_HIGH_CLK	(0x8)
#define SDC3_RDELAY_LOW_CLK	(0xf)
#define SDC3_RDELAY_MID_CLK	(0xa)
#define SDC3_RDELAY_HIGH_CLK	(0x8)
#define SDC3_PAD_DRV			PAD_DRV_MID
/*
 * SDC registers
 */
#define SD_EN_OFFSET			0x0000
#define SD_CTL_OFFSET			0x0004
#define SD_STATE_OFFSET		0x0008
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
#define HOST_EN(h)				((h)->iobase + SD_EN_OFFSET)
#define HOST_CTL(h)				((h)->iobase + SD_CTL_OFFSET)
#define HOST_STATE(h)			((h)->iobase + SD_STATE_OFFSET)
#define HOST_CMD(h)				((h)->iobase + SD_CMD_OFFSET)
#define HOST_ARG(h)				((h)->iobase + SD_ARG_OFFSET)
#define HOST_RSPBUF0(h)			((h)->iobase + SD_RSPBUF0_OFFSET)
#define HOST_RSPBUF1(h)			((h)->iobase + SD_RSPBUF1_OFFSET)
#define HOST_RSPBUF2(h)			((h)->iobase + SD_RSPBUF2_OFFSET)
#define HOST_RSPBUF3(h)			((h)->iobase + SD_RSPBUF3_OFFSET)
#define HOST_RSPBUF4(h)			((h)->iobase + SD_RSPBUF4_OFFSET)
#define HOST_DAT(h)				((h)->iobase + SD_DAT_OFFSET)
#define HOST_BLK_SIZE(h)		((h)->iobase + SD_BLK_SIZE_OFFSET)
#define HOST_BLK_NUM(h)		((h)->iobase + SD_BLK_NUM_OFFSET)
#define HOST_BUF_SIZE(h)		((h)->iobase + SD_BUF_SIZE_OFFSET)
/*
 * Register SD_EN
 */
#define SD_EN_RANE				(1 << 31)
/* bit 30 reserved */
#define SD_EN_RAN_SEED(x)		(((x) & 0x3f) << 24)
/* bit 23~13 reserved */
#define SD_EN_S18EN				(1 << 12)
/* bit 11 reserved */
#define SD_EN_RESE				(1 << 10)
#define SD_EN_DAT1_S			(1 << 9)
#define SD_EN_CLK_S				(1 << 8)
#define SD_ENABLE				(1 << 7)
#define SD_EN_BSEL				(1 << 6)
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
#define SD_CTL_SCC				(1 << 12)
#define SD_CTL_TCN(x)			(((x) & 0xf) << 8)
#define SD_CTL_TS				(1 << 7)
#define SD_CTL_LBE				(1 << 6)
#define SD_CTL_C7EN				(1 << 5)
/* bit 4 reserved */
#define SD_CTL_TM(x)			(((x) & 0xf) << 0)
/*
 * Register SD_STATE
 */
/* bit 31~19 reserved */
#define SD_STATE_DAT1BS		(1 << 18)
#define SD_STATE_SDIOB_P		(1 << 17)
#define SD_STATE_SDIOB_EN		(1 << 16)
#define SD_STATE_TOUTE			(1 << 15)
#define SD_STATE_BAEP			(1 << 14)
/* bit 13 reserved */
#define SD_STATE_MEMRDY		(1 << 12)
#define SD_STATE_CMDS			(1 << 11)
#define SD_STATE_DAT1AS		(1 << 10)
#define SD_STATE_SDIOA_P		(1 << 9)
#define SD_STATE_SDIOA_EN		(1 << 8)
#define SD_STATE_DAT0S			(1 << 7)
#define SD_STATE_TEIE			(1 << 6)
#define SD_STATE_TEI			(1 << 5)
#define SD_STATE_CLNR			(1 << 4)
#define SD_STATE_CLC			(1 << 3)
#define SD_STATE_WC16ER		(1 << 2)
#define SD_STATE_RC16ER		(1 << 1)
#define SD_STATE_CRC7ER		(1 << 0)
/*
 * DMA mode config
 */
/* Normal mode */
#define OWL_SDC0WT_DMAMODE		(0x00010202)	/* DDR->FIFO */
#define OWL_SDC1WT_DMAMODE		(0x00010203)
#define OWL_SDC2WT_DMAMODE		(0x00010204)
#define OWL_SDC3WT_DMAMODE		(0x0001022E)
#define OWL_SDC0RD_DMAMODE		(0x00040802)	/* FIFO->DDR */
#define OWL_SDC1RD_DMAMODE		(0x00040803)
#define OWL_SDC2RD_DMAMODE		(0x00040804)
#define OWL_SDC3RD_DMAMODE		(0x0004082E)
/*
 * PAD drive capacity config
 */
#define SD1_DRV_HIGH_MASK			(~(0x3 << 20))	/* SD1 DATA */
#define SD1_DRV_HIGH_LOW			(0x0 << 20)
#define SD1_DRV_HIGH_MID			(0x1 << 20)
#define SD1_DRV_HIGH_HIGH			(0x2 << 20)
#define SD1_DRV_HIGH_MASK2		(~(0xF << 12))	/* SD1 CMD, CLK */
#define SD1_DRV_HIGH2_LOW			(0x0 << 12)
#define SD1_DRV_HIGH2_MID			(0x5 << 12)
#define SD1_DRV_HIGH2_HIGH		(0xa << 12)
#define SD0_DRV_HIGH_MASK			(~(0x3 << 22))	/* SD0 DATA */
#define SD0_DRV_HIGH_LOW			(0x0 << 22)
#define SD0_DRV_HIGH_MID			(0x1 << 22)
#define SD0_DRV_HIGH_HIGH			(0x3 << 22)
#define SD0_DRV_HIGH_MASK2		(~(0xF << 16))	/* SD0 CMD, CLK */
#define SD0_DRV_HIGH2_LOW			(0x0 << 16)
#define SD0_DRV_HIGH2_MID			(0x5 << 16)
#define SD0_DRV_HIGH2_HIGH		(0xf << 16)

enum sdctr_id {
	SDC0_SLOT,
	SDC1_SLOT,
	SDC2_SLOT,
	SDC3_SLOT,
};

struct mmc_con_delay {
	unsigned char delay_lowclk;
	unsigned char delay_midclk;
	unsigned char delay_highclk;
};

struct owl_mmc_host {
	u32 id;			/* SD Controller number */
	u32 dma_irq;		/* SD Controller DMA IRQ */
	unsigned long iobase;
	u32 clock;		/* current clock frequency */
	int bus_width;		/* data bus width */
	u32 clk_on;		/* card module clock status */
	struct owl_dma_dev *dma_channel;
	u16 dma_dir;
	char pad_drv;
	struct mmc_con_delay wdelay;
	struct mmc_con_delay rdelay;

};

#endif /* end of _OWL_MMC_H_ */
