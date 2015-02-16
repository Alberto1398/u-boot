/**
**
**
**
**
**
**
**
*******/
#ifndef _DWC_REG_FUN_H_
#define _DWC_REG_FUN_H_

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

#include "asm/arch/usb/usb3/data_type_define.h"


//extern uint32_t readl(uint32_t volatile *reg);
//extern void writel(uint32_t value,uint32_t volatile *reg);

extern uint32_t DWC_READ_REG32(uint32_t volatile *reg);
#define dwc_read_reg32 DWC_READ_REG32

extern void DWC_WRITE_REG32(uint32_t volatile *reg, uint32_t value);
#define dwc_write_reg32 DWC_WRITE_REG32

extern void DWC_MODIFY_REG32(uint32_t volatile *reg, uint32_t clear_mask, uint32_t set_mask);
#define dwc_modify_reg32 DWC_MODIFY_REG32

//extern void memcpy(uint8_t*dst,const uint8_t*src,uint32_t size);

extern void dwc_udelay(uint32_t num);
extern void dwc_mdelay(uint32_t num);

extern void memcpy_actions(uint8_t*dst,const uint8_t*src,uint32_t size);

extern void *dwc_dma_malloc(uint32_t size);
extern void dwc_dma_mfree(void *ptr);

#endif

