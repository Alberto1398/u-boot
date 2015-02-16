/********************************************************************************
Date        : 2007-01-11                                                         
Author      : LaoFeng
Mender	    : HuangHe                                                            
File        : otg.h                                                              
********************************************************************************/

#ifndef   __OTG_H__
#define   __OTG_H__

#include "asm/arch/usb/usb2/usb200.h"

#pragma pack(1) //force sizeof() to byte alligned

#define EP1INSTARTADD   0x00000080  
#define EP2INSTARTADD   0x00000280
#define EP3INSTARTADD   0x00000480  
#define EP4INSTARTADD   0x00000680
#define EP5INSTARTADD   0x00000880  
 
#define EP1OUTSTARTADD  0x00000a80  
#define EP2OUTSTARTADD  0x00000c80  
#define EP3OUTSTARTADD  0x00000e80  
#define EP4OUTSTARTADD  0x00001080
#define EP5OUTSTARTADD  0x00001280 

//USB Interrupt Vector
#define UIV_SUDAV           0x00
#define UIV_SOF             0x04
#define UIV_SUTOK           0x08
#define UIV_SUSPEND         0x0c
#define UIV_USBRESET        0x10
#define UIV_HSPEED          0x14
#define UIV_HCOUT0ERR       0x16
#define UIV_EP0IN           0x18
#define UIV_HCEP0OUT        0x18
#define UIV_HCIN0ERR        0x1a
#define UIV_EP0OUT          0x1c
#define UIV_HCEP0IN         0x1c
#define UIV_EP0PING         0x20
#define UIV_HCOUT1ERR       0x22
#define UIV_EP1IN           0x24
#define UIV_HCEP1OUT        0x24
#define UIV_HCIN1ERR        0x26
#define UIV_EP1OUT          0x28
#define UIV_HCEP1IN         0x28
#define UIV_EP1PING         0x2c
#define UIV_HCOUT2ERR       0x2e
#define UIV_EP2IN           0x30
#define UIV_HCEP2OUT        0x30
#define UIV_HCIN2ERR        0x32
#define UIV_EP2OUT          0x34
#define UIV_HCEP2IN         0x34
#define UIV_EP2PING         0x38
#define UIV_HCOUT3ERR       0x3a
#define UIV_EP3IN           0x3c
#define UIV_HCEP3OUT        0x3c
#define UIV_HCIN3ERR        0x3e
#define UIV_EP3OUT          0x40
#define UIV_HCEP3IN         0x40
#define UIV_EP3PING         0x44
#define UIV_OTGIRQ          0xd8

//bit mask define:
#define SUDAVIE     		0x01
#define SOFIE						0x02
#define HCSOFIE     		0x02
#define SUTOKIE     		0x04
#define SUSPIE      		0x08
#define URESIE      		0x10
#define HSPIE       		0x20
#define HCHSPIE     		0x20
#define OUTX_INX_CON_STALL   	0x40		//bit6 of outxcon or inxcon; stall
#define OUTX_INX_CON_VAL     	0x80		//bit7 of outxcon or inxcon; valid

#define SUDAVIR   		0x01        	//bit 0 of usbirq(hcusbirq); setup data valid irq
#define SOFIR     		0x02        	//bit 1 of usbirq(hcusbirq); start-of-frame interupt irq
#define HCFRMNIRQ 		0x02        	//bit 1 of usbirq(hcusbirq); start-of-frame interupt irq
#define SUTOKIR   		0x04        	//bit 2 of usbirq(hcusbirq); setup token irq
#define SUSPIR    		0x08        	//bit 3 of usbirq(hcusbirq); suspend irq
#define URESIR    		0x10        	//bit 4 of usbirq(hcusbirq); USB reset irq
#define HCURESIR  		0x10        	//bit 4 of usbirq(hcusbirq); USB reset irq
#define HSPEEDIR  		0x20        	//bit 5 of usbirq(hcusbirq); high speed irq
#define HCHSPEEDIR		0x20        	//bit 5 of usbirq(hcusbirq); high speed irq

#define B_STALL 		0x01    	//bit 0 of ep0cs, stall
#define B_EPX_STALL 	0x40    	//bit 6 of epXCON, stall
#define B_HSNAK 		0x02    	//bit 1 of ep0cs, handshake nak
#define EPTYPE_BULK     	0x08    	//bit[3..2] of outxcon, endpoint type
#define EPTYPE_INT      	0x0c    	//bit[3..2] of outxcon, endpoint type
#define EPTYPE_ISO      	0x04    	//bit[3..2] of outxcon, endpoint type
#define EPMULTIBUFF_SINGLE  	0x00    	//bit[1..0] of outxcon, multiple buffering
#define EPMULTIBUFF_DOUBLE  	0x01    	//bit[1..0] of outxcon, multiple buffering
#define EPMULTIBUFF_TRIPLE  	0x02    	//bit[1..0] of outxcon, multiple buffering
#define EPMULTIBUFF_QUAD    	0x03    	//bit[1..0] of outxcon, multiple buffering

#define BM_PERIPHIEN   		0x10       	//bit4 of otgien
#define BM_VBUSERRIEN  		0x08       	//bit3 of otgien,
#define BM_LOCSOFIEN   		0x04       	//bit2 of otgien,
#define BM_SRPDETIEN   		0x02       	//bit1 of otgien,
#define BM_IDLEIEN     		0x01       	//bit1 of otgien,

#define BM_HCSETTOGGLE  	0x40        	//bit6 of hcep0cs,
#define BM_HCCLRTOGGLE  	0x20        	//bit5 of hcep0cs,
#define BM_HCSET        	0x10        	//bit4 of hcep0cs,
#define BM_HCINBSY      	0x08        	//bit3 of hcep0cs,
#define BM_HCOUTBSY     	0x04        	//bit2 of hcep0cs,
#define BM_HSNAK        	0x01        	//bit 1 of ep0cs, handshake nak
#define BM_STALL        	0x00        	//bit 0 of ep0cs, stall

//Ep0 Status
//#define IDLE    		0x00
//#define TX      		0x01
//#define RX      		0x02

//OTG state machine
#define OTG_FSM_A_IDLE          0
#define OTG_FSM_A_WAIT_VRISE    1
#define OTG_FSM_A_WAIT_BCON     2
#define OTG_FSM_A_HOST          3
#define OTG_FSM_A_SUSPEND       4
#define OTG_FSM_A_PERIPHERAL    5
#define OTG_FSM_A_VBUS_ERR      6
#define OTG_FSM_A_WAIT_VFALL    7
#define OTG_FSM_B_IDLE          8
#define OTG_FSM_B_PERIPHERAL    9
#define OTG_FSM_B_WAIT_ACON     10
#define OTG_FSM_B_HOST          11
#define OTG_FSM_B_SRP_INIT1     12
#define OTG_FSM_B_SRP_INIT2     13
#define OTG_FSM_B_DISCHARGE1    14
#define OTG_FSM_B_DISCHARGE2    15


#define OTG_EP0_MAX_PACKET_SIZE        64
#define OTG_EP1_MAX_PACKET_SIZE_HS     512
#define OTG_EP2_MAX_PACKET_SIZE_HS     512
#define OTG_EP3_MAX_PACKET_SIZE_HS     512
#define OTG_EP1_MAX_PACKET_SIZE_FS     64
#define OTG_EP2_MAX_PACKET_SIZE_FS     64
#define OTG_EP3_MAX_PACKET_SIZE_FS     64

#pragma pack()

#define EPCS_BUSY                 (1<<1)
#endif   /* __OTG_H__ */

