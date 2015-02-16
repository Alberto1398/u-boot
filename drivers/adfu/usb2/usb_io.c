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

#include "asm/arch/adfuserver.h"
#include "asm/arch/usb/usb2/usb2_io.h"
#include "asm/arch/usb/usb2/usb.h"
#include "asm/arch/usb/usb2/otg.h"


static struct usb_dev  my_dev = {
	/* control endpoint */
	.ep[0] = {
		.maxpacket    = CTRL_PACKET_SIZE,
		.address      = 0,
	},
	
	/* bulk out endpoint */
	.ep[1] = {
		.maxpacket	= BULK_FS_PACKET_SIZE,
		.address		= USB_DIR_OUT	| 1,
		
	},

	/* bulk in endpoint */
	.ep[2] = {
		.maxpacket	= BULK_FS_PACKET_SIZE,
		.address		= USB_DIR_IN | 2,
		
	},

	
};

struct usb_dev * udev = &my_dev;

void USB0_epin2_sendata(unsigned int dLength, unsigned char * pBuffer)
{
    /*in endpoint: in2*/
	
	unsigned short iMaxDataLength, iActualLength;
	unsigned short i, j;
	unsigned int  k;
	unsigned char  *pOtgSfr;
	unsigned long  *pData;
	
	iMaxDataLength = udev->ep[2].maxpacket;	
	
	act_writeb(0x12, USB0_FIFOCTRL);
	
	while(dLength > 0)
	{
			while( act_readb(USB0_IN2CS) & 0x02);
			
			if (dLength > iMaxDataLength)       
			{	
					iActualLength = iMaxDataLength;   
   			}
   			else                                                        
   			{
   					iActualLength = dLength;
        }				
   			j = iActualLength/4;
   			k=0;  			
  			if (j!= 0)
			{	
					pData = (unsigned long *)pBuffer;						
					for(i=0; i<j; i++)	/* data may not be alligned by word */
					{   			
    						//P_USB0_FIFO2DATA = *pData;
    						act_writel(*pData ,USB0_FIFO2DATA);
    						pData++;
    						k+=4;
    				}
    				pBuffer += k;
  		}
			pOtgSfr = (UCHAR *)USB0_FIFO2DATA;
			while (k < iActualLength)
			{
    		    *pOtgSfr=*pBuffer;
					pBuffer++;
    		    pOtgSfr++;
    		    k++;
    		}
    								
	
        dLength	-= iActualLength;
        act_writew(iActualLength & 0xffff, USB0_IN2BCL);
        act_writeb(0x00, USB0_IN2CS );
	
	
        for(i=0; i<30; i++)
        {
        ;
        }
	
	
	
        while(act_readb(USB0_IN2CS) & 0x02);	
                 
	
	}
	
	while(act_readb(USB0_IN2CS) & 0x02);		
	
	
	
}
	
/**************************************************************************
*
*
*
************************************************************************/
void USB0_epout1_receivedata(unsigned int dLength, unsigned char * pBuffer)
{
    /*out endpoint: out1(phy endpoint number is 2)*/
    
    unsigned char 	i;
	unsigned short 	j;
	unsigned short 	k;
	unsigned char 	bEnd=0;
	unsigned short 	bDataLength;
  	 unsigned int 		iActualLength=0;  	
 		unsigned long 	*pData;
	unsigned char 	*pOtgSfr;  
  
  act_writeb(0x01, USB0_FIFOCTRL);
	
	do
	{     				
			  while(act_readb(USB0_OUT1CS) & 0x02);
			
 		    bDataLength = act_readw(USB0_OUT1BCL);
 		    
   		    iActualLength += bDataLength;
   		    k=0;
   		    j = bDataLength/4;
  		    if (j!= 0)
		    {	
		    			 pData = (unsigned long *)pBuffer;
		    			 for(i=0; i<j; i++)
		    			 {
		    			 		   *pData = act_readl(USB0_FIFO1DATA);
		    			 		   pData++; 
		    			 		   k +=4;
		    			 }
		    			 pBuffer += k;
 		    }
		    pOtgSfr = (UCHAR *)USB0_FIFO1DATA;
		    while (k < bDataLength)
		    {
   		        *pBuffer = *pOtgSfr;
		    	  pBuffer++;
   		        pOtgSfr++;
   		        k++;
   		    }
   		    
   		    act_writeb(0x00, USB0_OUT1CS);
   		    
   		    if (iActualLength >= dLength)   
		    {
		      		bEnd = 1;
		    }	
	}while(bEnd == 0);				
	
	
	
	
		
}

/*
**************************************************************************************
*
*           dma   dma   dma
*
*
************************************************************************/
//	void USB0_epin2_sendata(unsigned int dLength, unsigned char * pBuffer)
//	{
//	    /*in endpoint: in2*/
//		
//			unsigned int fifoctrl;
//			
//			while ((act_readb(USB0_IN2CS) & EPCS_BUSY));
//		
//			fifoctrl = (udev->ep[2].address & 0xf) | 0x30;
//			act_writeb(fifoctrl, USB0_FIFOCTRL);   //set the auto bit
//			act_writel((unsigned int)pBuffer, USB0_UDMA1MEMADDR );
//			act_writeb((udev->ep[2].address & 0xf)<<1 | 0, USB0_UDMA1EPSEL);
//			act_writel(dLength, USB0_UDMA1CNTL);
//			act_writeb(1, USB0_UDMA1COM);       //start udma
//		
//			while(act_readb(USB0_UDMA1COM) & 0x01);   //wait for transfer complete
//			
//			
//	}
//		
//		
//		
//		
//	void USB0_epout1_receivedata(unsigned int dLength, unsigned char * pBuffer)
//	{
//	    /*out endpoint: out1(phy endpoint number is 2)*/
//	    
//	    unsigned int actual = 0;
//	    unsigned int fifoctrl;
//		  unsigned int dmaloadlength;
//		
//	    if(dLength == 0)
//	  	{
//	  	    return 0;
//	  	}
//	    else
//	  	{
//	  	    //if(dLength >DMA_MAX_LEN)
//	  	    //{
//	  	    //    dmaloadlength = DMA_MAX_LEN;
//	  	    //}
//	  		  //else
//	  		  //{
//	  		      dmaloadlength = dLength;
//	  		  //}
//	  		
//	  	    while ((act_readb(USB0_OUT1CS) & EPCS_BUSY));
//	  
//	  		  fifoctrl = (udev->ep[1].address & 0xf) | 0x20;
//	  		  act_writeb(fifoctrl, USB0_FIFOCTRL);   //set the auto bit
//	  		  act_writel((unsigned int)pBuffer, USB0_UDMA1MEMADDR);
//	  		  act_writeb((udev->ep[1].address & 0xf)<<1 | 1, USB0_UDMA1EPSEL);
//	  		  act_writel(dmaloadlength, USB0_UDMA1CNTL);
//	  		  act_writeb(1, USB0_UDMA1COM);       //start udma
//	        
//	  		  while(act_readb(USB0_UDMA1COM) & 0x01);   //wait for transfer complete
//	        
//	  		  actual= dmaloadlength - act_readl(USB0_UDMA1REML)&0xffffff;
//	  
//		
//	    } 
//			
//				
//	}
