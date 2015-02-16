/* ==========================================================================
 *
 *this file modify form synopsys Pcd.h
 *
 *
 * ========================================================================== */

#ifndef _DWC_PCD_H_
#define _DWC_PCD_H_

#include "asm/arch/usb/usb3/data_type_define.h"
#include "asm/arch/usb/usb3/usb3_regs.h"
#include "asm/arch/usb/usb3/usb.h"

/**
 * @file
 *
 * This file contains the structures, constants, and interfaces for
 * the Perpherial Contoller Driver (PCD).
 *
 * The Peripheral Controller Driver (PCD) for Linux will implement the
 * Gadget API, so that the existing Gadget drivers can be used. For
 * the Mass Storage Function driver the File-backed USB Storage Gadget
 * (FBS) driver will be used.  The FBS driver supports the
 * Control-Bulk (CB), Control-Bulk-Interrupt (CBI), and Bulk-Only
 * transports.
 *
 */

/** Invalid DMA Address */
#define DWC_DMA_ADDR_INVALID	(~(dwc_dma_t)0)

/** Maxpacket size for EP0 */
#define DWC_MAX_EP0_SIZE_SS	512
#define DWC_MAX_EP0_SIZE_HS	64

/** Maxpacket size for any EP */
#define DWC_MAX_PACKET_SIZE_SS	1024
#define DWC_MAX_PACKET_SIZE_HS	512
#define DWC_MAX_PACKET_SIZE_FS	64

#define DWC_DMA_ONCE_MAX_LENGTH      0x00ffffff
#define DWC_DMA_PER_MAX_LENGTH       0x00800000
#define DWC_DMA_PER_MAX_LEN_BIT      23
#define DWC_DMA_PER_MAX_LEN_BIT_MASK 0x007fffff

/**
 * States of EP0
 */
typedef enum ep0_state {
	EP0_UNCONNECTED,	/* no host */
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_IN_WAIT_GADGET,
	EP0_OUT_WAIT_GADGET,
	EP0_IN_WAIT_NRDY,
	EP0_OUT_WAIT_NRDY,
	EP0_IN_STATUS_PHASE,
	EP0_OUT_STATUS_PHASE,
	EP0_STALL,
} ep0state_e;

struct dwc_usb3_pcd_function_ops;
struct dwc_usb3_device;
struct dwc_usb3_pcd;

/**
 * The <code>dwc_req</code> structure represents the state of a single
 * transfer request when acting in device mode. It contains the data items
 * needed for a request to be started and completed.
 */
typedef struct dwc_req {
	void *buf;			/**< data buffer */
	dwc_dma_t dma;			/**< DMA address of data buffer */
	dwc_usb3_dma_desc_t *trb;	/**< TRB for this request */
	dwc_dma_t trb_dma;		/**< DMA address of TRB */
	uint32_t length;		/**< length of data buffer */
	uint32_t actual;		/**< actual amount of data transfered */
	uint32_t stream;		/**< stream # of this request */
	int flags;
#define DWC_PCD_REQ_ZERO	1
#define DWC_PCD_REQ_MAP_DMA	2
#define DWC_PCD_REQ_IN		4
#define DWC_PCD_REQ_STARTED	8
} dwc_req_t;

/** DWC_usb3 request structure.
 * This structure is used to form a list of requests.
 * WARNING! This struct must be kept in sync with struct dwc_usb3_pcd_request_real
 * in pcd_linux.c.
 * WARNING! This struct must never be used in a non-pointer context, since the
 * size of this struct is incorrect due to the dummy 'usb_req' member. Any non-
 * pointer use must be done with the OS-specific 'real' version of this struct
 * (struct dwc_usb3_pcd_request_real) in pcd_linux.c.
 */
typedef struct dwc_usb3_pcd_request {
	/** List entry for EP queue */
	//dwc_list_link_t entry;

	/** DWC_usb3 request data */
	dwc_req_t dwc_req;

	/** OS-specific USB request (opaque here, defined in pcd_linux.c) */
	//struct { void *dummy; } usb_req;
} dwc_usb3_pcd_request_t;

#undef dwc_usb3_get_pcd_req
#define dwc_usb3_get_pcd_req(usb_req) \
	container_of(usb_req, dwc_usb3_pcd_request_t, usb_req)

/**
 * The <code>dwc_ep</code> structure represents the state of a single
 * endpoint when acting in device mode. It contains the data items
 * needed for an endpoint to be activated and transfer packets.
 */
typedef struct dwc_ep {
	/** Pointer to PCD */
	struct dwc_usb3_pcd *pcd;

	/** Pointer to EP descriptor */
	const usb_endpoint_descriptor_t *desc;

	/** Queue of dwc_usb3_pcd_requests */
	//dwc_list_link_t queue;

	/** @{ */
	/** Status of the queue */
	unsigned int stopped	: 1;
	unsigned int disabling	: 1;
	unsigned int queue_sof	: 1;
	/** @} */

	/** @name Transfer state */
	/** @{ */
	/** Send ZLP */
	unsigned int send_zlp		: 1;

	/** Stall clear flag */
	unsigned int stall_clear_flag	: 1;

	/** True if 3-stage control transfer */
	unsigned int three_stage	: 1;

	/** True if Isoc endpoint has been started */
	unsigned int isoc_started	: 1;
	/** @} */

	/** EP direction 0 = OUT */
	unsigned int is_in		: 1;

	/** EP active */
	unsigned int active		: 1;

	/** DATA start PID for INTR and BULK EP */
	unsigned int data_pid_start	: 1;

	/** Frame (even/odd) for ISOC EP */
	unsigned int even_odd_frame	: 1;

	/** EP number used for register address lookup */
	uint8_t num;

	/** EP type: 0 - Control, 1 - ISOC, 2 - BULK, 3 - INTR */
	uint8_t type;

	/** Max Packet bytes */
	uint16_t maxpacket;

	/** 'bInterval' value for Isoc EPs */
	uint8_t intvl;

	/** 'mult' value for SS Isoc EPs */
	uint8_t mult;

	/** Max burst size for this EP (0 - 15, actual burst is 1 - 16) */
	uint8_t maxburst;

	/** Number of streams for this EP (0 - 16, actual number is 2^n) */
	uint8_t num_streams;

	/** Tx FIFO # for IN EPs */
	uint8_t tx_fifo_num;

	/** @{ */
	/** The Transfer Resource Index from the Start Transfer command */
	uint8_t tri_out;
	uint8_t tri_in;
	uint8_t tri_out_initialize;
	uint8_t tri_in_initialize;
	/** @} */

	/** Condition variable for EPCMD_CMPL interrupt */
	uint32_t condition;

	/** The array of DMA descriptors for this EP */
	char *dma_desc;
	dwc_dma_t dma_desc_dma;
	int dma_desc_size;
	int num_desc;
	int desc_avail;
	int desc_idx;
} dwc_ep_t;

/** PCD EP structure.
 * This structure describes an EP, there is an array of EP pointers in the
 * PCD structure.
 * WARNING! This struct must be kept in sync with struct dwc_usb3_pcd_ep_real
 * in pcd_linux.c.
 * WARNING! This struct must never be used in a non-pointer context, since the
 * size of this struct is incorrect due to the dummy 'usb_ep' member. Any non-
 * pointer use must be done with the OS-specific 'real' version of this struct
 * (struct dwc_usb3_pcd_ep_real) in pcd_linux.c.
 */
typedef struct dwc_usb3_pcd_ep {
	/** DWC_usb3 EP data */
	dwc_ep_t dwc_ep;

	/** OS-specific USB EP (opaque here, defined in pcd_linux.c) */
	struct { void *dummy; } usb_ep;
} dwc_usb3_pcd_ep_t;

#undef dwc_usb3_get_pcd_ep

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#define dwc_usb3_get_pcd_ep(usb_ep) \
	container_of(usb_ep, dwc_usb3_pcd_ep_t, usb_ep)

/** DWC_usb3 PCD Structure.
 * This structure encapsulates the data for the dwc_usb3 PCD.
 */
typedef struct dwc_usb3_pcd {
	/** The PCD function ops */
	//struct dwc_usb3_pcd_function_ops *fops;

	/** The DWC otg device pointer */
	struct dwc_usb3_device *usb3_dev;

	/** USB3 link state */
	int link_state;

	/** State of EP0 */
	ep0state_e ep0state;

	/** EP0 Status Request is pending */
	unsigned int ep0_status_pending		: 1;

	/** Indicates when SET CONFIGURATION Request is in process */
	unsigned int request_config		: 1;

	/** State of the Remote Wakeup Enable */
	unsigned int remote_wakeup_enable	: 1;

	/** True if ready for remote wakeup request from user */
	unsigned int wkup_rdy			: 1;

	/** Flag that shows we have enabled some EPs */
	unsigned int eps_enabled		: 1;

	unsigned int pad			: 3;
	unsigned int devnum			: 4;

	/** Count of pending Requests */
	unsigned int request_pending;

	/** EP0 */
	dwc_usb3_pcd_ep_t *ep0;

	/** Array of OUT EPs (not including EP0) */
	dwc_usb3_pcd_ep_t *out_ep[DWC_MAX_EPS - 1];

	/** Array of IN EPs (not including EP0) */
	dwc_usb3_pcd_ep_t *in_ep[DWC_MAX_EPS - 1];

	/** The spinlock for the PCD */
	//dwc_spinlock_t *lock;

	/** Tasklet to defer starting of TEST mode transmissions until
	 * Status Phase has been completed
	 */
	//dwc_tasklet_t *test_mode_tasklet;

	/** Tasklet to delay starting of xfer in DMA mode */
	//dwc_tasklet_t *start_xfer_tasklet;

	/** The test mode to enter when the tasklet is executed */
	int test_mode;

	/** Pointer to device Global registers.
	 * Device Global Registers starting at offset 800h
	 */
	dwc_usb3_dev_global_regs_t *dev_global_regs;

	/** Device Logical OUT Endpoint-Specific Registers 600h-7FCh */
	dwc_usb3_dev_ep_regs_t *out_ep_regs;

	/** Device Logical IN Endpoint-Specific Registers 300h-4FCh */
	dwc_usb3_dev_ep_regs_t *in_ep_regs;

	/* Device configuration information */
	uint8_t speed;		 /**< Device Speed - 0:Unk 1:LS 2:FS 3:HS 4:Var 5:SS */
	uint8_t num_out_eps;	 /**< Number # of Rx EP range: 0-15 except ep0 */
	uint8_t num_in_eps;	 /**< Number # of Tx EP range: 0-15 except ep0 */

	/** Size of Tx FIFOs (Bytes) */
	uint16_t tx_fifo_size[DWC_MAX_TX_FIFOS];

	/** @{ */
	/** Thresholding enable flags and length variables */
	uint16_t rx_thr_en;
	uint16_t iso_tx_thr_en;
	uint16_t non_iso_tx_thr_en;

	uint16_t rx_thr_length;
	uint16_t tx_thr_length;
	/** @} */

	/** SETUP packet buffer for EP0.
	 * This structure is allocated as a DMA buffer on PCD initialization.
	 */
	union {
		usb_device_request_t req;
		uint32_t d32[2];
	} *setup_pkt;

	dwc_dma_t setup_pkt_dma;

	/** dma buffer used to return data for GET_STATUS and
	    GET_DESCRIPTOR(BOS) up to 512 bytes in length. */
	uint8_t *status_buf;
#define CTRL_BUF_SIZE 512

	dwc_dma_t status_buf_dma;

	/**
	 * Pointers to the DMA Descriptors for EP0 Control
	 * transfers (virtual and physical)
	 */

	/** Descriptor for SETUP packets */
	dwc_dma_t dma_setup_desc_addr;
	dwc_usb3_dma_desc_t *setup_desc_addr;

	/** Descriptor for Data Out or Status Out phases */
	dwc_dma_t dma_out_desc_addr;
	dwc_usb3_dma_desc_t *out_desc_addr;

	/** Descriptor for Data In or Status In phases */
	dwc_dma_t dma_in_desc_addr;
	dwc_usb3_dma_desc_t *in_desc_addr;

	/*ep(excluded ep0) out data memroy temp, for the CBW*/
	dwc_dma_t *ep_out_data_addr_temp;

	/** 'dummy' request, for EP0 only */
	dwc_usb3_pcd_request_t *ep0_req;
} dwc_usb3_pcd_t;

extern int dwc_usb3_xmit_fn_remote_wake(dwc_usb3_pcd_t *pcd, uint32_t intf);
extern int dwc_usb3_dep_cfg(dwc_usb3_pcd_t *pcd, dwc_usb3_dev_ep_regs_t *ep_reg,
			    uint32_t depcfg0, uint32_t depcfg1);
extern int dwc_usb3_dep_xfercfg(dwc_usb3_pcd_t *pcd,
				dwc_usb3_dev_ep_regs_t *ep_reg,
				uint32_t depstrmcfg);
extern int dwc_usb3_dep_sstall(dwc_usb3_pcd_t *pcd,
			       dwc_usb3_dev_ep_regs_t *ep_reg);
extern int dwc_usb3_dep_cstall(dwc_usb3_pcd_t *pcd,
			       dwc_usb3_dev_ep_regs_t *ep_reg);
extern int dwc_usb3_dep_startxfer(dwc_usb3_pcd_t *pcd,
				  dwc_usb3_dev_ep_regs_t *ep_reg,
				  dwc_dma_t dma_addr, uint32_t stream_or_uf);
extern int dwc_usb3_dep_updatexfer(dwc_usb3_pcd_t *pcd,
				   dwc_usb3_dev_ep_regs_t *ep_reg,
				   uint32_t tri);
extern int dwc_usb3_dep_endxfer(dwc_usb3_pcd_t *pcd,
				dwc_usb3_dev_ep_regs_t *ep_reg,
				uint32_t tri, void *condition);
extern int dwc_usb3_dep_startnewcfg(dwc_usb3_pcd_t *pcd,
				    dwc_usb3_dev_ep_regs_t *ep_reg,
				    uint32_t rsrcidx);
extern void dwc_usb3_ep0_activate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_activate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_deactivate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_start_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				       dwc_usb3_pcd_request_t *req);
extern void dwc_usb3_ep_start_zl_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep0_start_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
					dwc_usb3_pcd_request_t *req);
extern void dwc_usb3_ep0_continue_transfer(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_set_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_clear_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
//extern void dwc_usb3_dump_dev_registers(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_start_next_request(dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_request_done(dwc_usb3_pcd_ep_t *ep,
				  dwc_usb3_pcd_request_t *req, int status);
extern void dwc_usb3_request_nuke(dwc_usb3_pcd_ep_t *ep);
extern int dwc_usb3_handle_out_ep_intr(dwc_usb3_pcd_t *pcd, int epnum,
				       uint32_t epint);
extern int dwc_usb3_handle_in_ep_intr(dwc_usb3_pcd_t *pcd, int epnum,
				      uint32_t epint);
extern int dwc_usb3_handle_dev_intr(dwc_usb3_pcd_t *pcd, uint32_t dint);

#endif /* _DWC_PCD_H_ */
