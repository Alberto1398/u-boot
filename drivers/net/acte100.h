#ifndef _ACTE100_H_
#define _ACTE100_H_
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

#include <asm/arch/regs.h>
#include <linux/netdevice.h>

#define ACTE100_DRIVERNAME "acte100"

#define ACTE100_RX_BUF CONFIG_ACTE100_RX_BUF
#define ACTE100_TX_BUF CONFIG_ACTE100_TX_BUF

#ifdef CONFIG_ACTE100_PHY_ADDR
#define ACTE100_PHY_ADDR CONFIG_ACTE100_PHY_ADDR
#else
/* realtek phy demo =1,micrel phy demo = 0,FARADAY PHY = 1 */
#define ACTE100_PHY_ADDR (0x1)
#endif

#define SETUP_FRAME_LEN (192) /* bytes */
#define ACTE100_BD_ADDR_ALIGN (16) /* bytes */


#define ACTE100_NOTICE

#define ACTE100_ERR(fmt, args...) \
	printf("acte100:%s():%d: ERR: " fmt, __func__, __LINE__, ##args)

#ifdef ACTE100_DEBUG
#define ACTE100_DBG(fmt, args...) \
	printf("acte100:%s():%d: DBG: " fmt, __func__, __LINE__, ##args)
#else
#define ACTE100_DBG(fmt, args...)	do { } while (0)
#endif

#ifdef ACTE100_INFO
#define ACTE100_INF(fmt, args...) \
	printf("acte100:%s():%d: INF: " fmt, __func__, __LINE__, ##args)
#else
#define ACTE100_INF(fmt, args...)	do { } while (0)
#endif

#ifdef ACTE100_NOTICE
#define ACTE100_NOT(fmt, args...) \
	printf("acte100:%s():%d: NOT: " fmt, __func__, __LINE__, ##args)
#else
#define ACTE100_NOT(fmt, args...)	do { } while (0)
#endif


/* must not access reserved*. */
struct acte100_regs {
	u32 busmode;   /* offset 0x0; bus mode reg. */
	u32 reserved0;
	u32 txpoll;    /* 0x08; transmit poll demand reg. */
	u32 reserved1;
	u32 rxpoll;    /* 0x10; receive poll demand reg. */
	u32 reserved2;
	u32 rxbdbase;  /* 0x18; receive descriptor list base address reg. */
	u32 reserved3;
	u32 txbdbase;  /* 0x20; transmit descriptor list base address reg. */
	u32 reserved4;
	u32 status;    /* 0x28; status reg. */
	u32 reserved5;
	u32 opmode;    /* 0x30; operation mode reg. */
	u32 reserved6;
	u32 ienable;   /* 0x38; interrupt enable reg. */
	u32 reserved7;
	u32 mfocnt;    /* 0x40; missed frames and overflow counter reg. */
	u32 reserved8;
	u32 miimng;    /* 0x48; software mii, don't use it here  */
	u32 reserved9;
	u32 miism;     /* 0x50; mii serial management */
	u32 reserved10;
	/* 0x58; general-purpose timer and interrupt mitigation control */
	u32 imctrl;
	u32 reserved11[9];
	u32 maclow;    /* 0x80; mac address low */
	u32 reserved12;
	u32 machigh;   /* 0x88; mac address high */
	u32 reserved13;
	u32 cachethr;  /* 0x90; pause time and cache thresholds */
	u32 reserved14;
	u32 fifothr;   /* 0x98; pause control fifo thresholds */
	u32 reserved15;
	u32 flowctrl;  /* 0xa0; flow control setup and status */
	u32 reserved16[3];
	u32 ctrl;      /* 0xb0; mac control with clk and rmii&smii selection */

	/* 0x200~0x3f8; statistics regs not used in u-boot. */
};

/* receive and transmit buffer descriptor */
struct buf_des {
	u32 status;
	u32 control;
	u32 buf_addr1;
	u32 buf_addr2;     /* we don't use second buffer address */
};

struct acte100_priv {
	struct acte100_regs *regs;
	struct eth_device *dev;

	volatile struct buf_des *tx_bd_base;
	volatile struct buf_des *cur_tx;

	volatile struct buf_des *rx_bd_base;
	volatile struct buf_des *cur_rx;

	struct net_device_stats stats;
};


/*
 * rx bd status and control information
 */
#define RXBD_STAT_OWN (0x1 << 31)
#define RXBD_STAT_FF  (0x1 << 30)       /*filtering fail */
#define RXBD_STAT_FL(x) (((x) >> 16) & 0x3FFF)  /* frame leng */
#define RXBD_STAT_ES  (0x1 << 15)       /* error summary */
#define RXBD_STAT_DE  (0x1 << 14)       /* descriptor error */
#define RXBD_STAT_RF  (0x1 << 11)       /* runt frame */
#define RXBD_STAT_MF  (0x1 << 10)       /* multicast frame */
#define RXBD_STAT_FS  (0x1 << 9)        /* first descriptor */
#define RXBD_STAT_LS  (0x1 << 8)        /* last descriptor */
#define RXBD_STAT_TL  (0x1 << 7)        /* frame too long */
#define RXBD_STAT_CS  (0x1 << 6)        /* collision */
#define RXBD_STAT_FT  (0x1 << 5)        /* frame type */
#define RXBD_STAT_RE  (0x1 << 3)        /* mii error */
#define RXBD_STAT_DB  (0x1 << 2)        /* byte not aligned */
#define RXBD_STAT_CE  (0x1 << 1)        /* crc error */
#define RXBD_STAT_ZERO  (0x1)

#define RXBD_CTRL_RER (0x1 << 25)       /* receive end of ring */
#define RXBD_CTRL_RCH (0x1 << 24)       /* using second buffer, not used here */
#define RXBD_CTRL_RBS1(x) ((x) & 0x7FF) /* buffer1 size */

/*
 * tx bd status and control information
 */
#define TXBD_STAT_OWN (0x1 << 31)
#define TXBD_STAT_ES  (0x1 << 15)       /* error summary */
#define TXBD_STAT_LO  (0x1 << 11)       /* loss of carrier */
#define TXBD_STAT_NC  (0x1 << 10)       /* no carrier */
#define TXBD_STAT_LC  (0x1 << 9)        /* late collision */
#define TXBD_STAT_EC  (0x1 << 8)        /* excessive collision */
#define TXBD_STAT_CC(x)  (((x) >> 3) & 0xF)     /* collision count  */
#define TXBD_STAT_UF  (0x1 << 1)        /* underflow error */
#define TXBD_STAT_DE  (0x1)             /* deferred */

#define TXBD_CTRL_IC   (0x1 << 31)      /* interrupt on completion */
#define TXBD_CTRL_LS   (0x1 << 30)      /* last descriptor */
#define TXBD_CTRL_FS   (0x1 << 29)      /* first descriptor */
#define TXBD_CTRL_FT1  (0x1 << 28)      /* filtering type  */
#define TXBD_CTRL_SET  (0x1 << 27)      /* setup packet */
#define TXBD_CTRL_AC   (0x1 << 26)      /* add crc disable */
#define TXBD_CTRL_TER  (0x1 << 25)      /* transmit end of ring */
#define TXBD_CTRL_TCH  (0x1 << 24)      /* second address chainded */
#define TXBD_CTRL_DPD  (0x1 << 23)      /* disabled padding */
#define TXBD_CTRL_FT0  (0x1 << 22)      /* filter type, togethor with 28bit*/
#define TXBD_CTRL_TBS2(x)  (((x) & 0x7FF) << 11) /* buf2 size, no use here */
#define TXBD_CTRL_TBS1(x)  ((x) & 0x7FF)         /* buf1 size */
#define TXBD_CTRL_TBS1M (0x7FF)

/*
 * bus mode register
 */
#define MAC_BMODE_DBO    (0x1 << 20)     /*descriptor byte ordering mode */
#define MAC_BMODE_TAP(x) (((x) & 0x7) << 17)     /*transmit auto-polling */
#define MAC_BMODE_PBL(x) (((x) & 0x3F) << 8)     /*programmable burst length */
#define MAC_BMODE_BLE    (0x1 << 7)  /*big or little endian for data buffers */
#define MAC_BMODE_DSL(x) (((x) & 0x1F) << 2)     /*descriptors skip length */
#define MAC_BMODE_BAR    (0x1 << 1)      /*bus arbitration mode */
#define MAC_BMODE_SWR    (0x1)   /*software reset */

/*
 * transmit and receive poll demand register
 */
/* leave suspended mode to running mode to start xmit */
#define MAC_TXPOLL_ST  (0x1)
/* leave suspended to running mode */
#define MAC_RXPOLL_SR  (0x1)

/*
 * status register
 */
#define MAC_STATUS_TSM  (0x7 << 20)      /*transmit process state */
#define MAC_STATUS_RSM  (0x7 << 17)      /*receive process state */
#define MAC_STATUS_NIS  (0x1 << 16)      /*normal interrupt summary */
#define MAC_STATUS_AIS  (0x1 << 15)      /*abnormal interrupt summary */
#define MAC_STATUS_ERI  (0x1 << 14)      /*early receive interrupt */
#define MAC_STATUS_GTE  (0x1 << 11)      /*general-purpose timer expiration */
#define MAC_STATUS_ETI  (0x1 << 10)      /*early transmit interrupt */
#define MAC_STATUS_RPS  (0x1 << 8)       /*receive process stopped */
#define MAC_STATUS_RU   (0x1 << 7)       /*receive buffer unavailable */
#define MAC_STATUS_RI   (0x1 << 6)       /*receive interrupt */
#define MAC_STATUS_UNF  (0x1 << 5)       /*transmit underflow */
#define MAC_STATUS_LCIS (0x1 << 4)       /* link change status */
#define MAC_STATUS_LCIQ (0x1 << 3)       /* link change interrupt */
#define MAC_STATUS_TU   (0x1 << 2)       /*transmit buffer unavailable */
#define MAC_STATUS_TPS  (0x1 << 1)       /*transmit process stopped */
#define MAC_STATUS_TI   (0x1)            /*transmit interrupt */

/*
 * operation mode register
 */
#define MAC_OPMODE_RA  (0x1 << 30)       /*receive all */
#define MAC_OPMODE_TTM (0x1 << 22)       /*transmit threshold mode */
#define MAC_OPMODE_SF  (0x1 << 21)       /*store and forward */
#define MAC_OPMODE_SPEED(x) (((x) & 0x3) << 16)  /*eth speed selection */
#define MAC_OPMODE_10M (0x1 << 17)  /* set when work on 10M, otherwise 100M */
#define MAC_OPMODE_TR(x)    (((x) & 0x3) << 14)  /*threshold control bits */
#define MAC_OPMODE_ST  (0x1 << 13)       /*start or stop transmit command */
#define MAC_OPMODE_LP  (0x1 << 10)       /*loopback mode */
#define MAC_OPMODE_FD  (0x1 << 9)        /*full duplex mode */
#define MAC_OPMODE_PM  (0x1 << 7)        /*pass all multicast */
#define MAC_OPMODE_PR  (0x1 << 6)        /*prmiscuous mode */
#define MAC_OPMODE_IF  (0x1 << 4)        /*inverse filtering */
#define MAC_OPMODE_PB  (0x1 << 3)        /*pass bad frames */
#define MAC_OPMODE_HO  (0x1 << 2)        /*hash only filtering mode */
#define MAC_OPMODE_SR  (0x1 << 1)        /*start or stop receive command */
#define MAC_OPMODE_HP  (0x1)        /*hash or perfect receive filtering mode */

/*
 * interrupt enable register
 */
#define MAC_IEN_LCIE (0x1 << 17) /*link change interrupt enable */
#define MAC_IEN_NIE (0x1 << 16)  /*normal interrupt summary enable */
#define MAC_IEN_AIE (0x1 << 15)  /*abnormal interrupt summary enable */
#define MAC_IEN_ERE (0x1 << 14)  /*early receive interrupt enable */
#define MAC_IEN_GTE (0x1 << 11)  /*general-purpose timer overflow */
#define MAC_IEN_ETE (0x1 << 10)  /*early transmit interrupt enable */
#define MAC_IEN_RSE (0x1 << 8)   /*receive stopped enable */
#define MAC_IEN_RUE (0x1 << 7)   /*receive buffer unavailable enable */
#define MAC_IEN_RIE (0x1 << 6)   /*receive interrupt enable */
#define MAC_IEN_UNE (0x1 << 5)   /*underflow interrupt enable */
#define MAC_IEN_TUE (0x1 << 2)   /*transmit buffer unavailable enable */
#define MAC_IEN_TSE (0x1 << 1)   /*transmit stopped enable */
#define MAC_IEN_TIE (0x1)        /*transmit interrupt enable */
#define MAC_IEN_ALL (0x3ffff)


/*
 * missed frames and overflow counter register
 */
#define MAC_MFOCNT_OCO  (0x1 << 28)      /*overflow flag */
#define MAC_MFOCNT_FOCM (0x3FF << 17)    /*fifo overflow counter */
#define MAC_MFOCNT_MFO  (0x1 << 16)      /*missed frame flag */
#define MAC_MFOCNT_MFCM (0xFFFF)         /*missed frame counter */

/*
 * general purpose timer and interrupt mitigation control register
 */
#define MAC_IMCTRL_CS     (0x1 << 31)    /*cycle size */
#define MAC_IMCTRL_TT(x)  (((x) & 0xF) << 27)    /*transmit timer */
#define MAC_IMCTRL_NTP(x) (((x) & 0x7) << 24)    /*number of transmit packets */
#define MAC_IMCTRL_RT(x)  (((x) & 0xF) << 20)    /*receive timer */
#define MAC_IMCTRL_NRP(x) (((x) & 0x7) << 17)    /*number of receive packets */
#define MAC_IMCTRL_CON    (0x1 << 16)    /*continuous mode */
#define MAC_IMCTRL_TIMM   (0xFFFF)       /*timer value */
#define MAC_IMCTRL_TIM(x) ((x) & 0xFFFF) /*timer value */

/*
 * pause time and cache thresholds register
 */
/*cache pause threshold level */
#define MAC_CACHETHR_CPTL(x) (((x) & 0xFF) << 24)
/*cache restart threshold level */
#define MAC_CACHETHR_CRTL(x) (((x) & 0xFF) << 16)
/*flow control pause quanta time */
#define MAC_CACHETHR_PQT(x)  ((x) & 0xFFFF)

/*
 * fifo thresholds register
 */
/*fifo pause threshold level */
#define MAC_FIFOTHR_FPTL(x) (((x) & 0xFFFF) << 16)
/*fifo restart threshold level */
#define MAC_FIFOTHR_FRTL(x) ((x) & 0xFFFF)

/*
 * flow control setup and status
 */
#define MAC_FLOWCTRL_FCE (0x1 << 31)     /*flow control enable */
#define MAC_FLOWCTRL_TUE (0x1 << 30)     /*transmit un-pause frames enable */
#define MAC_FLOWCTRL_TPE (0x1 << 29)     /*transmit pause frames enable */
#define MAC_FLOWCTRL_RPE (0x1 << 28)     /*receive pause frames enable */
/*back pressure enable (only half-dup) */
#define MAC_FLOWCTRL_BPE (0x1 << 27)
#define MAC_FLOWCTRL_ENALL (0x1F << 27)
#define MAC_FLOWCTRL_PRS (0x1 << 1)      /*pause request sent */
#define MAC_FLOWCTRL_HTP (0x1)           /*host transmission paused */

/*
 * mac control with clk and rmii&smii selection
 */
#define MAC_CTRL_SMII (0X41) /* use smii; bit8: 0 REFCLK output, 1 input*/
#define MAC_CTRL_RMII (0x0)  /* use rmii */


/*
 * phy control register
 */
#define PHY_CTRL_PDMC(x) (((x) & 0x3) << 16)  /*power down mode control */
#define PHY_CTRL_LOOP    (0x1 << 12)           /*loopback mode */
#define PHY_CTRL_MHZ(x)  (((x) & 0x3) << 4)   /* (x+1) times of 25MHZ */

/*
 * phy status register
 */
#define PHY_STATUS_LINKED (0x1)       /*link status */

/*
 * phy reset control signals
 */
#define PHY_RESET_RST  (0x1)  /*low active reset signal */

/*
 * the mii serial management register
 */
#define MII_MNG_SB  (0x1 << 31) /*start transfer or busy */
#define MII_MNG_CLKDIV(x) (((x) & 0x7) << 28)     /*clock divider */
#define MII_MNG_OPCODE(x) (((x) & 0x3) << 26)     /*operation mode */
#define MII_MNG_PHYADD(x) (((x) & 0x1F) << 21)    /*physical layer address */
#define MII_MNG_REGADD(x) (((x) & 0x1F) << 16)    /*register address */
#define MII_MNG_DATAM (0xFFFF)                     /*register data mask */
#define MII_MNG_DATA(x)   ((MII_MNG_DATAM) & (x))  /* data to write */
#define MII_OP_WRITE 0x1
#define MII_OP_READ  0x2
#define MII_OP_CDS   0x3

#endif /* _ACTE100_H_ */
