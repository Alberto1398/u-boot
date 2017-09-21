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

#ifdef CONFIG_USB_DWC3_OWL_S700
#define PWR_USB3	(1<<10)
#define ACK_USB3	(1<<10)

#define SPS_PG_BASE	0xE01B0100
#define SPS_PG_CTL	(SPS_PG_BASE+0x0000)
#define SPS_PG_ACK	(SPS_PG_BASE+0x0018)

#define USB3_CONTROLLER_CLOCK_ENABLE	(1 << 25)
#define USB3_MOD_RST	(1 << 25)

#define CMU_BASE	0xE0168000
#define CMU_DEVCLKEN0	(CMU_BASE+0x00a0)
#define CMU_DEVRST0	(CMU_BASE+0x00A8)
#define CMU_USBPLL	(CMU_BASE+0x00b0)

#define PLL_LDO_EN	(1<<31)
#define TIMER_BASE	0xE024C000
#define USB3_ECS	(TIMER_BASE+0x0090)
#define SOFTVBUSEN_P0	(1<<6)
#define SOFTVBUS_P0	(1<<7)

#define USB3_BASE 0xE040CE04

#define MAC3_SUSPEND_CLOCK (1<<27)
#define USB3_BACKDOOR   (0)
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
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
#endif

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

#ifdef CONFIG_USB_SOFTVBUS
	reg = readl(port_info->usbecs);
	reg |= (SOFTVBUSEN_P0 | SOFTVBUS_P0);
	writel(reg, port_info->usbecs);

	udelay(10);
#endif
	/* clock for phy */
	reg = readl(port_info->usbpll);
	reg |= (0xf);
	writel(reg, port_info->usbpll);

	/* clock for controller */
#ifdef CONFIG_USB_DWC3_OWL_S700
	reg = readl(port_info->cmu_devclken0);
	reg |= USB3_CONTROLLER_CLOCK_ENABLE;
	writel(reg, port_info->cmu_devclken0);
#endif
#ifdef CONFIG_USB_DWC3_OWL_S900
	reg = readl(port_info->cmu_assistpll);
	reg |= ASSIST_PLL_EN;
	reg &= (~USB3_MAC_DIV_MASK);
	reg |= USB3_MAC_DIV(3);
	writel(reg, port_info->cmu_assistpll);
#endif

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

#ifdef CONFIG_USB_SOFTVBUS
	reg = readl(port_info->usbecs);
	reg &= ~(SOFTVBUSEN_P0 | SOFTVBUS_P0);
	writel(reg, port_info->usbecs);

	udelay(10);
#endif

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

#ifdef CONFIG_USB_DWC3_OWL_S700
	owl->port_info.devrst = (void __iomem *)CMU_DEVRST0;
	owl->port_info.cmu_devclken0  = (void __iomem *)CMU_DEVCLKEN0;
#endif
#ifdef CONFIG_USB_DWC3_OWL_S900
	owl->port_info.devrst = (void __iomem *)CMU_DEVRST1;
	owl->port_info.cmu_assistpll  = (void __iomem *)CMU_ASSISTPLL;
	owl->port_info.sps_ldo_ctl = (void __iomem *)SPS_LDO_CTL;
#endif
	owl->port_info.usbecs = (void __iomem *)USB3_ECS;
	owl->port_info.usbpll = (void __iomem *)CMU_USBPLL;
	owl->port_info.sps_pg_ctl = (void __iomem *)SPS_PG_CTL;
	owl->port_info.sps_pg_ack = (void __iomem *)SPS_PG_ACK;

	dwc3_clk_init(owl);

	return 0;
}

int dwc3_owl_uboot_exit(void)
{
	struct dwc3_owl	*owl = &dwc_owl;

	dwc3_clk_exit(owl);

	return 0;
}
