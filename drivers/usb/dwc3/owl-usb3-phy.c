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
#define USB3_PHY_BASE		(0xe040cd00)

#define USB3_ANA00	(0x00)
#define USB3_ANA01	(0x04)
#define USB3_ANA02	(0x08)
#define USB3_ANA03	(0x0C)
#define USB3_ANA04	(0x10)
#define USB3_ANA05	(0x14)
#define USB3_ANA06	(0x18)
#define USB3_ANA07	(0x1C)
#define USB3_ANA08	(0x20)
#define USB3_ANA09	(0x24)
#define USB3_ANA0A	(0x28)
#define USB3_ANA0B	(0x2C)
#define USB3_ANA0C	(0x30)
#define USB3_ANA0D	(0x34)
#define USB3_ANA0E	(0x38)
#define USB3_ANA0F	(0x3C)

#define USB3_DMR		(0x40)
#define USB3_BACR		(0x44)
#define USB3_IER		(0x48)
#define USB3_BCSR		(0x4C)
#define USB3_BPR		(0x50)
#define USB3_BPNR2		(0x54)
#define USB3_BFNR		(0x58)
#define USB3_BRNR2		(0x5C)
#define USB3_BENR		(0x60)
#define USB3_REV0		(0x64)
#define USB3_REV1		(0x68)
#define USB3_REV2		(0x6C)
#define USB3_REV3		(0x70)
#define USB3_FLD0		(0x74)
#define USB3_FLD1		(0x78)
#define USB3_ANA1F		(0x7C)

#define USB3_PAGE1_REG00		(0x80)
#define USB3_PAGE1_REG01		(0x84)
#define USB3_PAGE1_REG02		(0x88)
#define USB3_PAGE1_REG03		(0x8C)
#define USB3_PAGE1_REG04		(0x90)
#define USB3_PAGE1_REG05		(0x94)
#define USB3_PAGE1_REG06		(0x98)
#define USB3_PAGE1_REG07		(0x9C)
#define USB3_PAGE1_REG08		(0xA0)
#define USB3_PAGE1_REG09		(0xA4)
#define USB3_PAGE1_REG0A		(0xA8)
#define USB3_PAGE1_REG0B		(0xAC)
#define USB3_PAGE1_REG0C		(0xB0)
#define USB3_PAGE1_REG0D		(0xB4)
#define USB3_PAGE1_REG0E		(0xB8)
#define USB3_PAGE1_REG0F		(0xBC)
#define USB3_PAGE1_REG10		(0xC0)
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
#define USB3_PHY_BASE		(0xe040cf00)
#define USB3_TX_DATA_PATH_CTRL		(0X5D)
#define USB3_RX_DATA_PATH_CTRL1	(0X87)
#endif

static struct owl_usbphy usb3_sphy = {
	.regs = (void __iomem *)USB3_PHY_BASE,
};

static inline u32 owl_phy_readl(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static inline void owl_phy_writel(void __iomem *base, u32 offset, u32 value)
{
	writel(value, base + offset);
}

int owl_usb3phy_uboot_init(void)
{
	void __iomem *phy_base = usb3_sphy.regs;

#ifdef CONFIG_USB_DWC3_OWL_S700
	printf("owl_usb3phy_uboot_init for s700\n");

	owl_phy_writel(phy_base, USB3_ANA02, 0x6046);
	owl_phy_writel(phy_base, USB3_ANA0E, 0x2010);
	owl_phy_writel(phy_base, USB3_ANA0F, 0x8000);
	owl_phy_writel(phy_base, USB3_REV1, 0x0);
	owl_phy_writel(phy_base, USB3_PAGE1_REG02, 0x0013);
	owl_phy_writel(phy_base, USB3_PAGE1_REG06, 0x0004);
	owl_phy_writel(phy_base, USB3_PAGE1_REG07, 0x22ed);
	owl_phy_writel(phy_base, USB3_PAGE1_REG08, 0xf802);
	owl_phy_writel(phy_base, USB3_PAGE1_REG09, 0x3080);
	owl_phy_writel(phy_base, USB3_PAGE1_REG0B, 0x2030);
	owl_phy_writel(phy_base, USB3_ANA0F, (1<<14));
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
	u32		reg;
	u32		aligned_offset;
	u32		aligned_adrress;

	/*IO_OR_U8(USB3_TX_DATA_PATH_CTRL, 0x02);*/
	aligned_adrress = USB3_TX_DATA_PATH_CTRL&(~0x3);
	aligned_offset = USB3_TX_DATA_PATH_CTRL&0x3;
	reg = owl_phy_readl(phy_base, aligned_adrress);
	reg |= (0x2<<(8*aligned_offset));
	owl_phy_writel(phy_base, aligned_adrress, reg);
	printf("%s 0x%x:0x%x\n", __func__, aligned_adrress,
		owl_phy_readl(phy_base, aligned_adrress));

	/*IO_OR_U8(USB3_RX_DATA_PATH_CTRL1, 0x20);*/
	aligned_adrress = USB3_RX_DATA_PATH_CTRL1&(~0x3);
	aligned_offset = USB3_RX_DATA_PATH_CTRL1&0x3;
	reg = owl_phy_readl(phy_base, aligned_adrress);
	reg |= (0x20<<(8*aligned_offset));
	owl_phy_writel(phy_base, aligned_adrress, reg);
	printf("%s 0x%x:0x%x\n", __func__, aligned_adrress,
		owl_phy_readl(phy_base, aligned_adrress));
#endif

	return 0;
}

