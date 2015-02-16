/* ==========================================================================
 * File: cil.c
 * Revision:
 * Date: 2010/09/07
 * Change: 
 * writed by: yujing
 *
 *this file is modify from Cil.c of synopsys
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
 *
 * The Common Interface Layer provides basic services for accessing and
 * managing the DWC_usb3 hardware. These services are used by both the
 * Host Controller Driver and the Peripheral Controller Driver.
 *
 * The CIL manages the memory map for the core so that the HCD and PCD
 * don't have to do this separately. The CIL also performs basic
 * services that are not specific to either the host or device modes
 * of operation. These services include management of the OTG Host
 * Negotiation Protocol (HNP) and Session Request Protocol (SRP). A
 * Diagnostic API is also provided to allow testing of the controller
 * hardware.
 *
 * The Common Interface Layer has the following requirements:
 * - Provides basic controller operations.
 * - Minimal use of OS services.
 * - The OS services used will be abstracted by using inline functions
 *	 or macros.
 *
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


/**
 * This function allocates the memory for an Event Buffer.
 */
static uint32_t *allocate_eventbuf(dwc_usb3_device_t *dev, int size,
				   dwc_dma_t *dma_addr_ret)
{
	//return dwc_dma_alloc(&dev->os_dep.dmactx, size << 2, dma_addr_ret);
    return dwc_dma_malloc(size << 2);
}

/**
 * This function frees the memory for an Event Buffer.
 */
static void free_eventbuf(dwc_usb3_device_t *dev, uint32_t *addr,
			  int size, dwc_dma_t dma_addr)
{
	//dwc_dma_free(&dev->os_dep.dmactx, size << 2, addr, dma_addr);
    dwc_dma_mfree(addr);
}

/**
 * This function initializes an Event Buffer.
 */
static void init_eventbuf(dwc_usb3_device_t *dev, int bufno,
			  uint32_t *addr, int size, dwc_dma_t dma_addr)
{
	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventadr_lo,
			dma_addr  & 0xffffffffU);

	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventadr_hi,
			0);

	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventsiz,
			size << 2);
	dwc_read_reg32(&dev->core_global_regs->geventbuf[bufno].geventsiz);
	
	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventcnt,
			0);
}

/**
 * This function enables the Event Buffer interrupt.
 */
static void ena_eventbuf_intr(dwc_usb3_device_t *dev, int bufno)
{
	dwc_modify_reg32(&dev->core_global_regs->geventbuf[bufno].geventsiz,
			 DWC_EVENTSIZ_INT_MSK_BIT, 0);
}

/**
 * This function disables the Event Buffer interrupt.
 */
static void dis_eventbuf_intr(dwc_usb3_device_t *dev, int bufno)
{
	dwc_modify_reg32(&dev->core_global_regs->geventbuf[bufno].geventsiz,
			 DWC_EVENTSIZ_INT_MSK_BIT, DWC_EVENTSIZ_INT_MSK_BIT);
}

/**
 * This function disables the Event Buffer interrupt and flushes any pending
 * events from the buffer.
 */
static void dis_flush_eventbuf_intr(dwc_usb3_device_t *dev, int bufno)
{
	uint32_t cnt;

	dis_eventbuf_intr(dev, bufno);
	cnt = dwc_read_reg32(&dev->core_global_regs->geventbuf[bufno].geventcnt);
	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventcnt, cnt);
}

/**
 * This function reads the current Event Buffer count.
 */
static int get_eventbuf_count(dwc_usb3_device_t *dev, int bufno)
{
	uint32_t cnt;

	cnt = dwc_read_reg32(&dev->core_global_regs->geventbuf[bufno].geventcnt);
	return cnt & DWC_EVENTCNT_CNT_BITS;
}

/**
 * This function writes the Event Buffer count.
 */
static void update_eventbuf_count(dwc_usb3_device_t *dev, int bufno, int cnt)
{
	dwc_write_reg32(&dev->core_global_regs->geventbuf[bufno].geventcnt, cnt);
}

/**
 * This function fetches the next event from the Event Buffer.
 */
static uint32_t get_eventbuf_event(dwc_usb3_device_t *dev, int bufno, int size)
{
	uint32_t event;

	//event = *dev->event_ptr[bufno]++;
	//if (dev->event_ptr[bufno] >= dev->event_buf[bufno] + size)
	//	dev->event_ptr[bufno] = dev->event_buf[bufno];

   event = *dev->event_ptr[bufno];
   while(event == 0)
   {
    
       event = *dev->event_ptr[bufno];
       dwc_udelay(1);
   }
   
   *dev->event_ptr[bufno] = 0;
   dev->event_ptr[bufno]++;
   
    
	return event;
}

/**
 * This function initializes the commmon interrupts, used in both
 * Device and Host modes.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 *
 */
void dwc_usb3_enable_common_interrupts(dwc_usb3_device_t *usb3_dev)
{
	/* Clear any pending interrupts */
	dis_flush_eventbuf_intr(usb3_dev, 0);

	ena_eventbuf_intr(usb3_dev, 0);
}

/**
 * This function enables the Device mode interrupts.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 */
void dwc_usb3_enable_device_interrupts(dwc_usb3_device_t *usb3_dev)
{
	/* Enable common interrupts */
	dwc_usb3_enable_common_interrupts(usb3_dev);

	/* Enable device interrupts */
	dwc_write_reg32(&usb3_dev->pcd.dev_global_regs->devten,
			DWC_DEVTEN_DISCONN_BIT |
			DWC_DEVTEN_USBRESET_BIT |
			DWC_DEVTEN_CONNDONE_BIT |
			DWC_DEVTEN_ULST_CHNG_EN_BIT /*|
			DWC_DEVTEN_WKUP_BIT*/);

}

/**
 * This is the common interrupt handler function.
 */
int dwc_usb3_irq(dwc_usb3_device_t *usb3_dev, int irq)
{
	dwc_usb3_pcd_t *pcd = &usb3_dev->pcd;
	int retval = 0;
	int count, intr, epnum, i;
	uint32_t event;

	//dwc_spinlock(pcd->lock);
/*
	if (!usb3_dev->common_irq_installed)
		goto out;
*/
/*
#if defined(CONFIG_IPMATE) || defined(COSIM)
again:
#endif
*/
	count = get_eventbuf_count(usb3_dev, 0);
	
	if ((count & DWC_EVENTCNT_CNT_BITS) == (0xffffffff & DWC_EVENTCNT_CNT_BITS) ||
	    count >= DWC_EVENT_BUF_SIZE * 4) 
	{
		//DWC_USB3_WARN("Bad event count 0x%01x in dwc_usb3_irq() !!", count);
		dis_eventbuf_intr(usb3_dev, 0);
		update_eventbuf_count(usb3_dev, 0, count);
		count = 0;
		retval |= 1;
	}

	if (count) 
	{
		for (i = 0; i < count; i += 4) 
		{
			//DWC_USB3_DEBUG("Event addr 0x%08lx\n", (unsigned long)usb3_dev->event_ptr[0]);
			event = get_eventbuf_event(usb3_dev, 0, DWC_EVENT_BUF_SIZE);
			update_eventbuf_count(usb3_dev, 0, 4);

			if (event == 0) 
			{
				//DWC_USB3_DEBUG("## Null event! ##\n");

				/* Ignore null events */
				retval |= 1;
				continue;
			}

			if (event & DWC_EVENT_NON_EP_BIT) 
			{
				intr = event & DWC_EVENT_INTTYPE_BITS;

				if (intr == DWC_EVENT_DEV_INT << DWC_EVENT_INTTYPE_SHIFT) 
				{
					//DWC_USB3_DEBUG("## Device interrupt 0x%08x ##\n", event);
					retval |= dwc_usb3_handle_dev_intr(pcd, event);
				}
				else 
				{
					//DWC_USB3_DEBUG("## Core interrupt 0x%08x ##\n", event);

					/* @todo Handle non-Device interrupts
					 * (OTG, CarKit, I2C)
					 */
					retval |= 1;
				}
			}
			else 
			{
				epnum = (event >> DWC_DEPEVT_EPNUM_SHIFT) &
					(DWC_DEPEVT_EPNUM_BITS >> DWC_DEPEVT_EPNUM_SHIFT);
				//DWC_USB3_DEBUG("## Physical EP%d interrupt 0x%08x ##\n", epnum, event);
				//DWC_USB3_DEBUG("Logical EP%d-%s\n", (epnum >> 1) & 0xf, epnum & 1 ? "IN" : "OUT");

				/* Out EPs are even, In EPs are odd */
				if (epnum & 1)
				{
					retval |= dwc_usb3_handle_in_ep_intr(pcd, (epnum >> 1) & 0xf, event);
				} 
				else
				{
					retval |= dwc_usb3_handle_out_ep_intr(pcd, (epnum >> 1) & 0xf, event);
				}
			}
		}

/*
#if defined(CONFIG_IPMATE) || defined(COSIM)
		goto again;
#endif
*/

	}
//out:
//	dwc_spinunlock(pcd->lock);
	return retval;
}

/**
 * Initializes the DevSpd field of the DCFG register depending on the PHY type
 * and the connection speed of the device.
 */
static void init_devspd(dwc_usb3_device_t *usb3_dev)
{
	dwc_modify_reg32(&usb3_dev->pcd.dev_global_regs->dcfg,
			 DWC_DCFG_DEVADDR_BITS, 0 << DWC_DCFG_DEVADDR_SHIFT);
}

/**
 * This function calculates the number of IN EPS (excluding EP0)
 * using GHWPARAMS3 register values
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 */
static uint32_t calc_num_in_eps(dwc_usb3_device_t *usb3_dev)
{
	uint32_t num_in_eps = (usb3_dev->hwparams3 >> DWC_HWPARAMS3_NUM_IN_EPS_SHIFT) &
			(DWC_HWPARAMS3_NUM_IN_EPS_BITS >> DWC_HWPARAMS3_NUM_IN_EPS_SHIFT);

	return num_in_eps - 1;
}

/**
 * This function calculates the number of OUT EPS (excluding EP0)
 * using GHWPARAMS3 register values
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 */
static uint32_t calc_num_out_eps(dwc_usb3_device_t *usb3_dev)
{
	uint32_t num_eps = (usb3_dev->hwparams3 >> DWC_HWPARAMS3_NUM_EPS_SHIFT) &
			(DWC_HWPARAMS3_NUM_EPS_BITS >> DWC_HWPARAMS3_NUM_EPS_SHIFT);
	uint32_t num_in_eps = (usb3_dev->hwparams3 >> DWC_HWPARAMS3_NUM_IN_EPS_SHIFT) &
			(DWC_HWPARAMS3_NUM_IN_EPS_BITS >> DWC_HWPARAMS3_NUM_IN_EPS_SHIFT);

	return num_eps - num_in_eps - 1;
}

/**
 * This function is called to initialize the DWC_usb3 CSR data
 * structures. The register addresses in the device and host
 * structures are initialized from the base address supplied by the
 * caller. The calling function must make the OS calls to get the
 * base address of the DWC_usb3 controller registers. The core_params
 * argument holds the parameters that specify how the core should be
 * configured.
 *
 * @param usb3_dev    Programming view of DWC_usb3 controller.
 * @param reg_base    Base address of DWC_usb3 core registers.
 * @param core_params Pointer to the core configuration parameters.
 *
 */
int dwc_usb3_common_init(dwc_usb3_device_t *usb3_dev, uint32_t reg_base,
                                 dwc_usb3_core_params_t *core_params)
{
		uint32_t *base = (uint32_t *)(long)reg_base;
		dwc_usb3_pcd_t *pcd;
		uint32_t *addr;
	    unsigned int i;
	    	
        usb3_dev->core_params = core_params;
		usb3_dev->core_global_regs = (dwc_usb3_core_global_regs_t *)((unsigned int)base + DWC_CORE_GLOBAL_REG_OFFSET);
	
		/* Up to 32 Event Buffers are supported by the hardware, but we only use 1 */
		usb3_dev->event_buf[0] = allocate_eventbuf(usb3_dev,DWC_EVENT_BUF_SIZE,&usb3_dev->event_buf_dma[0]);
		addr = usb3_dev->event_buf[0];
	    for(i=0; i<1024*4/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }
		
		
		if (!usb3_dev->event_buf[0]) 
		{
			//DWC_USB3_DEBUG("Allocation of Event Buffer failed!\n");
			while(1);
			return 1;
		}
	
		pcd = &usb3_dev->pcd;
	
		pcd->dev_global_regs =
			(dwc_usb3_dev_global_regs_t *)((unsigned int)base + DWC_DEV_GLOBAL_REG_OFFSET);
	
		pcd->in_ep_regs =
			(dwc_usb3_dev_ep_regs_t *)((unsigned int)base + DWC_DEV_IN_EP_REG_OFFSET);
		pcd->out_ep_regs =
			(dwc_usb3_dev_ep_regs_t *)((unsigned int)base + DWC_DEV_OUT_EP_REG_OFFSET);
	
		/*
		 * Store the contents of the hardware configuration registers here for
		 * easy access later.
		 */
		usb3_dev->hwparams0 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams0);
		usb3_dev->hwparams1 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams1);
		usb3_dev->hwparams2 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams2);
		usb3_dev->hwparams3 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams3);
		usb3_dev->hwparams4 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams4);
		usb3_dev->hwparams5 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams5);
		usb3_dev->hwparams6 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams6);
		usb3_dev->hwparams7 = dwc_read_reg32(&usb3_dev->core_global_regs->ghwparams7);
	
		/* Reset the Controller */
		dwc_usb3_core_reset(usb3_dev);
		//dwc_mdelay(2);
	
		usb3_dev->dcfg = dwc_read_reg32(&pcd->dev_global_regs->dcfg);
	
		/*initialize the event_buf_dma , added by yujing*/
		usb3_dev->event_buf_dma[0] = (dwc_dma_t)(usb3_dev->event_buf[0]);	
	
		init_eventbuf(usb3_dev, 0, usb3_dev->event_buf[0],
				  DWC_EVENT_BUF_SIZE, usb3_dev->event_buf_dma[0]);
		usb3_dev->event_ptr[0] = usb3_dev->event_buf[0];
	
		return 0;
}


/**
 * This function frees the structures allocated by dwc_usb3_common_init().
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 *
 */
void dwc_usb3_common_remove(dwc_usb3_device_t *usb3_dev)
{
	dwc_usb3_pcd_t *pcd;
	uint32_t *event_buf;
	dwc_dma_t event_buf_dma;
	//dwc_spinlock_t *lock;

	pcd = &usb3_dev->pcd;
	if (pcd) 
	{
		/* Clear the Run/Stop bit */
		dwc_modify_reg32(&pcd->dev_global_regs->dctl,DWC_DCTL_RUN_STOP_BIT, 0);

		/* Disable device interrupts */
		dwc_write_reg32(&usb3_dev->pcd.dev_global_regs->devten,0);
/*
		lock = pcd->lock;
		if (lock) 
		{
			pcd->lock = NULL;
			//dwc_spinlock_free(&usb3_dev->osdep.slctx, lock);
		}
*/	
	}

	event_buf = usb3_dev->event_buf[0];
	event_buf_dma = usb3_dev->event_buf_dma[0];
	if (event_buf && event_buf_dma) 
	{
		//usb3_dev->event_buf[0] = NULL;
		usb3_dev->event_buf_dma[0] = 0;
		dis_flush_eventbuf_intr(usb3_dev, 0);
		free_eventbuf(usb3_dev, event_buf, DWC_EVENT_BUF_SIZE, event_buf_dma);
	}
}

/**
 * This function initializes the DWC_usb3 controller registers and
 * prepares the core for device mode or host mode operation.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 *
 */
void dwc_usb3_core_init(dwc_usb3_device_t *usb3_dev)
{
	//DWC_USB3_DEBUG("dwc_usb3_core_init(%p)\n", usb3_dev);

	/* Initialize parameters from Hardware configuration registers. */
	usb3_dev->pcd.num_in_eps = calc_num_in_eps(usb3_dev);
	usb3_dev->pcd.num_out_eps = calc_num_out_eps(usb3_dev);
}

/**
 * This function initializes the DWC_usb3 controller registers for
 * device mode.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 *
 */
void dwc_usb3_core_device_init(dwc_usb3_device_t *usb3_dev)
{
	//dwc_usb3_pcd_t *pcd = &usb3_dev->pcd;
	 
#ifndef VIRTIO_FIFO_INIT_BROKEN
	dwc_usb3_core_global_regs_t *global_regs = usb3_dev->core_global_regs;
#endif

	/* Device configuration register */
	init_devspd(usb3_dev);     //clean the address

#ifndef VIRTIO_FIFO_INIT_BROKEN
	/* Set up FIFOs with hard-coded sizes (temporary) */
	dwc_write_reg32(&global_regs->gtxfsiz[0],
			        (2048 / 16) << DWC_FIFOSZ_DEPTH_SHIFT |
				    0 << DWC_FIFOSZ_STARTADDR_SHIFT);

	dwc_write_reg32(&global_regs->gtxfsiz[1],
			(2048 / 16)     << DWC_FIFOSZ_DEPTH_SHIFT |
			(2048 / 16)     << DWC_FIFOSZ_STARTADDR_SHIFT);
#endif
	/* Flush the FIFOs */
	dwc_usb3_flush_tx_fifo(usb3_dev, 0x10); /* all Tx FIFOs */
	dwc_usb3_flush_rx_fifo(usb3_dev);

	/* Clear all pending Device interrupts */
	dis_flush_eventbuf_intr(usb3_dev, 0);

	dwc_usb3_enable_device_interrupts(usb3_dev);

	/* Set the Run/Stop bit */
	//dwc_modify_reg32(&pcd->dev_global_regs->dctl, DWC_DCTL_RUN_STOP_BIT, DWC_DCTL_RUN_STOP_BIT);
}



/**
 * Flush a Tx FIFO.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 * @param num      Tx FIFO to flush.
 */
void dwc_usb3_flush_tx_fifo(dwc_usb3_device_t *usb3_dev, int num)
{
	/* Wait for 3 PHY Clocks */
	dwc_udelay(1);
}

/**
 * Flush Rx FIFO.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 */
void dwc_usb3_flush_rx_fifo(dwc_usb3_device_t *usb3_dev)
{
	/* Wait for 3 PHY Clocks */
	dwc_udelay(1);
}

/**
 * Do a soft reset of the core.  Be careful with this because it
 * resets all the internal state machines of the core.
 */
void dwc_usb3_core_reset(dwc_usb3_device_t *usb3_dev)
{

	/*core soft reset*/
    GCTL = GCTL | (1<<11);
	GUSB2CFG = GUSB2CFG | (1<<31);     //set the reset bit
    dwc_udelay(1);
	GUSB2CFG = GUSB2CFG & (~(1<<31));   //clean the reset bit
	GCTL = GCTL & (~(1<<11));
	/* Soft-reset the core */
	dwc_modify_reg32(&usb3_dev->pcd.dev_global_regs->dctl,
			 DWC_DCTL_CSFT_RST_BIT, DWC_DCTL_CSFT_RST_BIT);

	usb3_dev->pcd.link_state = 0;
	usb3_dev->pcd.wkup_rdy = 0;

	/* Wait for core to come out of reset */
	do {
		dwc_udelay(1);
	} while (dwc_read_reg32(&usb3_dev->pcd.dev_global_regs->dctl) & DWC_DCTL_CSFT_RST_BIT);

	/* Wait for at least 3 PHY clocks */
	

	/*
	 * Soft reset clears the GCTL register, so we must do this last
	 */


}

/**
 * Register PCD callbacks. The callbacks are used to start and stop
 * the PCD for interrupt processing.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 * @param cb       The PCD callback structure.
 * @param p        Pointer to be passed to callback function (pcd*).
 */
void dwc_usb3_cil_register_pcd_callbacks(dwc_usb3_device_t *usb3_dev,
					 dwc_usb3_cil_callbacks_t *cb,
					 void *p)
{
	//usb3_dev->pcd_cb = cb;
	cb->p = p;
}

/**
 * Spins on register bit until handshake completes or times out.
 *
 * @param usb3_dev Programming view of DWC_usb3 controller.
 * @param ptr      Address of register to read.
 * @param mask     Bit to look at in result of read.
 * @param done     Value of the bit when handshake succeeds.
 * @param usec     Timeout in microseconds.
 * @return 1 when the mask bit has the specified value (handshake done),
 *         0 when usec has passed (handshake failed).
 */
int dwc_usb3_handshake(dwc_usb3_device_t *usb3_dev, volatile uint32_t *ptr,
		       uint32_t mask, uint32_t done, uint32_t usec)
{
	uint32_t result;

	//DWC_USB3_DEBUG("%s()\n", __func__);

	do {
		result = dwc_read_reg32(ptr);

		if ((result & mask) == done) {
			//DWC_USB3_DEBUG("after DEPCMD=%08x\n", result);
			return 1;
		}

		dwc_udelay(1);
		usec -= 1;
	} while (usec > 0);

	return 0;
}
