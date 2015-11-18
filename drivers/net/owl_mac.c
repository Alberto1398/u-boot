/*
 * owl_mac.c - OWL MAC driver
 *
 * Copyright (C) 2012, Actions Semiconductor Co. LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <malloc.h>
#include <linux/types.h>
#include <linux/mii.h>
#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <net.h>
#include <asm/arch-owl/regs_s900.h>
#include <power/atc260x/actions_reg_atc2609a.h>
#include "owl_mac.h"

/* Structure/enum declaration ------------------------------- */
typedef struct owl_mac_info {
	struct buffer_descriptor *tx_bd_base;
	uint32_t tx_cur_idx;
	dma_addr_t tx_bd_paddr;

	struct buffer_descriptor *rx_bd_base;
	uint32_t rx_cur_idx;
	dma_addr_t rx_bd_paddr;

	int phy_addr;

	struct eth_device netdev;
} owl_mac_info_t;

int clockflag = 0;

#define OWL_MAC_DEBUG
/*#define OWL_PHY_DEBUG*/
#define OWL_MAC_TX_TIMEOUT 100
#define OWL_MAC_SWRESET_TIMEOUT 20
#define OWL_MAC_COMPAT "actions,owl-ethernet"

static int owl_mac_set_macaddr(struct eth_device *dev)
{
#ifdef OWL_PHY_DEBUG
	printf("owl_mac_set_macaddr:%d\n", clockflag);
#endif
	if (clockflag) {
		unsigned int mm = 0;
		u8 *mac = dev->enetaddr;
		if (!is_valid_ethaddr(mac)) {
			printf("Warning:Bad address,use ramdom address instead\n");
			net_random_ethaddr(dev->enetaddr);
			mac = dev->enetaddr;
		}

		mm = (mac[0] << 0) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] <<
								       24);
		printf("Update MAC: %pM\n", dev->enetaddr);

		writel((mac[0] << 0) | (mac[1] << 8) | (mac[2] << 16) |
		       (mac[3] << 24), MAC_CSR16);
		writel((mac[4] << 0) | (mac[5] << 8), MAC_CSR17);
	}
	return 0;
}

#ifdef OWL_MAC_DEBUG

static void owl_print_mac_register(struct eth_device *dev)
{
	printf("****** OWL MAC *******\n");
	printf("MAC_CSR0  : 0x%x\n", readl(MAC_CSR0));
	printf("MAC_CSR1  : 0x%x\n", readl(MAC_CSR1));
	printf("MAC_CSR2  : 0x%x\n", readl(MAC_CSR2));
	printf("MAC_CSR3  : 0x%x\n", readl(MAC_CSR3));
	printf("MAC_CSR4  : 0x%x\n", readl(MAC_CSR4));
	printf("MAC_CSR5  : 0x%x\n", readl(MAC_CSR5));
	printf("MAC_CSR6  : 0x%x\n", readl(MAC_CSR6));
	printf("MAC_CSR7  : 0x%x\n", readl(MAC_CSR7));
	printf("MAC_CSR8  : 0x%x\n", readl(MAC_CSR8));
	printf("MAC_CSR9  : 0x%x\n", readl(MAC_CSR9));
	printf("MAC_CSR10 : 0x%x\n", readl(MAC_CSR10));
	printf("MAC_CSR11 : 0x%x\n", readl(MAC_CSR11));
	printf("MAC_CSR16 : 0x%x\n", readl(MAC_CSR16));
	printf("MAC_CSR17 : 0x%x\n", readl(MAC_CSR17));
	printf("MAC_CSR18 : 0x%x\n", readl(MAC_CSR18));
	printf("MAC_CSR19 : 0x%x\n", readl(MAC_CSR19));
	printf("MAC_CSR20 : 0x%x\n", readl(MAC_CSR20));
	printf("MAC_CTRL  : 0x%x\n", readl(MAC_CTRL));
	printf("****** OWL MAC *******\n");
}

static void owl_print_tx_bds(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	struct buffer_descriptor *bdp;
	int i;

	printf("----- TX BDs -----\n");
	printf("      status\t\tcontrl\t\tbuff addr\n");
	for (i = 0; i < TX_RING_SIZE; i++) {
		bdp = &(owl_info->tx_bd_base[i]);
		printf("%03d: |0x%08x|\t0x%08x|\t0x%08x|\n", i,
		       (unsigned int)bdp->status, (unsigned int)bdp->control,
		       (unsigned int)bdp->buf_addr);
	}
	printf("----- TX BDs -----\n");
}

static void owl_print_rx_bds(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	struct buffer_descriptor *bdp;
	int i;

	printf("----- RX BDs -----\n");
	printf("      status\t\tcontrl\t\tbuff addr\n");
	for (i = 0; i < RX_RING_SIZE; i++) {
		bdp = &(owl_info->rx_bd_base[i]);
		printf("%03d: |0x%08x|\t0x%08x|\t0x%08x|\n", i,
		       (unsigned int)bdp->status, (unsigned int)bdp->control,
		       (unsigned int)bdp->buf_addr);
	}
	printf("----- RX BDs -----\n");
}

#endif

static void write_phy_reg(uint8_t phyreg, uint16_t phydata)
{
	u32 op_reg;
	int phyaddr = 3;
	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	writel(MII_MNG_SB | MII_MNG_OPCODE(MII_OP_WRITE) |
	       MII_MNG_REGADD(phyreg) | MII_MNG_PHYADD(phyaddr) | phydata,
	       MAC_CSR10);

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);
}

static uint16_t read_phy_reg(uint8_t phyreg)
{
	u32 op_reg;
	int phyaddr = 3;
	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	writel(MII_MNG_SB | MII_MNG_OPCODE(MII_OP_READ) | MII_MNG_REGADD(phyreg)
	       | MII_MNG_PHYADD(phyaddr), MAC_CSR10);

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	return (uint16_t) MII_MNG_DATA(op_reg);
}

static inline void phy_reg_set_bits(unsigned short reg_addr, int bits)
{
	unsigned short reg_val;

	reg_val = read_phy_reg(reg_addr);
	reg_val |= (unsigned short)bits;
	write_phy_reg(reg_addr, reg_val);
}

static inline void phy_reg_clear_bits(unsigned short reg_addr, int bits)
{
	unsigned short reg_val;

	reg_val = read_phy_reg(reg_addr);
	reg_val &= ~(unsigned short)bits;
	write_phy_reg(reg_addr, reg_val);
}

static uint16_t owl_mdio_read(struct eth_device *dev, uint8_t phyreg)
{
	return read_phy_reg(phyreg);
}

static void owl_mdio_write(struct eth_device *dev, uint8_t phyreg,
			   uint16_t phydata)
{
	write_phy_reg(phyreg, phydata);
}

static void owl_mac_clock_enable(struct eth_device *dev)
{
	unsigned int temp;
	temp = readl(MFP_CTL0);
	temp &= 0xffc8003f;	/* RMII pin */
	temp |= 0x00000000;
	writel(temp, MFP_CTL0);

	temp = readl(CMU_ASSISTPLL);
	temp |= 0x101;		/*CMU_ETHERNETPLL_ENABLE; */
	writel(temp, CMU_ASSISTPLL);

	temp = readl(PAD_CTL);
	temp |= 0x02;
	writel(temp, PAD_CTL);

	udelay(100);
	writel(readl(CMU_DEVCLKEN1) | ASOC_ETH_CLOCK_EN, CMU_DEVCLKEN1);
	udelay(100);

	temp = readl(MAC_CTRL);
	/* use RMII, clock reverse  bit8,0 all 0        */
	writel(temp | 0x02, MAC_CTRL);
	udelay(100);

	/*printf("MFP_CTL0:%x CMU_ASSISTPLL:%x CMU_DEVCLKEN1:%x MAC_CTL:%x\n",
	       readl(MFP_CTL0), readl(CMU_ASSISTPLL), readl(CMU_DEVCLKEN1),
	       readl(MAC_CTRL));*/
	/* reset ethernet clk */
	writel(readl(CMU_DEVRST1) & ~ASOC_ETH_CLOCK_RST, CMU_DEVRST1);
	udelay(100);
	writel(readl(CMU_DEVRST1) | ASOC_ETH_CLOCK_RST, CMU_DEVRST1);
	udelay(100);
	return;
}

static void owl_phy_power_on(struct eth_device *dev)
{
	atc260x_reg_setbits(ATC2609A_PMU_LDO8_CTL0, (0X1F << 1) | 0x1,
			    (0X1F << 1) | 0x1);

	mdelay(1000);
#ifdef OWL_PHY_DEBUG
	printf("owl mac power on\n");
#endif
	return;
}

static void owl_phy_power_off(struct eth_device *dev)
{
	atc260x_reg_setbits(ATC2609A_PMU_LDO8_CTL0, 0X1, 0);

	mdelay(500);
#ifdef OWL_PHY_DEBUG
	printf("owl mac power off\n");
#endif
	return;
}

static void owl_mac_phy_reset(struct eth_device *dev)
{
	int cnt = 0;

	mdelay(200);
	int temp = readl(MAC_CSR10);
	/*printf("owl_mac_phy_reset:%x-%x-%x\n", MII_BMCR,
	    temp, read_phy_reg(0));*/
	write_phy_reg(MII_BMCR, temp | 0xcc000000);
	writel(temp | 0xcc500000, MAC_CSR10);	/*100:/128 */
	/*printf("owl_mac_phy_reset:%x-%x-%x\n", MII_BMCR, readl(MAC_CSR10),
	       read_phy_reg(0));*/
	uint16_t reg_val;
	phy_reg_set_bits(MII_BMCR, BMCR_RESET);
	do {
		reg_val = read_phy_reg(MII_BMCR);
		/*printf("read phy bmcr:%x\n",reg_val); */
		if (cnt++ > 10000) {
			printf("ethernet phy BMCR_RESET\n");
			owl_phy_power_off(dev);
			owl_phy_power_on(dev);
			cnt = 0;
		}
	} while (reg_val & BMCR_RESET);
}

static int owl_mac_hardware_init(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	int tmp = 0;

	owl_mac_clock_enable(dev);
	/* hardware soft reset, and set bus mode */
	writel(readl(MAC_CSR0) | EC_BMODE_SWR, MAC_CSR0);
	do {
		udelay(10);
		if (tmp++ > OWL_MAC_SWRESET_TIMEOUT)
			break;
	} while (readl(MAC_CSR0) & EC_BMODE_SWR);
	if (tmp > OWL_MAC_SWRESET_TIMEOUT)
		printf("warning:MAC reset error 0x%x\n", readl(MAC_CSR0));
	writel(readl(MAC_CTRL) & (~(EC_MACCTRL_RSIS)), MAC_CTRL);
	writel(readl(MAC_CTRL) & (~(EC_MACCTRL_RCPS)), MAC_CTRL);
	writel((readl(MAC_CSR10) & REG_CLEAR) | MII_MNG_OPCODE(MII_OP_CDS),
	       MAC_CSR10);

	writel(owl_info->rx_bd_paddr, MAC_CSR3);
	writel(owl_info->tx_bd_paddr, MAC_CSR4);

	writel(readl(MAC_CSR6) | EC_OPMODE_FD | EC_OPMODE_SPEED(0), MAC_CSR6);
	writel(readl(MAC_CSR6) & ~EC_OPMODE_PR, MAC_CSR6);

	writel(0x004e0000, MAC_CSR11);

	owl_phy_power_on(dev);

	clockflag = 1;
	return 0;
}

static void owl_prepare_tx_bds(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	int i;

	for (i = 0; i < TX_RING_SIZE; i++) {
		owl_info->tx_bd_base[i].buf_addr = 0;
		owl_info->tx_bd_base[i].reserved = 0;
		owl_info->tx_bd_base[i].status = 0;
		owl_info->tx_bd_base[i].control = TXBD_CTRL_IC;
	}
	owl_info->tx_bd_base[i - 1].control |= TXBD_CTRL_TER;
	owl_info->tx_cur_idx = 0;
	flush_dcache_all();
}

static void owl_prepare_rx_bds(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	int i;

	for (i = 0; i < RX_RING_SIZE; i++) {
		void *buff = memalign(ARCH_DMA_MINALIGN, PKG_MAX_LEN);
		if (!buff)
			panic("owl_mac: fail to create rx's buff\n");
		owl_info->rx_bd_base[i].buf_addr = (unsigned int)buff;
		owl_info->rx_bd_base[i].reserved = 0;
		owl_info->rx_bd_base[i].status = RXBD_STAT_OWN;
		owl_info->rx_bd_base[i].control = RXBD_CTRL_RBS1(PKG_MAX_LEN);
	}
	owl_info->rx_bd_base[i - 1].control |= RXBD_CTRL_RER;
	owl_info->rx_cur_idx = 0;
	flush_dcache_all();
}

/*
 * init mac/phy
 */
static int owl_mac_init(struct eth_device *dev, bd_t *bd)
{
	struct owl_mac_info *owl_info = dev->priv;
	int temp = 0;
	int cnt = 0;

	owl_info->tx_bd_base = (struct buffer_descriptor *)
	    dma_alloc_coherent(sizeof(struct buffer_descriptor) * TX_RING_SIZE,
			       (unsigned long *)&owl_info->tx_bd_paddr);
	if (!owl_info->tx_bd_base)
		panic("owl_mac: fail to alloc tx's memory\n");
	/*printf("owl_mac_init:tx_bd_base %p paddr %p\n",
	   owl_info->tx_bd_base,owl_info->tx_bd_paddr); */

	memset(owl_info->tx_bd_base, 0,
	       sizeof(struct buffer_descriptor) * TX_RING_SIZE);
	owl_info->rx_bd_base = (struct buffer_descriptor *)
	    dma_alloc_coherent(sizeof(struct buffer_descriptor) * RX_RING_SIZE,
			       (unsigned long *)&owl_info->rx_bd_paddr);
	if (!owl_info->rx_bd_base)
		panic("owl_mac: fail to alloc rx's memory\n");
	/*printf("owl_mac_init:rx_bd_base %p paddr %p\n",
	   owl_info->rx_bd_base,owl_info->rx_bd_paddr); */
	memset(owl_info->rx_bd_base, 0,
	       sizeof(struct buffer_descriptor) * RX_RING_SIZE);
	owl_prepare_tx_bds(dev);
	owl_prepare_rx_bds(dev);

	owl_mac_hardware_init(dev);
	/* start to tx & rx packets */
	writel(EC_IEN_ALL, MAC_CSR7);
	writel(readl(MAC_CSR6) | EC_OPMODE_ST | EC_OPMODE_SR, MAC_CSR6);

	/*writel(0xb03100b0,MAC_CTRL); */

	owl_mac_set_macaddr(dev);

	owl_mac_phy_reset(dev);

	do {
		temp = read_phy_reg(MII_BMSR);
	} while ((temp & 0x4) != 0x4);	/*wait for Valid link established */

	/*set threshold | Duplex mode | start transmit |100M */
	temp = (3 << 14) | 0x00000200 | 0x00002000;
	writel(temp, MAC_CSR6);

	return 0;
}

/*
 * disable transmitter, receiver
 */
static void owl_mac_halt(struct eth_device *dev)
{
	u32 reg_val = 0;

	/* phy down */
#ifdef OWL_PHY_DEBUG
	printf("owl_mac_halt:%d\n", clockflag);
#endif

	/*owl_phy_power_off(dev); */

	/* Stop transmit and receive */
	if (clockflag) {
		reg_val = readl(MAC_CSR6);
		reg_val &= ~(EC_OPMODE_ST | EC_OPMODE_SR);
		writel(reg_val, MAC_CSR6);
		clockflag = 0;
	}

	return;
}

/*
 * Send a data block via Ethernet
 */
static int owl_mac_send(struct eth_device *dev, void *pkt, int len)
{
	struct buffer_descriptor *bdp;
	unsigned long status;
	struct owl_mac_info *owl_info = dev->priv;
	int i;
	u32 reg_val = 0;

	if (len <= 0 || len > ETH_PKG_MAX)
		printf("owl_mac:bad tx pkt len (%d)\n", len);

	/*  */
	printf("owl_mac_send: len %d\n", len);
	/*  */

	owl_prepare_tx_bds(dev);

	bdp = &owl_info->tx_bd_base[owl_info->tx_cur_idx];
	status = bdp->status;
	if (status & TXBD_STAT_OWN) {
		printf("owl_mac tx error: tx is full\n");
		return 0;
	}

	bdp->buf_addr = dma_map_single(pkt, len, DMA_TO_DEVICE);

	bdp->status = 0;

	bdp->control &= TXBD_CTRL_IC | TXBD_CTRL_TER;	/* clear others 31,25 */

	bdp->control |= TXBD_CTRL_TBS1(len);
	bdp->control |= TXBD_CTRL_FS | TXBD_CTRL_LS;
	bdp->status = TXBD_STAT_OWN;

	flush_dcache_all();

	writel(readl(MAC_CSR6) | EC_OPMODE_ST, MAC_CSR6);
	writel(EC_TXPOLL_ST, MAC_CSR1);

	/*owl_print_mac_register(dev); */

	/* wait for finish then return */
	for (i = 0; i < OWL_MAC_TX_TIMEOUT * 1000; i++) {
		reg_val = readl(MAC_CSR5);
		if (reg_val & EC_STATUS_TI) {
			/*printf("have interrupt:%x\n",bdp->status); */
			reg_val = reg_val & (~EC_STATUS_TI);
			writel(reg_val, MAC_CSR5);
			break;
		}
		udelay(10);
	}
	dma_unmap_single(pkt, len, bdp->buf_addr);
	if (i >= OWL_MAC_TX_TIMEOUT) {
		invalidate_dcache_range((uint32_t) bdp,
					(uint32_t) bdp +
					roundup(sizeof(*bdp) * TX_RING_SIZE,
						ARCH_DMA_MINALIGN));
		printf("owl_mac:Tx timeout 0x%d\n", bdp->status);
	}

	owl_info->tx_cur_idx = (owl_info->tx_cur_idx + 1) % TX_RING_SIZE;
	/*printf("owl_mac_send:%d-%x FINISH\n",
	   owl_info->tx_cur_idx,bdp->status); */
	return len;
}

/*
 * Get a data block via Ethernet
 */
static int owl_mac_recv(struct eth_device *dev)
{
	struct owl_mac_info *owl_info = dev->priv;
	u32 status, rx_cur_idx = owl_info->rx_cur_idx;
	struct buffer_descriptor *bdp = &owl_info->rx_bd_base[rx_cur_idx];
	int pkt_len = 0;
	uint32_t desc_start = (uint32_t) bdp;
	uint32_t desc_end =
	    desc_start + roundup(sizeof(*bdp), ARCH_DMA_MINALIGN);
	uint32_t data_start = (uint32_t) bdp->buf_addr;
	uint32_t data_end;

	/* */
	writel(readl(MAC_CSR6) | EC_OPMODE_RA | EC_OPMODE_SR, MAC_CSR6);
	/* */

	invalidate_dcache_range(desc_start, desc_end);

	status = bdp->status;
	if (!(status & RXBD_STAT_OWN)) {
		pkt_len = RXBD_STAT_FL(status);

		data_end = data_start + roundup(pkt_len, ARCH_DMA_MINALIGN);
		invalidate_dcache_range(data_start, data_end);
		net_process_received_packet((uchar *)bdp->buf_addr, pkt_len);
		bdp->status = RXBD_STAT_OWN;

		flush_dcache_range(desc_start, desc_end);

		if (++rx_cur_idx >= RX_RING_SIZE)
			rx_cur_idx = 0;
	}

	owl_info->rx_cur_idx = rx_cur_idx;
	/*if (pkt_len>0)
	   printf("owl_mac_recv:%d-%x >> FINISH\n",pkt_len,bdp->status); */
	return pkt_len;
}

DECLARE_GLOBAL_DATA_PTR;

#include <libfdt.h>
#include <fdtdec.h>

static int owl_mac_parse_fdtdec(struct owl_mac_info *info)
{
	int node, ret;
	/*const char *mac_compat = OWL_MAC_COMPAT; */

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, OWL_MAC_COMPAT);
	if (node <= 0) {
		debug("Can't get owl-ethernet node\n");
		return -1;
	}

	ret = fdtdec_get_is_enabled(gd->fdt_blob, node);

	if (!ret) {
		debug("Disable by dts\n");
		return -1;
	}

	info->phy_addr = fdtdec_get_int(gd->fdt_blob, node, "phy_addr", -1);
	printf("owl_mac_parse_fdtdec,phy_addr %d\n", info->phy_addr);

	return 0;
}

int owl_mac_initialize(bd_t *bis)
{
	struct owl_mac_info *priv;
	struct eth_device *dev;

	priv = (struct owl_mac_info *)malloc(sizeof(*priv));
	if (!priv)
		return -1;

	/*
	   if(owl_mac_parse_fdtdec(priv))
	   return -1;
	 */
	dev = (struct eth_device *)malloc(sizeof(*dev));
	if (!dev) {
		free(priv);
		return -1;
	}

	dev->init = owl_mac_init;
	dev->halt = owl_mac_halt;
	dev->send = owl_mac_send;
	dev->recv = owl_mac_recv;
	dev->write_hwaddr = owl_mac_set_macaddr;
	dev->priv = priv;
	sprintf(dev->name, "owl_mac");
	eth_register(dev);

#ifdef OWL_PHY_DEBUG
	printf("owl_mac_initialize finish\n");
#endif

	return 0;
}
