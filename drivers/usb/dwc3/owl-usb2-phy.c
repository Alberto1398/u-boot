/*
 * Actions OWL SoCs dwc3 phy driver
 *
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * tangshaoqing <tangshaoqing@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
 #include <common.h>
#include <asm/io.h>
#include <owl-usb-phy-uboot.h>


#ifdef CONFIG_USB_DWC3_OWL_S700
#define USB2_PHY_BASE (0xe040Ce00)
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
#define USB2_PHY_BASE (0xe040cd0c)
#endif


static struct owl_usbphy usb2_sphy = {
	.regs = (void __iomem *)USB2_PHY_BASE,
};

static void setphy(struct owl_usbphy *sphy, unsigned char reg_add, unsigned char value)
{
	void __iomem *usb3_usb_vcon = sphy->regs;
	unsigned char addr_low;
	unsigned char addr_high;
	unsigned int vstate;

	addr_low =  reg_add & 0x0f;
	addr_high =  (reg_add >> 4) & 0x0f;

	vstate = value;
	vstate = vstate << 8;

	addr_low |= 0x10;
	writel(vstate | addr_low, usb3_usb_vcon);
	mb();

	addr_low &= 0x0f;
	writel(vstate | addr_low, usb3_usb_vcon);
	mb();

	addr_low |= 0x10;
	writel(vstate | addr_low, usb3_usb_vcon);
	mb();

	addr_high |= 0x10;
	writel(vstate | addr_high, usb3_usb_vcon);
	mb();

	addr_high &= 0x0f;
	writel(vstate | addr_high, usb3_usb_vcon);
	mb();

	addr_high |= 0x10;
	writel(vstate | addr_high, usb3_usb_vcon);
	mb();
	return;
}

#ifdef CONFIG_USB_DWC3_OWL_S700
static int _owl_usb2phy_param_setup(struct owl_usbphy *sphy, int is_device_mode)
{
	unsigned char val_u8, slew_rate;
	static int dwc3_slew_rate = -1;
	static int dwc3_tx_bias = -1;

	slew_rate = 2;

	if (is_device_mode) {
		printf("%s device mode\n", __func__);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0x6<<5)|(0<<4)|(1<<3)|(1<<2)|(3<<0);
		setphy(sphy, 0xe1, val_u8);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (1<<7)|(4<<4)|(1<<3)|(1<<2)|(0<<0);
		setphy(sphy, 0xe6, val_u8);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		setphy(sphy, 0xe2, 0x2);
		setphy(sphy, 0xe2, 0x12);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0xa<<4)|(0<<1)|(1<<0);
		setphy(sphy, 0xe7, val_u8);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		setphy(sphy, 0xe0, 0x31);
		setphy(sphy, 0xe0, 0x35);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0xa<<4)|(0xc<<0);
		setphy(sphy, 0xe4, val_u8);

	} else {
		printf("%s host mode\n", __func__);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0x6<<5)|(0<<4)|(1<<3)|(1<<2)|(3<<0);
		setphy(sphy, 0xe1, val_u8);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (1<<7)|(4<<4)|(1<<3)|(1<<2)|(0<<0);
		setphy(sphy, 0xe6, val_u8);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		setphy(sphy, 0xe2, 0x2);
		setphy(sphy, 0xe2, 0x12);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0xa<<4)|(0<<1)|(1<<0);
		setphy(sphy, 0xe7, val_u8);

		val_u8 = (1<<7)|(1<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		setphy(sphy, 0xe0, 0x31);
		setphy(sphy, 0xe0, 0x35);

		val_u8 = (1<<7)|(0<<5)|(1<<4)|(2<<2)|(3<<0);
		setphy(sphy, 0xf4, val_u8);
		val_u8 = (0xa<<4)|(0x6<<0);
		setphy(sphy, 0xe4, val_u8);

		val_u8 = (1<<7)|(1<<6)|(1<<5)|(1<<4)|
			(1<<3)|(1<<2)|(0<<1)|(0<<0);
		setphy(sphy, 0xf0, val_u8);
	}

	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
static int _owl_usb2phy_param_setup(struct owl_usbphy *sphy, int is_device_mode)
{
	if (is_device_mode) {
		printf("%s device mode\n", __func__);

		setphy(sphy, 0xe7, 0x1b);
		setphy(sphy, 0xe7, 0x1f);

		udelay(10);

		setphy(sphy, 0xe2, 0x48);
		setphy(sphy, 0xe0, 0xa3);
		setphy(sphy, 0x87, 0x1f);
	} else {
		printf("%s host mode\n", __func__);

		setphy(sphy, 0xe7, 0x1b);
		setphy(sphy, 0xe7, 0x1f);

		udelay(10);

		setphy(sphy, 0xe2, 0x46);
		setphy(sphy, 0xe0, 0xa3);
		setphy(sphy, 0x87, 0x1f);
	}

	return 0;
}
#endif

int owl_usb2phy_param_uboot_setup(int is_device_mode)
{
	_owl_usb2phy_param_setup(&usb2_sphy, is_device_mode);
	return 0;
}

