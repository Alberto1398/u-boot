/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/serial.h>
#include <asm/arch/clocks.h>
#include <asm/arch/gmp.h>
#include <asm/io.h>
#include <common.h>
#include <fdtdec.h>
#include <serial.h>
#include <linux/compiler.h>

#define OWLXX_UART_OFSET (0x2000)
#define OWLXX_UART_BASE(x) (UART0_BASE + (x) * OWLXX_UART_OFSET)

#define OWLXX_UART_INDX ((afinfo->boot_dev != OWLXX_BOOTDEV_SD0)?(afinfo->uart_index):(afinfo->burn_uart))

struct serial_owlxx_regs {
	u32 ctl;
	u32 rxdat;
	u32 txdat;
	u32 stat;
};

enum UART_CTRL_INDEX {
	UART_CTRL_INDEX0 = 0,
	UART_CTRL_INDEX1,
	UART_CTRL_INDEX2,
	UART_CTRL_INDEX3,
	UART_CTRL_INDEX4,
	UART_CTRL_INDEX5,
	UART_CTRL_INDEX6,
};

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_OF_LIBFDT) && !defined(CONFIG_SPL_BUILD)

#define OWLXX_UART_NUM_CONTROLLERS 8
static int owlxx_serial_fdt_init(void)
{
	const void *blob = gd->fdt_blob;
	const char *cell;
	char *p;
	int node, index, count;
	int node_list[OWLXX_UART_NUM_CONTROLLERS];

	node = fdt_path_offset(blob, "/chosen");
	if (node < 0)
		return -1;

	cell = fdt_getprop(blob, node, "bootargs", NULL);
	if (!cell)
		return -1;

	p = strstr(cell, "console=");
	if (!p)
		return -1;

	/* console=ttySx, x -> 12 */
	index = p[12] - '0';
	count = fdtdec_find_aliases_for_id(blob, "serial",
		COMPAT_ACTIONS_OWLXX_UART, node_list,
		OWLXX_UART_NUM_CONTROLLERS);

	if (count < 0 || count < index)
		return -1;

	//if (fdtdec_get_is_enabled(blob, node_list[index]))
		//afinfo->uart_index = index;

	return 0;
}
#endif

static inline void owlxx_serial_config(int val)
{
	/* pad and mpf */
#define PADE_BIT (0x1 << 1)
	writel(PADE_BIT, PAD_CTL);
	switch (val) {
	case UART_CTRL_INDEX0:
		setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_UART0);
		/* P_UART0_RX, P_UART0_TX */
#define UART0_RX_BITS (0x7 << 0)
#define UART0_TX_BITS (0x7 << 19)
		clrbits_le32(MFP_CTL2, UART0_RX_BITS);
		clrbits_le32(MFP_CTL3, UART0_TX_BITS);
		break;
	case UART_CTRL_INDEX2:
		setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_UART2);
		/* 359P, P_SD0_D0 -> UART2_RX, P_SD0_D1 -> UART2_TX */
#define UART2_RX_BITS (0x4 << 17)
#define UART2_RX_BITS_MASK (0x7 << 17)
#define UART2_TX_BITS (0x4 << 14)
#define UART2_TX_BITS_MASK (0x7 << 14)
		clrsetbits_le32(MFP_CTL2,
				UART2_RX_BITS_MASK | UART2_TX_BITS_MASK,
				UART2_RX_BITS | UART2_TX_BITS);
		break;
	case UART_CTRL_INDEX3:
		setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_UART3);
		break;		
	case UART_CTRL_INDEX4:
#define UART4_RX_BITS (0x3 << 11)
#define UART4_RX_BITS_MASK (0x3 << 11)
#define UART4_TX_BITS (0x1 << 6)
#define UART4_TX_BITS_MASK (0x3 << 6)		
		setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_UART4);
		clrsetbits_le32(MFP_CTL0,
				UART4_RX_BITS_MASK | UART4_TX_BITS_MASK ,
				UART4_RX_BITS | UART4_TX_BITS);

		/* UART4 is auto enabled */
		break;
	case UART_CTRL_INDEX5:
		setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_UART5);
#if defined(CONFIG_ATM7059TC) || defined(CONFIG_ATM7059A)
		/*	P_SD0_D0 -> UART5_RX, P_SD0_D1 -> UART5_TX	*/
#define UART5_RX_BITS (0x5 << 17)
#define UART5_RX_BITS_MASK (0x7 << 17)
#define UART5_TX_BITS (0x5 << 14)
#define UART5_TX_BITS_MASK (0x7 << 14)
		clrsetbits_le32(MFP_CTL2,
				UART5_RX_BITS_MASK | UART5_TX_BITS_MASK ,
				UART5_RX_BITS | UART5_TX_BITS);
#else
		/* P_KS_OUT0 -> UART5_RX, P_KS_OUT2 -> UART5_TX */
#define UART5_RX_BITS (0x1 << 26)
#define UART5_RX_BITS_MASK (0x7 << 26)
#define UART5_TX_BITS (0x4 << 23)
#define UART5_TX_BITS_MASK (0x7 << 23)
		clrsetbits_le32(MFP_CTL1,
				UART5_RX_BITS_MASK | UART5_TX_BITS_MASK ,
				UART5_RX_BITS | UART5_TX_BITS);				

#endif
		break;
	default:
		break;
	}
}

static inline void owlxx_serial_reset(u32 index)
{
	u32 i, val;

	switch (index) {
	case UART_CTRL_INDEX0:
		val = CMU_RST_UART0;
		break;
	case UART_CTRL_INDEX1:
		val = CMU_RST_UART1;
		break;
	case UART_CTRL_INDEX2:
		val = CMU_RST_UART2;
		break;
	case UART_CTRL_INDEX3:
		val = CMU_RST_UART3;
		break;
	case UART_CTRL_INDEX4:
		val = CMU_RST_UART4;
		break;
	case UART_CTRL_INDEX5:
		val = CMU_RST_UART5;
		break;
	case UART_CTRL_INDEX6:
		val = CMU_RST_UART6;
		break;
	default:
		val = 0;
	}
	clrbits_le32(CMU_DEVRST1, val);
	for (i = 0; i < 1000; i++)
		;
	setbits_le32(CMU_DEVRST1, val);
}

void owlxx_serial_setbrg(void)
{
	u32 addr;
	struct serial_owlxx_regs *r = (struct serial_owlxx_regs *)
		OWLXX_UART_BASE(OWLXX_UART_INDX);

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	if (afinfo->uart_baudrate)
		gd->baudrate = afinfo->uart_baudrate;

	switch (OWLXX_UART_INDX) {
	case UART_CTRL_INDEX0:
		addr = CMU_UART0CLK;
		break;
	case UART_CTRL_INDEX1:
		addr = CMU_UART1CLK;
		break;
	case UART_CTRL_INDEX2:
		addr = CMU_UART2CLK;
		break;
	case UART_CTRL_INDEX3:
		addr = CMU_UART3CLK;
		break;
	case UART_CTRL_INDEX4:
		addr = CMU_UART4CLK;
		break;
	case UART_CTRL_INDEX5:
		addr = CMU_UART5CLK;
		break;
	case UART_CTRL_INDEX6:
		addr = CMU_UART6CLK;
		break;
	default:
		addr = 0;
	}
	/* error checking */
	if (!addr)
		return;
	/* uart clock settings */
	writel(0x19, addr);

	owlxx_serial_reset(OWLXX_UART_INDX);
	/* uart configure, UART0_CTL
	 * bit20 = 0, Loopback Disabled
	 * bit15=1, Uart EN
	 * bit[1:0]=11, 8bit Width
	 */
	writel(0x008003, &r->ctl);
}

void owlxx_serial_release(int uart_index)
{
	struct serial_owlxx_regs *r = (struct serial_owlxx_regs *)
		OWLXX_UART_BASE(uart_index);

	writel(0, &r->ctl);
}

int owlxx_serial_init(void)
{
    static int owlxx_uart_index = -1;
    
    if(owlxx_uart_index != OWLXX_UART_INDX) {
        if(owlxx_uart_index > 0) {    //同一时刻仅有一个串口可以使能，否则kernel的early_print将无法使用
            owlxx_serial_release(owlxx_uart_index);
        }
        owlxx_uart_index = OWLXX_UART_INDX;
    }
    
#if defined(CONFIG_OF_LIBFDT) && !defined(CONFIG_SPL_BUILD)
	/* parse fdt, get configure settings */
	//owlxx_serial_fdt_init();
#endif

	owlxx_serial_config(OWLXX_UART_INDX);
	/* uart configure, UART0_CTL
	 * bit20 = 0, Loopback Disabled
	 * bit15=1, Uart EN
	 * bit[1:0]=11, 8bit Width
	 */
	owlxx_serial_setbrg();

	return 0;
}

void owlxx_serial_putc(const char c)
{
	struct serial_owlxx_regs *r = (struct serial_owlxx_regs *)
		OWLXX_UART_BASE(OWLXX_UART_INDX);
#if defined(CONFIG_ATM7059TC) || defined(CONFIG_ATM7059A)
	writel(c, &r->txdat);	/*	ATM7059TC only support word operation	*/
#else
	writeb(c, &r->txdat);
#endif
	while (readl(&r->stat) & 0x20000)
		;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

int owlxx_serial_getc(void)
{
	struct serial_owlxx_regs *r = (struct serial_owlxx_regs *)
		OWLXX_UART_BASE(OWLXX_UART_INDX);

	/* reset uart status */
	writel(0x03, &r->stat);
	/* wait for data 0: not, 1: empty */
	while (readl(&r->stat) & 0x20)
		;

	return readl(&r->rxdat) & 0xFF;
}

int owlxx_serial_tstc(void)
{
	struct serial_owlxx_regs *r = (struct serial_owlxx_regs *)
		OWLXX_UART_BASE(OWLXX_UART_INDX);

	return !(readl(&r->stat) & 0x20);
}

static struct serial_device owlxx_serial_drv = {
	.name	= "owlxx_serial",
	.start	= owlxx_serial_init,
	.stop	= NULL,
	.setbrg	= owlxx_serial_setbrg,
	.putc	= owlxx_serial_putc,
	.puts	= default_serial_puts,
	.getc	= owlxx_serial_getc,
	.tstc	= owlxx_serial_tstc,
};

void owlxx_serial_initialize(void)
{
	serial_register(&owlxx_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &owlxx_serial_drv;
}
