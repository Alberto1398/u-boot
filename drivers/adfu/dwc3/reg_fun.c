/****
*
*
*
****/

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#include "asm/arch/usb/usb3/data_type_define.h"



/* Registers */

//uint32_t readl(uint32_t volatile *reg)
//{
//    return *(const volatile uint32_t *) reg;
//}
//
//void writel(uint32_t value,uint32_t volatile *reg)
//{
//    *(volatile uint32_t *) reg = value;
//}


uint32_t DWC_READ_REG32(uint32_t volatile *reg)
{
	return act_readl((uint32_t)reg);
}



void DWC_WRITE_REG32(uint32_t volatile *reg, uint32_t value)
{
	act_writel(value, (uint32_t)reg);
}



void DWC_MODIFY_REG32(uint32_t volatile *reg, uint32_t clear_mask, uint32_t set_mask)
{
	act_writel((act_readl((uint32_t)reg) & ~clear_mask) | set_mask, (uint32_t)reg);
}



void dwc_udelay(uint32_t num)
{
//    uint32_t i,j;
//    for(i=0; i<100; i++)
//    {
//        for(j=0; j<100; j++);
//    }
    udelay(num);
}

void dwc_mdelay(uint32_t num)
{
//    uint32_t i,j;
//    for(i=0; i<2; i++)
//    {
//        for(j=0; j<1000; j++);
//    }
    mdelay(num);
}

/*
void memcpy_actions(uint8_t *dst, const void *src, uint32_t size)
{
    uint32_t i;
    for(i=0; i<size; i++)
    {
        (*dst) = (*src);
		dst++;
		src++;
    }
}
*/

void memcpy_actions(void *destin, void *source, unsigned n)
{
// unsigned t;
// for (t=0;t<n;t++)
// {
//  *((char *)destin+t) = *((char *)source+t);
// }
    memcpy(destin, source, n);
} 





/*
void *memcpy(void *destin, void *source, unsigned n)
{
 unsigned t;
 for (t=0;t<n;t++)
 {
  *((char *)destin+t) = *((char *)source+t);
 }
 return destin;
} 
*/

/* no malloc function in SPL stage, we implement a simplest one here. */
#ifdef CONFIG_SPL_BUILD
static uint8_t __attribute__((aligned(ARCH_DMA_MINALIGN))) s_dwc_smpl_mempool[4096 + 3*1024];
static uint s_dwc_smpl_mempool_pos = 0;
#endif

void *dwc_dma_malloc(uint32_t size)
{
#ifdef CONFIG_SPL_BUILD
    void *p_new = NULL;
    uint free_size = sizeof(s_dwc_smpl_mempool) - s_dwc_smpl_mempool_pos;
    size = (size + ARCH_DMA_MINALIGN -1U) & ~(ARCH_DMA_MINALIGN -1U); // align.
    if(free_size >= size)
    {
        p_new = &s_dwc_smpl_mempool[s_dwc_smpl_mempool_pos];
        s_dwc_smpl_mempool_pos += size;
    }
    else
    {
        hang();
    }
    return p_new;
#else
    void *p_new = memalign(ARCH_DMA_MINALIGN, size);
    if(p_new == NULL)
    {
        puts("no mem for dwc controller\n");
        hang();
    }
    return p_new;
#endif
}

void dwc_dma_mfree(void *ptr)
{
#ifdef CONFIG_SPL_BUILD
    // do nothing.
#else
    free(ptr);
#endif
}
