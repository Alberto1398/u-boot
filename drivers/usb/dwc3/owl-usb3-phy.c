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

#define USB3_PHY_BASE		(0xe040cf00)
#define USB3_TX_DATA_PATH_CTRL		(0X5D)
#define USB3_RX_DATA_PATH_CTRL1	(0X87)

static struct owl_usbphy usb3_sphy = {
	.regs = (void __iomem *)USB3_PHY_BASE,
};


static inline u32 phy_readl(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static inline void phy_writel(void __iomem *base, u32 offset, u32 value)
{
	writel(value, base + offset);
}


int owl_usb3phy_uboot_init(void)
{
	void __iomem *phy_base = usb3_sphy.regs;
	u32		reg;
	u32		aligned_offset;
	u32		aligned_adrress;


	/*IO_OR_U8(USB3_TX_DATA_PATH_CTRL, 0x02);*/
	aligned_adrress = USB3_TX_DATA_PATH_CTRL&(~0x3);
	aligned_offset = USB3_TX_DATA_PATH_CTRL&0x3;
	reg = phy_readl(phy_base, aligned_adrress);
	reg |= (0x2<<(8*aligned_offset));
	phy_writel(phy_base, aligned_adrress, reg);
	printf("%s 0x%x:0x%x\n", __func__, aligned_adrress, phy_readl(phy_base, aligned_adrress));

	/*IO_OR_U8(USB3_RX_DATA_PATH_CTRL1, 0x20);*/
	aligned_adrress = USB3_RX_DATA_PATH_CTRL1&(~0x3);
	aligned_offset = USB3_RX_DATA_PATH_CTRL1&0x3;
	reg = phy_readl(phy_base, aligned_adrress);
	reg |= (0x20<<(8*aligned_offset));
	phy_writel(phy_base, aligned_adrress, reg);
	printf("%s 0x%x:0x%x\n", __func__, aligned_adrress, phy_readl(phy_base, aligned_adrress));

	return 0;
}

