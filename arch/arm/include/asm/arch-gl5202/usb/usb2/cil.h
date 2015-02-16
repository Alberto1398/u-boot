/* ==========================================================================
 *
 *this file modify form synopsys Cil.h
 *
 *
 * ========================================================================== */

#ifndef _DWC_CIL_H_
#define _DWC_CIL_H_

#include "asm/arch/usb/usb2/data_type_define.h"
#include "asm/arch/usb/usb2/reg_fun.h"

/**
 * @file
 * This file contains the interface to the Common Interface Layer.
 */

/**
 * This is the common interrupt handler function.
 */
extern int dwc_usb3_irq(dwc_usb3_device_t *usb3_dev, int irq);

/**
 * The following are work functions for handling some interrupts
 */
extern void w_wakeup_detected(void *p);

/*
 * The following functions support initialization of the CIL driver component
 * and the DWC_usb3 controller.
 */
extern int dwc_usb3_common_init(dwc_usb3_device_t *usb3_dev, uint32_t reg_base, dwc_usb3_core_params_t *core_params);
extern void dwc_usb3_common_remove(dwc_usb3_device_t *usb3_dev);
extern void dwc_usb3_core_init(dwc_usb3_device_t *usb3_dev);
//extern void dwc_usb3_core_host_init(dwc_usb3_device_t *usb3_dev);
extern void dwc_usb3_core_device_init(dwc_usb3_device_t *usb3_dev);

/** @name Host CIL Functions
 * The following functions support managing the DWC_usb3 controller in host
 * mode.
 */
/** @{ */
/** @}*/

/** @name Common CIL Functions
 * The following functions support managing the DWC_usb3 controller in either
 * device or host mode.
 */
/** @{ */
extern void dwc_usb3_enable_common_interrupts(dwc_usb3_device_t *usb3_dev);
extern void dwc_usb3_enable_device_interrupts(dwc_usb3_device_t *usb3_dev);

extern void dwc_usb3_dump_global_registers(dwc_usb3_device_t *usb3_dev);

extern void dwc_usb3_flush_tx_fifo(dwc_usb3_device_t *usb3_dev, int num);
extern void dwc_usb3_flush_rx_fifo(dwc_usb3_device_t *usb3_dev);
extern void dwc_usb3_core_reset(dwc_usb3_device_t *usb3_dev);

/**
 * This function returns the current operating mode, host or device.
 *
 * @return 0 - Device Mode, 1 - Host Mode
 */
static inline uint32_t dwc_usb3_mode(dwc_usb3_device_t *usb3_dev)
{
	return dwc_read_reg32(&usb3_dev->core_global_regs->gsts) & 0x1;
}

/**
 * This function returns true if the current operating mode is Device.
 *
 * @return 1 - Device mode, 0 - Not Device mode
 */
static inline int dwc_usb3_is_device_mode(dwc_usb3_device_t *usb3_dev)
{
	return dwc_usb3_mode(usb3_dev) != DWC_GSTS_HOST_MODE;
}

/**
 * This function returns true if the current operating mode is Host.
 *
 * @return 1 - Host mode, 0 - Not Host mode
 */
static inline int dwc_usb3_is_host_mode(dwc_usb3_device_t *usb3_dev)
{
	return dwc_usb3_mode(usb3_dev) == DWC_GSTS_HOST_MODE;
}

extern int dwc_usb3_handle_common_intr(dwc_usb3_device_t *usb3_dev);
/** @} */

/**
 * DWC_usb3 CIL callback structure. This structure allows the HCD and
 * PCD to register functions used for starting and stopping the PCD
 * and HCD, and for role change for a DRD.
 */
typedef struct dwc_usb3_cil_callbacks {
	/** Start function for role change */
	int (*start) (void *_p);
	/** Stop function for role change */
	int (*stop) (void *_p);
	/** Disconnect function for role change */
	int (*disconnect) (void *_p);
	/** Resume/Remote Wakeup function */
	int (*resume_wakeup) (void *_p);
	/** Suspend function */
	int (*suspend) (void *_p);
	/** Session Start (SRP) function */
	int (*session_start) (void *_p);
	/** Pointer passed to callback functions */
	void *p;
} dwc_usb3_cil_callbacks_t;

extern void dwc_usb3_cil_register_pcd_callbacks(dwc_usb3_device_t *usb3_dev,
						dwc_usb3_cil_callbacks_t *cb,
						void *p);
extern int dwc_usb3_handshake(dwc_usb3_device_t *usb3_dev,
			      volatile uint32_t *ptr, uint32_t mask,
			      uint32_t done, uint32_t usec);


#endif /* _DWC_CIL_H_ */
