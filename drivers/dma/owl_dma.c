#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/reset.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>

/*
 * Simple DMA transfer operations defines for MMC/SD card
 */
#define DMA_MODE				0x0000
#define DMA_SOURCE				0x0004
#define DMA_DESTINATION				0x0008
#define DMA_FRAME_LEN				0x000C
#define DMA_FRAME_CNT				0x0010
#define DMA_REMAIN_FRAME_CNT			0x0014
#define DMA_REMAIN_CNT				0x0018
#define DMA_SOURCE_STRIDE			0x001C
#define DMA_DESTINATION_STRIDE			0x0020
#define DMA_START				0x0024
#define DMA_PAUSE				0x0028
#define DMA_CHAINED_CTL				0x002C
#define DMA_CONSTANT				0x0030
#define DMA_LINKLIST_CTL			0x0034
#define DMA_NEXT_DESCRIPTOR			0x0038
#define DMA_CURRENT_DESCRIPTOR_NUM		0x003C
#define DMA_INT_CTL				0x0040
#define DMA_INT_STATUS				0x0044
#define DMA_CURRENT_SOURCE_POINTER		0x0048
#define DMA_CURRENT_DESTINATION_POINTER		0x004C
#define DMA_BASE(i)				(DMA0_BASE + 0x100 * (i))

/* DMAx_MODE */
#define DMA_MODE_TS(x)			(((x) & 0x3f) << 0)
#define DMA_MODE_ST(x)			(((x) & 0x3) << 8)
#define		DMA_MODE_ST_DEV			DMA_MODE_ST(0)
#define		DMA_MODE_ST_DCU			DMA_MODE_ST(2)
#define		DMA_MODE_ST_SRAM		DMA_MODE_ST(3)
#define DMA_MODE_DT(x)			(((x) & 0x3) << 10)
#define		DMA_MODE_DT_DEV			DMA_MODE_DT(0)
#define		DMA_MODE_DT_DCU			DMA_MODE_DT(2)
#define		DMA_MODE_DT_SRAM		DMA_MODE_DT(3)
#define DMA_MODE_SAM(x)			(((x) & 0x3) << 16)
#define		DMA_MODE_SAM_CONST		DMA_MODE_SAM(0)
#define		DMA_MODE_SAM_INC		DMA_MODE_SAM(1)
#define		DMA_MODE_SAM_STRIDE		DMA_MODE_SAM(2)
#define DMA_MODE_DAM(x)			(((x) & 0x3) << 18)
#define		DMA_MODE_DAM_CONST		DMA_MODE_DAM(0)
#define		DMA_MODE_DAM_INC		DMA_MODE_DAM(1)
#define		DMA_MODE_DAM_STRIDE		DMA_MODE_DAM(2)
#define DMA_MODE_PW(x)			(((x) & 0x7) << 20)
#define DMA_MODE_CB			(0x1 << 23)
#define DMA_MODE_NDDBW(x)		(((x) & 0x1) << 28)
#define		DMA_MODE_NDDBW_32BIT		DMA_MODE_NDDBW(0)
#define		DMA_MODE_NDDBW_8BIT		DMA_MODE_NDDBW(1)
#define DMA_MODE_CFE			(0x1 << 29)
#define DMA_MODE_LME			(0x1 << 30)
#define DMA_MODE_CME			(0x1 << 31)

struct owl_dma_dev {
	int in_use;
	unsigned long base;
};

static struct owl_dma_dev owl_dma_devs[OWL_MAX_DMA];

int owl_dma_init(void)
{
	int i;

	for (i = 0; i < OWL_MAX_DMA; i++) {
		owl_dma_devs[i].in_use = 0;
		owl_dma_devs[i].base = DMA_BASE(i);
	}

	/* FIXME: reserve two dma channel for nand */
	owl_dma_devs[0].in_use = 1;
	owl_dma_devs[1].in_use = 1;

	owl_clk_enable(CLOCK_DMAC);
	owl_reset(RESET_DMAC);

	return 0;
}

/**
 * owl_dma_config - request dma channel
 *
 * this function return the channel device,NULL for error
 *
 */
struct owl_dma_dev *owl_dma_request(void)
{
	int i;
	struct owl_dma_dev *dma_dev;

	for (i = 0; i < OWL_MAX_DMA; i++) {
		if (owl_dma_devs[i].in_use == 0)
			break;
	}
	if (i == OWL_MAX_DMA)
		return NULL;

	owl_dma_devs[i].in_use = 1;
	dma_dev = &owl_dma_devs[i];

	return dma_dev;
}

int owl_dma_free(struct owl_dma_dev *dev)
{
	if (dev->in_use != 1)
		return -EINVAL;

	dev->in_use = 0;

	return 0;
}
/**
 * owl_dma_config - config the dma
 * @dev: dma device that request a dma
 * @drq: drq number
 * @src: source address
 * @dst: dest address
 * @len: the length must less than 1M bytes
 * @flag: dma flag
 *
 * return 0 for success
 *
 */
int owl_dma_config(struct owl_dma_dev *dev, unsigned int drq,
		   unsigned int src, unsigned int dst,
		   unsigned int len, unsigned int flag)
{
	unsigned int mode = DMA_MODE_TS(drq);

	/* max transfer 1MB data per dma transfer */
	if (len > 1 * 1024 * 1024)
		return -EINVAL;
	
	/* dma cannot transfer for device to device */
	if (IS_DEVICE_ADDR(src) && IS_DEVICE_ADDR(dst))
		return -EINVAL;

	/* set source address mode */
	if (IS_DEVICE_ADDR(src))
		mode |= DMA_MODE_ST_DEV | DMA_MODE_SAM_CONST;
	else if (IS_SHARERAM_ADDR(src))
		mode |= DMA_MODE_ST_SRAM | DMA_MODE_SAM_INC;
	else
		mode |= DMA_MODE_ST_DCU | DMA_MODE_SAM_INC;

	/* set destination address mode */
	if (IS_DEVICE_ADDR(dst))
		mode |= DMA_MODE_DT_DEV | DMA_MODE_DAM_CONST;
	else if (IS_SHARERAM_ADDR(dst))
		mode |= DMA_MODE_DT_SRAM | DMA_MODE_DAM_INC;
	else
		mode |= DMA_MODE_DT_DCU | DMA_MODE_DAM_INC;

	if (flag & DMA_FLAG_BUSWIDTH_1_BYTE)
		mode |= DMA_MODE_NDDBW_8BIT;
	else 
		mode |= DMA_MODE_NDDBW_32BIT;

	writel(mode, dev->base + DMA_MODE);
	writel(src, dev->base + DMA_SOURCE);
	writel(dst, dev->base + DMA_DESTINATION);
	writel(len, dev->base + DMA_FRAME_LEN);
	writel(1, dev->base + DMA_FRAME_CNT);

	return 0;
}

/*start the dma*/
void owl_dma_start(struct owl_dma_dev *dev)
{
	writel(1, dev->base + DMA_START);
}

/*stop the dma*/
void owl_dma_stop(struct owl_dma_dev *dev)
{
	writel(0, dev->base + DMA_START);
}

/**
 * pause_dma() - pause the dma transfer
 * @dmanr: dma channel number
 */
void owl_pause_dma(struct owl_dma_dev *dev)
{
	writel(1, dev->base + DMA_PAUSE);
}

void owl_resume_dma(struct owl_dma_dev *dev)
{
	writel(0, dev->base + DMA_PAUSE);
}

/**
 * owl_dma_is_finshed - test dma is end
 *
 *this function return the 0 for end
 */
int owl_dma_is_finished(struct owl_dma_dev *dev)
{
	return readl(dev->base + DMA_START) ? 0 : 1;
}

/**
 * owl_dma_config - wait for dma to end
 * @dev: dma device that request a dma
 * @us: if us == 0 the function will wait until dma translate over
 *      if us > 0,the function will test and wait us times if translate
 *      over then return
 *
 * this function return the 0 for the success
 */
int owl_dma_wait_finished(struct owl_dma_dev *dev, unsigned int us)
{
	unsigned int finish = 0;
	int count;

	if (us == 0) {
		/* wait until end */
		while (readl(dev->base + DMA_START))
			;
		return 0;
	} else {
		/* wait until end or timeout */
		for (count = 0; count < us; count++) {
			finish = readl(dev->base + DMA_START);
			if (finish == 0)
				return 0;
			udelay(1);
		}
		return -ETIMEDOUT;
	}
}
