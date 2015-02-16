#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>


extern uint8_t g_afinfo_space[];

/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;


/* from board_init_f() */
int dram_init (void)
{
    /* We already have all parameter in the gd,
     * they should passed to the main stage here by the SPL.
     * We just need to print them out. */
    printf("%luMiB\n", gd->ram_size / (1024U*1024U));
    return 0;
}

/* from board_init_r() */
int board_init(void)
{
    // re-locate afinfo pointer
    gd->arch.p_afinfo = (afinfo_t*)g_afinfo_space;

    // init 260x pmu
    act_pmu_later_init();

    return 0;
}

int board_early_init_r (void)
{
    // TEMP
    return 0;
}

int board_late_init (void)
{
    int __maybe_unused ret;

    /* Here we init all storage drivers needed for boot stage */
#if defined(CONFIG_ACTS_STORAGE_NAND)
    ret = act_nandblk_init();
    if(ret != 0)
    {
        return ret;
    }
#endif

    return 0;
}

#if defined(CONFIG_ACTS_GL520X_MMC)
int board_mmc_init(bd_t *bis)
{
    return act_mmchc_init(bis);
}
#endif

void board_cleanup_before_linux(void)
{
    /* exit storage driver */
#if defined(CONFIG_ACTS_STORAGE_NAND)
    act_nandblk_exit();
#endif
#if defined(CONFIG_ACTS_GL520X_MMC)
    act_mmchc_exit();
#endif

    /* reset & disable unused modules here */
    /* USB */
#ifdef CONFIG_ACTS_FOR_UPGRADE
#if defined(CONFIG_ACTS_GL5207)
    /* 关了之后, kernel那边的adfu gadget其不来, 原因未知.  */
//    act_cmu_reset_modules(32+3); // usb2 hc0
//    act_writel(0, CMU_USBCLK);
#elif defined(CONFIG_ACTS_GL5202)
    act_cmu_reset_modules(32+14); // usb3
    act_writel(0, CMU_USBCLK);
#endif
#endif

    /* DE LCD ... */
    // these module should keep working after u-boot exit.

    /* SD */
    act_cmu_module_clk_ctrl(5, 0);
    act_cmu_module_clk_ctrl(6, 0);
    act_cmu_module_clk_ctrl(7, 0);

    /* NAND */
    act_cmu_module_clk_ctrl(4, 0);

    /* DMA */
    act_cmu_reset_modules(0);
    act_cmu_module_clk_ctrl(1, 0);
}
