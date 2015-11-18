/*
 * Actions OWL SoCs xhci driver
 *
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * tangshaoqing <tangshaoqing@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <usb.h>
#include <watchdog.h>
#include <asm/gpio.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>

#include <power/atc260x/owl_atc260x.h>

#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;


#define PMU_APDS_CTL0 0x11
#define VBUS_OTG 9


#ifndef CONFIG_DM_USB

#define GPIOD_OUT_EN	0xe01b0024
#define GPIOD_IN_EN	0xe01b0028
#define GPIOD_DAT	0xe01b002C
#define GPIOD1	(1<<1)

#define MFP_CTL1	0xe01b0044
#define LVDS_OXX_DIG	(1<<22)

void vbus_enable(void)
{
	void __iomem *reg;
	u32 val;

	atc260x_reg_setbits(PMU_APDS_CTL0, 1<<VBUS_OTG, 1<<VBUS_OTG);

	reg = (void __iomem *)GPIOD_OUT_EN;
	val = readl(reg);
	val |= GPIOD1;
	writel(val, reg);

	reg = (void __iomem *)GPIOD_IN_EN;
	val = readl(reg);
	val &= (~GPIOD1);
	writel(val, reg);

	reg = (void __iomem *)GPIOD_DAT;
	val = readl(reg);
	val |= GPIOD1;
	writel(val, reg);

	reg = (void __iomem *)MFP_CTL1;
	val = readl(reg);
	val |= LVDS_OXX_DIG;
	writel(val, reg);
}

void vbus_disable(void)
{
	void __iomem *reg;
	u32 val;

	reg = (void __iomem *)GPIOD_DAT;
	val = readl(reg);
	val &= (~GPIOD1);
	writel(val, reg);

	atc260x_reg_setbits(PMU_APDS_CTL0, 1<<VBUS_OTG, 0<<VBUS_OTG);
}


#define OWL_XHCI_BASE 0xe0400000

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	int ret = 0;

	vbus_enable();

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)(OWL_XHCI_BASE);
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("xhci-owl: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	board_usb_cleanup(index, USB_INIT_HOST);
	vbus_disable();
}

#endif




#ifdef CONFIG_DM_USB

struct owl_xhci_platdata {
	fdt_addr_t hcd_base;
	struct gpio_desc vbus_gpio;
};

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct owl_xhci {
	struct usb_platdata usb_plat;
	struct xhci_ctrl ctrl;
	struct xhci_hccr *hcd;
};

static int xhci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct owl_xhci_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	unsigned int node;
	int depth;

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	plat->hcd_base = fdtdec_get_addr(blob, dev->of_offset, "reg");
	if (plat->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	/* Vbus gpio */
	gpio_request_by_name(dev, "actions,vbus-gpio", 0,
			     &plat->vbus_gpio, GPIOD_IS_OUT);

	return 0;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct owl_xhci_platdata *plat = dev_get_platdata(dev);
	struct owl_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	printf("%s %d\n", __func__, __LINE__);


	/* setup the Vbus gpio here */
	if (dm_gpio_is_valid(&plat->vbus_gpio)) {
		atc260x_reg_setbits(PMU_APDS_CTL0, 1<<VBUS_OTG, 1<<VBUS_OTG);
		dm_gpio_set_value(&plat->vbus_gpio, 1);
	}

	ret = board_usb_init(0, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	hcor = (struct xhci_hcor *)((uint32_t)ctx->hcd +
			HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase)));

	return xhci_register(dev, ctx->hcd, hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	struct owl_xhci_platdata *plat = dev_get_platdata(dev);
	struct owl_xhci *ctx = dev_get_priv(dev);
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;
	board_usb_cleanup(0, USB_INIT_HOST);

	if (dm_gpio_is_valid(&plat->vbus_gpio)) {
		atc260x_reg_setbits(PMU_APDS_CTL0, 1<<VBUS_OTG, 0<<VBUS_OTG);
		dm_gpio_set_value(&plat->vbus_gpio, 0);
	}

	return 0;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "actions,s900-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name	= "xhci-owl",
	.id	= UCLASS_USB,
	.of_match = xhci_usb_ids,
	.ofdata_to_platdata = xhci_usb_ofdata_to_platdata,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops	= &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct owl_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct owl_xhci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

#endif
