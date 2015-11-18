/*
 * Actions OWL SoCs dwc3 phy driver
 *
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * tangshaoqing <tangshaoqing@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __OWL_USB_PHY_UBOOT_H_
#define __OWL_USB_PHY_UBOOT_H_


struct owl_usbphy {
	void __iomem	*regs;
};

int owl_usb2phy_param_uboot_setup(int is_device_mode);
int owl_usb3phy_uboot_init(void);

#endif /* __OWL_USB_PHY_UBOOT_H_ */

