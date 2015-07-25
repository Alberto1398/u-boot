/*
 * pmu.h - OWL PMIC driver
 *
 * Copyright (C) 2012, Actions Semiconductor Co. LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __ASM_ARM_ARCH_PMU_H__
#define __ASM_ARM_ARCH_PMU_H__

#include <afinfo.h>

#include <asm/arch/regs_map_atc2603a.h>
#include <asm/arch/regs_map_atc2603c.h>
#include <asm/arch/regs_map_atc2609a.h>

extern struct spi_slave g_atc260x_spi_slave;

/* ic versions */
#define OWLXX_PMU_ID_ATC2603A	0
#define OWLXX_PMU_ID_ATC2603B	1
#define OWLXX_PMU_ID_ATC2603C	2
#define OWLXX_PMU_ID_CNT	3

/* macro definition by PMU chip version */
#if defined(CONFIG_ATC2603A)
#define OWLXX_PMU_ID   OWLXX_PMU_ID_ATC2603A
#elif defined(CONFIG_ATC2603C)
#define OWLXX_PMU_ID   OWLXX_PMU_ID_ATC2603C
#elif defined(CONFIG_ATC2603B)
#define OWLXX_PMU_ID   OWLXX_PMU_ID_ATC2603B
#else
/* ԭ�����afinfo�Ǳ��õ�, ����������ʹSPL�����, �ʻ����ȶ���. */
#define OWLXX_PMU_ID   OWLXX_PMU_ID_ATC2603A /*(afinfo->pmu_id)*/
#endif

int pmu_init(void);
void pmu_prepare_for_s2(void);
void vdd_cpu_voltage_scan(void);
void vdd_cpu_voltage_store(void);
void set_ddr_voltage(int add_flag);

int atc260x_reg_read(unsigned short reg);
int atc260x_reg_write(unsigned short reg , unsigned short value);
int atc260x_set_bits(unsigned int reg, unsigned short mask, unsigned short val);
int atc260x_get_version(void);

/* for persistent storage */
/* ͳһ����260x�м���������/����λ��FW�üĴ���, ����������ɷ����������. */
/* ��ͬ��PMIC�и�����ķ���λ���ǲ�ͬ��, ����ͳһʹ�����׽ӿ�, ���Դ�����������Ҫ���鷳. */
enum {
	ATC260X_PSTORE_TAG_REBOOT_ADFU = 0,     /* Reboot into ADFU flag */
	ATC260X_PSTORE_TAG_REBOOT_RECOVERY,     /* Reboot into recovery flag */
	ATC260X_PSTORE_TAG_FW_S2,               /* Soft S2 flag */
	ATC260X_PSTORE_TAG_DIS_MCHRG,           /* disable mini_charger flag */
	ATC260X_PSTORE_TAG_RTC_MSALM,           /* RTC Alarm backup, used by reboot/suspend */
	ATC260X_PSTORE_TAG_RTC_HALM,            /* RTC Alarm backup */
	ATC260X_PSTORE_TAG_RTC_YMDALM,          /* RTC Alarm backup */
	ATC260X_PSTORE_TAG_GAUGE_CAP,           /* Save software gauge cap (8bits) */
	ATC260X_PSTORE_TAG_GAUGE_BAT_RES,       /* Save software battery estimated internal resistance (16bits) */
	ATC260X_PSTORE_TAG_GAUGE_ICM_EXIST,     /* ICM exists flag (1bit) */
	ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME,    /* Save shutdown time flag, used by gauge (31bits) */
	ATC260X_PSTORE_TAG_GAUGE_S2_CONSUMP,    /* ��������, ��¼S2�ڼ�Ĺ���. (6bits) */
	ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET,    /* ��������, coulomb_meter��λ��� (1bit) */
	ATC260X_PSTORE_TAG_RESUME_ADDR,         /* S2 resume address (low 32bit) */
	ATC260X_PSTORE_TAG_NUM
};

extern int atc260x_pstore_set(uint tag, u32 value);
extern int atc260x_pstore_get(uint tag, u32 *p_value);
extern ulong atc260x_pstore_get_noerr(uint tag);

extern int s2_resume;
extern void (*cpu_resume_fn)(void);
extern int alarm_wakeup;

#endif
