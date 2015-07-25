/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/compiler.h>
#include <version.h>
#include <g_dnl.h>
#include "f_fastboot.h"

#undef debug
#define debug printf

DECLARE_GLOBAL_DATA_PTR;

#define FASTBOOT_VERSION		"0.4"

#define FASTBOOT_INTERFACE_CLASS	0xff
#define FASTBOOT_INTERFACE_SUB_CLASS	0x42
#define FASTBOOT_INTERFACE_PROTOCOL	0x03

#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0  (0x0200)
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1  (0x0040)
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE      (0x0040)

/* The 64 defined bytes plus \0 */
#define RESPONSE_LEN	(64 + 1)

#define EP_BUFFER_SIZE			4096

struct f_fastboot {
	struct usb_function usb_function;

	/* IN/OUT EP's and correspoinding requests */
	struct usb_ep *in_ep, *out_ep;
	struct usb_request *in_req, *out_req;
};

static inline struct f_fastboot *func_to_fastboot(struct usb_function *f)
{
	return container_of(f, struct f_fastboot, usb_function);
}

static struct f_fastboot *fastboot_func;
static unsigned int download_size;
static unsigned int download_bytes;

static struct usb_endpoint_descriptor fs_ep_in = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize     = TX_ENDPOINT_MAXIMUM_PACKET_SIZE,
	.bInterval          = 0x00,
};

static struct usb_endpoint_descriptor fs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1,
	.bInterval		= 0x00,
};

static struct usb_endpoint_descriptor hs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0,
	.bInterval		= 0x00,
};

static struct usb_interface_descriptor interface_desc = {
	.bLength		= USB_DT_INTERFACE_SIZE,
	.bDescriptorType	= USB_DT_INTERFACE,
	.bInterfaceNumber	= 0x00,
	.bAlternateSetting	= 0x00,
	.bNumEndpoints		= 0x02,
	.bInterfaceClass	= FASTBOOT_INTERFACE_CLASS,
	.bInterfaceSubClass	= FASTBOOT_INTERFACE_SUB_CLASS,
	.bInterfaceProtocol	= FASTBOOT_INTERFACE_PROTOCOL,
};

static struct usb_descriptor_header *fb_runtime_descs[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&fs_ep_in,
	(struct usb_descriptor_header *)&hs_ep_out,
	NULL,
};

/*
 * static strings, in UTF-8
 */
static const char fastboot_name[] = "Android Fastboot";

static struct usb_string fastboot_string_defs[] = {
	[0].s = fastboot_name,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_fastboot = {
	.language	= 0x0409,	/* en-us */
	.strings	= fastboot_string_defs,
};

static struct usb_gadget_strings *fastboot_strings[] = {
	&stringtab_fastboot,
	NULL,
};

/**********flash partion write command*********/
struct flash_cmd {
	const char *cmd_name;
	int devNo;
	int	(*do_write)(struct flash_cmd *);
};

static int image_write(struct flash_cmd *cmd_data);
static int boot_write(struct flash_cmd *cmd_data);

static struct flash_cmd boot_cmd = {
	.cmd_name = "boot",
	.devNo = 0,
	.do_write = boot_write,
};
static struct flash_cmd misc_cmd = {
	.cmd_name = "misc",
	.devNo = 1,
	.do_write = image_write,

};
static struct flash_cmd recovery_cmd = {
	.cmd_name = "recovery",
	.devNo = 2,
	.do_write = image_write,

};
static struct flash_cmd system_cmd = {
	.cmd_name = "system",
	.devNo = 3,
	.do_write = image_write,

};
static struct flash_cmd vendor_cmd = {
	.cmd_name = "vendor",
	.devNo = 8,
	.do_write = image_write,

};
static struct flash_cmd media_cmd = {
	.cmd_name = "media",
	.devNo = 9,
	.do_write = image_write,

};


static struct flash_cmd *st_flash_cmd[] = {
	(struct flash_cmd *)&boot_cmd,
	(struct flash_cmd *)&misc_cmd,
	(struct flash_cmd *)&recovery_cmd,
	(struct flash_cmd *)&system_cmd,
	(struct flash_cmd *)&vendor_cmd,
	(struct flash_cmd *)&media_cmd,	
	NULL,
};

#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512

struct boot_img_hdr
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */
    
    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
};

/********** end*********/




static void rx_handler_command(struct usb_ep *ep, struct usb_request *req);

static void fastboot_complete(struct usb_ep *ep, struct usb_request *req)
{
	int status = req->status;
	if (!status)
		return;
	printf("status: %d ep '%s' trans: %d\n", status, ep->name, req->actual);
}

static int fastboot_bind(struct usb_configuration *c, struct usb_function *f)
{
	int id;
	struct usb_gadget *gadget = c->cdev->gadget;
	struct f_fastboot *f_fb = func_to_fastboot(f);

	/* DYNAMIC interface numbers assignments */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	interface_desc.bInterfaceNumber = id;

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;
	fastboot_string_defs[0].id = id;
	interface_desc.iInterface = id;

	f_fb->in_ep = usb_ep_autoconfig(gadget, &fs_ep_in);
	if (!f_fb->in_ep)
		return -ENODEV;
	f_fb->in_ep->driver_data = c->cdev;

	f_fb->out_ep = usb_ep_autoconfig(gadget, &fs_ep_out);
	if (!f_fb->out_ep)
		return -ENODEV;
	f_fb->out_ep->driver_data = c->cdev;

	hs_ep_out.bEndpointAddress = fs_ep_out.bEndpointAddress;

	return 0;
}

static void fastboot_unbind(struct usb_configuration *c, struct usb_function *f)
{
	memset(fastboot_func, 0, sizeof(*fastboot_func));
}

static void fastboot_disable(struct usb_function *f)
{
	struct f_fastboot *f_fb = func_to_fastboot(f);

	usb_ep_disable(f_fb->out_ep);
	usb_ep_disable(f_fb->in_ep);

	if (f_fb->out_req) {
		free(f_fb->out_req->buf);
		usb_ep_free_request(f_fb->out_ep, f_fb->out_req);
		f_fb->out_req = NULL;
	}
	if (f_fb->in_req) {
		free(f_fb->in_req->buf);
		usb_ep_free_request(f_fb->in_ep, f_fb->in_req);
		f_fb->in_req = NULL;
	}
}

static struct usb_request *fastboot_start_ep(struct usb_ep *ep)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, 0);
	if (!req)
		return NULL;

	req->length = EP_BUFFER_SIZE;
	req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, EP_BUFFER_SIZE);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	memset(req->buf, 0, req->length);
	return req;
}

static int fastboot_set_alt(struct usb_function *f,
			    unsigned interface, unsigned alt)
{
	int ret;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_gadget *gadget = cdev->gadget;
	struct f_fastboot *f_fb = func_to_fastboot(f);

	debug("%s: func: %s intf: %d alt: %d\n",
	      __func__, f->name, interface, alt);

	/* make sure we don't enable the ep twice */
	if (gadget->speed == USB_SPEED_HIGH)
		ret = usb_ep_enable(f_fb->out_ep, &hs_ep_out);
	else
		ret = usb_ep_enable(f_fb->out_ep, &fs_ep_out);
	if (ret) {
		puts("failed to enable out ep\n");
		return ret;
	}

	f_fb->out_req = fastboot_start_ep(f_fb->out_ep);
	if (!f_fb->out_req) {
		puts("failed to alloc out req\n");
		ret = -EINVAL;
		goto err;
	}
	f_fb->out_req->complete = rx_handler_command;

	ret = usb_ep_enable(f_fb->in_ep, &fs_ep_in);
	if (ret) {
		puts("failed to enable in ep\n");
		goto err;
	}

	f_fb->in_req = fastboot_start_ep(f_fb->in_ep);
	if (!f_fb->in_req) {
		puts("failed alloc req in\n");
		ret = -EINVAL;
		goto err;
	}
	f_fb->in_req->complete = fastboot_complete;

	ret = usb_ep_queue(f_fb->out_ep, f_fb->out_req, 0);
	if (ret)
		goto err;

	return 0;
err:
	fastboot_disable(f);
	return ret;
}

 int fastboot_add(struct usb_configuration *c)
{
	struct f_fastboot *f_fb = fastboot_func;
	int status;

	debug("%s: cdev: 0x%p\n", __func__, c->cdev);

	if (!f_fb) {
		f_fb = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*f_fb));
		if (!f_fb)
			return -ENOMEM;

		fastboot_func = f_fb;
		memset(f_fb, 0, sizeof(*f_fb));
	}

	f_fb->usb_function.name = "f_fastboot";
	f_fb->usb_function.hs_descriptors = fb_runtime_descs;
	f_fb->usb_function.bind = fastboot_bind;
	f_fb->usb_function.unbind = fastboot_unbind;
	f_fb->usb_function.set_alt = fastboot_set_alt;
	f_fb->usb_function.disable = fastboot_disable;
	f_fb->usb_function.strings = fastboot_strings;

	status = usb_add_function(c, &f_fb->usb_function);
	if (status) {
		free(f_fb);
		fastboot_func = f_fb;
	}

	return status;
}


int fastboot_tx_write(const char *buffer, unsigned int buffer_size)
{
	struct usb_request *in_req = fastboot_func->in_req;
	int ret;

	memcpy(in_req->buf, buffer, buffer_size);
	in_req->length = buffer_size;
	ret = usb_ep_queue(fastboot_func->in_ep, in_req, 0);
	if (ret)
		printf("Error %d on queue\n", ret);
	return 0;
}

static int fastboot_tx_write_str(const char *buffer)
{
	printf("fb tx:%s\n", buffer);
	return fastboot_tx_write(buffer, strlen(buffer));
}

static void compl_do_reset(struct usb_ep *ep, struct usb_request *req)
{
	do_reset(NULL, 0, 0, NULL);
}

static void cb_reboot(struct usb_ep *ep, struct usb_request *req)
{
	fastboot_func->in_req->complete = compl_do_reset;
	fastboot_tx_write_str("OKAY");
}

static int strcmp_l1(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;
	return strncmp(s1, s2, strlen(s1));
}
static unsigned int max_dl_buf(void)
{
	unsigned int maxSize = gd->start_addr_sp - 0x100000 - CONFIG_USB_FASTBOOT_BUF_ADDR;
	return  max(CONFIG_USB_FASTBOOT_BUF_SIZE, maxSize);
}


static void cb_getvar(struct usb_ep *ep, struct usb_request *req)
{
	char *cmd = req->buf;
	char response[RESPONSE_LEN];
	const char *s;
	size_t chars_left;

	strcpy(response, "OKAY");
	chars_left = sizeof(response) - strlen(response) - 1;

	strsep(&cmd, ":");
	if (!cmd) {
		fastboot_tx_write_str("FAILmissing var");
		return;
	}

	if (!strcmp_l1("version", cmd)) {
		strncat(response, FASTBOOT_VERSION, chars_left);
	} else if (!strcmp_l1("bootloader-version", cmd)) {
		strncat(response, U_BOOT_VERSION, chars_left);
	} else if (!strcmp_l1("downloadsize", cmd) 
			|| !strcmp_l1("max-download-size", cmd) ) {
		char str_num[14];
		sprintf(str_num, "0x%08x", max_dl_buf());
		strncat(response, str_num, chars_left);
	}  else if (!strcmp_l1("partition-type:", cmd) ) {
		char str_num[14];
		strcpy(str_num, "OK");
		strncat(response, str_num, chars_left);
	}else if (!strcmp_l1("serialno", cmd)) { 
		s = getenv("serial#");
		if (s)
			strncat(response, s, chars_left);
		else
			strcpy(response, "FAILValue not set");
	} else {
		strcpy(response, "FAILVariable not implemented");
	}
	fastboot_tx_write_str(response);
}

static unsigned int rx_bytes_expected(void)
{
	int rx_remain = download_size - download_bytes;
	if (rx_remain < 0)
		return 0;
	if (rx_remain > EP_BUFFER_SIZE)
		return EP_BUFFER_SIZE;
	return rx_remain;
}

#define BYTES_PER_DOT	0x100000
static void rx_handler_dl_image(struct usb_ep *ep, struct usb_request *req)
{
	char response[RESPONSE_LEN];
	unsigned int transfer_size = download_size - download_bytes;
	const unsigned char *buffer = req->buf;
	unsigned int buffer_size = req->actual;

	if (req->status != 0) {
		printf("Bad status: %d\n", req->status);
		return;
	}

	if (buffer_size < transfer_size)
		transfer_size = buffer_size;

	memcpy((void *)CONFIG_USB_FASTBOOT_BUF_ADDR + download_bytes,
	       buffer, transfer_size);
	
	download_bytes += transfer_size;
	
	//if ( download_size >  download_bytes -0x10000 )
		//printf("dl_size=%d, d=%d\n", download_bytes, transfer_size);

	/* Check if transfer is done */
	if (download_bytes >= download_size) {
		/*
		 * Reset global transfer variable, keep download_bytes because
		 * it will be used in the next possible flashing command
		 */
		download_size = 0;
		req->complete = rx_handler_command;
		req->length = EP_BUFFER_SIZE;

		sprintf(response, "OKAY");
		fastboot_tx_write_str(response);

		printf("\ndownloading of %d bytes finished\n", download_bytes);
	} else {
		req->length = rx_bytes_expected();
		if (req->length < ep->maxpacket)
			req->length = ep->maxpacket;
	}

	if (download_bytes && !(download_bytes % BYTES_PER_DOT)) {
		putc('.');
		if (!(download_bytes % (74 * BYTES_PER_DOT)))
			putc('\n');
	}
	req->actual = 0;	
	usb_ep_queue(ep, req, 0);
	
}

static void cb_download(struct usb_ep *ep, struct usb_request *req)
{
	char *cmd = req->buf;
	char response[RESPONSE_LEN];
	unsigned int maxSize = max_dl_buf();

	strsep(&cmd, ":");
	download_size = simple_strtoul(cmd, NULL, 16);
	download_bytes = 0;


	printf("Starting download of %d bytes, space=%d\n", download_size, maxSize);

	if (0 == download_size) {
		sprintf(response, "FAILdata invalid size");
	} else if (download_size > maxSize) {
		download_size = 0;
		sprintf(response, "FAILdata too large");
	} else {
		sprintf(response, "DATA%08x", download_size);
		req->complete = rx_handler_dl_image;
		req->length = rx_bytes_expected();
		if (req->length < ep->maxpacket)
			req->length = ep->maxpacket;
	}
	fastboot_tx_write_str(response);
}

static void do_bootm_on_complete(struct usb_ep *ep, struct usb_request *req)
{
	//char boot_addr_start[12];
	//char *bootm_args[] = { "bootm", boot_addr_start, NULL };

	//puts("Booting kernel..\n");

	//sprintf(boot_addr_start, "0x%lx", load_addr);
	//do_bootm(NULL, 0, 2, bootm_args);

	/* This only happens if image is somehow faulty so we start over */
	//do_reset(NULL, 0, 0, NULL);
	
    unsigned kernel_addr;
    unsigned ramdisk_addr;
    struct boot_img_hdr hdr;
	char boot_kernel[12],boot_ramdisk[12],boot_fdt[12] ;
	char *bootm_args[] = { "bootm", boot_kernel,boot_ramdisk, boot_fdt, NULL };	
    char *ptr = ((char*) CONFIG_USB_FASTBOOT_BUF_ADDR);
	unsigned page_mask;

    if (download_bytes < sizeof(hdr)) {
		fastboot_tx_write_str("FAILbootheader");        
        return;
    }
    memcpy(&hdr, ptr, sizeof(hdr));
	page_mask = hdr.page_size - 1;
    /* ensure commandline is terminated */
    hdr.cmdline[BOOT_ARGS_SIZE-1] = 0;
	hdr.magic[BOOT_MAGIC_SIZE-1] = 0;
	printf("hdr:magic=%s,k=%d,rd=%d,all=%d\n",hdr.magic,
		hdr.kernel_size,hdr.ramdisk_size,download_bytes );

    kernel_addr = CONFIG_USB_FASTBOOT_BUF_ADDR+hdr.page_size;
    ramdisk_addr = CONFIG_USB_FASTBOOT_BUF_ADDR+hdr.page_size
					+( (hdr.kernel_size+page_mask)&(~page_mask) );

    if (2048 + hdr.kernel_size + hdr.ramdisk_size> download_bytes) {
        printf("incomplete bootimage");
		fastboot_tx_write_str("FAIL");  
        return;
    }
	memmove(( void *) 0xffffc0,(void*) ramdisk_addr , hdr.ramdisk_size);
	sprintf(boot_kernel, "0x%x", kernel_addr);
	sprintf(boot_ramdisk, "0x%x", 0xffffc0);
	sprintf(boot_fdt, "0x%x", (unsigned int)gd->fdt_blob);
	printf("boot kernel,%s,%s,%s..\n",boot_kernel,boot_ramdisk, boot_fdt);
	do_bootm(NULL, 0, 4, bootm_args);

	
}

static void cb_boot(struct usb_ep *ep, struct usb_request *req)
{
	fastboot_func->in_req->complete = do_bootm_on_complete;
	fastboot_tx_write_str("OKAY");
}


static int image_write(struct flash_cmd *cmd_data)
{
	const char *ifname;
	block_dev_desc_t *dev_desc;
	unsigned int part_size, len, wrlen, blknum, ret;

	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, (512*32));
	
	ifname = getenv("devif");
	if ( ifname == NULL) {
		printf("fastboot: get devif fail\n");
		return -1;
	}
	
	dev_desc = get_dev(ifname, cmd_data->devNo);
	if ( dev_desc == NULL) {
		printf("Failed to find %s:%d\n", ifname, cmd_data->devNo);
		return -1;
	}	
	part_size =  dev_desc->lba* dev_desc->blksz;
	printf("if=%s,dev=%d,size=%d\n",ifname, cmd_data->devNo, part_size );
	if (download_bytes == 0 || download_bytes > part_size ) {
		printf("image_write: size err,%d:%d\n", download_bytes, part_size);
		return -1;
	}
	len = 0;
	
	while ( len < download_bytes ) {
		wrlen =  download_bytes - len;
		if ( wrlen > 512*32 )
			wrlen = 512*32;
		memcpy(buffer, (void *)(CONFIG_USB_FASTBOOT_BUF_ADDR+len), wrlen);
		blknum = (wrlen+dev_desc->blksz-1)/dev_desc->blksz;		

		ret = dev_desc->block_write(dev_desc->dev, len/dev_desc->blksz,
										blknum, buffer);
		
		if ( ret != blknum ) {
			printf("\nimage_write: wrerr,%d:%d\n", ret, blknum);
			return -1;
		}		
		len += wrlen;	
		if (len && !(len % BYTES_PER_DOT)) {
			putc('.');
			if (!(len % (74 * BYTES_PER_DOT)))
				putc('\n');
		}		
	}
	
	printf("\nimage_write ok, len=%d\n",len);
	
	return 0;
}
static unsigned int dwchecksum(char * buf, int start, int length)
{
    unsigned int sum = 0;
    unsigned int * tmp_buf = (unsigned int *)buf;
    int tmp_start = start / sizeof(unsigned int);
    int tmp_length = length / sizeof(unsigned int);
    int i;

    for(i = tmp_start; i < tmp_length; i++)
    {
        sum = sum + *(tmp_buf + i);
    }
 
    return sum;
}
static unsigned short wchecksum(char *buf, int start, int length)
{
	unsigned int sum = 0;
	unsigned short *tmp_buf = (unsigned short *)buf;
	int tmp_start = start / sizeof(unsigned short);
	int tmp_length = length / sizeof(unsigned short);
	int i;

	for(i = tmp_start + tmp_length -1; i >= tmp_start; i--)
		sum += *(tmp_buf + i);

	sum = sum & 0xffff;

	return sum;
}
#ifdef  CONFIG_OWLXX_NAND
extern int LDL_DeviceOpBootSectors(unsigned int rw, unsigned int start, unsigned int nsector, void *buf, int partitionNo);
extern int LDL_DeviceOpWriteBootFlush(int partitionNo);

static int nandboot_read(int start, int nblk, void *buf)
{
	printf("nand boot read\n");
	if (0 == LDL_DeviceOpBootSectors(0, start, nblk, buf, 0 ) )
		return nblk;
	else
		return -1;
}
static int nandboot_write(int start, int nblk, void *buf)
{
	int i;
	printf("nand boot write\n");
	for (i = 0; i < 4; i++ ) {
		if (0 == LDL_DeviceOpBootSectors(1, start, nblk, buf, 0 ) ) {
			LDL_DeviceOpWriteBootFlush(0);
			printf("nand boot write ok\n");			
		}else {
			printf("nand boot write err\n");
			return -1;
		}
	}
	return nblk;

}
#else
static int nandboot_read(int start, int nblk, void *buf){return -1;}

static int nandboot_write(int start, int nblk, void *buf){return -1;}

#endif
static int mmcboot_read(block_dev_desc_t *dev_desc, int start, int nblk, void *buf)
{
	return dev_desc->block_read(dev_desc->dev, start,
										nblk, buf);	

}
static int mmcboot_write(block_dev_desc_t *dev_desc, int start, int nblk, void *buf)
{
	return dev_desc->block_write(dev_desc->dev, start,
										nblk, buf);	

}
static void dump_buf(u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%02x ", *(buf + i));
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}

static int boot_write(struct flash_cmd *cmd_data)
{
	const char *ifname;	
	char *pdat; 
	int ret,  bootdev = 0; // default  is nand
	block_dev_desc_t *dev_desc;
	unsigned int  mbrc_len, sec_mbrc_len, trd_mbrc_len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, (512*128));	

	mbrc_len = (download_bytes+100+4095)& (~4095);	
	pdat = (char*) CONFIG_USB_FASTBOOT_BUF_ADDR;
	sec_mbrc_len = *((unsigned int*)(CONFIG_USB_FASTBOOT_BUF_ADDR +0x28));
	if ( mbrc_len < sec_mbrc_len + 0x400 ) {
		printf("mbrc err: secMbrc=%d, mbrc_len=%d\n",sec_mbrc_len, mbrc_len);
		return -1;
	}
	trd_mbrc_len = mbrc_len - sec_mbrc_len - 0x400;
	printf("mbrc:all=%x, sec_mbrc=%x, trd_mbrc=%x\n",mbrc_len,sec_mbrc_len,trd_mbrc_len );

	if ( memcmp(pdat +0x400 + 0x50, "AFI", 3) ) {
		printf("boot: afi err\n");
		return -1;
	}
	
	ifname = getenv("devif");
	if ( ifname == NULL) {
		printf("fastboot: get devif fail\n");
		return -1;
	}
	if (memcmp(ifname, "nand", 4) ) { //is not nand
		bootdev = 1;
		dev_desc = get_dev(ifname, cmd_data->devNo);
		if ( dev_desc == NULL) {
			printf("Failed to find %s:%d\n", ifname, cmd_data->devNo);
			return -1;
		}			
	}
	if ( bootdev )
		ret = mmcboot_read(dev_desc, 1, 128, buffer );
	else
		ret = nandboot_read(0, 128, buffer );
	
	if ( ret != 128 )
			return -1;
	
	if (memcmp(&buffer[0x400 + 0x50], "AFI", 3) ) {
		printf("read boot: afi err\n");
		dump_buf(buffer, 0x800);
		return -1;
	}
	
	
	/*cpy afi*/
	memcpy(pdat +0x400 + 0x50, &buffer[0x400 + 0x50], 0x800); 
	/*cpy nand para*/
	memcpy(pdat +0x08, &buffer[0x08], 20); 
	memcpy(pdat +0x30, &buffer[0x30], 12); 
	/*boot size*/
	memcpy(pdat +0x2c, &trd_mbrc_len, 4); 

	/*config*/
	    /* write first stage "ActBrm55aa" and check sum */
    sprintf(pdat + 500,"ActBrm%c%c",0xaa,0x55);
    *((unsigned int *)(pdat + 508)) = dwchecksum(pdat, 0, 508)+0x1234;
	
	if ( bootdev == 0 ) { // nand
    /* write second stage 0x55aa and check sum*/
    	sprintf(pdat + 0x400 + sec_mbrc_len - 4 ,"%c%c",0xaa,0x55);        /* 除掉第3阶段的16k数据，即0x4000 */
     	*((unsigned short *)(pdat + 0x400 + sec_mbrc_len - 2)) = wchecksum(pdat + 0x400, 0, sec_mbrc_len - 2);
 
		
	    sprintf(pdat + mbrc_len - 4,"%c%c",0xaa,0x55);	
	    *((unsigned short *)(pdat+mbrc_len -2)) = wchecksum(pdat + 0x400 + sec_mbrc_len, 0, trd_mbrc_len - 2);

		nandboot_write(0, mbrc_len/512, pdat);
	} else {
		/* write second stage 0x55aa and check sum*/
		sprintf(pdat + 0x400 + sec_mbrc_len - 6 ,"%c%c",0xaa,0x55);		 /* 除掉第3阶段的16k数据，即0x4000 */
		*((unsigned int *)(pdat + 0x400 + sec_mbrc_len - 4)) = dwchecksum(pdat + 0x400, 0, sec_mbrc_len - 4);


		sprintf(pdat + mbrc_len - 6,"%c%c",0xaa,0x55);  
		*((unsigned int *)(pdat+mbrc_len -4)) = dwchecksum(pdat + 0x400 + sec_mbrc_len, 0, trd_mbrc_len - 4);
		mmcboot_write(dev_desc, 0, mbrc_len/512, pdat);
	}	
	printf("\nboot_write ok\n");
	return 0;
}



static void cb_flash(struct usb_ep *ep, struct usb_request *req)
{
	char *cmd = req->buf;
	struct flash_cmd *pcmd;	
	int i;
	strsep(&cmd, ":");
	if (!cmd) {
		printf("cmd err\n");
		fastboot_tx_write_str("FAILcmd err");
		return;
	}
	for ( i=0; (pcmd=st_flash_cmd[i]) != NULL ; i++ ) {		
		if (!strcmp_l1(pcmd->cmd_name, cmd)) 
			break;
	}
	if ( pcmd == NULL ) {
		fastboot_tx_write_str("FAILnocmd");
		printf("cmd not eixt\n");
		return;
	}
	
	printf("flash writing ...\n");
	if ( pcmd->do_write(pcmd) ) {
		fastboot_tx_write_str("FAILwrite fail");
	} else {
		fastboot_tx_write_str("OKAY");
	}
}


struct cmd_dispatch_info {
	char *cmd;
	void (*cb)(struct usb_ep *ep, struct usb_request *req);
};

static const struct cmd_dispatch_info cmd_dispatch_info[] = {
	{
		.cmd = "reboot",
		.cb = cb_reboot,
	}, {
		.cmd = "getvar:",
		.cb = cb_getvar,
	}, {
		.cmd = "download:",
		.cb = cb_download,
	}, {
		.cmd = "boot",
		.cb = cb_boot,
	},{
		.cmd = "flash",
		.cb = cb_flash,
	},	
};

static void rx_handler_command(struct usb_ep *ep, struct usb_request *req)
{
	char *cmdbuf = req->buf;
	void (*func_cb)(struct usb_ep *ep, struct usb_request *req) = NULL;
	int i;
	printf("fb:cmd %s, len=%d\n", cmdbuf, req->actual);
	for (i = 0; i < ARRAY_SIZE(cmd_dispatch_info); i++) {
		if (!strcmp_l1(cmd_dispatch_info[i].cmd, cmdbuf)) {
			func_cb = cmd_dispatch_info[i].cb;
			break;
		}
	}

	if (!func_cb)
		fastboot_tx_write_str("FAILunknown command");
	else
		func_cb(ep, req);

	if (req->status == 0) {
		//*cmdbuf = '\0';
		memset(cmdbuf, 0, 64);
		req->actual = 0;
		usb_ep_queue(ep, req, 0);
	}
}
