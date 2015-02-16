
#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#include "asm/arch/adfuserver.h"
#include "asm/arch/usb/usb3/driver.h"
#include "asm/arch/usb/usb3/reg_fun.h"
#include "asm/arch/usb/usb3/cil.h"


/*******************************************************************************/
#define  USB_DT_DEVICE		        0x01
#define  USB_DT_CONFIG		        0x02
#define  USB_DT_STRING		        0x03
#define  USB_DT_INTERFACE	        0x04
#define  USB_DT_ENDPOINT		    0x05
#define  USB_DT_DEVICE_QUALIFIER	0x06
#define  USB_DT_OTHER_SPEED_CONFIGURATION 0x07
#define  USB_DT_INTERFACE_POWER	    0x08
#define  USB_DT_OTG		            0x09
#define  USB_DT_BOS         		0x0f
#define  USB_DT_DEVICE_CAPABILITY    0x10
#define  USB_DT_SS_USB_COMPANION	0x30

#define MAXPACKETSIZE_FS 64
#define MAXPACKETSIZE_HS 512
#define MAXPACKETSIZE_SS 1024

dwc_usb3_device_t _usb3_dev_;

/** EP0 */
dwc_usb3_pcd_ep_t ep0;

/** Array of OUT EPs (not including EP0) */
dwc_usb3_pcd_ep_t out_ep1;
dwc_usb3_pcd_ep_t out_ep2;


/** Array of IN EPs (not including EP0) */
dwc_usb3_pcd_ep_t in_ep1;
dwc_usb3_pcd_ep_t in_ep2;


/** 'dummy' request, for EP0 only */
dwc_usb3_pcd_request_t ep0_req;


extern usb_endpoint_descriptor_t out1_descriptor;
extern usb_endpoint_descriptor_t in2_descriptor;
static void dwc_usb3_pcd_endpoint_desc_init(dwc_usb3_device_t *usb3_dev)
{
  /*初始化描述符*/
  usb3_dev->pcd.out_ep[0]->dwc_ep.desc = &out1_descriptor;
	//usb3_dev->pcd.out_ep[1]->dwc_ep.desc = &_out2_desc_;
	//usb3_dev->pcd.in_ep[0]->dwc_ep.desc = &_in1_desc_;
	usb3_dev->pcd.in_ep[1]->dwc_ep.desc = &in2_descriptor;


}

/********************************************************************************
*Function: SetPhy  
*Description: Set phy registers
*Input: unsigned char bRegAdd, unsigned char bValue
*Output: none
*Other: 
*******************************************************************************/
void SetPhy(unsigned char bRegAdd, unsigned char bValue)
{
    #define P_USB_VCON    *(volatile unsigned long *)(0xB0400000 + 0x0000cd48)
    unsigned char bAddLow,bAddHigh;
    unsigned int vstate;
    
    bAddLow =  bRegAdd & 0x0f;
    bAddHigh =  (bRegAdd >> 4) & 0x0f;
	//temp = P_USB_VCON;
	//temp &= 0x80;

	vstate = bValue;
    //vstate = (vstate<<8) |  temp;
	vstate = vstate<<8;

	bAddLow |= 0x10;
	P_USB_VCON = vstate | bAddLow;

	bAddLow &= 0x0f; 
	P_USB_VCON = vstate | bAddLow;

	bAddLow |= 0x10;
	P_USB_VCON = vstate | bAddLow;

	bAddHigh |= 0x10;
	P_USB_VCON = vstate | bAddHigh;

	bAddHigh &= 0x0f; 
	P_USB_VCON = vstate | bAddHigh;

	bAddHigh |= 0x10;
	P_USB_VCON = vstate | bAddHigh;
		
        
}

#define DWC_TXTHRCFG_USB_TRANS_PCK_CNT_EN_SHIFT 29
#define DWC_TXTHRCFG_USB_TRANS_PCK_CNT_SHIFT 24

#define DWC_RXTHRCTL_PCK_CNT_EN_SHIFT 29
#define DWC_RXTHRCTL_PCK_CNT_SHIFT 24
#define DWC_RXTHRCTL_MAX_BURST_SHIFT 19

#define DWC_GCTL_U2RSTECN_SHIFT 16
#define DWC_GCTL_FRMCLDWN_SHIFT 14

#define DWC_PIPECTL_SKIP_DETECT_SHIFT 13

#define TX_FIFO0_START_ADDR       0x0
#define TX_FIFO1_START_ADDR       0x0
#define TX_FIFO2_START_ADDR       0x0
#define TX_FIFO3_START_ADDR       0x0
#define RX_FIFO0_START_ADDR       0x0

//	static void core_int(dwc_usb3_device_t *usb3_dev)
//	{
//	/*
//	    //GSBUSCFG0 = ;
//		//GSBUSCFG1 = ;
//		GTXTHRCFG = (0<<DWC_TXTHRCFG_USB_NON_ISO_THR_EN_SHIFT)
//		            | (1<<DWC_TXTHRCFG_USB_TRANS_PCK_CNT_EN_SHIFT)
//		            | (1<<DWC_TXTHRCFG_USB_TRANS_PCK_CNT_SHIFT)
//		            | (1<<DWC_TXTHRCFG_USB_THR_LEN_SHIFT)
//		            | (0<<DWC_TXTHRCFG_SBUS_NON_ISO_THR_EN_SHIFT)
//		            | (8<<DWC_TXTHRCFG_SBUS_THR_LEN_SHIFT);
//		GRXTHRCFG =  (0<<DWC_RXTHRCTL_PCK_CNT_EN_SHIFT)     //for device mode
//			       | (1<<DWC_RXTHRCTL_PCK_CNT_SHIFT)    
//			       | (0<<DWC_RXTHRCTL_MAX_BURST_SHIFT)
//			       | (8<<DWC_RXTHRCTL_THR_LEN_SHIFT);
//	*/
//	#if 1
//		GCTL =   (0<<DWC_GCTL_PWR_DN_SCALE_SHIFT)
//			   | (0<<DWC_GCTL_U2RSTECN_SHIFT)
//			   | (0<<DWC_GCTL_FRMCLDWN_SHIFT)    //125US
//			   | (DWC_GCTL_PRT_CAP_DEVICE<<DWC_GCTL_PRT_CAP_DIR_SHIFT)  //device mode
//			   //| (DWC_GCTL_PRT_CAP_DRD<<DWC_GCTL_PRT_CAP_DIR_SHIFT)  //DRD mode
//			   | (0<<DWC_GCTL_CORE_SOFT_RST_SHIFT)
//			   | (0<<DWC_GCTL_LOCAL_LPBK_EN_SHIFT)
//			   | (0<<DWC_GCTL_LPBK_EN_SHIFT)
//			   | (0<<DWC_GCTL_DEBUG_ATTACH_SHIFT)
//			   | (0<<DWC_GCTL_RAM_CLK_SEL_SHIFT)  //bus clock
//			   //| (1<<DWC_GCTL_SCALE_DOWN_SHIFT)   //scale down mode(01)
//			   | (0<<DWC_GCTL_DIS_SCRAMBLE_SHIFT)
//			   //| (1<<DWC_GCTL_SS_PWR_CLMP_SHIFT)
//			   //| (1<<DWC_GCTL_HSFSLS_PWR_CLMP_SHIFT)
//			   | (0<<DWC_GCTL_DSBL_CLCK_GTNG_SHIFT);
//	
//	#endif
//	
//		//GUSB2CFG = 0x00001448;
//	  GUSB2CFG = 0x00001408;
//	  GSBUSCFG0 = (GSBUSCFG0 & 0xffffff00) | 0x6;  //enable burst 8 and burst 4
//	  GSBUSCFG1 = (GSBUSCFG1 & 0xfffff0ff) | (0x2<<8); //AXI busrt requst limit is 3
//	
//		/*confige for the usb2 PHY*/
//		unsigned int lValue;
//	  SetPhy(0xe2,0x80);  //0x80
//	  for (lValue=0;lValue<1000;lValue++);
//	  SetPhy(0xe7,0x0b);
//	  SetPhy(0xe7,0x1f);	
//		  	  
//	  //sesitivetiy calibration
//		SetPhy(0xe3,0x9c);
//		SetPhy(0xe3,0xdc);
//			
//		//configurate the 45ohm and disconnet
//		for (lValue=0;lValue<1000;lValue++);
//		//SetPhy(0xe2,0x80);
//		SetPhy(0xe2,0x88);//0x88
//	  for (lValue=0;lValue<1000;lValue++);
//		
//		return;
//	
//	}

static void core_int(dwc_usb3_device_t *usb3_dev)
{
#if 1
	GCTL =   (0<<DWC_GCTL_PWR_DN_SCALE_SHIFT)
		   | (0<<DWC_GCTL_U2RSTECN_SHIFT)
		   | (0<<DWC_GCTL_FRMCLDWN_SHIFT)    //125US
		   | (DWC_GCTL_PRT_CAP_DEVICE<<DWC_GCTL_PRT_CAP_DIR_SHIFT)  //device mode
		   //| (DWC_GCTL_PRT_CAP_DRD<<DWC_GCTL_PRT_CAP_DIR_SHIFT)  //DRD mode
		   | (0<<DWC_GCTL_CORE_SOFT_RST_SHIFT)
		   | (0<<DWC_GCTL_LOCAL_LPBK_EN_SHIFT)
		   | (0<<DWC_GCTL_LPBK_EN_SHIFT)
		   | (0<<DWC_GCTL_DEBUG_ATTACH_SHIFT)
		   | (0<<DWC_GCTL_RAM_CLK_SEL_SHIFT)  //bus clock
		   //| (1<<DWC_GCTL_SCALE_DOWN_SHIFT)   //scale down mode(01)
		   | (0<<DWC_GCTL_DIS_SCRAMBLE_SHIFT)
		   //| (1<<DWC_GCTL_SS_PWR_CLMP_SHIFT)
		   //| (1<<DWC_GCTL_HSFSLS_PWR_CLMP_SHIFT)
		   | (0<<DWC_GCTL_DSBL_CLCK_GTNG_SHIFT);

#endif

	GUSB2CFG = 0x00001408;
	GSBUSCFG0 = (GSBUSCFG0 & 0xffffff00) | 0x6;  //enable burst 8 and burst 4
  GSBUSCFG1 = (GSBUSCFG1 & 0xfffff0ff) | (0x2<<8); //AXI busrt requst limit is 3

#if 0
	/*confige for the usb2 PHY*/
	unsigned int lValue;
  SetPhy(0xe2,0x80);  //0x80
  for (lValue=0;lValue<1000;lValue++);
  SetPhy(0xe7,0x0b);
	SetPhy(0xe7,0x1f);	
	  	  
	//sesitivetiy calibration
	SetPhy(0xe3,0x9c);
	SetPhy(0xe3,0xdc);
		
	//configurate the 45ohm and disconnet
	for (lValue=0;lValue<1000;lValue++);
	//SetPhy(0xe2,0x80);
	SetPhy(0xe2,0x88);//0x88
  for (lValue=0;lValue<1000;lValue++);
#endif	

	return;

}

/******************************************************************************/
/*!                    
* \par  Description:
*		usb相关硬件、变量初始化，因内容较多，所以从sys_init中抽离出来
* \param		void      
* \return       0 
* \ingroup      adfudec
* \par          exmaple code
* \code 
*               usb3_init();
* \endcode
*******************************************************************************/
extern int dwc_usb3_pcd_init(dwc_usb3_device_t *usb3_dev);
//	void usb3_init(void)
//	{
//	  dwc_usb3_device_t *usb3_dev = &_usb3_dev_;
//	
//		usb3_dev->pcd.ep0 = &ep0;
//		usb3_dev->pcd.out_ep[0]= &out_ep1;
//		usb3_dev->pcd.out_ep[1]= &out_ep2;
//		usb3_dev->pcd.in_ep[0]= &in_ep1;
//		usb3_dev->pcd.in_ep[1]= &in_ep2;
//	
//		usb3_dev->pcd.ep0_req = &ep0_req;
//	
//		/*初始化参数*/
//	  dwc_usb3_core_params_t core_params = {
//	      .burst = 0,
//	      .new = 0,
//	      .phy = 0,
//	      .wakeup = 0,
//	      .pwrctl= 0,
//	  };
//	
//		/*USB3 reset*/
//		#define P_CMU_DEVRST1 *(volatile unsigned long *)(CMU_DEVRST1)
//		P_CMU_DEVRST1 = P_CMU_DEVRST1 & (~(1<<14));  //usb3 reset
//		dwc_udelay(1);
//		dwc_udelay(1);
//		dwc_udelay(1);
//		P_CMU_DEVRST1 = P_CMU_DEVRST1 | (1<<14);  //clean USB3 reset
//	
//	    /*phy reset*/
//	    //USB3_PHY_TEMP = 0x0a4145d0;
//	  USB3_PHY_TEMP = USB3_PHY_TEMP | 0x00100000;   //USB3 BIAS enable
//	
//	    //GCTL = (GCTL & 0xffffffc0) | 0x00000010;	//scale-down
//	
//		/*disable the Supper speed,*/
//		DCFG = (DCFG & 0xfffffff8) | 0x0;
//	#if 0
//		//clean the TRB memory space
//		unsigned int *addr;
//		unsigned int i;
//		addr = (unsigned int *)0xb40d0000;
//		for(i=0; i<1024*16; i++)
//		{
//		    *addr = 0;
//			addr++;
//		}
//	#endif	
//	    	
//	  //reset controller, initialize the event buffer
//		dwc_usb3_common_init(usb3_dev, USB3_BASE_OFFSET, &core_params);	
//	
//		core_int(usb3_dev);
//	
//		//enable the event interrupt, enable the EP0, set the Run/Stop bit
//	  dwc_usb3_pcd_init(usb3_dev);
//	
//		//initialize the endpont decriptor
//		dwc_usb3_pcd_endpoint_desc_init(usb3_dev);
//	
//	  //GCTL = (GCTL & 0xffffffc0) | 0x00000010;	//scale-down
//	  GCTL = (GCTL & 0xffffcfff) | 0x2000;  //confige to decice mode
//	
//		/*phy reset*/
//	  USB3_PHY_TEMP = USB3_PHY_TEMP | 0x00100000; //USB3 BIAS enable
//	}

#define USB3_P0_CTL_PLLLDOEN 28
#define USB3_P0_CTL_LDOVREFSEL_SHIFT 29
void usb3_init(void)
{
    
    dwc_usb3_device_t *usb3_dev = &_usb3_dev_;

	usb3_dev->pcd.ep0 = &ep0;
	usb3_dev->pcd.out_ep[0]= &out_ep1;
	usb3_dev->pcd.out_ep[1]= &out_ep2;
	usb3_dev->pcd.in_ep[0]= &in_ep1;
	usb3_dev->pcd.in_ep[1]= &in_ep2;

	usb3_dev->pcd.ep0_req = &ep0_req;

	/*初始化参数*/
    dwc_usb3_core_params_t core_params = {
          .burst = 0,
	      .new = 0,
	      .phy = 0,
	      .wakeup = 0,
	      .pwrctl= 0,
    
    };

	  /*USB3 reset*/
	  act_writel(act_readl(CMU_DEVRST1)& (~(1<<14)), CMU_DEVRST1);//usb3 reset
	  dwc_udelay(1);
	  act_writel(act_readl(CMU_DEVRST1)| (1<<14), CMU_DEVRST1);//clean USB3 reset

    /*PLL1 enable*/
    act_writel(act_readl(USB3_CMU_DEBUG_LDO)| (1<<20), USB3_CMU_DEBUG_LDO); //USB3 BIAS enable

	  /*PLL2 enable*/
	  act_writel(0x0340a7bf,USB3_CMU_PLL2_BISTDEBUG);
    
	  /*usb2 LDO enable*/
	  act_writel(act_readl(USB3_P0_CTL)| (1 << USB3_P0_CTL_PLLLDOEN ) | (2 << USB3_P0_CTL_LDOVREFSEL_SHIFT), USB3_P0_CTL);

	/*disable the Supper speed,*/
	DCFG = (DCFG & 0xfffffff8) | 0x0;
#if 0
	//clean the TRB memory space
	unsigned int *addr;
	unsigned int i;
	addr = (unsigned int *)0xb40d0000;
	for(i=0; i<1024*16; i++)
	{
	    *addr = 0;
		addr++;
	}
#endif	
    	
    //reset controller, initialize the event buffer
    //dwc_usb3_common_init(usb3_dev, USB3_BASE_OFFSET);	
	dwc_usb3_common_init(usb3_dev, USB3_BASE_OFFSET, &core_params);	

	core_int(usb3_dev);

	GCTL = (GCTL & 0xffffcfff) | 0x2000;  //confige to decice mode
    act_writel(act_readl(USB3_CMU_DEBUG_LDO)| (1<<20), USB3_CMU_DEBUG_LDO); //USB3 BIAS enable
	
    //dwc_usb3_pcd_endpoint_desc_init(usb3_dev);

	//enable the event interrupt, enable the EP0, set the Run/Stop bit
    dwc_usb3_pcd_init(usb3_dev);

	//initialize the endpont decriptor
	dwc_usb3_pcd_endpoint_desc_init(usb3_dev);

	/* Set the Run/Stop bit */
	DWC_MODIFY_REG32(&(usb3_dev->pcd.dev_global_regs->dctl), DWC_DCTL_RUN_STOP_BIT, DWC_DCTL_RUN_STOP_BIT);
	
}

void usb3isr(void *p_usr_data)
{
    dwc_usb3_device_t *usb3_dev = &_usb3_dev_;
    //INFO("isr in\n");

    /*如果是usb3的中断，则跳到usb3的中断处理程序*/
    dwc_usb3_irq(usb3_dev, 0);

    //INFO("isr out\n");
}

/******************************************************************************/
/*!                    
* \par  Description:
*		将数据返回给pc，即从buffer中取出，放到usb的fifo中，最终传送给pc。
* \param[in]	leng，传输数据的长度，以byte为单位
* \param[in]	addr，存放数据的源地址
* \return       0 
* \ingroup      adfudec
* \par          exmaple code
* \code 
*               actions_data_in(0x400, 0xb4043000);
* \endcode
*******************************************************************************/
void actions_data_in(uint32_t leng,uint32_t addr)
{
    /*in endpoint: in2(phy endpoint number is 5)*/
  dwc_usb3_device_t *usb3_dev = &_usb3_dev_;
  dwc_usb3_pcd_t *pcd = NULL;
  dwc_usb3_pcd_ep_t *ep = NULL;
  dwc_usb3_dev_ep_regs_t *ep_reg;
  dwc_usb3_dma_desc_t *dma_desc;
  dwc_dma_t dma_desc_dma;
  uint32_t len;
  uint8_t tri;

  pcd = &usb3_dev->pcd;
  ep = pcd->in_ep[1];

  dma_desc = (dwc_usb3_dma_desc_t *)(pcd->in_desc_addr);
  dma_desc_dma = pcd->dma_in_desc_addr;
  
  len = leng;
  if (len > DWC_DSCSTS_XFRCNT_BITS)  /* 长度超过,进行拆分 */
  {
    int packet_index = 0, packet_num = 0;
  	
    /* how many dma descs need, one per 8M */
    /* also need one if the last is lesser 8M */
    packet_num = (len + DWC_DMA_PER_MAX_LENGTH - 1)  >> DWC_DMA_PER_MAX_LEN_BIT;
    //			if (ep->dwc_ep.desc_avail < (packet_index + 1))
    //			{
    //				  printf("length is override,error!\n");
    //			}
  	
    /* dma descs index if from 0 begin, fill 0 to packet_num -2 */
    for (packet_index = 0; packet_index < (packet_num - 1); packet_index++)
    {
  	  if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
      {
        dma_desc->control |= DWC_DSCCTL_HWO_BIT;
      } 
      else 
      {
        dma_desc->control = DWC_DSCCTL_CHN_BIT | DWC_DSCCTL_ISP_BIT |
  	      (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) | DWC_DSCCTL_HWO_BIT;
      }
    
      dma_desc->status = (DWC_DMA_PER_MAX_LENGTH) << DWC_DSCSTS_XFRCNT_SHIFT;
    
      /* DMA Descriptor Setup */
      dma_desc->bptl = (addr + (packet_index<<DWC_DMA_PER_MAX_LEN_BIT)) & 0xffffffffU;
      dma_desc->bpth = 0;
    
      //ep->dwc_ep.desc_idx++;
      dma_desc++;
    }
    
    /* the last packet_num - 1 */
    dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
  	  (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
  	  DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
  	
    dma_desc->status = (len - ((packet_num - 1) << DWC_DMA_PER_MAX_LEN_BIT)) << DWC_DSCSTS_XFRCNT_SHIFT;
  	
    /* DMA Descriptor Setup */
    dma_desc->bptl = (addr + ((packet_num - 1)<<DWC_DMA_PER_MAX_LEN_BIT)) & 0xffffffffU;
    dma_desc->bpth = 0;
  }
  else   /*长度小于*/
  {
    if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
    {
      dma_desc->control |= DWC_DSCCTL_HWO_BIT;
    } 
    else 
    {
      dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
  	    (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
  	    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
    }
    
    dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;
    
    /* DMA Descriptor Setup */
    dma_desc->bptl = addr & 0xffffffffU;
    dma_desc->bpth = 0;   
  }
  
  /*
  * Start DMA on EPn-IN
  */
  ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
  	
  /**/
  if(pcd->in_ep[ep->dwc_ep.num-1]->dwc_ep.tri_in_initialize == 1)
  {
      uint32_t __attribute__((unused)) tri;
      //ep_reg = &pcd->in_ep_regs[1];
      tri = pcd->in_ep[ep->dwc_ep.num-1]->dwc_ep.tri_in;
      //dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
  }
  else
  {
    pcd->in_ep[ep->dwc_ep.num-1]->dwc_ep.tri_in_initialize = 1;
  }
  	
  /* Issue "DEPSTRTXFER" command to EP */
  tri = dwc_usb3_dep_startxfer(pcd, ep_reg, dma_desc_dma,0);
  ep->dwc_ep.tri_in = tri;
  	
  ep->dwc_ep.desc_avail--;
		
}

/******************************************************************************/
/*!                    
* \par  Description:
*		从pc接收数据，即从usb的fifo中取出，放到buffer中。
* \param[in]	leng，传输数据的长度，以byte为单位
* \param[in]	addr，存放数据的目的地址
* \return       0 
* \ingroup      adfudec
* \par          exmaple code
* \code 
*               actions_data_out(0x400, 0xb4043000);
* \endcode
*******************************************************************************/
void actions_data_out(uint32_t leng,uint32_t addr)
{
  /*out endpoint: out1(phy endpoint number is 2)*/
  dwc_usb3_device_t *usb3_dev = &_usb3_dev_;
  dwc_usb3_pcd_t *pcd = NULL;
  dwc_usb3_pcd_ep_t *ep = NULL;
  dwc_usb3_dev_ep_regs_t *ep_reg;
  dwc_usb3_dma_desc_t *dma_desc;
  dwc_dma_t dma_desc_dma;
  uint32_t len;
  uint8_t tri;
  
  pcd = &usb3_dev->pcd;
  ep = pcd->out_ep[0];

  dma_desc = (dwc_usb3_dma_desc_t *)(pcd->out_desc_addr);
  dma_desc_dma = pcd->dma_out_desc_addr;
  
  /* clear dma desc */
  if (ep->dwc_ep.type == UE_BULK)
  {
    memset(dma_desc, 0, 512);
  }

  /* Must be power of 2, use cheap AND */
  len = (leng + ep->dwc_ep.maxpacket - 1) &
      ~(ep->dwc_ep.maxpacket - 1);

  /* 判断传输长度是否超过 */
  if (len > DWC_DSCSTS_XFRCNT_BITS)  /* 长度超过,进行拆分 */
  {
  	int packet_index = 0, packet_num = 0;
  	
  	/* how many dma descs need, one per 8M */
  	/* also need one if the last is lesser 8M */
  	packet_num = (len + DWC_DMA_PER_MAX_LENGTH - 1) >> DWC_DMA_PER_MAX_LEN_BIT;
  //			if (ep->dwc_ep.desc_avail < (packet_index + 1))
  //			{
  //				  printf("length is override,error!\n");
  //			}
  	
  	/* dma descs index if from 0 begin, fill 0 to packet_num -2 */
  	for (packet_index = 0; packet_index < (packet_num - 1); packet_index++)
    {
    	if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
      {
  	    dma_desc->control |= DWC_DSCCTL_HWO_BIT;
      } 
      else 
      {
  	    dma_desc->control = DWC_DSCCTL_CHN_BIT | DWC_DSCCTL_ISP_BIT |
  		    (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) | DWC_DSCCTL_HWO_BIT |
  		    0 << DWC_DSCCTL_STRMID_SOFN_SHIFT;
      }
      
      dma_desc->status = (DWC_DMA_PER_MAX_LENGTH) << DWC_DSCSTS_XFRCNT_SHIFT;
      
      /* DMA Descriptor Setup */
      dma_desc->bptl = (addr + (packet_index<<DWC_DMA_PER_MAX_LEN_BIT)) & 0xffffffffU;
      dma_desc->bpth = 0;  
      //ep->dwc_ep.desc_idx++;
      dma_desc++;
      
    }
    
    
    /* the last packet_num - 1 */
    dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
  		(DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
  		DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT |
  		0 << DWC_DSCCTL_STRMID_SOFN_SHIFT;
  	  
    dma_desc->status = (len - ((packet_num - 1)<<DWC_DMA_PER_MAX_LEN_BIT)) << DWC_DSCSTS_XFRCNT_SHIFT;	  
    /* DMA Descriptor Setup */
    dma_desc->bptl = (addr + ((packet_num - 1)<<DWC_DMA_PER_MAX_LEN_BIT)) & 0xffffffffU;
    dma_desc->bpth = 0;
  }
  else   /*长度小于*/
  {
  	dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
  		  (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
  		  DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT |
  		  0 << DWC_DSCCTL_STRMID_SOFN_SHIFT;
    
    dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;
    
    /* DMA Descriptor Setup */
    dma_desc->bptl = addr & 0xffffffffU;
    dma_desc->bpth = 0;  
  }
  
  /*
  * Start DMA on EPn-OUT
  */
  ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];

  /**/
  if(pcd->out_ep[ep->dwc_ep.num-1]->dwc_ep.tri_out_initialize == 1)
  {
      //ep_reg = &pcd->out_ep_regs[1];
    uint32_t tri = pcd->out_ep[ep->dwc_ep.num-1]->dwc_ep.tri_out;
    dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
  }
  else
  {
    pcd->out_ep[ep->dwc_ep.num-1]->dwc_ep.tri_out_initialize = 1;
  }
  
  /* Issue "DEPSTRTXFER" command to EP */
  tri = dwc_usb3_dep_startxfer(pcd, ep_reg, dma_desc_dma,0);
  ep->dwc_ep.tri_out = tri;
  
  ep->dwc_ep.desc_avail--;
}
