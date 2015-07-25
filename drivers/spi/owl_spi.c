#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>
#include <asm/io.h>
#include <common.h>
#include <asm/arch/pmu.h>
#include <asm/arch/spi.h>
#include <spi.h>

#define SPI_BASE(num) ((SPI0_BASE) + (num) * 0x4000)

struct owlxx_spi_regs {
	u32 ctl;
	u32 clkdiv;
	u32 stat;
	u32 rxdat;
	u32 txdat;
};

void spi_clk_init(int num)
{
	if (num != 0) {
		setbits_le32(CMU_DEVCLKEN1, (1 << (10 + num)));
		clrbits_le32(CMU_DEVRST1, (1 << (8 + num)));
		setbits_le32(CMU_DEVRST1, (1 << (8 + num)));
	}
}

void spi_mfp_init(int num)
{
	if (num == 1) {
		clrsetbits_le32(MFP_CTL2, 0x7, 0x2); /*MISO over uart0_rx */
		clrsetbits_le32(MFP_CTL3, (0x7 << 19), (0x2 << 19)); /*SS over uart0_tx */
		clrsetbits_le32(MFP_CTL3, (0x7 << 16), (0x4 << 16)); /*MOSI,CLK over i2c0 */
	}
}

void spi_init(void)
{
	int num = g_atc260x_spi_slave.bus;
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	debug("spi_init()\n");

	writel(0x1, CMU_ETHERNETPLL);

	/*	Test pad can not switch to 24MHz clock bug. so use CMU debug signal instead.	*/
#if defined(CONFIG_ATM7059TC) || defined(CONFIG_ATM7059A)
	writel(0x8000002D, CMU_DIGITALDEBUG);	/*	Select timer_clk	*/
	writel(0x10, DEBUG_SEL);	/*	CMU debug select	*/
	writel(0x20000000, DEBUG_OEN0);	/*	P_KS_OUT2: Debug29	*/
#else
	setbits_le32(MFP_CTL3, 0x1 << 30);
#endif
	spi_clk_init(num);
	spi_mfp_init(num);

	writel(15, &s->clkdiv);    /*set SPI_CLK=H_clk/(CLKDIV*2)*/
	/* fpga 6Mhz */
	writel(0x001d0, &s->ctl);  /* select SPI 16bit mode 3 */
	writel(0xff, &s->stat);    /* clear SPI status register */
	setbits_le32(&s->ctl, 0x1 << 18);
}

int wait_transmit_complete(int num)
{
	int timeout = 1000;
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	while ((timeout > 0) &&
		((readl(&s->stat) & 0x04) == 0))
		timeout--;

	if (timeout <= 0) {
		printf("spi transfer timeout! (SPICTL:%08x, SPISTAT:%x)\n",
			readl(&s->ctl), readl(&s->stat));
		return -1;
	}

	setbits_le32(&s->stat, 0x4);

	return 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	int num = slave->bus;
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	clrbits_le32(&s->ctl, 0x10);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	int num = slave->bus;
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	setbits_le32(&s->ctl, 0x10);
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	int num = slave->bus;
	unsigned int ahb_clk, core_clk, div;
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	core_clk = readl(CMU_COREPLL);
#ifdef CONFIG_ATM7059A
	div = ((readl(CMU_BUSCLK) & 0x0c) >> 2) + 1;
#else
	div = ((readl(CMU_BUSCLK) & 0x30) >> 4) + 1;
#endif
	ahb_clk = core_clk / div;
	div = (ahb_clk + 2 * hz - 1) / (2 * hz);
	writel(div, &s->clkdiv);
}
/*dout write din read*/

int  spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	int num = slave->bus;
	unsigned int val[2];
	struct owlxx_spi_regs *s =
		(struct owlxx_spi_regs *)SPI_BASE(num);

	if (!dout)
		return -1;
	switch (bitlen) {
	case 8:
		clrbits_le32(&s->ctl, 0x300);
		val[0] = (*(u8 *)dout) & 0xff;
		val[1] = *((u8 *)dout + 1) & 0xff;
		break;
	case 16:
	  clrsetbits_le32(&s->ctl, 0x300, 0x100);
		val[0] = (*(u16 *)dout) & 0xffff;
		val[1] = *((u16 *)dout + 1) & 0xffff;
	  break;
	case 32:
		setbits_le32(&s->ctl, 0x300);
		val[0] = *(u32 *)dout;
		val[1] = *((u32 *)dout + 1);
		break;
	default:
		return -1;
	}

	writel(0x30, &s->stat);    /*clear SPI FIFO */

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	writel(val[0], &s->txdat);
	wait_transmit_complete(num);

	writel(0x30, &s->stat);    /* clear SPI FIFO */

	if (din == NULL) { /* write */
		writel(val[1], &s->txdat);      /* write data */
		wait_transmit_complete(num);
	} else {
		writel(0, &s->txdat); /* generate SPI read clock */
		wait_transmit_complete(num);
		*(u32 *)din = readl(&s->rxdat);    /* read data */
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
