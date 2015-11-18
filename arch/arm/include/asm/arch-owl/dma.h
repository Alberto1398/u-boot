/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_DMA_H__
#define __ASM_ARCH_DMA_H__

#if defined(CONFIG_S900)
#include <asm/arch/dma_s900.h>
#else
#error Unknown SoC type
#endif

#ifndef __ASSEMBLY__

struct owl_dma_dev;

int owl_dma_init(void);
struct owl_dma_dev *owl_dma_request(void);
int owl_dma_free(struct owl_dma_dev *dev);

/* dma config flags */
#define DMA_FLAG_BUSWIDTH_1_BYTE	(1 << 0)
int owl_dma_config(struct owl_dma_dev *dev, unsigned int drq,
		   unsigned int src, unsigned int dst,
		   unsigned int len, unsigned int flag);

void owl_dma_start(struct owl_dma_dev *dev);
void owl_dma_stop(struct owl_dma_dev *dev);
void owl_pause_dma(struct owl_dma_dev *dev);
void owl_resume_dma(struct owl_dma_dev *dev);
int owl_dma_is_finished(struct owl_dma_dev *dev);
int owl_dma_wait_finished(struct owl_dma_dev *dev, unsigned int us);

#endif /* __ASSEMBLY__ */

#endif  /* __ASM_ARCH_DMA_H__ */
