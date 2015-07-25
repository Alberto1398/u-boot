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

#include <common.h>
#include <spl.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <spi.h>
#include <asm/arch/pmu.h>
#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>
#include <asm/arch/osboot.h>
#include <power/owlxx_power.h>

DECLARE_GLOBAL_DATA_PTR;

static const u16 sc_pmu_regtbl_rtc_ms[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_MS,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_MS,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_MS,
};
static const u16 sc_pmu_regtbl_rtc_h[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_H,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_H,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_H,
};
static const u16 sc_pmu_regtbl_rtc_ymd[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_YMD,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_YMD,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_YMD,
};
static const u16 sc_pmu_regtbl_rtc_msalm[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_MSALM,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_MSALM,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_MSALM,
};
static const u16 sc_pmu_regtbl_rtc_halm[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_HALM,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_HALM,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_HALM,
};
static const u16 sc_pmu_regtbl_rtc_ymdalm[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_RTC_YMDALM,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_RTC_YMDALM,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_RTC_YMDALM,
};
static const u16 sc_pmu_regtbl_sysctl0[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_SYS_CTL0,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_SYS_CTL0,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_SYS_CTL0,
};
static const u16 sc_pmu_regtbl_sysctl1[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_SYS_CTL1,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_SYS_CTL1,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_SYS_CTL1,
};
static const u16 sc_pmu_regtbl_sysctl3[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_SYS_CTL3,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_SYS_CTL3,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_SYS_CTL3,
};
static const u16 sc_pmu_regtbl_uv_status[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_UV_STATUS,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_UV_STATUS,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_UV_STATUS,
};
static const u16 sc_pmu_regtbl_uv_int_en[OWLXX_PMU_ID_CNT] = {
	[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_UV_INT_EN,
	[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_UV_INT_EN,
	[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_UV_INT_EN,
};


afinfo_t *afinfo = (afinfo_t *)(CONFIG_SPL_LOAD_BASE + 0x50);

unsigned int owlxx_adc_val = 0x40;

int leap(int year)
{
	if(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
		return 1;
	else
		return 0;
}

int month_day(int year, int month)
{
	int day_tab[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if(leap(year) && (month == 2))
		return 28;

	return day_tab[month - 1];
}

int adjust_time(unsigned short *rtc_ms_value,
	unsigned short *rtc_h_value,
	unsigned short *rtc_ymd_value,
	unsigned short timevalue)
{
	unsigned short min = 0, sec = 0, hour = 0 , day = 0, mon = 0, year = 0;
	unsigned short rtc_ms = 0, rtc_h = 0, rtc_ymd = 0;

	if(!rtc_ms_value || !rtc_h_value || !rtc_ymd_value)
		return -1;

	rtc_ms = *rtc_ms_value;
	rtc_h = *rtc_h_value;
	rtc_ymd = *rtc_ymd_value;

	hour = (rtc_h & 0x1f);
	min = ((rtc_ms & (0x3f << 6)) >> 6);
	sec = (rtc_ms & 0x3f);
	day = (rtc_ymd & 0x1f);
	mon = ((rtc_ymd & (0xf << 5)) >> 5);
	year = ((rtc_ymd & (0x7f << 9)) >> 9);

	sec = sec + timevalue;
	if(sec >= 60) {
		sec = sec - 60; 
		min = min + 1;
		if(min >= 60) {
			min = min - 60;
			hour = hour + 1;
			if(hour >= 24) {
				hour = hour - 24;
				day = day + 1;
				if(day >= month_day(year, mon)) {
					day = day - month_day(year, mon);
					mon= mon + 1;
					if(mon >= 12) {
						mon = mon - 12;
						year = year + 1;
					}
				}
			}
		}
	}

	*rtc_ymd_value = (((year & 0x7f) << 9) |
		((mon & 0xf) << 5) | (day & 0x1f));
	*rtc_h_value = (hour & 0x1f);
	*rtc_ms_value = (((min & 0x3f) << 6) | (sec)); 
	return 0;
}

void reset_cpu(ulong addr)
{
	u16 reg_rtc_ms, reg_rtc_h, reg_rtc_ymd;
	u16 reg_rtc_msalm, reg_rtc_halm, reg_rtc_ymdalm;
	u16 reg_pmu_sysctl0, reg_pmu_sysctl1, reg_pmu_sysctl3;
	int pmu_type, ret =0;
	unsigned short rtc_ms=0, rtc_h=0, rtc_ymd=0;
/*
	unsigned short rtc_dc = 0;
*/
	int timevalue = 3;

	printf("%s\n", __func__);

	/* get register address */
	pmu_type = OWLXX_PMU_ID;
	reg_rtc_ms      = sc_pmu_regtbl_rtc_ms[pmu_type];
	reg_rtc_h       = sc_pmu_regtbl_rtc_h[pmu_type];
	reg_rtc_ymd     = sc_pmu_regtbl_rtc_ymd[pmu_type];
	reg_rtc_msalm   = sc_pmu_regtbl_rtc_msalm[pmu_type];
	reg_rtc_halm    = sc_pmu_regtbl_rtc_halm[pmu_type];
	reg_rtc_ymdalm  = sc_pmu_regtbl_rtc_ymdalm[pmu_type];

	/*读取当前时间信息,设置alarm寄存器*/
	rtc_ms = atc260x_reg_read(reg_rtc_ms);
	rtc_h  = atc260x_reg_read(reg_rtc_h);
/*	rtc_dc = atc260x_reg_read(gl5302_RTC_DC); */
	rtc_ymd = atc260x_reg_read(reg_rtc_ymd);

	ret = adjust_time(&rtc_ms, &rtc_h, &rtc_ymd, timevalue);

	if(ret != 0) {
		printf("\n########### ERROR at %s %d#########", __FUNCTION__, __LINE__);
		return ;
	}

	atc260x_reg_write(reg_rtc_ymdalm, rtc_ymd);
	atc260x_reg_write(reg_rtc_halm, rtc_h);
	atc260x_reg_write(reg_rtc_msalm, rtc_ms);

	/**********************/
	/* SRC_ALARM, S1, S2, S3, 这些位 2603a/2603c/2609a 三个IC定义是一致的. */
	reg_pmu_sysctl0 = sc_pmu_regtbl_sysctl0[pmu_type];
	reg_pmu_sysctl1 = sc_pmu_regtbl_sysctl1[pmu_type];
	reg_pmu_sysctl3 = sc_pmu_regtbl_sysctl3[pmu_type];
	atc260x_set_bits(reg_pmu_sysctl0,
		(WAKEUP_SRC_ALARM << 5), (WAKEUP_SRC_ALARM << 5));
	atc260x_set_bits(reg_pmu_sysctl3,
		PMU_SYS_CTL3_EN_S2|PMU_SYS_CTL3_EN_S3, PMU_SYS_CTL3_EN_S3);
	atc260x_set_bits(reg_pmu_sysctl1,
		PMU_SYS_CTL1_EN_S1, 0);

	return ;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	dcache_enable();
}
#endif

#if defined(CONFIG_SPL_MTEST) && defined(CONFIG_SPL_BUILD)
void mem_veri(void)
{
	/* check afinfo value sanity, first 4bit, last 16bit must 0 */
	if ((afinfo->mtest_start | afinfo->mtest_end) & 0xF000FFFF)
		spl_mtest(CONFIG_SYS_MEMTEST_START,
			CONFIG_SYS_MEMTEST_END, 0, 0);
	else
		spl_mtest(afinfo->mtest_start,
			afinfo->mtest_end,
			afinfo->mtest_pattern,
			afinfo->mtest_limit);

}
#else
#define mem_veri() do {} while(0)
#endif

#ifdef CONFIG_SPL_BUILD
//int mem_test(ddr_param_t *ddr_info)
//{
//	u32 i, addr, val, top_addr;
//	u32 patten[] = { 0x12345678, 0xA5FFA5FF, 0x5678FC03, 0xD1C2B3A4};
//	top_addr = get_memory_size();
//
//	/* write address, 1M */
//	for (i = 0; i < sizeof(patten)/sizeof(u32); i++) {
//		addr = top_addr - ((i + 1) << 16);
//		writel(patten[i], addr);
//		val = readl(addr);
//		if (val != patten[i]) {
//			printf("ERR: ddr init fail, add(0x%x), "
//					"written(0x%x), read(0x%x)\n",
//					addr, patten[i], val);
//			return -1;
//		}
//	}
//	return 0;
//}

static void inline fdt_invalid_blob(void)
{
	writel(0, CONFIG_KERNEL_FDTADDR);
}

void config_jtag(void)
{
//    if(afinfo->jtag_mode == 2)
//    {
//        volatile int jtag_while = 1;
//        unsigned int tmp = 0;
//
//        if((afinfo->corepll & 0xFF) > 0x14)
//            afinfo->corepll = (afinfo->corepll & (~0xff)) | 0x14;   //jtag使用时corepll不能大于240M
//            
//        /**/
//        tmp = readl(GPIO_COUTEN);
//        tmp &= (~0x000c3400);
//        writel(tmp, GPIO_COUTEN);
//        
//        tmp = readl(GPIO_CINEN);
//        tmp &= (~0x000c3400);
//        writel(tmp, GPIO_CINEN);    
//    
//        tmp = readl(MFP_CTL1);
//        tmp &= (~((0x7<<29) | (0x7<<26)));
//        writel(tmp, MFP_CTL1);
//    
//        tmp = readl(MFP_CTL2);
//        tmp &= (~((0x3<<5) | (0x3<<7) | (0x7<<11) | (0x7<<17)));
//        tmp |= (0x2<<5) | (0x3<<7) | (0x3<<11) | (0x3<<17);
//        
//        writel(tmp, MFP_CTL2);
//    }
    if(afinfo->jtag_mode != 0)
    {
        volatile int jtag_wait = afinfo->jtag_wait;
        
        if((afinfo->corepll & 0xFF) > 0x14)
            afinfo->corepll = (afinfo->corepll & (~0xff)) | 0x14;   //jtag使用时corepll不能大于240M

        if(afinfo->jtag_mode == 2) {
            writel(readl(MFP_CTL1) & (~((0x7<<29) | (0x7<<26))), MFP_CTL1);
            writel((readl(MFP_CTL2) & (~((0x3<<5) | (0x3<<7) | (0x7<<11) | (0x7<<17))))
                | ((0x2<<5) | (0x3<<7) | (0x3<<11) | (0x3<<17)), MFP_CTL2);
        }
        while(jtag_wait);
    }
}

int checksum_ddr(void);
void restore_code_for_ddr_train(void);
static void resume_ddr_check(void)
{
    restore_code_for_ddr_train();
    if(afinfo->s2ddr_check)
    {
        if(checksum_ddr() == 0)
            printf("ddr checksum ok\n");
        else
        {
            printf("ddr checksum wrong!!!\n");
            afinfo->jtag_mode = 2;
            afinfo->jtag_wait = 1;
            config_jtag();
        }
    }
}

int mem_test_low(void)
{
	u32 i, val;
	u32 patten[] = { 0x12345678, 0xA5FFA5FF, 0x5678FC03, 0xD1C2B3A4};

	/* write address, 1M */
	for (i = 0; i < sizeof(patten)/sizeof(u32); i++) {
		writel(patten[i], i * 2048);
		val = readl(i * 2048);
		if (val != patten[i]) {
			printf("ERR: ddr init fail, add(0x%x), "
					"written(0x%x), read(0x%x)\n",
					i * 2048, patten[i], val);
			return -1;
		}
	}
	return 0;
}

#ifndef CONFIG_SPL_OWLXX_UPGRADE		
void enter_s2(int skip_ddr)
{
	u16 reg_pmu_sysctl1, reg_pmu_sysctl3;
	u32 val, i;
	uint pmu_type;

	val = readl(CMU_BUSCLK);
	val &= 0xfffffffc;
	val |= 0x1;
	writel(val, CMU_BUSCLK);
	udelay(5);

	if (skip_ddr == 0) {
		/* put ddr into low power */
		writel(3, 0xb01c0004);
		i = 0;
		while(readl(0xb01c0008) != 0x45) {
			udelay(5);
			i++;
			if ((i % 100) == 0)
				printf("ch0->LP %d times status %d\n",
					i, readl(0xb01c0008));
			if (i > 1000)
				break;
		}
		
#if defined(CONFIG_ATM7039C)
		if ((readl(0xb01b00e0) & 0xf) != 0xf) {
			/* cpu is 7039 */
			writel(3, 0xb01d0004);
			i = 0;
			while(readl(0xb01d0008) != 0x45) {
				udelay(5);
				i++;
				if ((i % 100) == 0)
					printf("ch1->LP %d times status %d\n",
						i, readl(0xb01d0008));
				if (i > 1000)
					break;
			}
		}
#endif

		writel(0x33fc3c1a, 0xb01c8024);
		writel(0x0000080c, 0xb01c8028);
		writel(0xc4fc001f, 0xb01c802c);
		udelay(10);

		val = readl(0xb0290000);
		val |= (1 << 1);
		writel(val, 0xb0290000);
		udelay(1);
		val &= 0xefffffff;
		writel(val, 0xb0290000);
		val |= (1 << 28);
		writel(val, 0xb0290000);
		mdelay(5);

	}

	pmu_prepare_for_s2(); /* PMU prapare for S2 */

	pmu_type = OWLXX_PMU_ID;
	reg_pmu_sysctl1 = sc_pmu_regtbl_sysctl1[pmu_type];
	reg_pmu_sysctl3 = sc_pmu_regtbl_sysctl3[pmu_type];
	/* 5302 5307 5303 三者进S2的方式一致, 仅仅是寄存器地址有差别. */	

	while (1) {
		/* enter s2 */
		val = atc260x_reg_read(reg_pmu_sysctl3);
		val = val & 0xffff;
		val |= 0x8000;
		for (i = 0; i < 10; i++) {
			atc260x_reg_write(reg_pmu_sysctl3, val);
			if (atc260x_reg_read(reg_pmu_sysctl3) & 0x8000)
				break;
			mdelay(1);
		}

		val = atc260x_reg_read(reg_pmu_sysctl1);
		val = val & 0xffff;
		val &= 0xfffe;
		for(i = 0; i < 1000; i++)
			atc260x_reg_write(reg_pmu_sysctl1, val);
		printf("try again write atc260x reg\n");
	}

}
#endif

void memory_sys_init(void)
{
	int i, mode;
	unsigned short rtc_ymd, rtc_h, rtc_ms;
	u32 val;
	ddr_param_t *ddr_info = scan_ddr_prepare(0);
	
#if defined(CONFIG_UPGRADE_SPL)
    mode = 0;
#else
    mode = 1;
#endif
	if ( afinfo->burn_flag & 0x01 ) { // cardburn ddr auto scand
		printf("card burn,ddr scan\n");
		mode = 0;
	}

#define MEMORY_TRY_INIT_CNT 10
	for (i = 0; i < MEMORY_TRY_INIT_CNT; i++) {
		if (mem_init(ddr_info, mode) == 0 && !mem_test_low()) {
			/* clear fdt data section */
		    fdt_invalid_blob();
			printf("OK: ddr init success (%d)\n", i);
			mem_veri();
			return;
		}
	}
#ifndef CONFIG_SPL_OWLXX_UPGRADE		
	if (s2_resume) {
		uint pmu_type = OWLXX_PMU_ID;
		u16 reg_rtc_ms      = sc_pmu_regtbl_rtc_ms[pmu_type];
		u16 reg_rtc_h       = sc_pmu_regtbl_rtc_h[pmu_type];
		u16 reg_rtc_ymd     = sc_pmu_regtbl_rtc_ymd[pmu_type];
		u16 reg_rtc_msalm   = sc_pmu_regtbl_rtc_msalm[pmu_type];
		u16 reg_rtc_halm    = sc_pmu_regtbl_rtc_halm[pmu_type];
		u16 reg_rtc_ymdalm  = sc_pmu_regtbl_rtc_ymdalm[pmu_type];
		u16 reg_pmu_sysctl0 = sc_pmu_regtbl_sysctl0[pmu_type];
		u16 reg_pmu_sysctl3 = sc_pmu_regtbl_sysctl3[pmu_type];

		rtc_ymd = atc260x_reg_read(reg_rtc_ymd);
		rtc_h = atc260x_reg_read(reg_rtc_h);
		rtc_ms = atc260x_reg_read(reg_rtc_ms);
		adjust_time(&rtc_ms, &rtc_h, &rtc_ymd, 3);
		atc260x_reg_write(reg_rtc_msalm, rtc_ms);
		atc260x_reg_write(reg_rtc_halm, rtc_h);
		atc260x_reg_write(reg_rtc_ymdalm, rtc_ymd);

		val = atc260x_reg_read(reg_pmu_sysctl0); /* 去掉WALL/VBUS唤醒. */
		val &= 0x3fff;
		atc260x_reg_write(reg_pmu_sysctl0, val);
		val = atc260x_reg_read(reg_pmu_sysctl3);  /* 置软件S2标志 */
		val |= PMU_SYS_CTL3_FW_FLAG_S2;
		atc260x_reg_write(reg_pmu_sysctl3, val);

		/* 清掉 首次resume 标志. */
		switch(pmu_type) {
		case OWLXX_PMU_ID_ATC2603A:
			atc260x_set_bits(ATC2603A_PMU_OC_STATUS, (1U<<5), 0);
			break;
		case OWLXX_PMU_ID_ATC2603C:
			atc260x_set_bits(ATC2603C_PMU_FW_USE0, (1U<<15), 0);
			break;
		case OWLXX_PMU_ID_ATC2603B:
			atc260x_set_bits(ATC2609A_PMU_SYS_CTL7, (1U<<15), 0);
			break;
		}

		printf("re-wakeup at %02d:%02d:%02d\n",
			rtc_h,
			(rtc_ms & (0x3f << 6)) >> 6,
			(rtc_ms & 0x3f));
		enter_s2(1);
	}
	printf("ERR: (%d) ddr init fail, try again\n", i);
	while(1);
#endif	
}
#endif
typedef void __noreturn (*enter_adfu_func_t)(void);

int owlxx_enter_adfu(void)
{
	u32 addr = 0xFFFF0000;

	owlxx_dss_remove();
	timer_init();
#if defined(CONFIG_ATM7059TC)
	enter_adfu_func_t adfu =
		(enter_adfu_func_t) 0xb4064000;	
#elif defined(CONFIG_ATM7059A)
	memcpy(0xb4060000, 0xFFFF5200, 0x2e00);
	enter_adfu_func_t adfu =
		(enter_adfu_func_t) 0xb4060000;	
#else
	enter_adfu_func_t adfu =
		(enter_adfu_func_t) SRAM_ADFULAUNCHER_JUMP_ADDR;
#endif

	mdelay(5);
	writel(0x30A, CMU_COREPLL);
	writel(0x100 + (120 / 6), CMU_DEVPLL);
	mdelay(5);
	setbits_le32(CMU_DEVPLL, 0x1 << 12);
#define NICDIVDIV_MASK	(0x3 << 18)
#define NICDIV_MASK	(0x3 << 16)
	clrbits_le32(CMU_BUSCLK, NICDIVDIV_MASK | NICDIV_MASK);
	udelay(5);
	asm ("mcr p15, 0, %0, c12, c0, 0": :"r" (addr));
	cleanup_before_linux();
	asm volatile("mov sp, %0\n" : : "r"(CONFIG_SYS_INIT_SP_ADDR));

	adfu();
}

#define JTAG_BITS	\
	((0x1 << 3) | (0x1 << 4) | (0x1 << 5) | (0x1 << 6) | (0x1 << 8))

void owlxx_enable_jtag(void)
{
#define KS_IN_MASK (0x7 << 29)
#define KS_IN_JLIK (0x1 << 29)

#define KS_IN3_OUT_MASK (0x7 << 26)
#define KS_IN3_OUT_JLIK (0x1 << 26)
	clrbits_le32(GPIO_BOUTEN, JTAG_BITS);
	clrbits_le32(GPIO_BDAT, JTAG_BITS);
	clrsetbits_le32(MFP_CTL1, KS_IN_MASK | KS_IN3_OUT_MASK,
		KS_IN_JLIK | KS_IN3_OUT_JLIK);
}

struct save_alarm {
	unsigned short msalm;
	unsigned short halm;
	unsigned short ymdalm;
};

/**********************/
struct kernel_reserve_info *kinfo;
unsigned int get_owlxx_reserved_size(void)
{
    int node;
    unsigned int size;

    if(get_memory_size() > 512 * 1024 * 1024) {
        node = fdt_path_offset(gd->fdt_blob, "/reserved/normal");
    } else {
        node = fdt_path_offset(gd->fdt_blob, "/reserved/tiny");
    }
    
    size = fdtdec_get_int(gd->fdt_blob, node, "fb_heap_size", 0);
    size += fdtdec_get_int(gd->fdt_blob, node, "kinfo_heap_size", 0);

    return size * 1024 * 1024;
}

void kinfo_init(void)
{
    uint64_t addr, size, rsv_size;
    
    rsv_size = get_owlxx_reserved_size();
    kinfo = (struct kernel_reserve_info *)(get_memory_size() - rsv_size);
    printf("set kinfo addr = %p\n", kinfo);
}
/**********************/

#ifndef CONFIG_SPL_OWLXX_UPGRADE
void check_adfu(void)
{
	unsigned int checksum, org_checksum = 0x55aa55aa;
	unsigned int rdat, wdat, sumdat = 0;
	unsigned int save_cin, save_cout;
	int i;
	u16 reg_pmu_uv_status, reg_val;

	reg_pmu_uv_status = sc_pmu_regtbl_uv_status[OWLXX_PMU_ID];
	reg_val = atc260x_reg_read(reg_pmu_uv_status);
	if (reg_val & (0x1 << 1)) {
		reg_val &= ~((0x1 << 1) | (1U << 0));
		atc260x_reg_write(reg_pmu_uv_status, reg_val);
		goto end;
	}

	if (afinfo->boot_dev != OWLXX_BOOTDEV_SD0 ) {
		writel(readl(PAD_PULLCTL1) & ~(1 << 17), PAD_PULLCTL1);

		save_cin= readl(GPIO_CINEN);
		save_cout= readl(GPIO_COUTEN);

		/* RX - INPUT */
		writel(readl(GPIO_COUTEN) & ~(1 << 10), GPIO_COUTEN);
		writel(readl(GPIO_CINEN) | (1 << 10), GPIO_CINEN);

		/* TX - OUTPUT */
		writel(readl(GPIO_CINEN) & ~(1 << 11), GPIO_CINEN);
		writel(readl(GPIO_COUTEN) | (1 << 11), GPIO_COUTEN);

		checksum = org_checksum;
		for (i = 0; i < 32; i++) {
			wdat = checksum & 0x1;

			/* write data */
			writel(readl(GPIO_CDAT) & ~(1 << 11), GPIO_CDAT);
			writel(readl(GPIO_CDAT) | (wdat << 11), GPIO_CDAT);
			mdelay(5);

			/* read data */
			rdat = readl(GPIO_CDAT) & (1 << 10);
			sumdat |= (!!rdat << i);

			checksum >>= 1;
		}

		writel(save_cin, GPIO_CINEN);
		writel(save_cout, GPIO_COUTEN);

		if (org_checksum == sumdat) {
			goto end;
		}
	}

	return;
end:
	printf("Prepare to entry ADFU mode\n");
	owlxx_enter_adfu();
}

#ifdef CONFIG_SPL_BUILD
void s_charger(void)
{
	
#ifdef CONFIG_SPL_POWER_SUPPORT
	int reg, ret;
	printf("inter s_charger\n");
	low_power_dect();

	reg = atc260x_reg_read(sc_pmu_regtbl_uv_int_en[OWLXX_PMU_ID]);
	printf("gl5302_PMU_UV_INT_EN: 0x%x\n", reg);

	if (reg & 0x2) {
	//	printf("close vddr!\n");
	//	atc260x_set_bits(gl5302_PMU_DC2_CTL0,0x8000,0);
	//	ret = atc260x_reg_read(gl5302_PMU_DC2_CTL0);
	//	printf("PMU_DC2_CTL0: 0x%x\n",ret);
		printf("chargering.......\n");
		ret = low_power_pcchg();
		if (ret < 0)
			return;
		ret = charger_monitor();
	}
	return;
#endif
}
#else
void s_charger(void)
{
}
#endif

#else
#define check_adfu() do {} while(0)
#define  s_charger() do {} while(0)
#endif

#ifdef CONFIG_SPL_BUILD

#define DEVCLKSS_MASK		(0x1 << 12)
#define DEVCLKSS_HOSC		(0x0 << 12)
#define DEVCLKSS_DEVPLL		(0x1 << 12)
#define DEVPLLEN		(0x1 << 8)
#define DEVPLLCLK_MASK		(0x7F << 0)
void cpu_resume_first(void)
{
#ifndef CONFIG_SPL_OWLXX_UPGRADE	
	if (s2_resume && cpu_resume_fn) {
		if (alarm_wakeup && ((u32)cpu_resume_fn & 0x80000000)) {
			clrsetbits_le32(CMU_DEVPLL, DEVCLKSS_MASK,
				DEVCLKSS_HOSC);

			udelay(5);

			clrsetbits_le32(CMU_DEVPLL, DEVPLLCLK_MASK,
				0x1132 | DEVPLLEN);
			udelay(70);
			clrsetbits_le32(CMU_DEVPLL, DEVCLKSS_MASK,
				DEVCLKSS_DEVPLL);

			udelay(100);
		}
	}
#endif	
}

void cpu_resume_second(void)
{
#ifndef CONFIG_SPL_OWLXX_UPGRADE	
	struct save_alarm *p_save_alarm;

	if (s2_resume && cpu_resume_fn) {
	    resume_ddr_check();
		if ((u32)cpu_resume_fn & 0x80000000) {
			uint pmu_type = OWLXX_PMU_ID;
			u16 reg_rtc_msalm   = sc_pmu_regtbl_rtc_msalm[pmu_type];
			u16 reg_rtc_halm    = sc_pmu_regtbl_rtc_halm[pmu_type];
			u16 reg_rtc_ymdalm  = sc_pmu_regtbl_rtc_ymdalm[pmu_type];

			p_save_alarm = (struct save_alarm *)1024;
			/* restore alarm */
			atc260x_reg_write(reg_rtc_msalm, p_save_alarm->msalm);
			atc260x_reg_write(reg_rtc_halm, p_save_alarm->halm);
			atc260x_reg_write(reg_rtc_ymdalm, p_save_alarm->ymdalm);
			if (alarm_wakeup) {
				enter_s2(0);
			} else {
				cpu_resume_fn = (void (*)(void))
					((u32)cpu_resume_fn & 0x7fffffff);
				cpu_resume_fn();
			}
		} else {
			cpu_resume_fn();
		}
	}
#endif	
}
#endif

struct _adc_key_t {
	u32 min;
	u32 max;
};
#if 0
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OWLXX_UPGRADE)
static struct _adc_key_t adc_key_val[] = {
	{967, 1023},
	{896, 966},
	{800, 895},
	{688, 799},
	{560, 687},
	{448, 559},
};

unsigned int owlxx_get_adc_val(void)
{
	int val, i;
	static cnt = 0;

	/* hack, delay to get adc key value */
	udelay(1200);

	val = 0;
	switch(OWLXX_PMU_ID) {
	case OWLXX_PMU_ID_ATC2603A:
		val = atc260x_reg_read(ATC2603A_PMU_RemConADC);
		break;
	case OWLXX_PMU_ID_ATC2603B:
		val = atc260x_reg_read(ATC2609A_PMU_RemConADC) / 4U;
		break;
	case OWLXX_PMU_ID_ATC2603C:
		val = atc260x_reg_read(ATC2603C_PMU_REMCONADC);
		break;
	}
	printf("%d adc value: 0x%x\n", cnt++, val);
	for (i = 0; i < sizeof(adc_key_val) / sizeof(struct _adc_key_t); i++) {
		if (val > adc_key_val[i].min && val < adc_key_val[i].max)
			owlxx_adc_val = i;
	}
}
void get_adc_cfg(void)
{
	if (afinfo->multi_cfg == 0) {
		printf("single cfg mode\n");
		return;
	}

	spl_board_init();
	pmu_early_init();

	owlxx_get_adc_val();

	adfusever_main();
}
#endif

#endif


extern ulong get_timer1(void);
extern void time1_init(void);
void print_btime(const char *prompt)
{
	ulong time_ms;
	time_ms = get_timer1();	
	printf("%s: %ld\n",prompt,time_ms);
}

void s_init(void)
{
#ifdef CONFIG_SPL_BUILD
    config_jtag();    
	time1_init();
	timer_init();	
	gd = &gdata;
	preloader_console_init();
	print_btime("s");
	prcm_init_f();
	pmu_init();
	//vdd_cpu_voltage_scan();
	prcm_init_r();
#else
#endif

#ifdef CONFIG_SPL_BUILD
	#ifndef CONFIG_ATM7059TC
	check_adfu();
	#endif
	cpu_resume_first();	
	memory_sys_init();
	cpu_resume_second();
	s_charger();

#endif
}

