#ifndef __DMA_OAL_H_
#define __DMA_OAL_H_

#define DMA_CHAN_TYPE_SPECIAL 0
extern void set_dma_mode(unsigned int dmanr, unsigned int mode);
extern void set_dma_src_addr(unsigned int dmanr, unsigned int a);
extern void set_dma_dst_addr(unsigned int dmanr, unsigned int a);
extern void set_dma_count(unsigned int dmanr, unsigned int count);
extern UINT32 dma_end(unsigned int dmanr);
extern void reset_dma(unsigned int dmanr);
extern void free_act213x_dma(unsigned int dmanr);

extern int clear_dma_tcirq_pend(unsigned int dmanr);
extern int clear_dma_htcirq_pend(unsigned int dmanr);
extern int request_act213x_dma(unsigned int chanmode, char owner[], void*p, int s, void*q);
extern int disable_dma_tcirq(unsigned int dmanr);
extern int disable_dma_htcirq(unsigned int dmanr);
extern void start_dma_with_param(unsigned int dmanr, unsigned int command);
extern void start_dma(unsigned int dmanr);
extern void stop_dma(unsigned int dmanr);

/*
 * definition excerp from linux/dma-mapping.h
 */
enum dma_data_direction {   //for compile only
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};

#endif
