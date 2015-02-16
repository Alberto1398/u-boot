
#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>
#include <asm/arch/dvfs_level.h>


extern uint8_t g_afinfo_space[];
DECLARE_GLOBAL_DATA_PTR;

static uint32_t s_saved_dvfs_level;
static uint32_t s_saved_sdram_size;


void save_boot_params(gd_t const *p_spl_gd, afinfo_t const *p_afinfo)
{
    /* use memcpy is OK here, we use the SPL's stack. */

    /* save dg data */
    s_saved_dvfs_level = p_spl_gd->arch.dvfs_level;
    s_saved_sdram_size = p_spl_gd->ram_size;

    /* save afinfo */
    memcpy(g_afinfo_space, p_afinfo, sizeof(afinfo_t));

#if 0
    /* stop & wait debugger. */
    __asm__ __volatile__("b .      \n\t"
                         "nop      \n\t"
            :::"memory");
#endif
}

/* we use this hook mainly for restoring gd. */
int arch_cpu_init(void)
{
    /* restore gd */
    gd->arch.dvfs_level = s_saved_dvfs_level;
    gd->ram_size = s_saved_sdram_size;

    /* restore afinfo */
    gd->arch.p_afinfo = g_afinfo_space;

    return 0;
}

int print_cpuinfo(void)
{
    uint dvfs_lv = gd->arch.dvfs_level;

    printf("soc: %x ver 0x%x\n",
            ASOC_GET_IC(dvfs_lv), ASOC_GET_VERSION(dvfs_lv));

    printf("clks: corepll=%u ddrpll=%u devclk=%u\n",
            act_cmu_get_corepll_clk(), act_cmu_get_ddrpll_clk(),
            act_cmu_get_dev_clk());
    printf("clks: cclk=%u hclk=%u pclk=%u\n",
            act_cmu_get_cclk(), act_cmu_get_hclk(), act_cmu_get_pclk());

    return 0;
}

