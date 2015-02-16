/* ==========================================================================
 * File: cil.c
 * Revision:
 * Date: 2010/09/07
 * Change: 
 * writed by: yujing
 *
 *this file is modify from pdc_intr.c of synopsys
 *
 *
 *
 *
 *
 *
 *
 *
 * ========================================================================== */

/** @file
 * This file contains the implementation of the PCD Interrupt handlers.
 *
 * The PCD handles the device interrupts. Many conditions can cause a
 * device interrupt. When an interrupt occurs, the device interrupt
 * service routine determines the cause of the interrupt and
 * dispatches handling to the appropriate function. These interrupt
 * handling functions are described below.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#include "asm/arch/usb/usb3/usb3_regs.h"
#include "asm/arch/usb/usb3/usb.h"
#include "asm/arch/usb/usb3/pcd.h"
#include "asm/arch/usb/usb3/driver.h"
#include "asm/arch/usb/usb3/cil.h"
#include "asm/arch/usb/usb3/data_type_define.h"
#include "asm/arch/usb/usb3/reg_fun.h"
#include "asm/arch/usb/usb3/usb200.h"


extern void Usb3Ep1OutDeal(void *buf);
extern void Usb3Ep2InDeal(void *buf);


/**
 * This function returns pointer to out ep struct with number ep_num
 */
static dwc_usb3_pcd_ep_t *get_out_ep(dwc_usb3_pcd_t *pcd, uint32_t ep_num)
{
	if (ep_num == 0) 
	{
		return pcd->ep0;
	} 
	else 
	{
		return pcd->out_ep[ep_num - 1];
	}
}

/**
 * This function returns pointer to in ep struct with number ep_num
 */
static dwc_usb3_pcd_ep_t *get_in_ep(dwc_usb3_pcd_t *pcd, uint32_t ep_num)
{
	if (ep_num == 0) 
	{
		return pcd->ep0;
	} 
	else
	{
		return pcd->in_ep[ep_num - 1];
	}
}

/**
 * This function gets a pointer to an EP from the wIndex address
 * value of the control request.
 */
static dwc_usb3_pcd_ep_t *get_ep_by_addr(dwc_usb3_pcd_t *pcd, uint16_t index)
{
	uint32_t ep_num = UE_GET_ADDR(index);

	if (UE_GET_DIR(index) == UE_DIR_IN) {
		return get_in_ep(pcd, ep_num);
	} else {
		return get_out_ep(pcd, ep_num);
	}
}

/**
 * This function enables EPO to receive SETUP packets.
 *
 * @param pcd	Programming view of the PCD.
 */
static void ep0_out_start(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_dma_desc_t *dma_desc;
	dwc_dma_t dma_addr;
	dwc_usb3_dev_ep_regs_t *ep_reg;
	uint8_t tri;

	dma_desc = pcd->setup_desc_addr;

	/* DMA Descriptor Setup */
	dma_desc->bptl = pcd->setup_pkt_dma & 0xffffffffU;

	dma_desc->bpth = 0;

	dma_desc->status = pcd->ep0->dwc_ep.maxpacket << DWC_DSCSTS_XFRCNT_SHIFT;
	dma_desc->control = DWC_DSCCTL_IOC_BIT | //DWC_DSCCTL_ISP_BIT |
			    (DWC_DSCCTL_TRBCTL_SETUP << DWC_DSCCTL_TRBCTL_SHIFT) |
			    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;

	/*
	 * Start DMA on EP0-OUT
	 */

	ep_reg = &pcd->out_ep_regs[0];
	dma_addr = pcd->dma_setup_desc_addr;

	/**/
	if(pcd->ep0->dwc_ep.tri_out_initialize == 1)
	{
	    ep_reg = &pcd->out_ep_regs[0];
		uint32_t tri = pcd->ep0->dwc_ep.tri_out;
        dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	}
	else
	{
        pcd->ep0->dwc_ep.tri_out_initialize = 1;
	}

	/* Issue "DEPSTRTXFER" command to EP0-OUT */
	tri = dwc_usb3_dep_startxfer(pcd, ep_reg, dma_addr, 0);
	pcd->ep0->dwc_ep.tri_out = tri;
}

/**
 * This function handles the SOF Interrupts. At this time the SOF
 * Interrupt is disabled.
 */
static int handle_sof_intr(dwc_usb3_pcd_t *pcd)
{
	return 1;
}

/**
 * This interrupt indicates that the DWC_usb3 controller has detected a
 * resume or remote wakeup sequence. If the DWC_usb3 controller is in
 * low power mode, the handler must brings the controller out of low
 * power mode. The controller automatically begins resume
 * signaling. The handler schedules a time to stop resume signaling.
 */
static int handle_wakeup_detected_intr(dwc_usb3_pcd_t *pcd)
{
	return 1;
}

/**
 * This interrupt indicates that the device has been disconnected.
 */
static int handle_disconnect_intr(dwc_usb3_pcd_t *pcd)
{
	uint32_t rd_data;

	if (pcd->usb3_dev->core_params->pwrctl)
	{
		rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
		rd_data &= ~DWC_DCTL_U1_DISABLE_BIT;
		dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
		rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
		rd_data &= ~DWC_DCTL_U2_DISABLE_BIT;
		dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
	}

	return 1;
}

/**
 * This interrupt occurs when a USB Reset is detected. When the USB
 * Reset Interrupt occurs the device state is set to DEFAULT and the
 * EP0 state is set to IDLE.
 *
 * The sequence for handling the USB Reset interrupt is shown below:
 * -# Unmask the following interrupt bits
 *	- DAINTMSK.INEP0 = 1 (Control 0 IN endpoint)
 *	- DAINTMSK.OUTEP0 = 1 (Control 0 OUT endpoint)
 *	- DEPMSK0.XferCmpl = 1
 * -# In order to transmit or receive data, it should initialize some more
 *    registers, as specified in DWC_usb3 HW spec, section Device DMA
 *    Initialization
 * -# Set up the data FIFO RAM for each of the FIFOs (only if Dynamic FIFO
 *    Sizing is enabled)
 *	- Program the GRXFSIZn Register(s), to be able to receive control OUT
 *	   data and setup data. (TODO - Describe minimum size requirements,
 *	   with and without threshold)
 *	- Program the GTXFSIZn Register(s) (depending on the FIFO number
 *	  choosen) in to be able to transmit IN data. At a minimum, this
 *	  should be equal to 1 max packet size of endpoint if threshold is
 *	  disabled or twice the threshold set if threshold enabled
 * -# Program the following fields in the endpoint specific registers for
 *    Control OUT EP 0, in order to receive a setup packet
 *	- DEPTDADR0 Register with a memory address pointing to setup descriptor
 *
 * At this point, all the required initialization, except for enabling
 * the control 0 OUT endpoint for receiving SETUP packets, is done.
 */
static int handle_usb_reset_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep;
	dwc_usb3_dev_ep_regs_t *ep_reg;
	int i;

	//DWC_USB3_PRINT("USB RESET\n");

	/* Reset Device Address */
	dwc_modify_reg32(&pcd->dev_global_regs->dcfg,
			 DWC_DCFG_DEVADDR_BITS, 0 << DWC_DCFG_DEVADDR_SHIFT);

	/* Enable EP0-OUT/IN in DALEPENA register */
	dwc_modify_reg32(&pcd->dev_global_regs->dalepena, 3, 3);

	/* Stop any active xfers on the non-EP0 IN endpoints */
	for (i = pcd->num_in_eps; i > 0; i--) 
	{
		ep = pcd->in_ep[i - 1];
		//DWC_USB3_DEBUG("DWC IN EP%d=%p tri-in=%d\n", i, ep, ep->dwc_ep.tri_in);
		if (ep->dwc_ep.tri_in) 
		{
			ep_reg = &pcd->in_ep_regs[i];
			ep->dwc_ep.condition = 0;
			dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_in,
					     &ep->dwc_ep.condition);
			ep->dwc_ep.tri_in = 0;
			ep->dwc_ep.stopped = 1;
		}

		ep->dwc_ep.desc_avail = ep->dwc_ep.num_desc;
	}

	/* Stop any active xfers on the non-EP0 OUT endpoints */
	for (i = pcd->num_out_eps; i > 0; i--)
	{
		ep = pcd->out_ep[i - 1];
		//DWC_USB3_DEBUG("DWC OUT EP%d=%p tri-out=%d\n", i, ep,ep->dwc_ep.tri_out);
		
		if (ep->dwc_ep.tri_out) {
			ep_reg = &pcd->out_ep_regs[i];
			ep->dwc_ep.condition = 0;
			dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_out,
					     &ep->dwc_ep.condition);
			ep->dwc_ep.tri_out = 0;
			ep->dwc_ep.stopped = 1;
		}

		ep->dwc_ep.desc_avail = ep->dwc_ep.num_desc;
	}

	/* Wait for Rx FIFO to drain */
	while (!(dwc_read_reg32(&pcd->dev_global_regs->dsts) & DWC_DSTS_RXFIFO_EMPTY_BIT)) 
	{
		dwc_udelay(2);
	}

	pcd->ep0state = EP0_IDLE;

	return 1;
}

/**
 * Get the device speed from the device status register and convert it
 * to USB speed constant.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
static int get_device_speed(dwc_usb3_pcd_t *pcd)
{
	uint32_t dsts;
	int speed = 0;

	dsts = dwc_read_reg32(&pcd->dev_global_regs->dsts);

	switch ((dsts >> DWC_DSTS_CONNSPD_SHIFT) &
		(DWC_DSTS_CONNSPD_BITS >> DWC_DSTS_CONNSPD_SHIFT)) {
	case DWC_SPEED_HS_PHY_30MHZ_OR_60MHZ:
		//DWC_USB3_DEBUG("HIGH SPEED\n");
		speed = USB_SPEED_HIGH;
		break;

	case DWC_SPEED_FS_PHY_30MHZ_OR_60MHZ:
	case DWC_SPEED_FS_PHY_48MHZ:
		//DWC_USB3_DEBUG("FULL SPEED\n");
		speed = USB_SPEED_FULL;
		break;

	case DWC_SPEED_LS_PHY_6MHZ:
		//DWC_USB3_DEBUG("LOW SPEED\n");
		speed = USB_SPEED_LOW;
		break;

	case DWC_SPEED_SS_PHY_125MHZ_OR_250MHZ:
		//DWC_USB3_DEBUG("SUPER SPEED\n");
		speed = USB_SPEED_SUPER;
		break;
	}

	return speed;
}

/**
 * This interrupt occurs when a Connect Done is detected.
 * Read the device status register and set the device speed in the data structure.
 * Set up EP0 to receive SETUP packets by calling dwc_ep0_activate.
 *
 * The sequence for handling the Connect Done interrupt is shown below:
 * -# On DEVT.Connection Done interrupt, read the DSTS register, to find out
 *    the speed of connection
 * -# Program max packet size field in the DEPCTL0 register. This step
 *    configures the Control EP 0. The max packet size for a control endpoint
 *    is dependent on the speed of connection
 * -# Program the DEPCTL0 register to enable the Control OUT EP 0, in order to
 *    receive a setup packet
 * -# Program DEPCFG0.USB Active Endpoint = 1
 *
 * At this point, the device is ready to receive SOF packets and is configured
 * to receive control transfers on endpoint 0.
 */
static int handle_connect_done_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	uint32_t diepcfg0, doepcfg0, diepcfg1, doepcfg1, rd_data;
	dwc_usb3_dev_ep_regs_t *ep_reg;
	int speed;
	static int once = 0;
	dwc_usb3_pcd_ep_t *ep;
	int ep_cntr, i;

	//DWC_USB3_DEBUG("CONNECT\n");

	pcd->ep0state = EP0_IDLE;
	ep0->dwc_ep.stopped = 0;
	speed = get_device_speed(pcd);
	pcd->speed = speed;
	//pcd->fops->set_speed(pcd, speed);

	diepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	diepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT | DWC_EPCFG1_EP_DIR_BIT;

	doepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	doepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT;

	/* Set the MPS of the EP based on the connection speed */
	switch (speed) 
	{
	case USB_SPEED_SUPER:
		diepcfg0 |= 512 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 512 << DWC_EPCFG0_MPS_SHIFT;
		break;

	case USB_SPEED_HIGH:
	case USB_SPEED_FULL:
		diepcfg0 |= 64 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 64 << DWC_EPCFG0_MPS_SHIFT;
		break;

	case USB_SPEED_LOW:
		diepcfg0 |= 8 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 8 << DWC_EPCFG0_MPS_SHIFT;
		break;
	}

	/*
	 * Issue "DEPCFG" command to EP0-OUT
	 */

	ep_reg = &pcd->out_ep_regs[0];
	dwc_usb3_dep_cfg(pcd, ep_reg, doepcfg0, doepcfg1);

	/*
	 * Issue "DEPCFG" command to EP0-IN
	 */

	ep_reg = &pcd->in_ep_regs[0];
	dwc_usb3_dep_cfg(pcd, ep_reg, diepcfg0, diepcfg1);

	if (!once)
	{
		once = 1;

		/* Start EP0 to receive SETUP packets */
		ep0_out_start(pcd);
	}

	if (pcd->usb3_dev->core_params->pwrctl) 
	{
		rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
		rd_data |= DWC_DCTL_U1_DISABLE_BIT;
		dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
		rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
		rd_data |= DWC_DCTL_U2_DISABLE_BIT;
		dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
	}

    if(speed == USB_SPEED_FULL )
	{
	    ep_cntr = 0;
        for (i = 1; ep_cntr < 2; i++) 
	    {
	        ep = pcd->out_ep[ep_cntr];
		    ep->dwc_ep.maxpacket = DWC_MAX_PACKET_SIZE_FS;
		    ep = pcd->in_ep[ep_cntr];
		    ep->dwc_ep.maxpacket = DWC_MAX_PACKET_SIZE_FS;
			ep_cntr++;
	    }
	}
	
	return 1;
}

/**
 * This interrupt indicates the end of the portion of the micro-frame
 * for periodic transactions. If there is a periodic transaction for
 * the next frame, load the packets into the EP periodic Tx FIFO.
 */
static int handle_end_periodic_frame_intr(dwc_usb3_pcd_t *pcd)
{
	return 1;
}

/**
 * This funcion stalls EP0.
 */
static void ep0_do_stall(dwc_usb3_pcd_t *pcd, int err_val)
{
	//usb_device_request_t *ctrl = &pcd->setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;

	ep0->dwc_ep.is_in = 0;
	dwc_usb3_ep_set_stall(pcd, ep0);
	ep0->dwc_ep.stopped = 1;
	pcd->ep0state = EP0_IDLE;
	ep0_out_start(pcd);
}

/**
 * This functions delegates the setup command to the gadget driver.
 */
static void do_gadget_setup(dwc_usb3_pcd_t *pcd, usb_device_request_t *ctrl)
{
	//int ret = 0;

}

/**
 * This function starts the Zero-Length Packet for the IN status phase
 * of a control write transfer.
 */
static void do_setup_in_status_phase(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;

	if (pcd->ep0state == EP0_STALL) 
	{
		//DWC_USB3_DEBUG("EP0 STALLED\n");
		return;
	}

	ep0->dwc_ep.is_in = 1;
	pcd->ep0state = EP0_IN_STATUS_PHASE;

	if(pcd->ep0->dwc_ep.tri_in_initialize == 1)
	{
	    dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->in_ep_regs[0];
		uint8_t tri = pcd->ep0->dwc_ep.tri_in;
        dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	}
	else
	{
        pcd->ep0->dwc_ep.tri_in_initialize = 1;
	}

	pcd->ep0_req->dwc_req.buf = pcd->setup_pkt;
	pcd->ep0_req->dwc_req.dma = pcd->setup_pkt_dma;
	pcd->ep0_req->dwc_req.length = 0;
	pcd->ep0_req->dwc_req.actual = 0;
	dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);
}

/**
 * This function starts the Zero-Length Packet for the OUT status phase
 * of a control read transfer.
 */
static void do_setup_out_status_phase(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;

	if (pcd->ep0state == EP0_STALL)
	{
		//DWC_USB3_DEBUG("EP0 STALLED\n");
		return;
	}

	ep0->dwc_ep.is_in = 0;
	pcd->ep0state = EP0_OUT_STATUS_PHASE;

	if(pcd->ep0->dwc_ep.tri_out_initialize == 1)
	{
	    dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->out_ep_regs[0];
		uint32_t tri = pcd->ep0->dwc_ep.tri_out;
        dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	}
	else
	{
        pcd->ep0->dwc_ep.tri_out_initialize = 1;
	}

	pcd->ep0_req->dwc_req.buf = pcd->setup_pkt;
	pcd->ep0_req->dwc_req.dma = pcd->setup_pkt_dma;
	pcd->ep0_req->dwc_req.length = 0;
	pcd->ep0_req->dwc_req.actual = 0;
	dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);
}

/**
 * Clear the EP halt (STALL) and if pending requests start the transfer.
 */
static void pcd_clear_halt(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	if (ep->dwc_ep.stall_clear_flag == 0)
		dwc_usb3_ep_clear_stall(pcd, ep);

	if (ep->dwc_ep.stopped) 
	{
		ep->dwc_ep.stopped = 0;

		/* If there is a request in the EP queue start it */
		ep->dwc_ep.queue_sof = 1;
		//dwc_task_schedule(pcd->start_xfer_tasklet);
	}

	/* Start Control Status Phase */
	pcd->ep0->dwc_ep.is_in = 1;
	pcd->ep0state = EP0_IN_WAIT_NRDY;
}


//#ifndef DWC_BOS_IN_GADGET
/* WUSB BOS Descriptor (Binary Object Store) */
struct bos_desc {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumDeviceCaps;
};

struct usb20_ext_cap_desc {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDevCapabilityType;
	uint32_t bmAttributes;
};

struct superspeed_cap_desc {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDevCapabilityType;
	uint8_t bmAttributes;
	uint16_t wSpeedsSupported;
	uint8_t bFunctionalitySupport;
	uint8_t bU1DevExitLat;
	uint16_t wU2DevExitLat;
} ;

struct container_id_cap_desc {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDevCapabilityType;
	uint8_t bReserved;
	uint8_t containerID[16];
} ;
//__attribute__((__packed__))


/** The BOS Descriptor */

#define USB_CAP_20_EXT	0x2
#define USB_CAP_SS	    0x3
#define USB_CAP_CID	    0x4

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
#define USB_DT_DEVICE_CAPABILITY    0x10
#define  USB_DT_SS_USB_COMPANION	0x30




#define EP0_MAXPACKETSIZE_FS 8
#define EP0_MAXPACKETSIZE_HS 64
#define EP0_MAXPACKETSIZE_SS 9      //means 512 bytes

//static uWord bcdUSB;
//static USETW(bcdUSB,UD_USB_3_0)

static usb_device_descriptor_t device_descriptor = 
{
    .bLength = USB_DEVICE_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = USETW_ACTIONS(UD_USB_2_0),
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize = EP0_MAXPACKETSIZE_HS,
	.idVendor = USETW_ACTIONS(0x10d6),  //actions
	.idProduct = USETW_ACTIONS(0x10d6),
	.bcdDevice = USETW_ACTIONS(0x0100),
	.iManufacturer = 0x00,
	.iProduct = 0x00,
	.iSerialNumber = 0,
	.bNumConfigurations = 0x01,

};
static usb_device_descriptor_t device_fs_descriptor = 
{
    .bLength = USB_DEVICE_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = USETW_ACTIONS(UD_USB_1_1),
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize = EP0_MAXPACKETSIZE_HS,
	.idVendor = USETW_ACTIONS(0x10d6),  //actions
	.idProduct = USETW_ACTIONS(0x10d6),
	.bcdDevice = USETW_ACTIONS(0x0100),
	.iManufacturer = 0x00,
	.iProduct = 0x00,
	.iSerialNumber = 0,
	.bNumConfigurations = 0x01,

};




static usb_config_descriptor_t config_descriptor =
{
    .bLength = USB_CONFIG_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_CONFIG,
	.wTotalLength = USETW_ACTIONS(USB_CONFIG_DESCRIPTOR_SIZE + \
	                              USB_INTERFACE_DESCRIPTOR_SIZE + \
	                              USB_ENDPOINT_DESCRIPTOR_SIZE*2 ), 
	.bNumInterface = 0x01,
	.bConfigurationValue = 0x01,
	.iConfiguration = 0x00,
	.bmAttributes = 0x80, //bus power
	.bMaxPower = UC_SELF_POWERED,
		
};

#define INTERFACE_CLASS_CODE_MSC     0X08
#define INTERFACE_SUBCLASS_CODE_SCSI 0x06
#define INTERFACE_PROTOCOL_BULK      0x50
#define INTERFACE_CLASS_CODE_VendorSpecific    0xff
#define INTERFACE_SUBCLASS_CODE_VendorSpecific 0xff
#define INTERFACE_PROTOCOL_VendorSpecific      0xff

static usb_interface_descriptor_t interface_descriptor =
{
    .bLength = USB_INTERFACE_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0x00,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0x02,
	.bInterfaceClass = INTERFACE_CLASS_CODE_VendorSpecific,  //adfu
	.bInterfaceSubClass = INTERFACE_SUBCLASS_CODE_VendorSpecific,
	.bInterfaceProtocol = INTERFACE_PROTOCOL_VendorSpecific,  //adfu
	.iInterface = 0,
		
};

#define MAXPACKETSIZE_FS 64
#define MAXPACKETSIZE_HS 512
#define MAXPACKETSIZE_SS 1024

usb_endpoint_descriptor_t out1_descriptor =
{
  .bLength = USB_ENDPOINT_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = 0x02,   //bit[1:0]   bulk=2   iso=1   interrupt=3
	.wMaxPacketSize = USETW_ACTIONS(MAXPACKETSIZE_HS),
		
};
static usb_endpoint_descriptor_t out1_fs_descriptor =
{
  .bLength = USB_ENDPOINT_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = 0x02,   //bit[1:0]   bulk=2   iso=1   interrupt=3
	.wMaxPacketSize = USETW_ACTIONS(MAXPACKETSIZE_FS),

};



 usb_endpoint_descriptor_t in2_descriptor =
{
    .bLength = USB_ENDPOINT_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = 0x02,   //bit[1:0]   bulk=2   iso=1   interrupt=3
	.wMaxPacketSize = USETW_ACTIONS(MAXPACKETSIZE_HS),
		
};
static usb_endpoint_descriptor_t in2_fs_descriptor =
{
    .bLength = USB_ENDPOINT_DESCRIPTOR_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = 0x02,   //bit[1:0]   bulk=2   iso=1   interrupt=3
	.wMaxPacketSize = USETW_ACTIONS(MAXPACKETSIZE_FS),
		
};


#ifndef MIN
#define MIN(__a__, __b__) ((__a__ < __b__)? __a__ : __b__)
#endif

static void do_get_descriptor(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	uint8_t *buf = pcd->status_buf;
	uint8_t dt = UGETW(ctrl.wValue) >> 8;
	uint8_t String0[4];
	uint8_t String2[33];
	uint8_t wValue_l= ctrl.wValue[0] ;
	int speed;

	speed = get_device_speed(pcd);

	switch (dt)
	{
    case UDESC_DEVICE:
        if(speed == USB_SPEED_HIGH)
        {
		    memcpy_actions(buf, (uint8_t *)(&device_descriptor), sizeof(device_descriptor));
		    pcd->ep0_req->dwc_req.length = MIN(sizeof(device_descriptor),UGETW(ctrl.wLength));
        }
		if(speed == USB_SPEED_FULL)
        {
		    memcpy_actions(buf, (uint8_t *)(&device_fs_descriptor), sizeof(device_fs_descriptor));
		    pcd->ep0_req->dwc_req.length = MIN(sizeof(device_fs_descriptor),UGETW(ctrl.wLength));
        }
		
		break;

	case UDESC_CONFIG:
		memcpy_actions(buf, (uint8_t *)(&config_descriptor), sizeof(config_descriptor));
		buf += sizeof(config_descriptor);
		memcpy_actions(buf, (uint8_t *)(&interface_descriptor), sizeof(interface_descriptor));
		buf += sizeof(interface_descriptor);

		//speed = get_device_speed(pcd);
		if(speed == USB_SPEED_HIGH )
		{
		    memcpy_actions(buf, (uint8_t *)(&out1_descriptor), sizeof(out1_descriptor));
		    buf += sizeof(out1_descriptor);
 		    memcpy_actions(buf, (uint8_t *)(&in2_descriptor), sizeof(in2_descriptor));
		    buf += sizeof(in2_descriptor);
		}

		if(speed == USB_SPEED_FULL )
		{
		    memcpy_actions(buf, (uint8_t *)(&out1_fs_descriptor), sizeof(out1_fs_descriptor));
		    buf += sizeof(out1_descriptor);
 		    memcpy_actions(buf, (uint8_t *)(&in2_fs_descriptor), sizeof(in2_fs_descriptor));
		    buf += sizeof(in2_descriptor);
		}
	    		
		pcd->ep0_req->dwc_req.length = MIN(UGETW(config_descriptor.wTotalLength),UGETW(ctrl.wLength));
		break;

	case UDESC_STRING:

        /* for string0 descriptor */
        /* LANGID code array, US english */
        String0[0] = 0x04;
        String0[1] = USB_STRING_DESCRIPTOR_TYPE;
        String0[2] = 0x09;         // LANGID is English
        String0[3] = 0x04;              
        /* for string2 descriptor */
        String2[0]  = 0x21;
        String2[1]  = USB_STRING_DESCRIPTOR_TYPE;
        String2[2]  = 'A' ;
        String2[3]  = 0x00;
        String2[4]  = 'c' ;
        String2[5]  = 0x00;
        String2[6]  = 't' ;
        String2[7]  = 0x00;
        String2[8]  = 'i' ;
        String2[9]  = 0x00;
        String2[10] = 'o' ;
        String2[11] = 0x00;
        String2[12] = 'n' ;
        String2[13] = 0x00;
        String2[14] = 's' ;
        String2[15] = 0x00;
        String2[16] = ' ' ;
        String2[17] = 0x00;
        String2[18] = 'U' ;
        String2[19] = 0x00;
        String2[20] = 'S' ;
        String2[21] = 0x00;
        String2[22] = 'B' ;
        String2[23] = 0x00;
        String2[24] = ' ' ;
        String2[25] = 0x00;
        String2[26] = 'O' ;
        String2[27] = 0x00;
        String2[28] = 'T' ;
        String2[29] = 0x00;
        String2[30] = 'G' ;
        String2[31] = 0x00;
        String2[32] = ' ' ;

		if(wValue_l == 0)
		{
		    memcpy_actions(buf, (uint8_t *)String0, sizeof(String0));
		    pcd->ep0_req->dwc_req.length = MIN(sizeof(String0),UGETW(ctrl.wLength));
		    //pcd->ep0_req->dwc_req.length = MIN(4,UGETW(ctrl.wLength));
		}
		else if(wValue_l == 2)
		{
		    memcpy_actions(buf, (uint8_t *)String2, sizeof(String2));
			pcd->ep0_req->dwc_req.length = MIN(sizeof(String2),UGETW(ctrl.wLength));
		    //pcd->ep0_req->dwc_req.length = MIN(33,UGETW(ctrl.wLength));
		}
		else
		{
		    dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->out_ep_regs[0];
		    dwc_usb3_dep_sstall(pcd,ep_reg);
			//ep_reg = &pcd->in_ep_regs[0];
			//dwc_usb3_dep_sstall(pcd,ep_reg); 
		    //while(1); //send stall command
		    //pcd->ep0state = EP0_IN_STATUS_PHASE;
		    pcd->ep0state = EP0_STALL;
		    return;
		}

				
		break;

	case UDESC_OTG:
		break;
		
	default:
		break;

		
	}

	pcd->ep0_status_pending = 1;
	pcd->ep0_req->dwc_req.buf = pcd->status_buf;
	pcd->ep0_req->dwc_req.dma = pcd->status_buf_dma;
	pcd->ep0_req->dwc_req.actual = 0;
	dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);

	pcd->ep0state = EP0_IN_DATA_PHASE;    //added by yujing
	
}
//#endif

/**
 * This function processes the GET_STATUS Setup Commands.
 */
static void do_get_status(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;
	uint8_t *status = pcd->status_buf;
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	dwc_usb3_pcd_ep_t *ep;
	uint32_t rd_data;


	if (UGETW(ctrl.wLength) != 2) 
	{
		ep0_do_stall(pcd, 0);
		return;
	}

	switch (UT_GET_RECIPIENT(ctrl.bmRequestType))
	{
	case UT_DEVICE:
		//*status = 1; /* Self powered */
		*status = 0; 

		if (pcd->speed == USB_SPEED_SUPER) 
		{
			rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
			if (rd_data & DWC_DCTL_U1_ENABLE_BIT) 
			{
				*status |= 1 << 2;
			}

			if (rd_data & DWC_DCTL_U2_ENABLE_BIT)
			{
				*status |= 1 << 3;
			}
		}
		else
		{
			*status |= pcd->remote_wakeup_enable << 1;
		}

		//DWC_USB3_DEBUG("GET_STATUS(Device)=%02x\n", *status);
		*(status + 1) = 0;
		break;

	case UT_INTERFACE:
		*status = 0;
		if (pcd->usb3_dev->core_params->wakeup)
			*status |= 1;
		*status |= pcd->remote_wakeup_enable << 1;
		
		*(status + 1) = 0;
		break;

	case UT_ENDPOINT:
		ep = get_ep_by_addr(pcd, UGETW(ctrl.wIndex));

		/* @todo check for EP stall */
		*status = ep->dwc_ep.stopped;
		
		*(status + 1) = 0;
		break;

	default:
		ep0_do_stall(pcd, 0);
		return;
	}

	pcd->ep0_status_pending = 1;
	pcd->ep0_req->dwc_req.buf = status;
	pcd->ep0_req->dwc_req.dma = pcd->status_buf_dma;
	pcd->ep0_req->dwc_req.length = 2;
	pcd->ep0_req->dwc_req.actual = 0;
	dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);

	pcd->ep0state = EP0_IN_DATA_PHASE;    //added by yujing
	
}

/**
 * This function processes the SET_FEATURE Setup Commands.
 */
static void do_set_feature(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep;
	uint32_t rd_data;

	switch (UT_GET_RECIPIENT(ctrl.bmRequestType)) 
	{
	case UT_DEVICE:
		switch (UGETW(ctrl.wValue)) 
		{
		case UF_DEVICE_REMOTE_WAKEUP:
			pcd->remote_wakeup_enable = 1;
			break;

		case UF_TEST_MODE:
			/* Setup the Test Mode tasklet to do the Test
			 * Packet generation after the SETUP Status
			 * phase has completed. */

			/* @todo This has not been tested since the
			 * tasklet struct was put into the PCD
			 * struct! */
			pcd->test_mode = UGETW(ctrl.wIndex) >> 8;
			//dwc_task_schedule(pcd->test_mode_tasklet);
			break;

		case UF_DEVICE_B_HNP_ENABLE:
			break;

		case UF_DEVICE_A_HNP_SUPPORT:
			/* RH port supports HNP */
			break;

		case UF_DEVICE_A_ALT_HNP_SUPPORT:
			/* other RH port does */
			break;

		case UF_U1_ENABLE:
			rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
			rd_data |= DWC_DCTL_U1_ENABLE_BIT;
			dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
			break;

		case UF_U2_ENABLE:
			rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
			rd_data |= DWC_DCTL_U2_ENABLE_BIT;
			dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
			break;

		default:
			ep0_do_stall(pcd, 0);
			return;
		}

		break;

	case UT_INTERFACE:
		/* if FUNCTION_SUSPEND ... */
		if (UGETW(ctrl.wValue) == 0) 
		{
			/* if Function Remote Wake Enabled ... */
			if ((UGETW(ctrl.wIndex) >> 8) & 2) 
			{
				pcd->remote_wakeup_enable = 1;
			}
			else
			{
				pcd->remote_wakeup_enable = 0;
			}

			/* if Function Low Power Suspend ... */
			// TODO

			break;
		}

		do_gadget_setup(pcd, &ctrl);
		break;

	case UT_ENDPOINT:
		ep = get_ep_by_addr(pcd, UGETW(ctrl.wIndex));
		if (UGETW(ctrl.wValue) != UF_ENDPOINT_HALT)
		{
			ep0_do_stall(pcd, 0);
			return;
		}

		ep->dwc_ep.stopped = 1;
		dwc_usb3_ep_set_stall(pcd, ep);
		break;
	}

	pcd->ep0->dwc_ep.is_in = 1;
	pcd->ep0state = EP0_IN_WAIT_NRDY;
}

/**
 * This function processes the CLEAR_FEATURE Setup Commands.
 */
static void do_clear_feature(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep;
	uint32_t rd_data;

	switch (UT_GET_RECIPIENT(ctrl.bmRequestType))
	{
	case UT_DEVICE:
		switch (UGETW(ctrl.wValue)) 
		{
		case UF_DEVICE_REMOTE_WAKEUP:
			pcd->remote_wakeup_enable = 0;
			break;

		case UF_TEST_MODE:
			/* @todo Add CLEAR_FEATURE for TEST modes. */
			break;

		case UF_U1_ENABLE:
			rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
			rd_data &= ~DWC_DCTL_U1_ENABLE_BIT;
			dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
			break;

		case UF_U2_ENABLE:
			rd_data = dwc_read_reg32(&pcd->dev_global_regs->dctl);
			rd_data &= ~DWC_DCTL_U2_ENABLE_BIT;
			dwc_write_reg32(&pcd->dev_global_regs->dctl, rd_data);
			break;

		default:
			ep0_do_stall(pcd, 0);
			return;
		}

		break;

	case UT_INTERFACE:
		/* if FUNCTION_SUSPEND ... */
		if (UGETW(ctrl.wValue) == 0) 
		{
			/* if Function Remote Wake Enabled ... */
			if ((UGETW(ctrl.wIndex) >> 8) & 2)
			{
				pcd->remote_wakeup_enable = 0;
			}

			/* if Function Low Power Suspend ... */
			// TODO

			break;
		}

		ep0_do_stall(pcd, 0);
		return;

	case UT_ENDPOINT:
		ep = get_ep_by_addr(pcd, UGETW(ctrl.wIndex));
		if (UGETW(ctrl.wValue) != UF_ENDPOINT_HALT) 
		{
			ep0_do_stall(pcd, 0);
			return;
		}

		pcd_clear_halt(pcd, ep);
		break;
	}

	pcd->ep0->dwc_ep.is_in = 1;
	pcd->ep0state = EP0_IN_WAIT_NRDY;
}

/**
 * This function processes the SET_ADDRESS Setup Commands.
 */
static void do_set_address(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;

	if (ctrl.bmRequestType == UT_DEVICE) {

		dwc_modify_reg32(&pcd->dev_global_regs->dcfg,DWC_DCFG_DEVADDR_BITS,
				 UGETW(ctrl.wValue) << DWC_DCFG_DEVADDR_SHIFT);
		pcd->ep0->dwc_ep.is_in = 1;
		pcd->ep0state = EP0_IN_WAIT_NRDY;
	}
}


/**
 * This function processes the SET_CONFIG Setup Commands.
 */
extern int dwc_usb3_pcd_ep_enable(dwc_usb3_pcd_t *pcd, const void *ep_desc,
			   dwc_usb3_pcd_ep_t *ep, int maxvalues); 
extern void actions_data_out(uint32_t data_leng,uint32_t addr);
static void do_set_configuration(dwc_usb3_pcd_t *pcd)
{
    //dwc_usb3_pcd_ep_t *ep;
	//usb_device_request_t ctrl = pcd->setup_pkt->req;

	dwc_usb3_pcd_ep_enable(pcd, pcd->out_ep[0]->dwc_ep.desc,pcd->out_ep[0], 0);
	dwc_usb3_pcd_ep_enable(pcd, pcd->in_ep[1]->dwc_ep.desc,pcd->in_ep[1], 0);

	pcd->ep0->dwc_ep.is_in = 1;
	pcd->ep0state = EP0_IN_WAIT_NRDY;

	//ep = get_out_ep(pcd, 1);  //out 1
		
	//prepare the CBW TRB
	actions_data_out(512, (unsigned int)(pcd->ep_out_data_addr_temp));
}

static void do_usb_class_setup(dwc_usb3_pcd_t *pcd, usb_device_request_t *ctrl)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	dwc_usb3_dev_ep_regs_t *ep_reg;
	
  switch (ctrl->bRequest) 
	{
  default:
	    //UsbOtgEp0Stall(1);
	    ep_reg = &pcd->out_ep_regs[0];
	    dwc_usb3_dep_sstall(pcd, ep_reg);
	    break;
  }

  pcd->ep0_status_pending = 1;
	pcd->ep0_req->dwc_req.buf = pcd->status_buf;
	pcd->ep0_req->dwc_req.dma = pcd->status_buf_dma;
	pcd->ep0_req->dwc_req.actual = 0;
	dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);
	
}

/**
 * This function processes SETUP commands. In Linux, the USB Command
 * processing is done in two places - the first being the PCD and the
 * second being the Gadget Driver (for example, the File-Backed Storage
 * Gadget Driver).
 *
 * <table>
 * <tr><td> Command </td><td> Driver </td><td> Description </td></tr>
 *
 * <tr><td> GET_STATUS </td><td> PCD </td><td> Command is processed
 * as defined in chapter 9 of the USB 2.0 Specification. </td></tr>
 *
 * <tr><td> SET_FEATURE </td><td> PCD / Gadget Driver </td><td> Device
 * and Endpoint requests are processed by the PCD. Interface requests
 * are passed to the Gadget Driver. </td></tr>
 *
 * <tr><td> CLEAR_FEATURE </td><td> PCD </td><td> Device and Endpoint
 * requests are processed by the PCD. Interface requests are ignored.
 * The only Endpoint feature handled is ENDPOINT_HALT. </td></tr>
 *
 * <tr><td> SET_ADDRESS </td><td> PCD </td><td> Program the DCFG register
 * with device address received. </td></tr>
 *
 * <tr><td> GET_DESCRIPTOR </td><td> Gadget Driver </td><td> Return the
 * requested descriptor. </td></tr>
 *
 * <tr><td> SET_DESCRIPTOR </td><td> Gadget Driver </td><td> Optional -
 * not implemented by any of the existing Gadget Drivers. </td></tr>
 *
 * <tr><td> GET_CONFIGURATION </td><td> Gadget Driver </td><td> Return
 * the current configuration. </td></tr>
 *
 * <tr><td> SET_CONFIGURATION </td><td> Gadget Driver </td><td> Disable
 * all EPs and enable EPs for new configuration. </td></tr>
 *
 * <tr><td> GET_INTERFACE </td><td> Gadget Driver </td><td> Return the
 * current interface. </td></tr>
 *
 * <tr><td> SET_INTERFACE </td><td> Gadget Driver </td><td> Disable all
 * EPs and enable EPs for new interface. </td></tr>
 * </table>
 *
 * When the SETUP Phase Done interrupt occurs, the PCD SETUP commands are
 * processed by pcd_setup. Calling the Function Driver's setup function from
 * pcd_setup processes the gadget SETUP commands.
 */
static void pcd_setup(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;

	/* Clean up the request queue */
	//dwc_usb3_request_nuke(ep0);
	ep0->dwc_ep.stopped = 0;
	ep0->dwc_ep.three_stage = 1;

	if (ctrl.bmRequestType & UE_DIR_IN) 
	{
		ep0->dwc_ep.is_in = 1;
		pcd->ep0state = EP0_IN_DATA_PHASE;
	} 
	else
	{
		ep0->dwc_ep.is_in = 0;
		pcd->ep0state = EP0_OUT_DATA_PHASE;
	}

	if (UGETW(ctrl.wLength) == 0) 
	{
		ep0->dwc_ep.is_in = 1;
		pcd->ep0state = EP0_IN_WAIT_GADGET;
		ep0->dwc_ep.three_stage = 0;
	}

	if ((UT_GET_TYPE(ctrl.bmRequestType)) != UT_STANDARD) 
	{
	    if((UT_GET_TYPE(ctrl.bmRequestType)) == USB_REQUEST_BMREQUESTTYPE_TPYE_CLASS)
	    {
	        if(pcd->ep0->dwc_ep.tri_in_initialize == 1)
	        {
	            dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->in_ep_regs[0];
		        uint8_t tri = pcd->ep0->dwc_ep.tri_in;
                dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	        }
	        else
	        {
                pcd->ep0->dwc_ep.tri_in_initialize = 1;
	        }
	        do_usb_class_setup(pcd, &ctrl);
			pcd->ep0state = EP0_IN_DATA_PHASE;    //added by yujing
	    }
		
		/* handle non-standard (class/vendor) requests in the gadget driver */
		do_gadget_setup(pcd, &ctrl);
		return;
	}
    else 
    {
//// --- Standard Request handling --- ////

	switch (ctrl.bRequest) 
	{
	case UR_GET_STATUS:
		if(pcd->ep0->dwc_ep.tri_in_initialize == 1)
	    {
	        dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->in_ep_regs[0];
		    uint8_t tri = pcd->ep0->dwc_ep.tri_in;
            dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	    }
	    else
	    {
            pcd->ep0->dwc_ep.tri_in_initialize = 1;
	    }
	
		do_get_status(pcd);
		break;

	case UR_CLEAR_FEATURE:
		do_clear_feature(pcd);
		break;

	case UR_SET_FEATURE:
		do_set_feature(pcd);
		break;

	case UR_SET_ADDRESS:
		do_set_address(pcd);
		break;

	case UR_SET_INTERFACE:
		//DWC_USB3_DEBUG("USB_REQ_SET_INTERFACE\n");
		break;
		
		/* FALL-THRU */
	case UR_SET_CONFIG:
		//do_gadget_setup(pcd, &ctrl);
		do_set_configuration(pcd);
		break;

	case UR_SYNCH_FRAME:
		//do_gadget_setup(pcd, &ctrl);
		break;

	case UR_GET_DESCRIPTOR:
        if(pcd->ep0->dwc_ep.tri_in_initialize == 1)
	    {
	        dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->in_ep_regs[0];
		    uint8_t tri = pcd->ep0->dwc_ep.tri_in;
            dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	    }
	    else
	    {
            pcd->ep0->dwc_ep.tri_in_initialize = 1;
	    }
		
        do_get_descriptor(pcd);
		//pcd->ep0state = EP0_IN_DATA_PHASE;    //added by yujing
/*			
#ifndef DWC_BOS_IN_GADGET
		do_get_descriptor(pcd);
#else
		do_gadget_setup(pcd, &ctrl);
#endif
*/
		break;

	case 0x30:	/* Set_SEL */
	    {

		if(pcd->ep0->dwc_ep.tri_in_initialize == 1)
	    {
	        dwc_usb3_dev_ep_regs_t *ep_reg = &pcd->in_ep_regs[0];
		    uint8_t tri = pcd->ep0->dwc_ep.tri_in;
            dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
	    }
	    else
	    {
            pcd->ep0->dwc_ep.tri_in_initialize = 1;
	    }
		
		pcd->ep0_status_pending = 1;
		pcd->ep0_req->dwc_req.buf = pcd->status_buf;
		pcd->ep0_req->dwc_req.dma = pcd->status_buf_dma;
		pcd->ep0_req->dwc_req.length = CTRL_BUF_SIZE;
		pcd->ep0_req->dwc_req.actual = 0;
		ep0->dwc_ep.send_zlp = 0;
		dwc_usb3_ep0_start_transfer(pcd, ep0, pcd->ep0_req);
		break;
	    }

	default:
		/* Call the Gadget Driver's setup functions */
		do_gadget_setup(pcd, &ctrl);
		while(1);  //stop here
		break;
	}
	
    }
	
}

/**
 * This function completes the ep0 control transfer.
 */
//static int ep0_complete_request(dwc_usb3_pcd_ep_t *ep, int status)
//{}

/**
 * This function handles EP0 Control transfers.
 *
 * The state of the control tranfers are tracked in
 * <code>ep0state</code>.
 */
static void handle_ep0(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	//dwc_usb3_pcd_request_t *req;
	//dwc_usb3_dma_desc_t *desc;
	//uint32_t byte_count;
	//int status;
	//dwc_usb3_dev_ep_regs_t *ep_reg;

	usb_device_request_t ctrl = pcd->setup_pkt->req;
		

	switch (pcd->ep0state) 
	{
	case EP0_UNCONNECTED:
		break;

	case EP0_IDLE:
		pcd->request_config = 0;
		pcd_setup(pcd);
		break;

	case EP0_IN_DATA_PHASE:
		//usb_device_request_t ctrl = pcd->setup_pkt->req;
		switch (ctrl.bRequest) 
		{
		    case UR_GET_DESCRIPTOR:
				//do_get_descriptor(pcd);
				//pcd->ep0state = EP0_IN_WAIT_NRDY;
				pcd->ep0state = EP0_OUT_WAIT_NRDY;    //modify 20110321
				break;
				
				
        case UR_GET_STATUS:
				pcd->ep0state = EP0_OUT_WAIT_NRDY;    //modify 20111219
				break;
				
			default:
				break;
		}

		break;

	case EP0_OUT_DATA_PHASE:
		/*here ........*/

		break;

	case EP0_IN_WAIT_GADGET:
		pcd->ep0state = EP0_IN_WAIT_NRDY;
		break;

	case EP0_OUT_WAIT_GADGET:
		pcd->ep0state = EP0_OUT_WAIT_NRDY;
		break;

	case EP0_IN_WAIT_NRDY:
        do_setup_in_status_phase(pcd);
		break;
		
	case EP0_OUT_WAIT_NRDY:
		do_setup_out_status_phase(pcd);
/*		
		if (ep0->dwc_ep.is_in)
		{
			do_setup_in_status_phase(pcd);
		}
		else 
		{
			do_setup_out_status_phase(pcd);
		}
*/
		break;

	case EP0_IN_STATUS_PHASE:
	case EP0_OUT_STATUS_PHASE:

		//ep0_complete_request(ep0, 0);
		
		pcd->ep0state = EP0_IDLE;
		ep0->dwc_ep.stopped = 0;
		ep0->dwc_ep.is_in = 0;	/* OUT for next SETUP */

		/* Prepare for more SETUP Packets */
        //ep_reg = &pcd->out_ep_regs[0];
		//uint8_t tri = pcd->ep0->dwc_ep.tri_out;
        //dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer

		//ep_reg = &pcd->in_ep_regs[0];
		//tri = pcd->ep0->dwc_ep.tri_in;
        //dwc_usb3_dep_updatexfer(pcd,ep_reg,tri);    //updata the transfer
		
		ep0_out_start(pcd);

		break;

	case EP0_STALL:
		//DWC_USB3_ERROR("EP0 STALLed, should not get here\n");
		pcd->ep0state = EP0_IDLE;
		ep0->dwc_ep.stopped = 0;
		ep0->dwc_ep.is_in = 0;	/* OUT for next SETUP */
		
		//ep_reg = &pcd->out_ep_regs[0];
		//dwc_usb3_dep_cstall(pcd, ep_reg);

		ep0_out_start(pcd);
		
		break;
	}

}



/**
 * This interrupt indicates that an IN EP has a pending Interrupt.
 *
 * The sequence for handling the IN EP interrupt is shown below:
 * -# If epint == "Transfer Complete" call the request complete function
 * -# If epint == "DMA/Descriptor Not Ready" ???
 * -# If epint == "FIFO Overrun/Underrun" log error
 */
int dwc_usb3_handle_in_ep_intr(dwc_usb3_pcd_t *pcd, int epnum, uint32_t epint)
{
	dwc_usb3_pcd_ep_t *ep;

	/* Get EP pointer */
	ep = get_in_ep(pcd, epnum);

	switch (epint & DWC_DEPEVT_INTTYPE_BITS) 
	{
	case DWC_DEPEVT_XFER_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		if (ep->dwc_ep.type != UE_ISOCHRONOUS)
		{
			/* Complete the transfer */
			if (epnum == 0) 
			{
				handle_ep0(pcd);
			} 
			else 
			{
				//ep_complete_request(ep, epint);
				//Usb3EpIn2DealWith(pcd,ep,FLAG_XFERCMP);
				Usb3Ep2InDeal(pcd->ep_out_data_addr_temp);
			}
		} 
		else
		{
			//DWC_USB3_WARN("xfer complete for ISOC EP!\n");
		}

		break;

	case DWC_DEPEVT_XFER_IN_PROG << DWC_DEPEVT_INTTYPE_SHIFT:
		if (ep->dwc_ep.type == UE_ISOCHRONOUS)
		{
			/* Complete the transfer */
			//ep_complete_request(ep, epint);
		} 
		else
		{
			//DWC_USB3_WARN("xfer in progress for non-ISOC EP!\n");
		}

		break;

	case DWC_DEPEVT_XFER_NRDY << DWC_DEPEVT_INTTYPE_SHIFT:
		if (epnum == 0) 
		{
			switch (pcd->ep0state) 
			{
			case EP0_IN_WAIT_NRDY:
			case EP0_IN_WAIT_GADGET:
				handle_ep0(pcd);
				return 1;
			default:
				break;
			}
		} 
		else if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
		{
		    /*
			if (!ep->dwc_ep.isoc_started) 
			{
				isoc_ep_start(pcd, ep, epint);
				ep->dwc_ep.isoc_started = 1;
			}
                    */
			break;
		}
		else
		{
		    /*send 16k data directly for simulation*/
		    //actions_sim_in_data_16k(pcd,ep);
			//UsbOtgEpIn1DealWith(pcd,ep,FLAG_NRDY);
		}

		break;

	case DWC_DEPEVT_FIFOXRUN << DWC_DEPEVT_INTTYPE_SHIFT:
		//DWC_USB3_ERROR("EP%d IN FIFO Underrun Error!\n", epnum);
		break;

	case DWC_DEPEVT_EPCMD_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		//DWC_USB3_DEBUG("EP%d Command Complete\n", epnum);
		if (epnum == 0) 
		{
			switch (pcd->ep0state) 
			{
			case EP0_STALL:
				handle_ep0(pcd);
				return 1;
			default:
				break;
			}
		} 
		break;
	}

	return 1;
}

/**
 * This interrupt indicates that an OUT EP has a pending Interrupt.
 *
 * The sequence for handling the OUT EP interrupt is shown below:
 * -# If epint == "Transfer Complete" call the request complete function
 * -# If epint == "DMA/Descriptor Not Ready" ???
 * -# If epint == "FIFO Overrun/Underrun" log error
 */
int dwc_usb3_handle_out_ep_intr(dwc_usb3_pcd_t *pcd, int epnum, uint32_t epint)
{
	dwc_usb3_pcd_ep_t *ep;

	/* Get EP pointer */
	ep = get_out_ep(pcd, epnum);

	switch (epint & DWC_DEPEVT_INTTYPE_BITS) 
	{
	case DWC_DEPEVT_XFER_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		if (ep->dwc_ep.type != UE_ISOCHRONOUS) 
		{
			/* Complete the transfer */
			if (epnum == 0) 
			{
				handle_ep0(pcd);
			}
			else
			{
				//ep_complete_request(ep, epint);
				//Usb3EpOut1DealWith(pcd,ep,FLAG_XFERCMP);
				Usb3Ep1OutDeal(pcd->ep_out_data_addr_temp);
			}
		} 
		else 
		{
			//DWC_USB3_WARN("xfer complete for ISOC EP!\n");
		}

		break;

	case DWC_DEPEVT_XFER_IN_PROG << DWC_DEPEVT_INTTYPE_SHIFT:
		if (ep->dwc_ep.type == UE_ISOCHRONOUS)
		{
			/* Complete the transfer */
			//ep_complete_request(ep, epint);
		} 
		else 
		{
			//DWC_USB3_WARN("xfer in progress for non-ISOC EP!\n");
		}

		break;

	case DWC_DEPEVT_XFER_NRDY << DWC_DEPEVT_INTTYPE_SHIFT:
		if (epnum == 0)
		{
			switch (pcd->ep0state)
			{
			case EP0_OUT_WAIT_NRDY:
				handle_ep0(pcd);
				return 1;
			default:
				break;
			}
		}
		else if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
		{
		    /*
			if (!ep->dwc_ep.isoc_started) 
			{
				isoc_ep_start(pcd, ep, epint);
				ep->dwc_ep.isoc_started = 1;
			}
                    */
			break;
		}
		else         //receive 16k bytes data for simulation directly
		{
		    //actions_sim_out_data_16k(pcd,ep);
			//UsbOtgEpOut1DealWith(pcd,ep,FLAG_NRDY);
			
		}

		break;

	case DWC_DEPEVT_FIFOXRUN << DWC_DEPEVT_INTTYPE_SHIFT:
		//DWC_USB3_ERROR("EP%d OUT FIFO Overrun Error!\n", epnum);
		break;

	case DWC_DEPEVT_EPCMD_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		//DWC_USB3_DEBUG("EP%d Command Complete\n", epnum);
		if (epnum == 0) 
		{
			switch (pcd->ep0state) 
			{
			case EP0_STALL:
				handle_ep0(pcd);
				return 1;
			default:
				break;
			}
		} 
		break;
	}

	return 1;
}

/**
 * PCD interrupt handler.
 *
 * The PCD handles the device interrupts. Many conditions can cause a
 * device interrupt. When an interrupt occurs, the device interrupt
 * service routine determines the cause of the interrupt and
 * dispatches handling to the appropriate function.
 */
int dwc_usb3_handle_dev_intr(dwc_usb3_pcd_t *pcd, uint32_t dint)
{
	uint32_t status, state;
	int32_t retval = 0;
	uint32_t event = (dint >> DWC_DEVT_SHIFT) & (DWC_DEVT_BITS >> DWC_DEVT_SHIFT);

	switch (event) {
	case DWC_DEVT_DISCONN:
		retval |= handle_disconnect_intr(pcd);
		break;

	case DWC_DEVT_USBRESET:
		retval |= handle_usb_reset_intr(pcd);
		break;

	case DWC_DEVT_CONNDONE:
		retval |= handle_connect_done_intr(pcd);
		break;

	case DWC_DEVT_WKUP:
		retval |= handle_wakeup_detected_intr(pcd);
		break;

	case DWC_DEVT_ULST_CHNG:
		//DWC_USB3_DEBUG("Link Status Change\n");

		status = dwc_read_reg32(&pcd->dev_global_regs->dsts);
		state = (status >> DWC_DSTS_USBLNK_STATE_SHIFT) &
			(DWC_DSTS_USBLNK_STATE_BITS >> DWC_DSTS_USBLNK_STATE_SHIFT);
		
		switch (state)
		{
		case DWC_LINK_STATE_ON:
			/* If transitioning from 3->0 */
			if (pcd->link_state == DWC_LINK_STATE_SUSPEND)
			{
				//DWC_USB3_DEBUG("Enabling function remote wake\n");
				pcd->wkup_rdy = 1;
			}
			else 
			{
				pcd->wkup_rdy = 0;
			}

			pcd->link_state = state;
			break;

		default:
			pcd->link_state = state;
			pcd->wkup_rdy = 0;
		}

		retval |= 1;
		break;

	case DWC_DEVT_EOPF:
		retval |= handle_end_periodic_frame_intr(pcd);
		break;

	case DWC_DEVT_SOF:
		retval |= handle_sof_intr(pcd);
		break;

	case DWC_DEVT_ERRATICERR:
		//DWC_USB3_DEBUG("Erratic Error\n");
		break;

	case DWC_DEVT_CMD_CMPL:
		//DWC_USB3_DEBUG("Command Complete\n");
		break;

	case DWC_DEVT_OVERFLOW:
		//DWC_USB3_DEBUG("Overflow\n");
		break;
	}

	return retval;
}
