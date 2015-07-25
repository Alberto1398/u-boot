/*
 * Actions ACTE100 Ethernet
 *
 * (C) Actions 2012 Ferry Zhou, zhoutiangui@actions-semi.com
 *     add initial verison 1.0
 *
 * This program is loaded into SRAM in bootstrap mode, where it waits
 * for commands on UART1 to read and write memory, jump to code etc.
 * A design goal for this program is to be entirely independent of the
 * target board. All the board specifics can be handled on the host.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <net.h>
#include <malloc.h>
#include <linux/mii.h>
#include "acte100.h"

static uchar def_eth_addr[6] = {0x00, 0x18, 0xFE, 0x66, 0x66, 0x66};

static void check_timeout(u32 *reg, u32 bits, u32 clrset,
	u32 time, u8 *info)
{
	while (time) {
		if (!clrset && !(readl(reg) & bits))
			break;
		if (clrset && (readl(reg) & bits))
			break;
		udelay(10);
		time--;
	}

	if (!time) {
		ACTE100_ERR("%s\n", info);
		hang();
	}
}

#define setbits_timeout(reg, bits, info) \
	check_timeout((reg), (bits), 1, 900000, info)
#define clrbits_timeout(reg, bits, info) \
	check_timeout((reg), (bits), 0, 900000, info)

static void acte100_get_enetaddr(struct eth_device *dev)
{
	memcpy(dev->enetaddr, def_eth_addr, sizeof(dev->enetaddr));
}

static uchar *ethernet_addr(struct eth_device *dev)
{
	return dev->enetaddr;
}

static u16 read_phy_reg(u32 reg)
{
	u32 op_reg;

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	writel(MII_MNG_SB | MII_MNG_OPCODE(MII_OP_READ) | MII_MNG_REGADD(reg) |
		MII_MNG_PHYADD(ACTE100_PHY_ADDR), MAC_CSR10);

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	return (u16)MII_MNG_DATA(op_reg);
}

static void write_phy_reg(u32 reg, u16 val)
{
	u32 op_reg;

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);

	writel(MII_MNG_SB | MII_MNG_OPCODE(MII_OP_WRITE) | MII_MNG_REGADD(reg) |
		MII_MNG_PHYADD(ACTE100_PHY_ADDR) | val, MAC_CSR10);

	do {
		op_reg = readl(MAC_CSR10);
	} while (op_reg & MII_MNG_SB);
}

static void clock_enable(void)
{
	setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_ETHERNET);

	/* reset ethernet clk */
	clrbits_le32(CMU_DEVRST1, 0x1 << 20);
	udelay(10);
	setbits_le32(CMU_DEVRST1, 0x1 << 20);
}

static void clock_disable(void)
{
	/* disable ethernet clk */
	clrbits_le32(CMU_DEVCLKEN1, 0x1 << 22);
}

static volatile struct buf_des *init_rx_buf_des(struct acte100_priv *priv)
{
	static volatile struct buf_des *rx_bds;
	int i;

	if (!rx_bds) {
		rx_bds = memalign(ACTE100_BD_ADDR_ALIGN,
				ACTE100_RX_BUF * sizeof(*rx_bds));
		if (!rx_bds) {
			ACTE100_ERR("malloc rx_bds failed\n");
			return NULL;
		}
	}
	for (i = 0; i < ACTE100_RX_BUF; i++) {
		rx_bds[i] = (struct buf_des) {
			.status = RXBD_STAT_OWN, /* mac own it */
			.control = RXBD_CTRL_RBS1(PKTSIZE_ALIGN),
			.buf_addr1 = (u32)NetRxPackets[i],
			.buf_addr2 = 0
		};
	}
	rx_bds[i - 1].control |= RXBD_CTRL_RER;

	priv->rx_bd_base = rx_bds;
	priv->cur_rx = rx_bds;
	return rx_bds;
}

static volatile struct buf_des *init_tx_buf_des(struct acte100_priv *priv)
{
	static volatile struct buf_des *tx_bds;
	int i;

	if (!tx_bds) {
		tx_bds = memalign(ACTE100_BD_ADDR_ALIGN,
				ACTE100_TX_BUF * sizeof(*tx_bds));
		if (!tx_bds) {
			ACTE100_ERR("malloc tx_bds failed\n");
			return NULL;
		}
	}

	for (i = 0; i < ACTE100_TX_BUF; i++) {
		tx_bds[i] = (struct buf_des) {
			.status = 0,  /* host own it */
			.control = TXBD_CTRL_IC,
			.buf_addr1 = 0,
			.buf_addr2 = 0
		};
	}
	tx_bds[i - 1].control |= TXBD_CTRL_TER;

	priv->tx_bd_base = tx_bds;
	priv->cur_tx = tx_bds;
	return tx_bds;
}

/* NOTE: dst modified */
#define COPY_MAC_ADDR(dst, src)  do {\
	*(unsigned short *)(dst) = *(unsigned short *)(src); \
	*(unsigned short *)((dst) + 4) = *(unsigned short *)((src) + 2); \
	*(unsigned short *)((dst) + 8) = *(unsigned short *)((src) + 4); \
	(dst) += 12; \
} while (0)

static void build_setup_frame(struct acte100_priv *priv, uchar *frame)
{
	uchar broadcast[8];
	uchar *mac;
	uchar *frame_save;

	if (NULL == frame) {
		ACTE100_ERR("frame is NULL!\n");
		return;
	}
	memset(broadcast, 0xff, 6);
	memset(frame, 0, SETUP_FRAME_LEN);
	frame_save = frame;
	mac = ethernet_addr(priv->dev);
	COPY_MAC_ADDR(frame, mac);
	mac = broadcast;
	COPY_MAC_ADDR(frame, mac);

	flush_dcache_range((ulong)frame_save,
			(ulong)frame_save + SETUP_FRAME_LEN);
}

static void trans_setup_frame(struct acte100_priv *priv)
{
	struct acte100_regs *regs = priv->regs;
	volatile struct buf_des *tx_bd = (struct buf_des *)regs->txbdbase;
	u32 reg;

	build_setup_frame(priv, NetTxPacket);

	tx_bd[0].status = TXBD_STAT_OWN;  /* make mac own it */
	tx_bd[0].control = TXBD_CTRL_IC | TXBD_CTRL_SET |
				TXBD_CTRL_TBS1(SETUP_FRAME_LEN);
	tx_bd[0].buf_addr1 = (u32)NetTxPacket;
	tx_bd[0].buf_addr2 = 0;

	writel(readl(&regs->opmode) |  MAC_OPMODE_ST, &regs->opmode);
	writel(MAC_TXPOLL_ST, &regs->txpoll);

	ACTE100_INF("opmode: %x\n", readl(&regs->opmode));

	/* ETI indicates data fully transfered into mac fifo */
	setbits_timeout(&regs->status, MAC_STATUS_ETI,
		"wait for transmit setup frame timeout...\n");

	do { /* wait for that host own the buf_des, transmit finished */
		reg = readl(&tx_bd->status);
	} while (reg & TXBD_STAT_OWN);

	writel(readl(&regs->opmode) & ~MAC_OPMODE_ST, &regs->opmode);
	tx_bd[0] = (struct buf_des) { /* restore to non-setup frame */
		.status = 0,  /* host own it */
		.control = TXBD_CTRL_IC,
		.buf_addr1 = 0,
		.buf_addr2 = 0
	};
	/* make acte100_send() use next tx_bd to match with mac internal
	 * pointer that's pointed to next tx_bd after setup frame trans. */
	priv->cur_tx++;
	ACTE100_NOT("transmit setup frame ok\n");
}


static void mac_reset(struct eth_device *dev)
{
	writel(MAC_BMODE_SWR, MAC_CSR0);
	clrbits_timeout(MAC_CSR0, MAC_BMODE_SWR,
		"mac reset timeout\n");
}

static void phy_reset(struct eth_device *dev)
{
	u16 reg = read_phy_reg(MII_BMCR);
	u32 timeout;

	reg |= BMCR_RESET;
	write_phy_reg(MII_BMCR, reg);

	/* wait for PHY reset completed */
	timeout = 9000;
	do {
		mdelay(1);
		timeout--;
		reg = read_phy_reg(MII_BMCR);
	} while (timeout && (reg & BMCR_RESET));
	if (!timeout)
		ACTE100_ERR("wait for phy reset timeout\n");

	timeout = 9000;
	do { /* wait auto-negotiation complete and valid link status */
		mdelay(1);
		timeout--;
		reg = read_phy_reg(MII_BMSR);
	} while (timeout &&
		!((reg & BMSR_ANEGCOMPLETE) && (reg & BMSR_LSTATUS)));
	if (!timeout)
		ACTE100_ERR("phy reset timeout\n");
}

static void acte100_reset(struct eth_device *dev)
{
	struct acte100_priv *priv = (struct acte100_priv *)dev->priv;
	struct acte100_regs *regs = priv->regs;

	mac_reset(dev);

#ifdef CONFIG_SMII_INTERFACE
	writel(MAC_CTRL_SMII, &regs->ctrl); /* use smii interface */
#elif defined(CONFIG_RMII_INTERFACE)
	writel(MAC_CTRL_RMII, &regs->ctrl); /* use rmii interface */
#endif

	phy_reset(dev);
}

static void acte100_setup(struct eth_device *dev)
{
	struct acte100_priv *priv = (struct acte100_priv *)dev->priv;
	struct acte100_regs *regs = priv->regs;

	/* little endian, TAP disabled, descriptor skip length 0 */
	setbits_le32(&regs->busmode, MAC_BMODE_BAR);
	writel((u32)init_rx_buf_des(priv), &regs->rxbdbase);
	writel((u32)init_tx_buf_des(priv), &regs->txbdbase);
	/* enable all interrupts */
	setbits_le32(&regs->ienable, MAC_IEN_ALL);

	/* fill mac address used by flow control to generate pause frame */
	uchar *enet_addr = ethernet_addr(dev);
	writel(*(u32 *)enet_addr, &regs->maclow);
	writel(*(u32 *)(enet_addr + 4), &regs->machigh);

	if (read_phy_reg(MII_BMCR) & BMCR_SPEED100) {
		ACTE100_NOT("BMCR_SPEED 100M\n");
		writel(MAC_OPMODE_FD, &regs->opmode); /* 100Mbps full duplex */
	} else {
		ACTE100_NOT("BMCR_SPEED 10M\n");
		writel(MAC_OPMODE_FD | MAC_OPMODE_10M, &regs->opmode);
	}

	setbits_le32(&regs->opmode, MAC_OPMODE_SF);
	clrbits_le32(&regs->opmode, MAC_OPMODE_PR);

	trans_setup_frame(priv);
}

static void acte100_start(struct eth_device *dev)
{
	struct acte100_priv *priv = (struct acte100_priv *)dev->priv;
	struct acte100_regs *regs = priv->regs;

	setbits_le32(&regs->opmode, MAC_OPMODE_SR);
	writel(MAC_RXPOLL_SR, &regs->rxpoll);

	setbits_le32(&regs->opmode, MAC_OPMODE_ST);
}

static int acte100_init(struct eth_device *dev, bd_t *bd)
{
	clock_enable();
	acte100_reset(dev);
	acte100_setup(dev);
	acte100_start(dev);
	return 0;
}

static void acte100_halt(struct eth_device *dev)
{
	clock_disable();
}


#define RX_ERROR_CARED \
	(RXBD_STAT_DE | RXBD_STAT_RF | RXBD_STAT_TL | RXBD_STAT_CS \
	| RXBD_STAT_DB | RXBD_STAT_CE | RXBD_STAT_ZERO)

/* Get data blocks via Ethernet */
static int acte100_recv(struct eth_device *dev)
{
	struct acte100_priv *priv = (struct acte100_priv *)dev->priv;
	struct acte100_regs *regs = priv->regs;
	volatile struct buf_des *cur_rx = priv->cur_rx;
	u32 status = cur_rx->status;
	u32 reg;
	int pkt_len;

	do {
		reg = readl(&regs->status);
	} while (!(reg & MAC_STATUS_RI));
	/* regs->status = reg; clear status, this make recv slow! */
	writel(reg & ~MAC_STATUS_RI, &regs->status);

	while (0 == (status & RXBD_STAT_OWN)) {
		if (status & RX_ERROR_CARED) {
			priv->stats.rx_errors++;
			ACTE100_ERR("status: %x, rx errors: %lu\n",
					status, priv->stats.rx_errors);
			if (status & RXBD_STAT_TL)
				priv->stats.rx_length_errors++;
			if (status & RXBD_STAT_CE)
				priv->stats.rx_crc_errors++;
			if (status & (RXBD_STAT_RF | RXBD_STAT_DB))
				priv->stats.rx_frame_errors++;
			if (status & RXBD_STAT_ZERO)
				priv->stats.rx_fifo_errors++;
			if (status & RXBD_STAT_DE)
				priv->stats.rx_over_errors++;
			if (status & RXBD_STAT_CS)
				priv->stats.collisions++;
			goto rx_done;
		}

		pkt_len = RXBD_STAT_FL(status);
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pkt_len;

		invalidate_dcache_range((ulong)cur_rx->buf_addr1,
					(ulong)(cur_rx->buf_addr1 + pkt_len));
		/* pass the packet up to the protocol layers, remove 4
		 * crc bytes added by mac at the end of packet */
		NetReceive((uchar *)cur_rx->buf_addr1, pkt_len - 4);
		ACTE100_INF("received a packet, length: %d\n", pkt_len);
rx_done:
		/* mark mac owns the buf_des, and clear other status */
		status = RXBD_STAT_OWN;
		cur_rx->status = status;

		if (cur_rx->control & RXBD_CTRL_RER)
			cur_rx = priv->rx_bd_base;
		else
			cur_rx++;

		status = cur_rx->status;
	}
	priv->cur_rx = (struct buf_des *)cur_rx;

	return 0;
}

/* Send a data block via Ethernet. */
static int acte100_send(struct eth_device *dev, void *packet, int length)
{
	struct acte100_priv *priv = (struct acte100_priv *)dev->priv;
	struct acte100_regs *regs = priv->regs;
	volatile struct buf_des *cur_tx = priv->cur_tx;
	u32 reg;

	ACTE100_INF("packet: %p, length: %d\n", packet, length);

	cur_tx->buf_addr1 = (u32)packet;
	flush_dcache_range((ulong)cur_tx->buf_addr1,
				(ulong)(cur_tx->buf_addr1 + length));

	cur_tx->control &= TXBD_CTRL_IC | TXBD_CTRL_TER; /* clear others */
	cur_tx->control |= TXBD_CTRL_TBS1(length);
	cur_tx->control |= TXBD_CTRL_FS | TXBD_CTRL_LS;
	cur_tx->status = TXBD_STAT_OWN;

	writel(MAC_TXPOLL_ST, &regs->txpoll);

	do { /* TI indicates data fully out to network */
		reg = readl(&regs->status);
	} while (!(reg & MAC_STATUS_TI));
	writel(reg, &regs->status); /* clear status */
	ACTE100_INF("csr6 status: %x\n", reg);
	ACTE100_INF("cur_tx status: %x\n", cur_tx->status);

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += length;

	if (cur_tx->control & TXBD_CTRL_TER)
		priv->cur_tx = priv->tx_bd_base;
	else
		priv->cur_tx++;

	return 0;
}

int acte100_register(bd_t *bis)
{
	struct eth_device *dev;
	struct acte100_priv *priv;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		ACTE100_ERR("malloc dev failed\n");
		return 0;
	}
	memset(dev, 0, sizeof(*dev));

	priv = malloc(sizeof(*priv));
	if (!priv) {
		ACTE100_ERR("malloc priv failed\n");
		free(dev);
		return 0;
	}
	memset(priv, 0, sizeof(*priv));
	priv->regs = (struct acte100_regs *)ETHERNET_BASE;
	priv->dev  = dev;

	dev->iobase = ETHERNET_BASE;
	dev->priv = priv;
	dev->init = acte100_init;
	dev->halt = acte100_halt;
	dev->send = acte100_send;
	dev->recv = acte100_recv;

	acte100_get_enetaddr(dev);

	sprintf(dev->name, ACTE100_DRIVERNAME);

	eth_register(dev);
	return 1;
}

