
#include <common.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/actions_reg_atc2603.h>

#include "brom_drv/brom_drv.h"
#include "key_scan.h"


extern uint8_t g_afinfo_space[];
extern void adfuserver_main(void);

typedef void (act_3rd_mbrc_entry_t)(gd_t const *p_spl_gd, afinfo_t const *p_afinfo);


/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));



static void __attribute__((noreturn,unused)) _route_for_S2_resume(void)
{
    uint32_t addr = act_spl_get_S2_resum_addr();

    printf("SPL S2 resume @ 0x%08x ...\n\n\n\n\n\n", addr);

    icache_disable();
    __asm__ __volatile__("isb":::"memory");

    ((void(*)(void))((void*)addr))(); // no return!

    hang();
}

static void __attribute__((noreturn,unused)) _route_for_boot(void)
{
    int ret;
    act_3rd_mbrc_entry_t *p_3rd_entry;

	/* init BROM storage driver & load the 3rd stage. */
    ret = act_load_third_mbrc();
    if(ret != 0)
    {
        puts("failed to load 3rd MBRC\n");
        goto label_err;
    }

    /* init timer again (BROM driver will used the timer HW, and reset them) */
    timer_init();

    // boost clocks.
    puts("boost clocks ...\n");
    act_cmu_middle_clk_config();
    act_cmu_latter_clk_config(gd->arch.dvfs_level, ACTS_AFINFO_PTR);
    ret = act_cmu_boot_config_check(gd->arch.dvfs_level);
    if(ret != 0)
    {
        printf("boot clk chk failed, ret=%d\n", ret);
        printf("CMU_BUSCLK:0x%x, CMU_DMACLK:0x%x, CMU_DEVPLL:0x%x, CMU_DISPLAYPLL:0x%x\n",
                act_readl(CMU_BUSCLK), act_readl(CMU_DMACLK),
                act_readl(CMU_DEVPLL), act_readl(CMU_DISPLAYPLL));
        goto label_err;
    }
    ret = act_cmu_adjust_max_cpufreq(gd->arch.dvfs_level);
    if(ret != 0)
    {
        puts("failed to boost up to max cpufreq\n");
        goto label_err;
    }

    puts("jump to 3rd stage...\n\n\n\n\n\n\n\n\n\n\n");

    p_3rd_entry = (act_3rd_mbrc_entry_t *)(void*)(CONFIG_SYS_TEXT_BASE);
    icache_disable();
    __asm__ __volatile__("isb":::"memory");
    p_3rd_entry((gd_t*)gd, ACTS_AFINFO_PTR); // no return

    puts("3rd MBRC returns!\n");

    label_err:
    //act_brom_drv_exit();
    hang();
}

static void __attribute__((noreturn,unused)) _route_for_upgrade(void)
{
    // boost clocks.
    act_cmu_middle_clk_config();
    act_cmu_latter_clk_config(gd->arch.dvfs_level, ACTS_AFINFO_PTR);

    adfuserver_main();
    hang();
}

static afinfo_t const *_locate_afinfo(void)
{
    /* AFINFO 放在距SPL开始处固定偏移的位置, boot时随着SPL一并被加载到SRAM,
     * upgrade时由ADFUlauncher和量产脚本下载到固定的位置.
     * 总之, SPL执行的时候已经准备好了, 直接拿来用即可. */
    afinfo_t *p = (afinfo_t*)g_afinfo_space;

    // check
    if(memcmp(p->magic, "AFI\x00", 4) != 0 ||
            p->blength != 2 || p->btype != 6)
    {
        hang();
    }
    gd->arch.p_afinfo = p;
	return p;
}

static void _get_valid_dvfs_level(afinfo_t const *p_afinfo)
{
    /* only call this when VDD=1.0v */
    int  ret;
    uint fixed;

    if(p_afinfo->fixed_dvfslevel != 0)
    {
        gd->arch.dvfs_level = p_afinfo->fixed_dvfslevel;
        fixed = 1;
    }
    else
    {
        ret = act_get_dvfslevel();
        if(ret < 0)
        {
            puts("get dvfslevel err\n");
            hang();
        }
        gd->arch.dvfs_level = ret;
        fixed = 0;
    }
    printf("dvfs_level=0x%x fixed=%u\n", gd->arch.dvfs_level, fixed);
}

static void _handle_enter_adfu_request(void)
{
    uint ret = act_pmu_chk_enter_adfu_rq_flag(1);
    if(ret)
    {
#if defined(CONFIG_ACTS_FOR_BOOT)
        // enter ADFU only in boot branch.
        act_enter_adfu_launcher();
#endif
    }
}

#ifdef CONFIG_ACTS_FOR_UPGRADE
/* Only upgrade has this function */
static void _handle_board_options(afinfo_t *p_afinfo)
{
    uint adc_keycode, board_opt_idx;

    if(has_board_opt(p_afinfo))
    {
        act_adc_key_boot_init();
        while(1)
        {
            adc_keycode = act_adc_key_scan();
            if(adc_keycode >= KEY_BOARD_OPTION0 && adc_keycode <= KEY_BOARD_OPTION5)
            {
                break;
            }
            printf("board_options: read adc_key err %u\n", adc_keycode);
        }

        board_opt_idx = adc_keycode - KEY_BOARD_OPTION0;
        if(board_opt_idx != 0)
        {
            memcpy(&(p_afinfo->ddr_param),
                   &(p_afinfo->board_opt.ddr_param[board_opt_idx - 1U]),
                   sizeof(ddr_param_t));
        }
        p_afinfo->board_opt.cur_opt = board_opt_idx;

        printf("select board option #%u\n", board_opt_idx);
    }
    else
    {
        puts("no board option\n");
    }
}
#endif


/*
 * In the context of SPL, board_init_f must ensure that any clocks/etc for
 * DDR are enabled, ensure that the stack pointer is valid, clear the BSS
 * and call board_init_f.  We provide this version by default but mark it
 * as __weak to allow for platforms to do this in their own way if needed.
 */
void board_init_f(ulong dummy)
{
    /* Clear the BSS. */
    memset(__bss_start, 0, __bss_end - __bss_start);

    /* Set global data pointer. */
    gd = &gdata;

    board_init_r(NULL, 0);
}

void __attribute__((noreturn)) board_init_r(gd_t *dummy1, ulong dummy2)
{
    int ret;
    uint __attribute__((unused)) exit_from_S2;
	afinfo_t const *p_afinfo;

	/* init afinfo */
	p_afinfo = _locate_afinfo();

	/* init GPIO */
	act_spl_init_gpio_cfg();

	/* init timer */
	timer_init();

	/* init serial (print) */
	act_spl_serial_init();

    /* get dvfs level */
    _get_valid_dvfs_level(p_afinfo);

	/* do misc init (the former "Startup" stub) */
	act_spl_board_misc_init(p_afinfo);

	// init interrupt
	interrupt_init();

	/* init SPI */
	// no need to do anything.

	/* init GL5302 PMU */
	ret = act_spl_pmu_early_init();
	if(ret < 0)
	{
	    puts("SPL PMU init err\n");
	    hang();
	}
	exit_from_S2 = ret;
	puts("SPL PMU init okay.\n");

	/* handle enter_ADFU request */
	_handle_enter_adfu_request();

#if defined(CONFIG_ACTS_FOR_UPGRADE)
    /* apply board-option, do this after gl5302 init*/
	_handle_board_options((afinfo_t *)p_afinfo);
#endif

	/* EMI config for PLL */
	if(p_afinfo->spread_spectrum_flag != 0)
	{
	    // need to set EMI config to PLLs for NAND LCD and DDR
	    act_ext_pll_emi_config();
	}

	/* init DDR */
	{
	    uint ddr_boot_mode, ddr_total_cap;

#if defined(CONFIG_ACTS_FOR_BOOT)
	    ddr_boot_mode = (exit_from_S2 != 0) ? 1 : 2; /* S2 or cold_boot */
#elif defined(CONFIG_ACTS_FOR_UPGRADE)
	    ddr_boot_mode = 0; /* for upgrade */
#endif
	    ret = _DDR_init(&(p_afinfo->ddr_param), ddr_boot_mode, &ddr_total_cap);
	    if(ret)
	    {
	        printf("ddr init error %d\n", ret);
	        hang();
	    }
	    gd->ram_size = ddr_total_cap * 1024U * 1024U;
	    printf("SPL DDR init okay, total %uMiB\n", ddr_total_cap);
	}

	/* branch */
#if defined(CONFIG_ACTS_FOR_BOOT)
    /* resume from S2 */
    if(exit_from_S2)
    {
        _route_for_S2_resume();
    }
    _route_for_boot();
#elif defined(CONFIG_ACTS_FOR_UPGRADE)
	_route_for_upgrade();
#else
#error "CONFIG_ACTS_FOR_BOOT CONFIG_ACTS_FOR_UPGRADE not defined"
#endif
}

