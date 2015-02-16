#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "brom_api.h"

void __attribute((noreturn)) act_enter_adfu_launcher(void)
{
    puts("enter ADFU................\n");

    icache_disable();

#if ! defined(CONFIG_SPL_BUILD)
    dcache_disable();
#endif

    act_writel(0x00, SHARESRAM_CTL); //switch shareram to AHB
    act_readl(SHARESRAM_CTL);

    act_brom_enter_adfu();
}
