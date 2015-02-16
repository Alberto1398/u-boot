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
#include <spi.h>

#define SPI_CTL(BASE)       ((BASE)+0x0000U)
#define SPI_CLKDIV(BASE)    ((BASE)+0x0004U)
#define SPI_STAT(BASE)      ((BASE)+0x0008U)
#define SPI_RXDAT(BASE)     ((BASE)+0x000cU)
#define SPI_TXDAT(BASE)     ((BASE)+0x0010U)
#define SPI_TCNT(BASE)      ((BASE)+0x0014U)
#define SPI_SEED(BASE)      ((BASE)+0x0018U)
#define SPI_TXCR(BASE)      ((BASE)+0x001cU)
#define SPI_RXCR(BASE)      ((BASE)+0x0020U)


typedef struct {
    struct {
        uint32_t reg_addr;
        uint32_t mask;
        uint32_t val;
    } mfp_cfg[2];
} act_spi_bus_mfp_cfg_t;

typedef struct  {
    struct spi_slave slave;
    uint32_t io_base;
    uint32_t reg_ctrl;
    uint16_t reg_clk_div;
    uint8_t  mfp_valid;
    uint8_t  bus_actived;
    uint8_t  allocated;
} act_spi_slave_t;

static act_spi_bus_mfp_cfg_t s_act_spi_bus_mfp[] = {
    {
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202) // 7029B
            {
                .reg_addr = MFP_CTL2,
                .mask = (1U << 1),
                .val  = 0,
            },
#elif defined(CONFIG_ACTS_GL5207) // 7021
            {
                .reg_addr = MFP_CTL2,
                .mask = (7U << 0),
                .val  = 0,
            },
            {
                .reg_addr = PAD_DRV1,
                .mask = (3U << 10), // [11:10]
                .val  = (1U << 10), // 1
            },
#endif
        },
    },
    {
        .mfp_cfg = {
#if defined(CONFIG_ACTS_GL5202) // 7029B
            {
                .reg_addr = MFP_CTL3,
                .mask = (3U<<22)|(7U<<19)|(7U<<16),
                .val  = (2U<<22)|(2U<<19)|(4U<<16),
            },
#elif defined(CONFIG_ACTS_GL5207) // 7021
            {
                .reg_addr = MFP_CTL3,
                .mask = (7U<<28)|(7U<<19)|(7U<<16),
                .val  = (2U<<28)|(2U<<19)|(4U<<16),
            },
            {
                .reg_addr = PAD_DRV2,
                .mask = (3U<<30)|(3U<<28)|(3U<<23),
                .val  = (1U<<30)|(1U<<28)|(1U<<23),
            },
#endif
        },
    },
};

/* The only-one slave (ie. gl5302) */
static act_spi_slave_t s_act_spi_slaves[1] = {
    {
        {
            .bus = 0,
            .cs = 0,
        },
        .allocated = 0,
    },
};

static void _pre_calc_regctl_val(act_spi_slave_t *p, uint mode)
{
    uint32_t reg_ctrl;

    reg_ctrl = (mode & 3U) << 6;

    if(mode & SPI_LSB_FIRST)
    {
        reg_ctrl |= (1U << 5);
    }
    if(mode & SPI_LOOP)
    {
        reg_ctrl |= (1U << 11);
    }
    p->reg_ctrl = reg_ctrl;
}

static void _pre_calc_clkdiv_val(act_spi_slave_t *p, uint clk_rate_hz)
{
    uint clk_div;

    clk_div = (act_cmu_get_hclk() -1U) / (clk_rate_hz*2U) +1U;
    //if(clk_div == 0) // no need, will not be 0
    //{
    //    clk_div = 1;
    //}
    if(clk_div > 1023U)
    {
        clk_div = 1023U;
    }
    p->reg_clk_div = clk_div;
}

void spi_init(void)
{
    /* do nothing */
}

void *spi_do_alloc_slave(int offset, int size, unsigned int bus,
             unsigned int cs)
{
    /* This API is for SPI driver internal used, and we do not use it. */
    return NULL;
}

static void _spi_module_switch_on(uint bus)
{
    uint i;

    setbits_le32(CMU_DEVCLKEN1, (1U << (10U + bus))); // clock

    clrbits_le32(CMU_DEVRST1, (1U << (8U + bus))); // reset
    act_readl(CMU_DEVRST1);
    setbits_le32(CMU_DEVRST1, (1U << (8U + bus)));
    act_readl(CMU_DEVRST1);

    for(i=0; i<ARRAY_SIZE(s_act_spi_bus_mfp[bus].mfp_cfg); i++)
    {
        if(s_act_spi_bus_mfp[bus].mfp_cfg[i].reg_addr != 0)
        {
            clrsetbits_le32(
                    s_act_spi_bus_mfp[bus].mfp_cfg[i].reg_addr,
                    s_act_spi_bus_mfp[bus].mfp_cfg[i].mask,
                    s_act_spi_bus_mfp[bus].mfp_cfg[i].val);
        }
    }
}

static void _spi_module_switch_off(uint bus)
{
    clrbits_le32(CMU_DEVCLKEN1, (1U << (10U + bus)));
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
        unsigned int max_hz, unsigned int mode)
{
    act_spi_slave_t *p_new;
    uint i;

    if(cs != 0 || bus >= ARRAY_SIZE(s_act_spi_bus_mfp))
    {
        return NULL;
    }

    for(i=0; i<ARRAY_SIZE(s_act_spi_slaves); i++)
    {
        if(s_act_spi_slaves[i].allocated == 0)
        {
            p_new = &s_act_spi_slaves[i];
            memset(p_new, sizeof(act_spi_slave_t), 0);
            p_new->allocated = 1;
            p_new->slave.bus = bus;
            p_new->slave.cs  = cs;
            p_new->slave.wordlen = SPI_DEFAULT_WORDLEN;

            p_new->io_base = SPI0_BASE + bus * (SPI1_BASE - SPI0_BASE);
            _pre_calc_regctl_val(p_new, mode);
            _pre_calc_clkdiv_val(p_new, max_hz);

            return &(p_new->slave);
        }
    }
    return NULL;
}

void spi_free_slave(struct spi_slave *slave)
{
    act_spi_slave_t *p_slave = container_of(slave, act_spi_slave_t, slave);

    if(p_slave->allocated != 0 && p_slave->mfp_valid != 0)
    {
        _spi_module_switch_off(p_slave->slave.bus);
        p_slave->mfp_valid = 0;
    }
    p_slave->allocated = 0;
}

int  spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
    return (bus==0) && (cs==0);
}

void spi_cs_activate(struct spi_slave *slave)
{
    /* do nothing */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
    /* do nothing */
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
    act_spi_slave_t *p_slave = container_of(slave, act_spi_slave_t, slave);
    _pre_calc_clkdiv_val(p_slave, hz);
}

int spi_claim_bus(struct spi_slave *slave)
{
    act_spi_slave_t *p_slave;

    p_slave = container_of(slave, act_spi_slave_t, slave);
    if(p_slave->bus_actived != 0)
    {
        return -1;
    }
    p_slave->bus_actived = 1;

    if(p_slave->mfp_valid == 0)
    {
        p_slave->mfp_valid = 1;
        _spi_module_switch_on(p_slave->slave.bus);
    }

    act_writel(p_slave->reg_clk_div, SPI_CLKDIV(p_slave->io_base)); // clock

    act_writel(p_slave->reg_ctrl, SPI_CTL(p_slave->io_base));
    setbits_le32(SPI_CTL(p_slave->io_base), (1U << 18)); /* enable SPI */

    return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
    act_spi_slave_t *p_slave;

    p_slave = container_of(slave, act_spi_slave_t, slave);
    if(p_slave->bus_actived == 0)
    {
        return;
    }
    p_slave->bus_actived = 0;

    setbits_le32(SPI_CTL(p_slave->io_base), (1U << 4)); // CS high
    clrbits_le32(SPI_CTL(p_slave->io_base), (1U << 18)); // disable SPI
}

int  spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
        void *din, unsigned long flags)
{
    act_spi_slave_t *p_slave;
    uint32_t dataw_mode, count, tmp;
    uint16_t *pi2;
    uint16_t const *po2;

    p_slave = container_of(slave, act_spi_slave_t, slave);
    if(p_slave->bus_actived == 0)
    {
        return -1;
    }

    switch(p_slave->slave.wordlen)
    {
    //case 8:
    //    dataw_mode = 0; break;
    case 16:
        dataw_mode = 1; break;
    //case 32:
    //    dataw_mode = 2; break;
    default:
        return -1;
    }
    clrsetbits_le32(SPI_CTL(p_slave->io_base), (3U<<8), (dataw_mode<<8));

    // reset FIFO
    act_writel(3U << 4, SPI_STAT(p_slave->io_base));
    act_readl(SPI_STAT(p_slave->io_base));

    // CS low if need
    if(flags & SPI_XFER_BEGIN)
    {
        clrbits_le32(SPI_CTL(p_slave->io_base), (1U << 4));
    }

    switch(dataw_mode)
    {
    case 1:
        count = bitlen / 16U;
        po2 = (uint16_t const *)dout;
        pi2 = (uint16_t *)din;
        while(count-- != 0)
        {
            tmp = (po2 != NULL) ? *po2++ : 0;
            act_writel(tmp, SPI_TXDAT(p_slave->io_base));
            while((act_readl(SPI_STAT(p_slave->io_base)) & ((1U<<9)|(1U<<6))) ==
                    ((1U<<9)|(1U<<6)));
            tmp = act_readl(SPI_RXDAT(p_slave->io_base));
            if(pi2 != NULL)
            {
                *pi2++ = tmp;
            }
        }
        break;
    }

    // CS high if need
    if(flags & SPI_XFER_BEGIN)
    {
        setbits_le32(SPI_CTL(p_slave->io_base), (1U << 4));
    }

    return 0;
}

uint act_spi_atc260x_reg_rw(struct spi_slave *slave, uint dir, uint reg_addr, uint reg_data)
{
    act_spi_slave_t *p_slave;
    uint32_t tmp;

    p_slave = container_of(slave, act_spi_slave_t, slave);
    if(p_slave->bus_actived == 0)
    {
        return 0;
    }

    clrsetbits_le32(SPI_CTL(p_slave->io_base), (1U<<5)|(3U<<8), (2U<<8)); // 32bit MSB

    // reset FIFO
    act_writel(3U << 4, SPI_STAT(p_slave->io_base));
    act_readl(SPI_STAT(p_slave->io_base));

    tmp = (((dir<<15) | ((reg_addr<<3) & 0x7fffU)) << 16) | (reg_data & 0xffffU);

    // CS low if need
    clrbits_le32(SPI_CTL(p_slave->io_base), (1U << 4));

    act_writel(tmp, SPI_TXDAT(p_slave->io_base));
    while((act_readl(SPI_STAT(p_slave->io_base)) & ((1U<<9)|(1U<<6))) ==
            ((1U<<9)|(1U<<6)));
    tmp = act_readl(SPI_RXDAT(p_slave->io_base));

    // CS high if need
    setbits_le32(SPI_CTL(p_slave->io_base), (1U << 4));

    return tmp & 0xffffU;
}

