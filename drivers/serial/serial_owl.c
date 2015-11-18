/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

/* UART	Register offset	*/
#define	UART_CTL		(0x0)
#define	UART_RXDAT		(0x4)
#define	UART_TXDAT		(0x8)
#define	UART_STAT		(0xc)

/* UART_STAT */
#define	UART_STAT_TFES		(0x1 <<	10)	/* TX FIFO Empty Status	*/
#define	UART_STAT_RFFS		(0x1 <<	9)	/* RX FIFO full	Status */
#define	UART_STAT_TFFU		(0x1 <<	6)	/* TX FIFO full	Status */
#define	UART_STAT_RFEM		(0x1 <<	5)	/* RX FIFO Empty Status	*/

struct owl_uart_dev {
	unsigned long base;
	unsigned int id;
};

int owl_serial_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int owl_serial_getc(struct udevice *dev)
{
	struct owl_uart_dev *uart = dev_get_priv(dev);

	if (readl(uart->base + UART_STAT) & UART_STAT_RFEM)
		return -EAGAIN;

	return (int)(readl(uart->base +	UART_RXDAT));
}

static int owl_serial_putc(struct udevice *dev,	const char ch)
{
	struct owl_uart_dev *uart = dev_get_priv(dev);

	if (readl(uart->base + UART_STAT) & UART_STAT_TFFU)
		return -EAGAIN;

	writel(ch, uart->base +	UART_TXDAT);

	return 0;
}

static int owl_serial_pending(struct udevice *dev, bool	input)
{
	struct owl_uart_dev *uart = dev_get_priv(dev);
	unsigned int stat = readl(uart->base + UART_STAT);

	if (input)
		return !(stat &	UART_STAT_RFEM);
	else
		return !(stat &	UART_STAT_TFES);
}

static int owl_serial_probe(struct udevice *dev)
{
	return 0;
}

static int owl_serial_ofdata_to_platdata(struct	udevice	*dev)
{
	struct owl_uart_dev *uart = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = fdtdec_get_addr(gd->fdt_blob, dev->of_offset, "reg");
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	uart->base = (unsigned long)addr;
	uart->id = dev->req_seq;

	return 0;
}

static const struct dm_serial_ops owl_serial_ops = {
	.putc =	owl_serial_putc,
	.pending = owl_serial_pending,
	.getc =	owl_serial_getc,
	.setbrg	= owl_serial_setbrg,
};

static const struct udevice_id owl_serial_ids[] = {
	{ .compatible =	"actions,s900-serial" },
	{ }
};

U_BOOT_DRIVER(serial_owl) = {
	.name	= "serial_owl",
	.id	= UCLASS_SERIAL,
	.of_match = owl_serial_ids,
	.ofdata_to_platdata = owl_serial_ofdata_to_platdata,
	.priv_auto_alloc_size =	sizeof(struct owl_uart_dev),
	.probe = owl_serial_probe,
	.ops	= &owl_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
