/* ==========================================================================
*
*
*this file is modify from Driver.h of synopsys
*
*
*
*
*
*
*
* ========================================================================== */

#ifndef _DWC_DRIVER_H_
#define _DWC_DRIVER_H_

#include "asm/arch/usb/usb3/pcd.h"

/** @file
 * This file contains the interface to the Linux driver.
 */

/**
 * The following parameters may be specified when loading the module. These
 * parameters define how the DWC_usb3 controller should be configured.
 * Parameter values are passed to the CIL initialization function
 * dwc_usb3_common_init.
 */
typedef struct dwc_usb3_core_params {
	int burst;
	int new;
	int phy;
	int wakeup;
	int pwrctl;
} dwc_usb3_core_params_t;

/**
 * This structure is a wrapper that encapsulates the driver components used to
 * manage a single DWC_usb3 controller.
 */
typedef struct dwc_usb3_device {
	/** Structure containing OS-dependent stuff. KEEP THIS STRUCT AT THE
	 * VERY BEGINNING OF THE DEVICE STRUCT. OSes such as FreeBSD and NetBSD
	 * require this. */
	//struct os_dependent os_dep;      //remove by yujing

	/** PCD structure */
	struct dwc_usb3_pcd pcd;
	//dwc_usb3_pcd_t pcd;

	/** Value from SNPSID register */
	uint32_t snpsid;

	/** Parameters that define how the core should be configured */
	dwc_usb3_core_params_t *core_params;

	/** Core Global registers starting at offset 000h. */
	dwc_usb3_core_global_regs_t *core_global_regs;

	/** Event Buffers for receiving interrupts. Up to 32 buffers are
	 * supported by the hardware, but we only use 1
	 */
#define DWC_EVENT_BUF_SIZE	1024	// size in dwords
#define DWC_NUM_EVENT_BUFS	1
	uint32_t *event_ptr[DWC_NUM_EVENT_BUFS];
	uint32_t *event_buf[DWC_NUM_EVENT_BUFS];
	dwc_dma_t event_buf_dma[DWC_NUM_EVENT_BUFS];

	/** Total RAM for FIFOs (Bytes) */
	uint16_t total_fifo_size;
	/** Size of Rx FIFO (Bytes) */
	uint16_t rx_fifo_size;

	/* @{ */
	/** Hardware Configuration - stored here for convenience */
	uint32_t hwparams0;
	uint32_t hwparams1;
	uint32_t hwparams2;
	uint32_t hwparams3;
	uint32_t hwparams4;
	uint32_t hwparams5;
	uint32_t hwparams6;
	uint32_t hwparams7;
	/* @} */

	/** Device Configuration -- stored here for convenience */
	uint32_t dcfg;

	/** PCD callbacks */
	//struct dwc_usb3_cil_callbacks *pcd_cb;

	/** Flag to indicate the common functionality has been initialized */
	unsigned int cmn_initialized		: 1;

	/** Flag to indicate the PCD has been initialized */
	unsigned int pcd_initialized		: 1;

	/** Flag to indicate the common IRQ handler is installed */
	unsigned int common_irq_installed	: 1;

	/** Flag to indicate the sysfs functions have been installed */
	unsigned int sysfs_initialized		: 1;

	/** Flag to indicate the core PHY interface bits in USBCFG have been
	 * initialized
	 */
	unsigned int phy_init_done		: 1;
} dwc_usb3_device_t;

#endif /* _DWC_DRIVER_H_ */
