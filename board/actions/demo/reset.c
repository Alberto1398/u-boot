#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_atc2603.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/afinfo.h>

#include "brom_drv/brom_drv.h"

DECLARE_GLOBAL_DATA_PTR;


void __attribute__((noreturn)) hang(void)
{
    uint32_t ret_addr;

    __asm__ __volatile__("mov %[raddr], lr" : [raddr] "=r" (ret_addr) :: "memory");

#if defined(CONFIG_SPL_BUILD)
    printf("SPL HANG @ 0x%08x\n", ret_addr);
#else
    printf("HANG @ 0x%08x\n", ret_addr);
#endif

#if defined(CONFIG_ACTS_FOR_BOOT) && defined(CONFIG_SPL_BUILD)
    act_enter_adfu_launcher();
#else
    /* We can not enter ADFU in upgrade branch or in main stage of the boot branch,
     * so, just stop here. */
    __asm__ __volatile__("cpsid if":::"memory"); // disable IRQ & FIQ
    while(1);
#endif
}

void __attribute__((noreturn)) reset_cpu(ulong addr)
{
    uint rtc_second;
    uint rtc_ms=0, rtc_h=0, rtc_ymd=0;

    __asm__ __volatile__("cpsid if":::"memory"); // disable IRQ & FIQ

    rtc_ms = act_260x_reg_read(atc2603_RTC_MS);
    rtc_second = rtc_ms & 0x3fU;
    if(rtc_second >= 57)
    {
        uint tmp = -1;
        // race condition, just wait
        do
        {
            rtc_second = rtc_ms & 0x3fU;
            if(rtc_second != tmp)
            {
                tmp = rtc_second;
                printf("%s: wait RTC ... %u\n", __FUNCTION__, rtc_second);
            }
            mdelay(10);
            rtc_ms = act_260x_reg_read(atc2603_RTC_MS);
        } while(rtc_second != 0);
    }
    else
    {
        // wait edge
        uint tmp = rtc_ms;
        do
        {
            mdelay(10);
            rtc_ms = act_260x_reg_read(atc2603_RTC_MS);
        } while(rtc_ms == tmp);
    }
    rtc_h  = act_260x_reg_read(atc2603_RTC_H);
    rtc_ymd = act_260x_reg_read(atc2603_RTC_YMD);

    rtc_ms += 2; // set alarm to 2 second later.

    act_260x_reg_write(atc2603_RTC_MSALM, rtc_ms);  // second set first.
    act_260x_reg_write(atc2603_RTC_HALM, rtc_h);
    act_260x_reg_write(atc2603_RTC_YMDALM, rtc_ymd);

    // set alarm as wake-up source
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL0,
            (1U<<15)|(1U<<14)|(1U<<8), (1U<<8));

    puts("enter S3................\n");

    // enter S3
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL3, (1U<<14)|(1U<<15), 1U<<14);
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL1, 1U<<0, 0);

    while(1);
}
