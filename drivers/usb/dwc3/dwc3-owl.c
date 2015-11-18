/*
 * Actions OWL SoCs dwc3 driver
 *
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * tangshaoqing <tangshaoqing@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <linux/err.h>
#include <asm/io.h>
#include <linux/usb/dwc3-owl.h>

#define USB3_MOD_RST  (1 << 14)

#define PWR_USB3	(1<<8)
#define ACK_USB3	(1<<8)
#define USB3_AVDD_EN	(1<<17)

#define ASSIST_PLL_EN		(1<<0)
#define USB3_MAC_DIV(n)   ((n)<<12)
#define USB3_MAC_DIV_MASK		(3<<12)

#define     SPS_PG_BASE	0xE012e000
#define     SPS_PG_CTL	(SPS_PG_BASE+0x0000)
#define     SPS_PG_ACK	(SPS_PG_BASE+0x0004)
#define     SPS_LDO_CTL	(SPS_PG_BASE+0x0014)

#define     CMU_BASE		0xE0160000
#define     CMU_USBPLL	(CMU_BASE+0x0080)
#define     CMU_ASSISTPLL	(CMU_BASE+0x0084)
#define     CMU_DEVRST1	(CMU_BASE+0x00AC)

#define     TIMER_BASE	0xE0228000
#define     USB3_ECS		(TIMER_BASE+0x0090)


static struct dwc3_owl dwc_owl;


static void dwc3_clk_init(struct dwc3_owl *owl)
{
	u32		reg;
	u32		i;
	struct dwc3_port_info *port_info = &owl->port_info;

	/*USB3 Cmu Reset */
	reg = readl(port_info->devrst);
	reg &= ~(USB3_MOD_RST);
	writel(reg, port_info->devrst);

	/* power for controller and phy */
	reg = readl(port_info->sps_pg_ctl);
	reg |= PWR_USB3;
	writel(reg, port_info->sps_pg_ctl);

	i = 0;
	while (1) {
		reg = readl(port_info->sps_pg_ack)&ACK_USB3;
		if (reg)
			break;

		if (i++ >= 1000) {
			printf("%s %d enable usb3 power timeout!\n",
				__func__, __LINE__);
			break;
		}
		udelay(1);
	}

	/* power for phy */
	reg = readl(port_info->sps_ldo_ctl);
	reg |= USB3_AVDD_EN;
	writel(reg, port_info->sps_ldo_ctl);

	udelay(10);

	/* clock for phy */
	reg = readl(port_info->usbpll);
	reg |= (0xf);
	writel(reg, port_info->usbpll);

	/* clock for controller */
	reg = readl(port_info->cmu_assistpll);
	reg |= ASSIST_PLL_EN;
	reg &= (~USB3_MAC_DIV_MASK);
	reg |= USB3_MAC_DIV(3);
	writel(reg, port_info->cmu_assistpll);

	udelay(10);

	reg = readl(port_info->devrst);
	reg |= (USB3_MOD_RST);
	writel(reg, port_info->devrst);

	return;
}

static void dwc3_clk_exit(struct dwc3_owl *owl)
{
	u32		reg;
	int i;
	struct dwc3_port_info *port_info = &owl->port_info;


	/*USB3 Cmu Reset */
	reg = readl(port_info->devrst);
	reg &= ~(USB3_MOD_RST);
	writel(reg, port_info->devrst);

	reg = readl(port_info->usbpll);
	reg &= ~(0xf);
	writel(reg, port_info->usbpll);

	udelay(10);

	/* power for phy */
	reg = readl(port_info->sps_ldo_ctl);
	reg &= ~(USB3_AVDD_EN);
	writel(reg, port_info->sps_ldo_ctl);

	udelay(10);

	reg = readl(port_info->sps_pg_ctl);
	reg &= ~PWR_USB3;
	writel(reg, port_info->sps_pg_ctl);

	i = 0;
	while (1) {
		reg = readl(port_info->sps_pg_ack)&ACK_USB3;
		if (!reg)
			break;

		if (i++ >= 1000) {
			printf("%s %d disable usb3 power timeout!\n",
				__func__, __LINE__);
			break;
		}
		udelay(1);
	}

	reg = readl(port_info->devrst);
	reg |= (USB3_MOD_RST);
	writel(reg, port_info->devrst);
}


int dwc3_owl_uboot_init(void)
{
	struct dwc3_owl	*owl = &dwc_owl;

	printf("%s %d\n", __func__, __LINE__);

	owl->port_info.usbecs = (void __iomem *)USB3_ECS;

	owl->port_info.devrst = (void __iomem *)CMU_DEVRST1;
	owl->port_info.usbpll = (void __iomem *)CMU_USBPLL;
	owl->port_info.cmu_assistpll  = (void __iomem *)CMU_ASSISTPLL;

	owl->port_info.sps_pg_ctl = (void __iomem *)SPS_PG_CTL;
	owl->port_info.sps_pg_ack = (void __iomem *)SPS_PG_ACK;
	owl->port_info.sps_ldo_ctl = (void __iomem *)SPS_LDO_CTL;

	dwc3_clk_init(owl);

	return 0;
}

int dwc3_owl_uboot_exit(void)
{
	struct dwc3_owl	*owl = &dwc_owl;

	dwc3_clk_exit(owl);

	return 0;
}


