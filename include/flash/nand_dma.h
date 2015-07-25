#ifndef __DMA_OAL_H_
#define __DMA_OAL_H_

extern void NAND_SPIN_UNLOCK(void);
extern void NAND_SPIN_LOCK(void);


#ifdef OS_LINUX

#include <linux/dmaengine.h>
#include <linux/dma-direction.h>
#include <linux/spinlock.h>

#include <mach/acts_hdmac.h>

extern struct acts_dma_slave Gl520x_NandDmaSlave;
extern struct dma_slave_config NandDmaSlave;
extern struct dma_async_tx_descriptor *NandDma_TxDesc;
extern struct dma_chan *NandDMAChannel;
extern UINT32 rb_dma_pend_enable;
extern struct completion dma_complete;
extern unsigned long irq_flag;
extern spinlock_t nand_lock;
extern UINT32 rb_dma_pend_enable;

extern struct dma_chan * _NandDmaRequest(void);


extern void _NandDmaRelease(struct dma_chan *chan);


extern void _NandDmaReset(struct dma_chan *chan);


#else

extern UINT32 NandDMAChannel;

extern int _NandDmaRequest(void);


extern void _NandDmaRelease(int dmanr);


extern void _NandDmaReset(int dmanr);

#endif

extern void _NandDmaWriteConfig(UINT32 nDmaNum, void* Mem_Addr, UINT32 ByteCount);

extern void _NandDmaReadConfig(UINT32 nDmaNum, void* Mem_Addr, UINT32 ByteCount);

extern UINT32  _DmaOperation(UINT8 * pMain, UINT32 dir);

#endif
