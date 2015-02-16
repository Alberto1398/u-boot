/********************************************************************************
*Date        : 2012-07-19
*Author      : YuJing
*File        : usb2isr.c
********************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#include "asm/arch/adfuserver.h"


void usb2isr(void *p_usr_data)
{
        
    INFO("%s\n", __FUNCTION__);
    ID_67_UsbOtgInt0();
}


