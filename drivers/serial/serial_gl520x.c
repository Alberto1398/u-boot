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

/* 因为打算在SPL中使用, 这里要尽量节省空间.
 * 搞得很多表也是因为这个原因.  */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>
#include <serial.h>


/* UART Register offset */
#define UART_CTL_OFFSET                (0x0)
#define UART_RXDAT_OFFSET              (0x4U)
#define UART_TXDAT_OFFSET              (0x8U)
#define UART_STAT_OFFSET               (0xcU)

#define UART_CTL    (s_gl520x_serial_drv.reg_base + UART_CTL_OFFSET)
#define UART_RXDAT  (s_gl520x_serial_drv.reg_base + UART_RXDAT_OFFSET)
#define UART_TXDAT  (s_gl520x_serial_drv.reg_base + UART_TXDAT_OFFSET)
#define UART_STAT   (s_gl520x_serial_drv.reg_base + UART_STAT_OFFSET)

#define GL520X_UART_BASE(x) (UART0_BASE + (x) * 0x2000U)
#define GL520X_UART_CH_CNT  6

struct act_uart_mfp_cfg_entry
{
    uint32_t reg;
    uint32_t mask;
    uint32_t val;
};

struct act_uart_mfp_cfg
{
    uint8_t     chan;
    uint8_t     mfp;
    struct act_uart_mfp_cfg_entry mfp_cfg[2];
};


typedef struct
{
	struct serial_device base_dev;
	uint32_t reg_base;
	uint8_t  uart_ch;
	uint8_t  mfp_grp;
} act_serial_device_t;


static int _gl520x_serial_init(void);
#ifndef CONFIG_SPL_BUILD
static int _gl520x_serial_exit(void);
#endif
static void _gl520x_serial_setbrg(void);
static void _gl520x_serial_putc(const char c);
static int _gl520x_serial_getc(void);
static int _gl520x_serial_tstc(void);


static const struct act_uart_mfp_cfg sc_gl520x_uart_mfp_cfg[] = {
    {
        .chan = 5,
        .mfp = 0,       /* from sd0 pads */
        .mfp_cfg = {
            {
                .reg = MFP_CTL2,
                .mask = 0x000fc000,
                .val = 0x000b4000,
            },
        },
    },

    {
        .chan = 5,
        .mfp = 1,   /* from ks pads */
        .mfp_cfg = {
            {
                .reg = MFP_CTL1,
                .mask = 0x1f800000,
                .val = 0x06000000,
            },
        },
    },

    {
        .chan = 3,
        .mfp = 0,   /* from SENS1_D[1:0] */
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202)
            {0},
#elif defined(CONFIG_ACTS_GL5207)
            {
                .reg = MFP_CTL2,
                .mask = 0x00800000,
                .val = 0x00000000,
            },
            {
                .reg = MFP_CTL3,
                .mask = 0x00000e00,
                .val = 0x00000000,
            },
#endif
        },
    },

    {
        .chan = 3,
        .mfp = 1,   /* from UART3_TX_RX pads */
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202)
            {0},
#elif defined(CONFIG_ACTS_GL5207)
            {
                .reg = MFP_CTL2,
                .mask = 0x00800000,
                .val = 0x00800000,
            },
            {
                .reg = MFP_CTL3,
                .mask = 0x00000e00,
                .val = 0x00000800,
            },
#endif
        },
    },

    {
        .chan = 2,
        .mfp = 0,
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202)
            {
                .reg = MFP_CTL0,    /* from RMII_TX_EN pads */
                .mask = 0x0000e000,
                .val = 0x00002000,
            },
#elif defined(CONFIG_ACTS_GL5207)
            {
                .reg = MFP_CTL2,    /* from UART2 original pads */
                .mask = 0x04000000,
                .val = 0x00000000,
            },
#endif
        },
    },

    {
        .chan = 2,
        .mfp = 1,
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202)
            {0},                    /* from UART2 original pads */
#elif defined(CONFIG_ACTS_GL5207)
            {
                .reg = MFP_CTL2,    /* from UART2 original pads */
                .mask = 0x04000000,
                .val = 0x00000000,
            },
#endif
        },
    },
};

static const uint8_t sc_gl520x_uart_cmu_clkdiv_reg_ofs[6] = {
    0,
    CMU_UART1CLK - CMU_UART0CLK,
    CMU_UART2CLK - CMU_UART0CLK,
    CMU_UART3CLK - CMU_UART0CLK,
    CMU_UART4CLK - CMU_UART0CLK,
    CMU_UART5CLK - CMU_UART0CLK
};

static const uint8_t sc_gl520x_uart_cmu_clken_bitpos[6] = {
    6, 7, 8, 19, 20, 21
};

static const uint8_t sc_gl520x_uart_cmu_rst_bitpos[6] = {
    5, 6, 7, 15, 16, 17
};


DECLARE_GLOBAL_DATA_PTR;

static act_serial_device_t s_gl520x_serial_drv = {
	.base_dev = {
		.name	= "gl520x_serial",
		.start	= _gl520x_serial_init,
#ifndef CONFIG_SPL_BUILD
		.stop   = _gl520x_serial_exit,
#else
		.stop	= NULL,
#endif
		.setbrg	= _gl520x_serial_setbrg,
		.putc	= _gl520x_serial_putc,
		.puts	= default_serial_puts,
		.getc	= _gl520x_serial_getc,
		.tstc	= _gl520x_serial_tstc,
	}
};


static void _gl520x_serial_reset(uint32_t uart_ch)
{
    uint32_t mask;

    mask = 1U << sc_gl520x_uart_cmu_rst_bitpos[uart_ch];

    /* assert reset */
    act_writel(act_readl(CMU_DEVRST1) & ~mask, CMU_DEVRST1);
    act_readl(CMU_DEVRST1);

    /* wait a bit */
    __udelay(10);

    /* release reset */
    act_writel(act_readl(CMU_DEVRST1) | mask, CMU_DEVRST1);
    act_readl(CMU_DEVRST1);

    /* wait a bit */
    __udelay(50);
}

static void _gl520x_serial_config_hw(uint uart_ch, uint mfp_grp)
{
    struct act_uart_mfp_cfg const *p_cfg;
    uint cfg_cnt, reg_cnt, i;

    if(uart_ch >= GL520X_UART_CH_CNT)
        return;

    /* MFP */
    p_cfg = sc_gl520x_uart_mfp_cfg;
    cfg_cnt = ARRAY_SIZE(sc_gl520x_uart_mfp_cfg);
    while(cfg_cnt-- != 0) {
        if((p_cfg->chan == uart_ch) && (p_cfg->mfp == mfp_grp)) {
            reg_cnt = ARRAY_SIZE(p_cfg->mfp_cfg);
            for(i=0; i<reg_cnt; i++) {
                if(p_cfg->mfp_cfg[i].reg == 0)
                    break;
                clrsetbits_le32(p_cfg->mfp_cfg[i].reg,
                        p_cfg->mfp_cfg[i].mask,
                        p_cfg->mfp_cfg[i].val);
            }
            break;
        }
        p_cfg++;
    }

    /* clock */
    setbits_le32(CMU_DEVCLKEN1, 1U << sc_gl520x_uart_cmu_clken_bitpos[uart_ch]);
    act_readl(CMU_DEVCLKEN1);
}

static void __attribute__((unused)) _gl520x_serial_deconfig_hw(uint uart_ch, uint mfp_grp)
{
    /* clock */
    clrbits_le32(CMU_DEVCLKEN1, 1U << sc_gl520x_uart_cmu_clken_bitpos[uart_ch]);
    act_readl(CMU_DEVCLKEN1);
}

static void _gl520x_serial_setbrg(void)
{
	uint reg_addr, baudrate, div, uart_ch;

	baudrate = gd->baudrate;
	if (baudrate == 0)
		baudrate = CONFIG_BAUDRATE;

	div = (24000000U + baudrate * 8U /2U) / (baudrate * 8U);
	if(div != 0)
	    div--;
    //if(div > 311U)
    //    div = 311U;  // no used.

	uart_ch = s_gl520x_serial_drv.uart_ch;

    reg_addr = CMU_UART0CLK + sc_gl520x_uart_cmu_clkdiv_reg_ofs[uart_ch];
    act_writel(div, reg_addr);

	_gl520x_serial_reset(uart_ch);

	/* uart configure, UART0_CTL
	 * bit20 = 0, Loopback Disabled
	 * bit15=1, Uart EN
	 * bit[1:0]=11, 8bit Width
	 */
	act_writel(0x008003, UART_CTL);
}

static int _gl520x_serial_init(void)
{
    if(ACTS_AFINFO_PTR->uart_config.enable != 0) {
        s_gl520x_serial_drv.uart_ch = ACTS_AFINFO_PTR->uart_config.chan;
        s_gl520x_serial_drv.mfp_grp = ACTS_AFINFO_PTR->uart_config.group;
        gd->baudrate = ACTS_AFINFO_PTR->uart_config.baudrate;
    } else {
        s_gl520x_serial_drv.uart_ch = 5;
        s_gl520x_serial_drv.mfp_grp = 0;
        if (gd->baudrate == 0) {
            gd->baudrate = CONFIG_BAUDRATE;
        }
    }

    s_gl520x_serial_drv.reg_base =
            GL520X_UART_BASE(s_gl520x_serial_drv.uart_ch);

    _gl520x_serial_config_hw(
            s_gl520x_serial_drv.uart_ch, s_gl520x_serial_drv.mfp_grp);

	/* uart configure, UART0_CTL
	 * bit20 = 0, Loopback Disabled
	 * bit15=1, Uart EN
	 * bit[1:0]=11, 8bit Width
	 */
    _gl520x_serial_setbrg();

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int _gl520x_serial_exit(void)
{
    _gl520x_serial_reset(s_gl520x_serial_drv.uart_ch);
    _gl520x_serial_deconfig_hw(
            s_gl520x_serial_drv.uart_ch, s_gl520x_serial_drv.mfp_grp);
    return 0;
}
#endif

static void _gl520x_serial_putc(const char c)
{
    /* send a \r for compatibility */
    if (c == '\n')
        _gl520x_serial_putc('\r');

    /* write to HW_FIFO and exit, do not wait for transmit. */
    while(act_readl(UART_STAT) & (1U<<6));
    act_writel(c, UART_TXDAT);
}

static int _gl520x_serial_getc(void)
{
    while(act_readl(UART_STAT) & (1U<<5));
    return act_readl(UART_RXDAT);
}

static int _gl520x_serial_tstc(void)
{
	return ((act_readl(UART_STAT) >> 5) & 1U) ^ 1U;
}

void gl520x_serial_initialize(void)
{
	serial_register(&(s_gl520x_serial_drv.base_dev));
}

struct serial_device * __weak default_serial_console(void)
{
	return &(s_gl520x_serial_drv.base_dev);
}
