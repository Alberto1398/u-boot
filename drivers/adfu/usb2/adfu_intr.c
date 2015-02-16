/********************************************************************************
*
* History:         
*      <author>    <time>           <version >             <desc>
*      yujing      2011-12-29       1.0                    build this file 
********************************************************************************/
/*!
* \file		adfu.c
* \brief    adfu的代码主体
* \author	 yujing
*
* \version 1.0
* \date  
*******************************************************************************/


#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#include "asm/arch/usb/usb2/otg.h"
#include "asm/arch/adfuserver.h"


void UsbOtgEp1OutIsr(void)
{
    //clear interrupt:
    act_writeb(0x02,USB0_OUTXIRQ ); //clear OUT1 irq bit
    
    Usb3Ep1OutDeal();       
    
} 
   
void ID_67_UsbOtgInt0(void)
{
    unsigned char bIntVector;
    
    bIntVector = act_readb(USB0_IVECT);
    act_writeb(act_readb(USB0_USBEIRQ)|0x80,USB0_USBEIRQ );
     
    //we should clear main interrupt source, and then clear individual interrupt source:
    switch(bIntVector)        
    {
    		case UIV_EP1OUT   :
    				UsbOtgEp1OutIsr();
    				break;
    		default:    
    	      break;
    }

}

