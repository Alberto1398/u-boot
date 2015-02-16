

#include <common.h>
#include <asm/io.h>
#include <asm/armv7.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#define ASOC_PA_L2CC                0xB0022000
#define L2X0_CTL                    (ASOC_PA_L2CC + 0x100)
#define L2X0_AUX_CTL                (ASOC_PA_L2CC + 0x104)
#define L2X0_CACHE_SYNC             (ASOC_PA_L2CC + 0x730)
#define L2X0_INV_LINE_PA            (ASOC_PA_L2CC + 0x770)
#define L2X0_INV_WAY                (ASOC_PA_L2CC + 0x77C)
#define L2X0_CLEAN_LINE_PA          (ASOC_PA_L2CC + 0x7B0)
#define L2X0_CLEAN_WAY              (ASOC_PA_L2CC + 0x7BC)



void v7_outer_cache_enable(void)
{
    /* Check if l2x0 controller is already enabled */
    if (readl(L2X0_CTL) & 1)
        return;

    // DON'T touch aux, the default value is OK.
    //aux = readl(L2X0_AUX_CTL);
    //aux &= 0xc0000fff;
    //aux |= 0x3e050000;
    //writel(aux, L2X0_AUX_CTL);

    v7_outer_cache_inval_all();

    /* enable l2x0 */
    writel(1, L2X0_CTL);

    puts("l2c-310 enabled\n");
}

void v7_outer_cache_disable(void)
{
    if ((readl(L2X0_CTL) & 1) == 0)
        return;

    v7_outer_cache_flush_all();
    writel(0, L2X0_CTL);
    __asm__ __volatile__("dsb":::"memory");

    puts("l2c-310 disabled\n");
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
    /* Enable D-cache. I-cache is already enabled in start.S */
    dcache_enable();
}
#endif
