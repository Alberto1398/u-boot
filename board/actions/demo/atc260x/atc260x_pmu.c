// TESTCODE
#define DEBUG

#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_atc2603.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/afinfo.h>

#include "atc260x.h"


/* for check gl5302 is exist */
//#define atc2603_MAGICNUM_REG             (atc2603_PMU_OC_INT_EN)
//#define atc2603_MAGICNUM_REG_VALUE       (0x1bc0)
#define atc2603_MAGICNUM_REG             (atc2603_CMU_HOSCCTL)
#define atc2603_MAGICNUM_REG_VALUE       (0x2aab)

/* atc2603_PMU_SYS_CTL3 FW reserved flag */
#define PMU_SYS_CTL3_FW_FLAG_S2         (1 << 4)    /* suspend to S2 flag */
#define PMU_SYS_CTL3_FW_FLAG_Reset      (1 << 5)

/* wakeup sources */
#define WAKEUP_SRC_IR                   (1 << 0)
#define WAKEUP_SRC_RESET                (1 << 1)
#define WAKEUP_SRC_HDSW                 (1 << 2)
#define WAKEUP_SRC_ALARM                (1 << 3)
#define WAKEUP_SRC_REMCON               (1 << 4)
#define WAKEUP_SRC_TP                   (1 << 5)
#define WAKEUP_SRC_WKIRQ                (1 << 6)
#define WAKEUP_SRC_ONOFF_SHORT          (1 << 7)
#define WAKEUP_SRC_ONOFF_LONG           (1 << 8)
#define WAKEUP_SRC_WALL_IN              (1 << 9)
#define WAKEUP_SRC_VBUS_IN              (1 << 10)

/* S1 S2 S3 */
#define PMU_SYS_CTL3_EN_S3              (1 << 14)
#define PMU_SYS_CTL3_EN_S2              (1 << 15)
#define PMU_SYS_CTL1_EN_S1              (1 << 0)


extern uint act_spi_atc260x_reg_rw(struct spi_slave *slave, uint dir, uint reg_addr, uint reg_data);

static struct spi_slave *s_spi_5302;


/* Pointer to global data structure */
DECLARE_GLOBAL_DATA_PTR;


/*
    config 24M output to 5302
    debug_port:
        0---clko;
        1---ks_out1
*/
static void _switch_24m_output(int debug_port)
{
    unsigned int val;

    if (debug_port)
    {
        /* disable CLKO_24M output */
#if defined(CONFIG_ACTS_GL5207)
        val = act_readl(MFP_CTL3);
        val &= ~(0x1<<31);
        act_writel(val, MFP_CTL3);
#elif defined(CONFIG_ACTS_GL5202)
        val = act_readl(MFP_CTL3);
        val &= ~(0x1<<30);
        act_writel(val, MFP_CTL3);
#else
#error "unknown SoC config"
#endif
        /* output 24M clk from KSOUT1 debug port */
        act_writel(0x0, DEBUG_SEL);
        act_writel(0x10000000, DEBUG_OEN0);
        act_writel(0x80000038, CMU_DIGITALDEBUG);

        puts("24MHz from ks_out1\n");
    }
    else
    {
        act_writel(0x0, DEBUG_OEN0);
        act_writel(0x0, CMU_DIGITALDEBUG);

//        val = act_readl(MFP_CTL3);
//        val &= 0xbfffffff;
//        val |= 0x40000000;
//        act_writel(val, MFP_CTL3);

#if defined(CONFIG_ACTS_GL5207)
        val = act_readl(MFP_CTL3);
        val |= (0x1<<31);
        act_writel(val, MFP_CTL3);
#elif defined(CONFIG_ACTS_GL5202)
        val = act_readl(MFP_CTL3);
        val |= (0x1<<30);
        act_writel(val, MFP_CTL3);
        act_writel(act_readl(CMU_ANALOGDEBUG) | (0x1f), CMU_ANALOGDEBUG);
#endif

        puts("24MHz from clko\n");
    }
}

uint act_260x_reg_read(uint addr)
{
    uint ret;

    if(spi_claim_bus(s_spi_5302) == 0)
    {
        ret = act_spi_atc260x_reg_rw(s_spi_5302, 0, addr, 0);
        spi_release_bus(s_spi_5302);
    }
    else
    {
        printf("failed to read 260x reg0x%x\n", addr);
        hang();
    }
    return ret;
}

void act_260x_reg_write(uint addr, uint data)
{
    if(spi_claim_bus(s_spi_5302) == 0)
    {
        act_spi_atc260x_reg_rw(s_spi_5302, 1, addr, data);
        spi_release_bus(s_spi_5302);
    }
    else
    {
        printf("failed to write 260x reg0x%x\n", addr);
        hang();
    }
}

void act_260x_reg_setbits(uint addr, uint mask, uint data)
{
    uint val, old_val;
    val = old_val = act_260x_reg_read(addr);
    if(val <= 0xffffU)
    {
        val &= ~mask;
        val |= data;
        if(old_val != val)
        {
            act_260x_reg_write(addr, val);
        }
    }
}

static int _act_260x_spi_init(void)
{
    uint bond_opt, bus;

    if(ACTS_AFINFO_PTR->atc260x_cfg & 0x2U)
    {
        bus = ACTS_AFINFO_PTR->atc260x_cfg & 0x1U;
    }
    else
    {
        bond_opt = act_readl(0xb01c00e0) & 0xf;
        bus = (bond_opt==3) ? 0 : 1;
    }
    debug("260x use SPI%u\n", bus);

    s_spi_5302 = spi_setup_slave(bus, 0, 3000000, SPI_MODE_3);
    if(s_spi_5302 == NULL)
    {
        puts("init 260x spi error\n");
        return -1;
    }
    //spi_set_wordlen(s_spi_5302, 16); // no need
    return 0;
}

int act_spl_pmu_early_init(void)
{
    int ret;
    uint hosc_clk_output_pin, retry, dat, tmp;

    ret = _act_260x_spi_init();
    if(ret)
        return ret;

    /* output 24MHz clock to 5302 */
    // 5207: clko
    // 5202c: try clko firstly.If fail, use ks_out1
    hosc_clk_output_pin = 0;
#if defined(CONFIG_ACTS_GL5207)
    hosc_clk_output_pin = 0;
    retry = 1;
#elif defined(CONFIG_ACTS_GL5202)
    hosc_clk_output_pin = 1;
    retry = 0;
#endif
    while(1) // for retry
    {
        _switch_24m_output(hosc_clk_output_pin);

        // check 5302 connection
        dat = act_260x_reg_read(atc2603_MAGICNUM_REG) & 0x7fff;
        if (dat != atc2603_MAGICNUM_REG_VALUE)
        {
            if (retry == 0)
            {
                retry = 1;
                hosc_clk_output_pin ^= 1U;
                puts("probe 260x failed, retry...\n");
                continue;
            }
            puts("probe 260x err\n");
            return -1;
        }
        break;
    }
    debug("%s: probe 260x OK\n", __FUNCTION__);

    // print wakeup source
    debug("%s: 260x wakeup src, SYS_CTL0=0x%x SYS_CTL1=0x%x\n",
            __FUNCTION__,
            act_260x_reg_read(atc2603_PMU_SYS_CTL0),
            act_260x_reg_read(atc2603_PMU_SYS_CTL1));

    //disable EN_S3
    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL3);
    act_260x_reg_write(atc2603_PMU_SYS_CTL3, dat & ~PMU_SYS_CTL3_EN_S3);
    //like delay, ensure last write successful
    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL0);

    //wurui 2014.1.27：开机后置位bit5，表示正常开机，关机时清除bit5。
    //若在此处发现bit5为1，则表示上次是非正常关机，则需要再次关机一次，以保证vcc供电的外部器件能正常复位。
    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL3);
    if ((dat & PMU_SYS_CTL3_FW_FLAG_Reset) == 0)
    {
        /* set the vcc reset flag */
        act_260x_reg_write(atc2603_PMU_SYS_CTL3, dat | PMU_SYS_CTL3_FW_FLAG_Reset);
    }
    else if((dat & PMU_SYS_CTL3_FW_FLAG_S2) == 0)   //排除从s2状态启动的情况
    {
        /* set the vcc reset flag */
        act_260x_reg_write(atc2603_PMU_SYS_CTL3, (dat & (~(PMU_SYS_CTL3_FW_FLAG_Reset))) | (1 << 14));

        dat = act_260x_reg_read(atc2603_PMU_SYS_CTL1);
        act_260x_reg_write(atc2603_PMU_SYS_CTL1, dat & (~(1 << 0)));

        while(1);
    }

//    delay();

    //li    a1, 0x802f       //VDD=0.7V
    //li    a1, 0x822f       //VDD=0.8V
    //li    a1, 0x842f       //VDD=0.9V
    //li    a1, 0x862f       //VDD=1.0V
    //li    a1, 0x882f       //VDD=1.1V
    //li    a1, 0x8a2f       //VDD=1.2V
    //li    a1, 0x8c2f       //VDD=1.3V
    //li    a1, 0x8e2f       //VDD=1.4V

    //If vdd >=1.0v, don't set vdd.
    tmp = act_260x_reg_read(atc2603_PMU_DC1_CTL0);
    tmp = (tmp >> 7) & 0x1f;
    if(tmp < 0xc )
    {
        tmp = 0xc;
    }
    act_260x_reg_write(atc2603_PMU_DC1_CTL0, 0x804f | (tmp<<7));

    act_260x_reg_read(atc2603_PMU_DC1_CTL1);
//    act_260x_reg_write(atc2603_PMU_DC1_CTL1, 0x68ae);
    act_260x_reg_write(atc2603_PMU_DC1_CTL1, 0x6cae);

    act_260x_reg_read(atc2603_PMU_DC1_CTL2);
    act_260x_reg_write(atc2603_PMU_DC1_CTL2, 0x334b);

    //li    a1, 0x8440      //1.50V
    //li    a1, 0x8540      //1.55V
    //li    a1, 0x8640      //1.60V
    //li    a1, 0x8740      //1.65V
    //li    a1, 0x8840      //1.70V
    //li    a1, 0x8940      //1.75V
    //li    a1, 0x8a40      //1.80V
    act_260x_reg_read(atc2603_PMU_DC2_CTL0);
    act_260x_reg_write(atc2603_PMU_DC2_CTL0, 0x85af);

    act_260x_reg_read(atc2603_PMU_DC2_CTL1);
//    act_260x_reg_write(atc2603_PMU_DC2_CTL1, 0xe8ae);
    act_260x_reg_write(atc2603_PMU_DC2_CTL1, 0xecae);

    act_260x_reg_read(atc2603_PMU_DC2_CTL2);
    act_260x_reg_write(atc2603_PMU_DC2_CTL2, 0x334b);

    /* DCDC, PWM */
    act_260x_reg_read(atc2603_PMU_DC3_CTL0);
    act_260x_reg_write(atc2603_PMU_DC3_CTL0, 0x8acf);

    act_260x_reg_read(atc2603_PMU_DC3_CTL1);
//    act_260x_reg_write(atc2603_PMU_DC3_CTL1, 0x214c);
    act_260x_reg_write(atc2603_PMU_DC3_CTL1, 0x254c);

    act_260x_reg_read(atc2603_PMU_DC3_CTL2);
    act_260x_reg_write(atc2603_PMU_DC3_CTL2, 0x338a);

    //set ldo1 2.9v
    act_260x_reg_read(atc2603_PMU_LDO1_CTL);
    act_260x_reg_write(atc2603_PMU_LDO1_CTL, 0x6000);

    //set ic ov temp
    dat = act_260x_reg_read(atc2603_PMU_OT_CTL);
    act_260x_reg_write(atc2603_PMU_OT_CTL, dat | ((0x3<<9) | (0x3<<13)));

    /* enable phy clock */
    dat = act_260x_reg_read(atc2603_CMU_DEVRST);
    act_260x_reg_write(atc2603_CMU_DEVRST, dat | (0x1 << 9));

    /* ethenet phy power down */
    dat = act_260x_reg_read(atc2603_PHY_CONFIG);
    act_260x_reg_write(atc2603_PHY_CONFIG, (dat & ~(0x3 << 12)) | (0x2 << 12));

    /* disable phy clock */
    dat = act_260x_reg_read(atc2603_CMU_DEVRST);
    act_260x_reg_write(atc2603_CMU_DEVRST, dat & ~(0x1 << 9));

    /*enable sd power to use uart5*/
    {
        uint ver;
//  #if defined(CONFIG_GL5201_PADDEMO)
        /*get 5302 ic verD*/
        dat = act_260x_reg_read(atc2603_PMU_SWITCH_CTL);
        ver = (act_260x_reg_read(atc2603_CVER)&0x7);

        if(ver == 0x7)
        {
            /*ver D,
             enable is 0, disable is 1
             switch is 1, LDO is 0
            */
            dat &= ~0x8038;
            dat |= 0x8;
        }
        else
        {
            /*ver ABC,
             enable is 1, disable is 0
             switch is 0, LDO is 1
            */
            dat &= ~0x38;
            dat |= 0x8028;    /* enable switch 1, LDO, 3.1v */
        }
        act_260x_reg_write(atc2603_PMU_SWITCH_CTL, dat);
//  #endif
    }

    //avdd 1.2v
#if 1
    dat = act_260x_reg_read(atc2603_PMU_LDO6_CTL);
    dat &= ~0xf800;
    dat |= 0xa000;
    act_260x_reg_write(atc2603_PMU_LDO6_CTL, dat);
#endif

    /* check suspend to S2 flag */
    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL3);
    if (dat & PMU_SYS_CTL3_FW_FLAG_S2)
    {
        /* clear the S2 flag anyway */
        act_260x_reg_write(atc2603_PMU_SYS_CTL3, dat & ~(PMU_SYS_CTL3_FW_FLAG_S2));

        /* check if wakeup from S2 */
        if (dat & 0x8000)
        {
            /* wakeup from S2, return directly */
            return 1;
        }
    }

    /* set wake-up source */
    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL0);
    dat &= (~(0x7ff<<5));
    dat |= (( WAKEUP_SRC_RESET |
                    WAKEUP_SRC_ONOFF_LONG |
                    WAKEUP_SRC_WALL_IN |
                    WAKEUP_SRC_VBUS_IN)<<5);
    act_260x_reg_write(atc2603_PMU_SYS_CTL0, dat);

//    /*for gsensor*/
//    /*LDO7 output 1.8v*/
//    dat = act_260x_reg_read(atc2603_PMU_LDO7_CTL);
//    dat &= ~0xe000;
//    dat |= 0x6001;
//    act_260x_reg_write(atc2603_PMU_LDO7_CTL, dat);
//
//  tmp = act_readl(GPIO_AOUTEN);
//  tmp |= (0x1<<14);
//  act_writel(tmp, GPIO_AOUTEN);
//
//  tmp = act_readl(GPIO_ADAT);
//  tmp |= (0x1<<14);
//  act_writel(tmp, GPIO_ADAT);
//
//  for(i=0;i<10000;i++)
//  {
//  }
//
//  tmp = act_readl(GPIO_COUTEN);
//  tmp |= (0x3<<23);
//  act_writel(tmp, GPIO_COUTEN);
//
//  tmp = act_readl(GPIO_CDAT);
//  tmp |= (0x3<<23);
//  act_writel(tmp, GPIO_CDAT);

    return 0;
}

int act_pmu_later_init(void)
{
    int ret;
    uint dat;

    ret = _act_260x_spi_init();
    if(ret)
        return ret;

    // check 5302 connection
    dat = act_260x_reg_read(atc2603_MAGICNUM_REG) & 0x7fff;
    if (dat != atc2603_MAGICNUM_REG_VALUE)
    {
        puts("probe 260x err\n");
        return -1;
    }
    debug("%s: probe 260x OK\n", __FUNCTION__);

    // TESTCODE
    // for sensor
    act_260x_reg_setbits(atc2603_PMU_LDO7_CTL, 0x1, 0x1);

    return 0;
}

void act_pmu_adjust_vdd_voltage(uint vdd_voltage)
{
    uint val;

//    MBRC_INFO("vdd %d, corepll %d\n", vdd, corepll);

//    val = 0x804f;
//    val |= (((vdd - 700) / 25) & 0x1f) << 7;

    val = act_260x_reg_read(atc2603_PMU_DC1_CTL0);
    val &= ~(0x1f<<7);
    val |= (((vdd_voltage - 700) / 25) & 0x1f) << 7;
    act_260x_reg_write(atc2603_PMU_DC1_CTL0, val);
    mdelay(1);
}

uint32_t act_spl_get_S2_resum_addr(void)
{
    /*
    Get leopard_cpu_resume function phy address
          PMU_SYS_CTL8 [15:0]--- resume_func_phy[15:0]
          PMU_SYS_CTL9 bit[7:0]--- resume_func_phy[23:16]
          PMU_OC_Status bit [5:2] ---- resume_func_phy[31:28]
          PMU_SYS_CTL3 bit [9:6]  ---- resume_func_phy[27:24]
    */
    unsigned int temp = 0, addr=0;
    temp = act_260x_reg_read(atc2603_PMU_SYS_CTL8);
    addr = temp;

    temp = (act_260x_reg_read(atc2603_PMU_SYS_CTL9) & 0xff);
    addr |= (temp << 16);

    temp = ((act_260x_reg_read(atc2603_PMU_SYS_CTL3)>>6) & 0xf);
    addr |= (temp<<24);

    temp = ((act_260x_reg_read(atc2603_PMU_OC_Status)>>2) & 0xf);
    addr |= (temp<<28);

    return addr;
}

// shutoff
void act_pmu_shutoff(void)
{
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL3,  PMU_SYS_CTL3_EN_S2 | PMU_SYS_CTL3_EN_S3, 0);
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL1, PMU_SYS_CTL1_EN_S1, 0);
    while(1);
}

int act_pmu_helper_get_attached_spi_bus(void)
{
    return s_spi_5302->bus;
}

void act_pmu_set_enter_adfu_rq_flag(uint value)
{
    uint reg_val, old_reg_val;

    value = (value != 0) ? 1 : 0;

    reg_val = old_reg_val = act_260x_reg_read(atc2603_PMU_UV_Status);
    reg_val = (reg_val & ~(1U << 1)) | (value << 1);
    if(reg_val != old_reg_val)
    {
        act_260x_reg_write(atc2603_PMU_UV_Status, reg_val);
    }
}

int act_pmu_chk_enter_adfu_rq_flag(uint clear)
{
    uint reg_val = act_260x_reg_read(atc2603_PMU_UV_Status) & (1U << 1);
    if((reg_val & (1U << 1)) && clear)
    {
        act_260x_reg_write(atc2603_PMU_UV_Status, reg_val & ~(1U << 1));
    }
    return reg_val & (1U << 1);
}

int act_pmu_chk_dis_enter_charger_rq_flag(uint clear)
{
    uint reg_val = act_260x_reg_read(atc2603_PMU_OC_Status) & (1U << 1);
    if((reg_val & (1U << 1)) && clear)
    {
        act_260x_reg_write(atc2603_PMU_OC_Status, reg_val & ~(1U << 1));
    }
    return reg_val & (1U << 1);
}

int act_pmu_chk_enter_recovery_rq_flag(uint clear)
{
    uint reg_val = act_260x_reg_read(atc2603_PMU_SYS_CTL2) & (3U << 1);
    if((reg_val & (3U << 1)) && clear)
    {
        act_260x_reg_write(atc2603_PMU_SYS_CTL2, reg_val & ~(3U << 1));
    }
    return (reg_val & (3U << 1)) >> 1; // 0, 1 or 2
}

int act_pmu_chk_enter_charger_rq_flag(void)
{
    return act_260x_reg_read(atc2603_PMU_UV_INT_EN) & (1U << 0);
}

int act_pmu_chk_reboot_mode_rq_flag(void)
{
    return act_260x_reg_read(atc2603_PMU_UV_INT_EN) & (1U << 0);
}

