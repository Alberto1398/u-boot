
#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>

DECLARE_GLOBAL_DATA_PTR;


static void _act_spl_early_ic_bugfix(void)
{
#if defined(CONFIG_ACTS_GL5202)
    /* 不能使用CLKO_24M输出, PAD驱动能力太强影响到AGND, 进而影响到CoreCLK. */
    act_writel(act_readl(MFP_CTL3) & ~(1U<<30), MFP_CTL3);
#elif defined(CONFIG_ACTS_GL5207)
	/* do nothing. */
#endif
}

/* early SOC init in SPL stage
 * 这里还没有拿到afinfo, 也没有gd, 所以能做的事情很少, 主要是一些IC的BUGFIX
 * 事实上, relocate之前因为没有开d-cache, CPU很慢, 不应该在这里做太多事情. */
void s_init(void)
{
	/* early IC BGUFIX. */
    _act_spl_early_ic_bugfix();

    /* exception stacks */
    act_init_exception_stacks();

	/* config clocks
	 * 将CPU和总线的CLK提升到一个合理的值.
	 * 注意: 此时PMU还没有配置, 供电可能有限, 不能升的太高. */
	act_cmu_early_clk_config();
}

void save_boot_params(void)
{
    /* We use this hook to do very early init */
    /* not stack space can be use here. NO -O0 in GCC command line! */

    /* disable IRQ FIQ */
    __asm__ __volatile__("cpsid if":::"memory");

    /* enabled the JTAG (via SD card slot) */
#if 1
    act_writel(act_readl(CMU_DEVCLKEN0) | (1U<<18), CMU_DEVCLKEN0);
    act_writel(act_readl(PAD_CTL) | (1U<<1), PAD_CTL);
    act_writel(act_readl(PAD_CTL) | (3U<<2), PAD_CTL);
    act_readl(PAD_CTL);
#if defined(CONFIG_ACTS_GL5202)
    //将jtag默认从key的出口先关闭
    act_writel(act_readl(MFP_CTL1) & ~0xff800000U, MFP_CTL1);
    //将jtag从SD卡的出口放出来测试
    act_writel((act_readl(MFP_CTL2) & ~0x000ff9e0U) | 0x000b59c0U, MFP_CTL2);
#elif defined(CONFIG_ACTS_GL5207)
    act_writel(act_readl(GPIO_COUTEN) & ~0x000c3400, GPIO_COUTEN);
    act_writel(act_readl(GPIO_CINEN) & ~0x000c3400, GPIO_CINEN);
    act_writel(act_readl(MFP_CTL1) & ~0xff800000U, MFP_CTL1);
    act_writel((act_readl(MFP_CTL2) & ~0x000ff9e0U) | 0x000b59c0U, MFP_CTL2);
#endif

#if 0
    /* stop & wait debugger. */
    __asm__ __volatile__("b .      \n\t"
                         "nop      \n\t"
            :::"memory");
#endif
#endif
}
