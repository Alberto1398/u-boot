/* ==========================================================================
 * File: pcd.c
 * Revision: 
 * Date: 2010/09/07 
 * Change: 
 * writed by: yujing
 *
 *
 *this file is modify from Pcd.c of synopsys
 *
 *
 *
** * ========================================================================== */

/** @file
 * This file implements the Peripheral Controller Driver.
 *
 * The Peripheral Controller Driver (PCD) is responsible for
 * translating requests from the Function Driver into the appropriate
 * actions on the DWC_usb3 controller. It isolates the Function Driver
 * from the specifics of the controller by providing an API to the
 * Function Driver.
 *
 * The Peripheral Controller Driver for Linux will implement the
 * Gadget API, so that the existing Gadget drivers can be used.
 * (Gadget Driver is the Linux terminology for a Function Driver.)
 *
 * The Linux Gadget API is defined in the header file
 * <code><linux/usb_gadget.h></code>. The USB EP operations API is
 * defined in the structure <code>usb_ep_ops</code> and the USB
 * Controller API is defined in the structure
 * <code>usb_gadget_ops</code>.
 *
 * An important function of the PCD is managing interrupts generated
 * by the DWC_usb3 controller. The implementation of the DWC_usb3 device
 * mode interrupt service routines is in dwc_otg_pcd_intr.c.
 */
/*
 * todo Add Device Mode test modes (Test J mode, Test K mode, etc).
 * todo Does it work when the request size is greater than DEPTSIZ
 * transfer size
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
 * Routines for sending the various device commands to the hardware
 */

int dwc_usb3_xmit_fn_remote_wake(dwc_usb3_pcd_t *pcd, uint32_t intf)
{
	//DWC_USB3_DEBUG("%s()\n", __func__);

	/* Set param */
	dwc_write_reg32(&pcd->dev_global_regs->dgcmdpar, intf);

	/* Start the command */
	dwc_write_reg32(&pcd->dev_global_regs->dgcmd,
			DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF | DWC_DGCMD_ACT_BIT);

	//DWC_USB3_DEBUG("DGCMD=%08x\n", DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF |
	//				DWC_DGCMD_ACT_BIT);

	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, 
				&pcd->dev_global_regs->dgcmd,
				DWC_DGCMD_ACT_BIT, 0, 10000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	
	return 0;
}

/**
 * Routines for sending the various endpoint commands to the hardware
 */

/**
 * DEPCFG command
 */
int dwc_usb3_dep_cfg(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
		     uint32_t depcfg0, uint32_t depcfg1)
{
	/* Set param 1 */
	dwc_write_reg32(&ep_reg->depcmdpar1, depcfg1);
	
	/* Set param 0 */
	dwc_write_reg32(&ep_reg->depcmdpar0, depcfg0);
	
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd, DWC_EPCMD_SET_EP_CFG | DWC_EPCMD_ACT_BIT);
	
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	
	return 0;
}

/**
 * DEPXFERCFG command
 */
int dwc_usb3_dep_xfercfg(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			 uint32_t depstrmcfg)
{
	/* Set param 0 */
	dwc_write_reg32(&ep_reg->depcmdpar0, depstrmcfg);
	
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd,DWC_EPCMD_SET_XFER_CFG | DWC_EPCMD_ACT_BIT);
	
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	return 0;
}

/**
 * DEPSSTALL command
 */
int dwc_usb3_dep_sstall(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg)
{
	/* Start the command */
	//dwc_write_reg32(&ep_reg->depcmd,DWC_EPCMD_SET_STALL | DWC_EPCMD_ACT_BIT);
	dwc_write_reg32(&ep_reg->depcmd,DWC_EPCMD_SET_STALL | DWC_EPCMD_ACT_BIT | DWC_EPCMD_IOC_BIT);  //modify by yujng
		
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	return 0;
}

/**
 * DEPCSTALL command
 */
int dwc_usb3_dep_cstall(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg)
{
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd,DWC_EPCMD_CLR_STALL | DWC_EPCMD_ACT_BIT);
	
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	return 0;
}

/**
 * DEPSTRTXFER command
 */
int dwc_usb3_dep_startxfer(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			   dwc_dma_t dma_addr, uint32_t stream_or_uf)//, dwc_usb3_pcd_ep_t ep)
{
	uint32_t depcmd;

	/* Set param 1 */
	dwc_write_reg32(&ep_reg->depcmdpar1,dma_addr & 0xffffffffU);
	
	/* Set param 0 */
	dwc_write_reg32(&ep_reg->depcmdpar0, 0);
	
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd,
			(stream_or_uf << DWC_EPCMD_STR_NUM_OR_UF_SHIFT) |
			DWC_EPCMD_START_XFER | DWC_EPCMD_ACT_BIT);
	
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	
	depcmd = dwc_read_reg32(&ep_reg->depcmd);

	return (depcmd >> DWC_EPCMD_XFER_RSRC_IDX_SHIFT) &
	       (DWC_EPCMD_XFER_RSRC_IDX_BITS >> DWC_EPCMD_XFER_RSRC_IDX_SHIFT);
}

/**
 * DEPUPDTXFER command
 */
int dwc_usb3_dep_updatexfer(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			    uint32_t tri)
{
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd,
			(tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
			DWC_EPCMD_UPDATE_XFER | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 10000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	
	return 0;
}

/**
 * DEPENDXFER command
 */
int dwc_usb3_dep_endxfer(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			 uint32_t tri, void *condition)
{
	uint32_t depcmd;

	/* Fill end transfer command */
	depcmd = (tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) | DWC_EPCMD_END_XFER;
	depcmd |= DWC_EPCMD_ACT_BIT;
	depcmd |= DWC_EPCMD_HP_FRM_BIT;

	/* Start the command. */
	dwc_write_reg32(&ep_reg->depcmd, depcmd);
	
	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 10000))
	{
	    //DWC_USB3_ERROR("Handshake is not passed\n");
	}
	
	return 0;
}

/**
 * DEPSTRTNEWCFG command
 */
int dwc_usb3_dep_startnewcfg(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			     uint32_t rsrcidx)
{
	/* Start the command */
	dwc_write_reg32(&ep_reg->depcmd,
			(rsrcidx << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
			DWC_EPCMD_START_NEW_CFG | DWC_EPCMD_ACT_BIT);
	

	/* Wait for command completion */
	if (!dwc_usb3_handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0, 1000))
	{
		//DWC_USB3_ERROR("Handshake is not passed\n");
	}
	return 0;
}

/**********************/

/**
 * Gets the current USB frame number. This is the frame number from the last
 * SOF packet.
 */
uint32_t dwc_usb3_get_frame_number(dwc_usb3_pcd_t *pcd)
{
	uint32_t dsts;

	/* read current frame/microframe number from DSTS register */
	dsts = dwc_read_reg32(&pcd->dev_global_regs->dsts);

	return (dsts >> DWC_DSTS_SOF_FN_SHIFT) &
	       (DWC_DSTS_SOF_FN_BITS >> DWC_DSTS_SOF_FN_SHIFT);
}

/**
 * This function enables EP0 OUT to receive SETUP packets and configures
 * EP0 IN for transmitting packets. It is normally called when the
 * "Connect Done" interrupt occurs.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP0 data.
 */
void dwc_usb3_ep0_activate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	uint32_t diepcfg0, doepcfg0, diepcfg1, doepcfg1;
	dwc_usb3_dev_ep_regs_t *ep_reg;

	
	diepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	diepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | //DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT | DWC_EPCFG1_EP_DIR_BIT;

	doepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	doepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | //DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT;

	/* Default to MPS of 512 (will reconfigure after ConnectDone event) */
	diepcfg0 |= DWC_MAX_EP0_SIZE_HS << DWC_EPCFG0_MPS_SHIFT;
	doepcfg0 |= DWC_MAX_EP0_SIZE_HS << DWC_EPCFG0_MPS_SHIFT;

	/*
	 * Issue "DEPCFG" command to EP0-OUT
	 */

	ep_reg = &pcd->out_ep_regs[0];

	/* Must issue DEPSTRTNEWCFG command first */
	dwc_usb3_dep_startnewcfg(pcd, ep_reg, 0);

	dwc_usb3_dep_cfg(pcd, ep_reg, doepcfg0, doepcfg1);

	/*
	 * Issue "DEPSTRMCFG" command to EP0-OUT
	 */

	/* One stream */
	dwc_usb3_dep_xfercfg(pcd, ep_reg, 1);

	/*
	 * Issue "DEPCFG" command to EP0-IN
	 */

	ep_reg = &pcd->in_ep_regs[0];
	dwc_usb3_dep_cfg(pcd, ep_reg, diepcfg0, diepcfg1);

	/*
	 * Issue "DEPSTRMCFG" command to EP0-IN
	 */

	/* One stream */
	dwc_usb3_dep_xfercfg(pcd, ep_reg, 1);
}

/**
 * This function activates an EP. The Device EP control register for
 * the EP is configured as defined in the ep structure.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to activate.
 */
void dwc_usb3_ep_activate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;
	uint32_t dalepena, depcfg0, depcfg1;
	uint8_t ep_index_num;

	/*
	 * Get the appropriate EP registers, and the index into the
	 * DALEPENA register
	 */
	ep_index_num = ep->dwc_ep.num * 2;

	if (ep->dwc_ep.is_in) 
	{
		ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
		ep_index_num += 1;
	}
	else 
	{
		ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];
	}

	dalepena = dwc_read_reg32(&pcd->dev_global_regs->dalepena);

	/* If we have already configured this endpoint, leave it alone
	 * (shouldn't happen)
	 */
	if (dalepena & (1 << ep_index_num))
		goto out;

	/*
	 * Issue "DEPCFG" command to EP
	 */
	depcfg0 = ep->dwc_ep.type << DWC_EPCFG0_EPTYPE_SHIFT;
	depcfg0 |= ep->dwc_ep.maxpacket << DWC_EPCFG0_MPS_SHIFT;

	if (ep->dwc_ep.is_in) 
	{
		depcfg0 |= ep->dwc_ep.tx_fifo_num << DWC_EPCFG0_TXFNUM_SHIFT;
	}

	if (pcd->usb3_dev->core_params->burst) 
	{
		//DWC_USB3_DEBUG("Setting maxburst to %u\n", ep->dwc_ep.maxburst);
        //depcfg0 |= ep->dwc_ep.maxburst << DWC_EPCFG0_BRSTSIZ_SHIFT;

        /*no burst here, just for simulation*/
		depcfg0 |= 0 << DWC_EPCFG0_BRSTSIZ_SHIFT;
	}

	depcfg1 = ep->dwc_ep.num << DWC_EPCFG1_EP_NUM_SHIFT;
	if (ep->dwc_ep.is_in)
		depcfg1 |= DWC_EPCFG1_EP_DIR_BIT;

	depcfg1 |= DWC_EPCFG1_XFER_CMPL_BIT;
	depcfg1 |= DWC_EPCFG1_XFER_IN_PROG_BIT;
	//depcfg1 |= DWC_EPCFG1_XFER_NRDY_BIT;
	depcfg1 |= ep->dwc_ep.intvl << DWC_EPCFG1_BINTERVAL_SHIFT;

#ifdef DWC_UASP_GADGET_STREAMS
	if (ep->dwc_ep.num_streams) {
		DWC_USB3_DEBUG("Setting stream-capable bit\n");
		depcfg1 |= DWC_EPCFG1_STRM_CAP_BIT;
	}
#endif


//	if (ep->dwc_ep.num == 1 && !ep->dwc_ep.is_in &&
//	    (pcd->usb3_dev->snpsid & 0xffff) >= 0x109a) 
//	{
//		/* Must issue DEPSTRTNEWCFG command first */
//		dwc_usb3_dep_startnewcfg(pcd, ep_reg, 2);    //???
//	}


    /* Must issue DEPSTRTNEWCFG command first */
	//dwc_usb3_dep_startnewcfg(pcd, ep_reg, 2);      //added by yujing
	dwc_usb3_dep_startnewcfg(pcd, &pcd->out_ep_regs[0], 2);      //added by yujing
	
	dwc_usb3_dep_cfg(pcd, ep_reg, depcfg0, depcfg1);

	/*
	 * Issue "DEPSTRMCFG" command to EP
	 */

#ifdef DWC_UASP_GADGET_STREAMS
	if (ep->dwc_ep.num_streams) {
		DWC_USB3_DEBUG("Setting %u streams\n", 1U << ep->dwc_ep.num_streams);
		dwc_usb3_dep_xfercfg(pcd, ep_reg, 1U << ep->dwc_ep.num_streams);
	} else {
		DWC_USB3_DEBUG("Setting 1 stream\n");
#endif
		/* One stream */
		dwc_usb3_dep_xfercfg(pcd, ep_reg, 1);

#ifdef DWC_UASP_GADGET_STREAMS
	}
#endif
	/* Enable EP in DALEPENA reg */
	dalepena |= 1 << ep_index_num;
	dwc_write_reg32(&pcd->dev_global_regs->dalepena, dalepena);
out:
	ep->dwc_ep.stall_clear_flag = 0;
}

/**
 * This function deactivates an EP.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to deactivate.
 */
void dwc_usb3_ep_deactivate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;
	uint32_t dalepena;
	uint8_t ep_index_num;
	uint8_t tri;

	
	/* Clear flag that shows we have enabled some EPs */
	pcd->eps_enabled = 0;

	/*
	 * Get the appropriate EP registers, and the index into the
	 * DALEPENA register
	 */
	ep_index_num = ep->dwc_ep.num * 2;

	if (ep->dwc_ep.is_in)
	{
		ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
		ep_index_num += 1;
		tri = ep->dwc_ep.tri_in;
		ep->dwc_ep.tri_in = 0;
	} 
	else
	{
		ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];
		tri = ep->dwc_ep.tri_out;
		ep->dwc_ep.tri_out = 0;
	}

	dalepena = dwc_read_reg32(&pcd->dev_global_regs->dalepena);

	/* If we have already de-configured this endpoint, leave it alone
	 * (shouldn't happen)
	 */
	if (!(dalepena & (1 << ep_index_num))) 
	{
		return;
	}

	//DWC_USB3_DEBUG("DWC EP=%p tri=%d\n", ep, tri);
	if (tri)
	{
		ep->dwc_ep.condition = 0;

		/* Execute remove stream command */
		dwc_usb3_dep_endxfer(pcd, ep_reg, tri, &ep->dwc_ep.condition);
	}

	/* Disable EP in DALEPENA reg */
	dalepena &= ~(1 << ep_index_num);
	dwc_write_reg32(&pcd->dev_global_regs->dalepena, dalepena);
}

/**
 * This function does the setup for a data transfer for an EP and
 * starts the transfer.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to start the transfer on.
 * @param req The request to start.
 */
void dwc_usb3_ep_start_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				dwc_usb3_pcd_request_t *req)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;
	dwc_usb3_dma_desc_t *dma_desc;
	dwc_dma_t dma_desc_dma;
	uint32_t len;
	uint8_t tri;


	/* Get the next DMA Descriptor (TRB) for this EP */
	dma_desc = (dwc_usb3_dma_desc_t *)(ep->dwc_ep.dma_desc + ep->dwc_ep.desc_idx * 16);
	dma_desc_dma = ep->dwc_ep.dma_desc_dma + ep->dwc_ep.desc_idx * 16;

	if (++ep->dwc_ep.desc_idx >= ep->dwc_ep.num_desc)
		ep->dwc_ep.desc_idx = 0;

	req->dwc_req.trb = dma_desc;
	req->dwc_req.trb_dma = dma_desc_dma;

	/* DMA Descriptor Setup */
	dma_desc->bptl = req->dwc_req.dma & 0xffffffffU;

	dma_desc->bpth = 0;

	if (ep->dwc_ep.is_in) 
	{
		/*
		 * Start DMA on EPn-IN
		 */
		ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
		len = req->dwc_req.length;
		dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;

		if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
		{
			dma_desc->control |= DWC_DSCCTL_HWO_BIT;
		} 
		else 
		{
			dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
				(DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
				DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT |
				req->dwc_req.stream << DWC_DSCCTL_STRMID_SOFN_SHIFT;
		}


		/* If Isoc */
		if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
		{
			/* Issue "DEPUPDTXFER" command to EP */
			dwc_usb3_dep_updatexfer(pcd, ep_reg, ep->dwc_ep.tri_in);
		}
		else 
		{
			/* Issue "DEPSTRTXFER" command to EP */
			tri = dwc_usb3_dep_startxfer(pcd, ep_reg, dma_desc_dma,
						     req->dwc_req.stream);
			ep->dwc_ep.tri_in = tri;
		}
	} 
	else 
	{
	
		/*
		 * Start DMA on EPn-OUT
		 */
		ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];

		if ((ep->dwc_ep.type == UE_ISOCHRONOUS ||
		     ep->dwc_ep.type == UE_INTERRUPT) && ep->dwc_ep.maxpacket != 1024)
			/* Might not be power of 2, so use (expensive?) divide */
			len = (req->dwc_req.length + ep->dwc_ep.maxpacket - 1) /
				ep->dwc_ep.maxpacket;
		else
			/* Must be power of 2, use cheap AND */
			len = (req->dwc_req.length + ep->dwc_ep.maxpacket - 1) &
				~(ep->dwc_ep.maxpacket - 1);

		req->dwc_req.length = len;
		dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;

		if (ep->dwc_ep.type == UE_ISOCHRONOUS)
		{
			dma_desc->control |= DWC_DSCCTL_HWO_BIT;
		} 
		else 
		{
			dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
				(DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
				DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT |
				req->dwc_req.stream << DWC_DSCCTL_STRMID_SOFN_SHIFT;
		}


		/* If Isoc */
		if (ep->dwc_ep.type == UE_ISOCHRONOUS) 
		{
			/* Issue "DEPUPDTXFER" command to EP */
			dwc_usb3_dep_updatexfer(pcd, ep_reg, ep->dwc_ep.tri_out);
		} 
		else 
		{
			/* Issue "DEPSTRTXFER" command to EP */
			tri = dwc_usb3_dep_startxfer(pcd, ep_reg, dma_desc_dma,
						     req->dwc_req.stream);
			ep->dwc_ep.tri_out = tri;
		}
	}

	ep->dwc_ep.desc_avail--;
}

/**
 * This function does the setup for a 0-length transfer for an EP and
 * starts the transfer.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to start the transfer on.
 */
void dwc_usb3_ep_start_zl_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
}

/**
 * This function does the setup for a data transfer for EP0 and starts
 * the transfer.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP0 data.
 * @param req The request to start.
 */
void dwc_usb3_ep0_start_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				 dwc_usb3_pcd_request_t *req)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;
	dwc_usb3_dma_desc_t *dma_desc;
	uint32_t len;
	uint8_t tri;


	if (ep->dwc_ep.is_in) 
	{
		/*
		 * Start DMA on EP0-IN
		 */
		ep_reg = &pcd->in_ep_regs[0];
		//dma_desc = pcd->in_desc_addr;
		dma_desc = pcd->setup_desc_addr;

		/* DMA Descriptor Setup */
		dma_desc->bptl = req->dwc_req.dma & 0xffffffffU;

		dma_desc->bpth = 0;

		len = req->dwc_req.length;
		dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;
		dma_desc->control = DWC_DSCCTL_IOC_BIT | //DWC_DSCCTL_ISP_BIT |
				    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
		//DWC_USB3_DEBUG("IN EP0STATE=%d\n", pcd->ep0state);

		if (pcd->ep0state == EP0_IN_STATUS_PHASE) 
		{
			if (pcd->ep0->dwc_ep.three_stage)
				dma_desc->control |= DWC_DSCCTL_TRBCTL_STATUS_3
							<< DWC_DSCCTL_TRBCTL_SHIFT;
			else
				dma_desc->control |= DWC_DSCCTL_TRBCTL_STATUS_2
							<< DWC_DSCCTL_TRBCTL_SHIFT;
		} 
		else 
		{
			dma_desc->control |= DWC_DSCCTL_TRBCTL_CTLDATA_1ST
						<< DWC_DSCCTL_TRBCTL_SHIFT;
		}


		/* Issue "DEPSTRTXFER" command to EP0-IN */
		//tri = dwc_usb3_dep_startxfer(pcd, ep_reg, pcd->dma_in_desc_addr, 0);
		tri = dwc_usb3_dep_startxfer(pcd, ep_reg, (uint32_t)pcd->setup_desc_addr, 0);
		pcd->ep0->dwc_ep.tri_in = tri;
	} 
	else 
	{
	
		/*
		 * Start DMA on EP0-OUT
		 */
		ep_reg = &pcd->out_ep_regs[0];
		//dma_desc = pcd->out_desc_addr;
		dma_desc = pcd->setup_desc_addr;

		/* DMA Descriptor Setup */
		dma_desc->bptl = req->dwc_req.dma & 0xffffffffU;

		dma_desc->bpth = 0;

		len = (req->dwc_req.length + ep->dwc_ep.maxpacket - 1) &
			~(ep->dwc_ep.maxpacket - 1);
		dma_desc->status = len << DWC_DSCSTS_XFRCNT_SHIFT;
		dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
				    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
		//DWC_USB3_DEBUG("OUT EP0STATE=%d\n", pcd->ep0state);

		if (pcd->ep0state == EP0_OUT_STATUS_PHASE) 
		{
			if (pcd->ep0->dwc_ep.three_stage)
				dma_desc->control |= DWC_DSCCTL_TRBCTL_STATUS_3
						<< DWC_DSCCTL_TRBCTL_SHIFT;
			else
				dma_desc->control |= DWC_DSCCTL_TRBCTL_STATUS_2
						<< DWC_DSCCTL_TRBCTL_SHIFT;
		} 
		else 
		{
			dma_desc->control |= DWC_DSCCTL_TRBCTL_CTLDATA_1ST
						<< DWC_DSCCTL_TRBCTL_SHIFT;
		}


		/* Issue "DEPSTRTXFER" command to EP0-OUT */
		//tri = dwc_usb3_dep_startxfer(pcd, ep_reg, pcd->dma_out_desc_addr, 0);
		tri = dwc_usb3_dep_startxfer(pcd, ep_reg, (uint32_t)pcd->setup_desc_addr, 0);
		pcd->ep0->dwc_ep.tri_out = tri;
	}
}

/**
 * This function continues control IN transfers started by
 * dwc_usb3_ep0_start_transfer, when the transfer does not fit in a
 * single packet.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP0 data.
 */
void dwc_usb3_ep0_continue_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
#ifdef FIXME
	dwc_usb3_dma_desc_t *dma_desc;

	if (ep->dwc_ep.is_in) {
		/* Program the transfer size and packet count
		 * as follows: xfersize = N * maxpacket +
		 * short_packet pktcnt = N + (short_packet
		 * exist ? 1 : 0)
		 */
		ep->dwc_ep.xfer_len =
			(ep->dwc_ep.total_len - ep->dwc_ep.xfer_count)
				> ep->dwc_ep.maxpacket
			? ep->dwc_ep.maxpacket
			: (ep->dwc_ep.total_len - ep->dwc_ep.xfer_count);

		dma_desc = pcd->in_desc_addr;

		/* DMA Descriptor Setup */
		dma_desc->bptl = ep->dwc_ep.dma_addr & 0xffffffffU & MASK_PHYSICAL_ADDR_CHAGE;

		dma_desc->bpth = 0;

		dma_desc->status = ep->dwc_ep.xfer_len << DWC_DSCSTS_XFRCNT_SHIFT;
		dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
				    (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
				    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
	}
	else 
	{
	
		dma_desc = pcd->out_desc_addr;

		/* DMA Descriptor Setup */
		dma_desc->bptl = ep->dwc_ep.dma_addr & 0xffffffffU & MASK_PHYSICAL_ADDR_CHAGE;

		dma_desc->bpth = 0;

		dma_desc->status = ep->dwc_ep.maxpacket << DWC_DSCSTS_XFRCNT_SHIFT;
		dma_desc->control = DWC_DSCCTL_IOC_BIT | DWC_DSCCTL_ISP_BIT |
				    (DWC_DSCCTL_TRBCTL_NORMAL << DWC_DSCCTL_TRBCTL_SHIFT) |
				    DWC_DSCCTL_LST_BIT | DWC_DSCCTL_HWO_BIT;
	}
#endif
}

/**
 * Set the EP STALL.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to set the stall on.
 */
void dwc_usb3_ep_set_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;

	if (ep->dwc_ep.is_in)
		ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
	else
		ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];

	dwc_usb3_dep_sstall(pcd, ep_reg);
}

/**
 * Clear the EP STALL.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep The EP to clear stall from.
 */
void dwc_usb3_ep_clear_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	dwc_usb3_dev_ep_regs_t *ep_reg;

	if (ep->dwc_ep.is_in)
		ep_reg = &pcd->in_ep_regs[ep->dwc_ep.num];
	else
		ep_reg = &pcd->out_ep_regs[ep->dwc_ep.num];

	dwc_usb3_dep_cstall(pcd, ep_reg);
}



/**
 * This function checks the EP request queue, if the queue is not
 * empty the next request is started.
 */
//void dwc_usb3_start_next_request(dwc_usb3_pcd_ep_t *ep)


/**
 * This function completes a request. It calls the request callback.
 */
/*
void dwc_usb3_request_done(dwc_usb3_pcd_ep_t *ep, dwc_usb3_pcd_request_t *req,
			   int status)
*/

/**
 * This function terminates all the requests in the EP request queue.
 */
//void dwc_usb3_request_nuke(dwc_usb3_pcd_ep_t *ep)




/* USB Endpoint Operations */
/*
 * The following sections briefly describe the behavior of the Gadget
 * API endpoint operations implemented in the DWC_usb3 driver
 * software. Detailed descriptions of the generic behavior of each of
 * these functions can be found in the Linux header file
 * include/linux/usb_gadget.h.
 *
 * The Gadget API provides wrapper functions for each of the function
 * pointers defined in usb_ep_ops. The Gadget Driver calls the wrapper
 * function, which then calls the underlying PCD function. The
 * following sections are named according to the wrapper
 * functions. Within each section, the corresponding DWC_usb3 PCD
 * function name is specified.
 *
 */
#ifdef FIXME
/**
 * This function assigns periodic Tx FIFO to a periodic EP
 * in shared Tx FIFO mode
 */
static uint32_t assign_perio_tx_fifo(dwc_usb3_pcd_t *pcd)
{
	return 0;
}
#endif

/**
 * This function releases periodic Tx FIFO
 * in shared Tx FIFO mode
 */
static void release_perio_tx_fifo(dwc_usb3_pcd_t *pcd, uint32_t fifo_num)
{
}

#ifdef FIXME
/**
 * This function assigns Tx FIFO to an EP
 */
static uint32_t assign_tx_fifo(dwc_usb3_pcd_t *pcd)
{
	return 0;
}
#endif

/**
 * This function releases Tx FIFO
 */
static void release_tx_fifo(dwc_usb3_pcd_t *pcd, uint32_t fifo_num)
{
}





/**
 * This function is called by the Gadget Driver for each EP (except EP0) to be
 * configured for the current configuration (SET_CONFIGURATION).
 *
 * This function initializes the dwc_usb3_ep_t data structure, and then
 * calls dwc_usb3_ep_activate.
 */
int dwc_usb3_pcd_ep_enable(dwc_usb3_pcd_t *pcd, const void *ep_desc,
			   dwc_usb3_pcd_ep_t *ep, int maxvalues)
{
	const usb_endpoint_descriptor_t *desc =
				(const usb_endpoint_descriptor_t *)ep_desc;
	//dwc_usb3_pcd_ep_t *ep;
	//dwc_irqflags_t flags;
	int dir;
	uint16_t maxpacket;
	uint8_t type;
	uint8_t numstreams = maxvalues & 0x1f;
	uint8_t mult = (maxvalues >> 5) & 0x3;
	uint8_t maxburst = (maxvalues >> 8) & 0xf;

	//ep = dwc_usb3_get_pcd_ep(usb_ep);
		
	//dwc_spinlock_irqsave(pcd->lock, &flags);
#if 0
	if (ep->dwc_ep.desc)
	{
		//DWC_USB3_WARN("%s, bad ep or descriptor\n", __func__);
		//dwc_spinunlock_irqrestore(pcd->lock, flags);
		//return -DWC_E_INVALID;
		while(1);   //error
	}
#endif
	ep->dwc_ep.desc = desc;

	/*
	 * Activate the EP
	 */
	ep->dwc_ep.stopped = 0;

	dir = UE_GET_DIR(desc->bEndpointAddress);
	if (ep->dwc_ep.is_in != (dir == UE_DIR_IN)) 
	{
		//DWC_USB3_WARN("%s, EP dir mismatch, is %d asked %d\n",
		//	      __func__, ep->dwc_ep.is_in, dir == UE_DIR_IN);
	}

	type = desc->bmAttributes & UE_XFERTYPE;
	maxpacket = UGETW(desc->wMaxPacketSize);

	ep->dwc_ep.type = type;
	ep->dwc_ep.maxpacket = maxpacket & 0x7ff;
	ep->dwc_ep.intvl = 0;
	ep->dwc_ep.mult = 0;
	ep->dwc_ep.maxburst = maxburst;
	ep->dwc_ep.num_streams = numstreams;
	ep->dwc_ep.isoc_started = 0;

	switch (type) 
	{
	case UE_INTERRUPT:
		if (pcd->speed == USB_SPEED_SUPER)
		{
			ep->dwc_ep.intvl = desc->bInterval - 1;
			break;
		}
		/* FALL THRU */
	case UE_ISOCHRONOUS:
		if (pcd->speed != USB_SPEED_SUPER) 
		{
			/*
			 * Bits 12:11 specify number of _additional_
			 * packets per microframe.
			 */
			mult = ((maxpacket >> 11) & 3) + 1;

			/* 3 packets at most */
			if (mult > 3) 
			{
				//dwc_spinunlock_irqrestore(pcd->lock, flags);
				//return -EINVAL;
				while(1);  //error
			}
		}

		ep->dwc_ep.mult = mult;
		ep->dwc_ep.intvl = desc->bInterval - 1;
		break;
	}


	if (ep->dwc_ep.is_in) 
	{
#ifdef FIXME
		if (!pcd->usb3_dev->en_multiple_tx_fifo) 
		{
			ep->dwc_ep.tx_fifo_num = 0;

			if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
				/*
				 * if ISOC EP then assign a Periodic Tx FIFO.
				 */
				ep->dwc_ep.tx_fifo_num = assign_perio_tx_fifo(pcd);
			 }
		} else {
			/*
			 * if Dedicated FIFOs mode is on then assign a Tx FIFO.
			 */
			ep->dwc_ep.tx_fifo_num = assign_tx_fifo(pcd);

		}
#endif
	}

	/* Set initial data PID. */
	if (ep->dwc_ep.type == UE_BULK)
	{
		ep->dwc_ep.data_pid_start = 0;
	}

	dwc_usb3_ep_activate(pcd, ep);
	//dwc_spinunlock_irqrestore(pcd->lock, flags);
	return 0;
}

/**
 * This function is called when an EP is disabled due to disconnect or
 * change in configuration. Any pending requests will terminate with a
 * status of -ESHUTDOWN.
 *
 * This function modifies the dwc_usb3_ep_t data structure for this EP,
 * and then calls dwc_usb3_ep_deactivate.
 */
int dwc_usb3_pcd_ep_disable(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	//dwc_usb3_pcd_ep_t *ep;
	//dwc_irqflags_t flags;

	//ep = dwc_usb3_get_pcd_ep(usb_ep);

	if (!ep->dwc_ep.desc)
	{
		//return -DWC_E_INVALID;
	}

	//dwc_spinlock_irqsave(pcd->lock, &flags);

	//dwc_usb3_request_nuke(ep);
	dwc_usb3_ep_deactivate(ep->dwc_ep.pcd, ep);
	//ep->dwc_ep.desc = NULL;
	ep->dwc_ep.desc = 0;
		ep->dwc_ep.stopped = 1;

	if (ep->dwc_ep.is_in) {
		dwc_usb3_flush_tx_fifo(ep->dwc_ep.pcd->usb3_dev, ep->dwc_ep.tx_fifo_num);
		release_perio_tx_fifo(ep->dwc_ep.pcd, ep->dwc_ep.tx_fifo_num);
		release_tx_fifo(ep->dwc_ep.pcd, ep->dwc_ep.tx_fifo_num);
	}

	//dwc_spinunlock_irqrestore(ep->dwc_ep.pcd->lock, flags);
	return 0;
}

/**
 * This function is used to submit an I/O Request to an EP.
 *
 * - When the request completes the request's completion callback
 *   is called to return the request to the driver.
 * - An EP, except control EPs, may have multiple requests
 *   pending.
 * - Once submitted the request cannot be examined or modified.
 * - Each request is turned into one or more packets.
 * - A BULK EP can queue any amount of data; the transfer is
 *   packetized.
 * - Zero length Packets are specified with the request 'zero'
 *   flag.
 */
/*
int dwc_usb3_pcd_ep_queue(dwc_usb3_pcd_t *pcd, void *usb_ep, void *usb_req,
			  void *buf, dwc_dma_t dma_buf, uint32_t buflen,
			  uint32_t stream, int rflags, int is_atomic)

*/








/**
 * This function cancels an I/O request from an EP.
 */
/*
int dwc_usb3_pcd_ep_dequeue(dwc_usb3_pcd_t *pcd, void *usb_ep, void *usb_req,
			    uint32_t stream)
*/

/**
 * usb_ep_set_halt stalls an endpoint.
 *
 * usb_ep_clear_halt clears an endpoint halt and resets its data
 * toggle.
 *
 * Both of these functions are implemented with the same underlying
 * function. The behavior depends on the value argument.
 *
 * @param pcd
 * @param usb_ep the Endpoint to halt or clear halt.
 * @param value
 *	- 0 means clear_halt.
 *	- 1 means set_halt,
 *	- 2 means clear stall lock flag.
 *	- 3 means set stall lock flag.
 */
//int dwc_usb3_pcd_ep_set_halt(dwc_usb3_pcd_t *pcd, void *usb_ep, int value)



/**
 * Tasklet
 *
 */
//static void start_xfer_tasklet_func(void *data)




/**
 * This function initializes the pcd EP structures to their default
 * state.
 *
 * @param pcd the pcd structure.
 */
static void pcd_epinit(dwc_usb3_pcd_t *pcd)
{
	//uint32_t num_out_eps = pcd->num_out_eps;
	//uint32_t num_in_eps = pcd->num_in_eps;
	uint32_t num_out_eps = 2;
	uint32_t num_in_eps = 2;
	dwc_usb3_pcd_ep_t *ep;
	int ep_cntr, i;

	/*
	 * Initialize the EP0 structure.
	 */
	ep = pcd->ep0;

	/* Init EP structure */
	ep->dwc_ep.desc = NULL;
	ep->dwc_ep.pcd = pcd;
	ep->dwc_ep.stopped = 1;
	ep->dwc_ep.num = 0;
	ep->dwc_ep.active = 0;
	ep->dwc_ep.tx_fifo_num = 0;
	ep->dwc_ep.type = DWC_USB3_EP_TYPE_CONTROL;
	ep->dwc_ep.maxpacket = DWC_MAX_EP0_SIZE_HS;
	ep->dwc_ep.send_zlp = 0;
	ep->dwc_ep.queue_sof = 0;

	ep->dwc_ep.tri_out_initialize = 0;   //add by yujing 20110331
	ep->dwc_ep.tri_in_initialize = 0;

	pcd->ep0_req->dwc_req.buf = NULL;
	pcd->ep0_req->dwc_req.dma = 0;
	pcd->ep0_req->dwc_req.length = 0;
	pcd->ep0_req->dwc_req.actual = 0;

	//DWC_LIST_INIT(&ep->dwc_ep.queue);

	/*
	 * Initialize the EP structures.
	 */
	ep_cntr = 0;

	for (i = 1; ep_cntr < num_out_eps; i++) 
	{
		//DWC_USB3_DEBUG("initializing EP%d-OUT (out_ep[%d])\n", i, ep_cntr);
		ep = pcd->out_ep[ep_cntr];
		ep_cntr++;

		ep->dwc_ep.tri_out_initialize = 0;   //add by yujing 20110527
	
		/* Init EP structure */
		ep->dwc_ep.desc = NULL;
		ep->dwc_ep.pcd = pcd;
		ep->dwc_ep.stopped = 1;
		ep->dwc_ep.is_in = 0;
		ep->dwc_ep.num = i;
		ep->dwc_ep.active = 0;
		ep->dwc_ep.tx_fifo_num = 0;

		/* Control until ep is activated */
		ep->dwc_ep.type = DWC_USB3_EP_TYPE_CONTROL;
		ep->dwc_ep.maxpacket = DWC_MAX_PACKET_SIZE_HS;
		ep->dwc_ep.send_zlp = 0;
		ep->dwc_ep.queue_sof = 0;

		//DWC_LIST_INIT(&ep->dwc_ep.queue);
	}

	ep_cntr = 0;

	for (i = 1; ep_cntr < num_in_eps; i++) 
	{
		//DWC_USB3_DEBUG("initializing EP%d-IN (in_ep[%d])\n", i, ep_cntr);
		ep = pcd->in_ep[ep_cntr];
		ep_cntr++;

		ep->dwc_ep.tri_in_initialize = 0; //add by yujing 20110527

		/* Init EP structure */
		ep->dwc_ep.desc = NULL;
		ep->dwc_ep.pcd = pcd;
		ep->dwc_ep.stopped = 1;
		ep->dwc_ep.is_in = 1;
		ep->dwc_ep.num = i;
		ep->dwc_ep.active = 0;
		ep->dwc_ep.tx_fifo_num = ep_cntr+1;

		/* Control until ep is activated */
		ep->dwc_ep.type = DWC_USB3_EP_TYPE_CONTROL;
		ep->dwc_ep.maxpacket = DWC_MAX_PACKET_SIZE_HS;
		ep->dwc_ep.send_zlp = 0;
		ep->dwc_ep.queue_sof = 0;

		//DWC_LIST_INIT(&ep->dwc_ep.queue);
	}

	pcd->ep0state = EP0_UNCONNECTED;

	pcd->ep0->dwc_ep.maxpacket = DWC_MAX_EP0_SIZE_HS;
	pcd->ep0->dwc_ep.type = DWC_USB3_EP_TYPE_CONTROL;
}

/**
 * This function initializes the PCD portion of the driver.
 *
 */
	int dwc_usb3_pcd_init(dwc_usb3_device_t *usb3_dev)
	{
		dwc_usb3_pcd_t *pcd = &usb3_dev->pcd;
	
		pcd->usb3_dev = usb3_dev;
		pcd->speed = 0; // unknown
	
		/*
		 * Initialize EP structures
		 */
		pcd_epinit(pcd);
	
		/*
		 * Register the PCD Callbacks.
		 */
		//dwc_usb3_cil_register_pcd_callbacks(usb3_dev, &pcd_callbacks, pcd);
	
#if 0	
		/*setup the TRB and setup packet/status buffer directly,   added by yujing*/
		pcd->setup_pkt = (void *)(DWC_SETUP_PKT_ADDR);
		pcd->setup_pkt_dma= DWC_SETUP_PKT_ADDR;
		pcd->status_buf = (uint8_t *)(DWC_STATUS_BUG_ADDR);
		pcd->status_buf_dma = DWC_STATUS_BUG_ADDR;
		pcd->setup_desc_addr = (dwc_usb3_dma_desc_t *)(DWC_TRB_ADDR_SETUP);
		pcd->dma_setup_desc_addr = DWC_TRB_ADDR_SETUP;
		pcd->in_desc_addr = (dwc_usb3_dma_desc_t *)(DWC_TRB_ADDR_IN);
		pcd->dma_in_desc_addr = DWC_TRB_ADDR_IN;
		pcd->out_desc_addr = (dwc_usb3_dma_desc_t *)(DWC_TRB_ADDR_OUT);
		pcd->dma_out_desc_addr = DWC_TRB_ADDR_OUT;
		pcd->ep_out_data_addr_temp = (dwc_dma_t *)(DWC_DATA_ADDR_OUT);
#endif
#if 1		
		pcd->setup_pkt_dma= (dwc_dma_t)dwc_dma_malloc(512);
		pcd->setup_pkt = (void *)(pcd->setup_pkt_dma);
		pcd->status_buf_dma = (dwc_dma_t)dwc_dma_malloc(512);
		pcd->status_buf = (uint8_t *)(pcd->status_buf_dma);
		pcd->dma_setup_desc_addr = (dwc_dma_t)dwc_dma_malloc(512);
		pcd->setup_desc_addr = (dwc_usb3_dma_desc_t *)(pcd->dma_setup_desc_addr );
		pcd->dma_in_desc_addr = (dwc_dma_t)dwc_dma_malloc(512);
		pcd->in_desc_addr = (dwc_usb3_dma_desc_t *)(pcd->dma_in_desc_addr);
		pcd->dma_out_desc_addr = (dwc_dma_t)dwc_dma_malloc(512);
		pcd->out_desc_addr = (dwc_usb3_dma_desc_t *)(pcd->dma_out_desc_addr);

		pcd->ep_out_data_addr_temp = (dwc_dma_t *)dwc_dma_malloc(512);
#endif

        //clean the TRB memory space
	    unsigned int *addr;
	    unsigned int i;
	    addr = (unsigned int *)(pcd->setup_pkt_dma);
	    for(i=0; i<512/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }

		addr = (unsigned int *)(pcd->status_buf_dma);
	    for(i=0; i<512/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }
		
		addr = (unsigned int *)(pcd->dma_setup_desc_addr);
	    for(i=0; i<512/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }
		addr = (unsigned int *)(pcd->dma_in_desc_addr);
	    for(i=0; i<512/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }
		addr = (unsigned int *)(pcd->dma_out_desc_addr);
	    for(i=0; i<512/4; i++)
	    {
	        *addr = 0;
		    addr++;
	    }
		
		/*
		 * Initialize the Core for Device mode.
		 */
		if (dwc_usb3_is_device_mode(usb3_dev)) 
		{
			dwc_usb3_core_device_init(usb3_dev);
		}
	
		/*just for debug*/
		//DCTL = DCTL | 0x80000000;
	
		dwc_usb3_ep0_activate(pcd, pcd->ep0);
	
		/* Set the Run/Stop bit */
		dwc_modify_reg32(&pcd->dev_global_regs->dctl,
				 DWC_DCTL_RUN_STOP_BIT, DWC_DCTL_RUN_STOP_BIT);
		
		return 0;
	}




