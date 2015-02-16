#ifndef __USB3_IO_H__
#define __USB3_IO_H__

#include "asm/arch/usb/usb3/data_type_define.h"

void usb3_init(void);
void usb3isr(void *p_usr_data);
void actions_data_in(uint32_t leng,uint32_t addr);
void actions_data_out(uint32_t leng,uint32_t addr);

#endif /* __USB3_IO_H__ */
