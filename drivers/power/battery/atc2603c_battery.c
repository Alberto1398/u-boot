/*
* Actions atc2603c PMIC battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*#define DEBUG*/
#include <common.h>
#include <fdtdec.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/power_charger.h>
#include <power/power_battery.h>
#include "atc260x_battery.h"
#include "atc2603c_battery.h"

#define CONST_ROUNDING				(5 * 100)
#define CONST_FACTOR				(1000)
#define PERCENTAGE_1				(1000)
#define ADC_LSB_FOR_10mohm			(2258)
#define ADC_LSB_FOR_20mohm			(4638)

#define BATTERY_INVALID_SOC			(0xfffff)

/*the reson why re-calc soc*/
#define CALC_SOC_DUETO_CLEAR			(1 << 0)
#define CALC_SOC_DUETO_RESET			(1 << 1)
#define CALC_SOC_DUETO_LOWPWR			(1 << 2)
#define CALC_SOC_DUETO_BIGGAP			(1 << 3)
#define CALC_SOC_DUETO_NOTINIT			(1 << 4)
#define EMPTY_CHARGE_SOC			(0)

/*current threshold(mA)*/
#define CHARGE_CURRENT_THRESHOLD		(60) /*if bati more than this, is charging*/
#define DISCHARGE_CURRENT_THRESHOLD		(30) /*if less than this, is discharging*/

#define ATC2603C_PMU_BASE			(0x00)
#define ATC2603C_PMU_SYS_CTL9			(ATC2603C_PMU_BASE + 0x09)
#define ATC2603C_PMU_BAT_CTL0			(ATC2603C_PMU_BASE + 0x0A)
#define ATC2603C_PMU_BAT_CTL1			(ATC2603C_PMU_BASE + 0x0B)
#define PMU_BAT_CTL0_BAT_UV_VOL_SHIFT		(14)
#define PMU_BAT_CTL0_BAT_UV_VOL_MASK		(0x3 << PMU_BAT_CTL0_BAT_UV_VOL_SHIFT)
#define PMU_BAT_CTL0_BAT_UV_VOL_3100MV		(0)
#define PMU_BAT_CTL0_BAT_UV_VOL_3300MV		(1)
#define PMU_BAT_CTL0_BAT_UV_VOL_3400MV		(2)
#define PMU_BAT_CTL0_BAT_UV_VOL_3500MV		(3)

#define ATC2603C_PMU_SYS_PENDING		(ATC2603C_PMU_BASE + 0x10)
#define PMU_SYS_PENDING_BAT_OV_STATUS           (1 << 15)
#define PMU_SYS_PENDING_BAT_UV_STATUS           (1 << 14)
#define PMU_SYS_PENDING_BAT_OC_STATUS           (1 << 13)
#define PMU_SYS_PENDING_BAT_CLR_STATUS          (1 << 0)

#define PMU_ICMADC				(ATC2603C_PMU_BASE + 0x50)
#define PMU_ICMADC_MASK				(0x7ff)
#define PMU_ICMADC_SIGN_BIT			(1 << 10)
#define PMU_ICMADC_VAL_BIT			(0x3ff)
#define MAX_SHUTDOWN_HOUR			(120*24)
#define FULL_CHARGE_SOC						(100000)

enum POWER_SUPPLY_STATUS {
	POWER_SUPPLY_STATUS_UNKNOWN = 0,
	POWER_SUPPLY_STATUS_CHARGING,
	POWER_SUPPLY_STATUS_DISCHARGING,
	POWER_SUPPLY_STATUS_NOT_CHARGING,
	POWER_SUPPLY_STATUS_FULL,
};

enum BAT_HEALTH {
	BAT_NORMAL,
	BAT_SHORTING,
	BAT_ABNORMAL_EVER
};
/**
 * bat_dts_items - battery config items from dts.
 *
 * @capacity : battery norminal capacity.
 * @shutdn_consume_pre : the consume during pre-powerdown, unit:uA.
 * @shutdn_consume : the consume during powerdown, unit:uA.
 */
struct  bat_dts_items {
	int capacity;
	int shutdown_current;
};

/**
 * atc2603c_clmt - atc2603c clmt information.
 * @clmb_r : remain clmb.
 * @soc_r : remain soc.
 * @clmb_a : available clmb.
 * @soc_a : available soc.
 * @soc_stored : stored soc.
 * @soc_cur : current soc.
 * @soc_show : soc showing.
 * @soc_pre : previous soc.
 * @fcc : full charge capacity, normal capacity.
 * @reason : record the reson why recalc soc.
 */
struct atc2603c_clmt {
	int health;
	int clmb_r;
	int soc_r;
	int clmb_a;
	int soc_a;
	unsigned int soc_stored;
	int soc_cur;
	int soc_show;
	int soc_pre;
	int fcc;
	int reason;

};

/**
 * atc2603c_battery - atc2603c battery information.
 *
 * @charging : record charge state, charing or discharging
 * @online : whether if battery is online.
 * @batv_pre : the bat vol before detecting battery.
 * @batv_post : the bat vol after detecting battery.
 * @health : battery is shorted or not.
 * @bat_vol : record bat voltage.
 * @bat_cur : record bat current.
 * @chg_type : record charge phrase.
 * @bat_temp : record bat temprature.
 * @clmt : the 2603c gauge.
 */
struct atc2603c_battery {
	int charging;
	int detected;
	int online;
	int batv_pre;
	int batv_post;
	int health;
	int bat_cur;
	int chg_type;
	int bat_temp;
	struct bat_dts_items items;
	struct atc2603c_clmt *clmt;
};
/**
 * battery_data - battery info, measured by BettaTeQ equipment.
 * @soc : the state of charge, unit:%;
 * @ocv : the open circuit voltage of battery, corresponding with soc;
 * @resistor : the inner resistor of battery, corresponding with soc&&ocv.
 */
struct battery_data {
	int soc;
	int ocv;
	int resistor;
};

static int rsense;
static struct atc2603c_battery battery = {.detected = 0};
static struct atc2603c_clmt clmt = {.soc_cur = 0,
									.soc_stored = 0,
									.reason = 0};
struct battery_data bat_info[16] = {
	{0, 3000, 400}, {4, 3620, 400}, {8, 3680, 200}, {12, 3693, 120},
	{16, 3715, 120}, {20, 3734, 120}, {28, 3763, 120}, {36, 3787, 120},
	{44, 3806, 120}, {52, 3832, 120}, {60, 3866, 120}, {68, 3920, 120},
	{76, 3973, 120}, {84, 4028, 120}, {92, 4090, 120}, {100, 4169, 120}
};

static int atc2603c_bat_get_pending_flag(void)
{
	int flag = 0;
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_SYS_PENDING);
	if (data  & PMU_SYS_PENDING_BAT_OV_STATUS)
		flag |= PMU_SYS_PENDING_BAT_OV_STATUS;
	if (data  & PMU_SYS_PENDING_BAT_UV_STATUS)
		flag |= PMU_SYS_PENDING_BAT_UV_STATUS;
	if (data  & PMU_SYS_PENDING_BAT_OC_STATUS)
		flag |= PMU_SYS_PENDING_BAT_OC_STATUS;

	return flag;
}

static void atc2603c_bat_clr_pending_flag(void)
{
	atc260x_reg_setbits(ATC2603C_PMU_SYS_PENDING,
		PMU_SYS_PENDING_BAT_CLR_STATUS,
		PMU_SYS_PENDING_BAT_CLR_STATUS);
	printf("%s PMU_SYS_PENDING(0x%x)\n",
		__func__, atc260x_reg_read(ATC2603C_PMU_SYS_PENDING));
}

static void atc2603c_bat_get_uv(void)
{
	int data = atc260x_reg_read(ATC2603C_PMU_BAT_CTL0);
	data &= PMU_BAT_CTL0_BAT_UV_VOL_MASK;

	switch (data) {
	case PMU_BAT_CTL0_BAT_UV_VOL_3100MV:
		debug("%s bat uv:3100mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3300MV:
		debug("%s bat uv:3300mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3400MV:
		debug("%s bat uv:3400mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3500MV:
		debug("%s bat uv:3500mv\n", __func__);
		break;
	default:
		printf("%s invalid bat uv threshold!", __func__);
		break;
	}

}

static void atc2603c_bat_set_uv(int vol)
{
	switch (vol) {
	case PMU_BAT_CTL0_BAT_UV_VOL_3100MV:
		atc260x_reg_setbits(ATC2603C_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3100MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3300MV:
		atc260x_reg_setbits(ATC2603C_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3300MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3400MV:
		atc260x_reg_setbits(ATC2603C_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3400MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3500MV:
		atc260x_reg_setbits(ATC2603C_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3500MV);
		break;
	default:
		printf("%s invalid bat uv threshold!", __func__);
		break;
	}

	atc2603c_bat_get_uv();

}

static int atc2603c_bat_measure_vol(void)
{
	int ret, vbat;

	ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_BATV, &vbat);
	if (ret) {
		printf("[%s] get vbat auxadc err!\n", __func__);
		return ret;
	}

	return vbat;
}

int atc2603c_bat_measure_vol_avr(int *batv)
{
	int data = 0;
	int sum = 0;
	int count = 0;
	int i;

	for (i = 0; i < 5; i++) {
		data = atc2603c_bat_measure_vol();
		if (data >= 0) {
			sum += data;
			count++;
		}
		udelay(2000);
	}
	data = sum / count;
	debug("[%s]: average batv = %d /%d = %dmv\n",
		__func__, sum, count, data);

	*batv = data;
	debug("enter %s, bat:%d\n", __func__, data);
	return 0;
}
static int  atc2603c_bat_measure_current(void)
{
	int reg_val;
	int ret;
	int tmp;
	while (1) {
		ret = atc260x_reg_read(PMU_ICMADC);
		if (ret < 0)
			return -1;
		reg_val = ret;
		if (reg_val & (1U << 11))	/* CM_DATAOK ? */
			break;
		udelay(200);
	}
	tmp = reg_val & 0x3ff;
	if (rsense == RSENSE_20mohm)
		tmp = (tmp * 9375U) >> 12;	/*tmp = tmp * 2343.75U / 1024U; */
	else
		tmp = (tmp * 9375U) >> 11;	/*tmp = tmp * 4687.5U / 1024U; */
	if (reg_val & 0x400U)
		tmp = -tmp;
	if (((tmp >= 0) && (tmp <= CHARGE_CURRENT_THRESHOLD)) ||
		((tmp <= 0) && (abs(tmp) <= DISCHARGE_CURRENT_THRESHOLD)))
		return 0;

	return tmp;
}

static void atc2603c_bat_measure_current_avr(int *bati)
{
	int data = 0;
	int sum = 0;
	int count = 0;
	int i;

	for (i = 0; i < 10; i++) {
		data = atc2603c_bat_measure_current();
		if ((data != 0) && (data != -1)) {
			sum += data;
			count++;
		}
		udelay(2000);
	}

	*bati = sum / count;
}

/**
 * atc2603c_clmt_get_asoc_lookup - lookup soc from ocv-soc table by ocv.
 * ocv = batv + ir,  (charging);
 * ocv = batv - ir,  (discharging)
 */
static int atc2603c_clmt_get_asoc_lookup(void)
{
	int ocv;
	int batv;
	int bati;
	int soc;
	int i;

	atc2603c_bat_measure_vol_avr(&batv);
	atc2603c_bat_measure_current_avr(&bati);
	debug("[%s] batv:%dmV,bati:%dmA!\n", __func__, batv, bati);
	ocv = batv - bati * bat_info[15].resistor / 1000;
	printf("[%s] batv:%dmV, bati:%dmA, ocv:%dmV\n", __func__, batv, bati, ocv);

	for (i = 0; i < 16; i++)
		if (ocv < bat_info[i].ocv)
			break;

	if (i == 0)
		return 0;
	else if (i > 15)
		return 100*1000;
	else {
		soc = (bat_info[i].soc - bat_info[i - 1].soc) * 1000 /
			(bat_info[i].ocv - bat_info[i - 1].ocv);
		soc = soc * (ocv - bat_info[i - 1].ocv);
		soc = (soc + CONST_ROUNDING) / 1000 + bat_info[i - 1].soc;
	}
	debug("%s ocv:%dmv, soc:%d, i:%d\n", __func__, ocv, soc, i);

	return soc*1000+163;
}

static int atc2603c_bat_cfg_init(struct atc2603c_battery *battery,
	const void *blob)
{
	struct bat_dts_items *items = &battery->items;
	int node;
	int error = 0;

	node = fdt_node_offset_by_compatible(blob, 0,
		"actions,atc2603c-battery");

	if (node < 0) {
		printf("%s no match in dts\n", __func__);
		return -1;
	}

	items->capacity = fdtdec_get_int(blob, node,
		"capacity", 0);
	error |= (items->capacity == -1);
	items->shutdown_current = fdtdec_get_int(blob, node,
		"shutdown_current", 50);
	if (error) {
		printf("%s get dts value failed\n", __func__);
		return -1;
	}
	debug("%s capacity(%d)\n",
		__func__, items->capacity);
	debug("%s shutdown_current(%d)\n",
		__func__, items->shutdown_current);

	return 0;
}

/*check the reason why re-calc soc*/
static unsigned int atc2603c_bat_chk_reason(void)
{
	unsigned int clmt_reset;
	unsigned int wakeup_flag;
	if (!clmt.soc_cur) {
		clmt.soc_cur = atc2603c_clmt_get_asoc_lookup();
		debug("[%s %d], clmt.soc_cur:%d\n", __func__, __LINE__, clmt.soc_cur);
	}
	if (!clmt.soc_stored) {
		atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP, &clmt.soc_stored);
		debug("[%s %d], soc_get:%d\n", __func__, __LINE__, clmt.soc_stored);
	}
	wakeup_flag = atc260x_misc_get_wakeup_flag();

	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET,
		&clmt_reset);
	debug("%s clmt_reset:%d\n", __func__, clmt_reset);
	/*clmt_reset is 0 means upgrade or pmu revive from power down(s5)*/
	if (!clmt_reset) {
		debug("RECALC SOC DUETO CLEAR!\n");
		atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP, BATTERY_INVALID_SOC);
		debug("[%s, %d], soc_set:%d\n", __func__, __LINE__, BATTERY_INVALID_SOC);
		atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET, 1);
		clmt.reason |= CALC_SOC_DUETO_CLEAR;
	}
	/*the stored soc not init*/
	if (clmt.soc_stored == BATTERY_INVALID_SOC) {
		debug("RECALC SOC DUETO NOTINIT!\n");
		clmt.reason |= CALC_SOC_DUETO_NOTINIT;
	}
	/*the machine was reset by onoff8s or reset key*/
	if (wakeup_flag & ATC260X_WAKEUP_SRC_RESET) {
		debug("RECALC SOC DUETO RESET!\n");
		clmt.reason |= CALC_SOC_DUETO_RESET;
	}
	/*the mathine entered into s4 due to battery vol is low*/
	if (atc2603c_bat_get_pending_flag() &
		PMU_SYS_PENDING_BAT_UV_STATUS) {
		debug("RECALC SOC DUETO LOWPWR!\n");
		printf("PMU_BAT_CTL0:0x%x, PMU_BAT_CTL1:0x%x, PMU_SYS_PENDING:0x%x\n",
			atc260x_reg_read(ATC2603C_PMU_BAT_CTL0),
			atc260x_reg_read(ATC2603C_PMU_BAT_CTL1),
			atc260x_reg_read(ATC2603C_PMU_SYS_PENDING));
//		atc2603c_bat_clr_pending_flag();
//		clmt.reason |= CALC_SOC_DUETO_LOWPWR;
	}
	/*big gap between the stored soc and calced soc due to leakage,etc.*/
	if ((int)abs(clmt.soc_cur - clmt.soc_stored) > 30 &&
		clmt.soc_stored != -1) {
		debug("RECALC SOC DUETO BIGGAP!\n");
		/*clmt.reason |= CALC_SOC_DUETO_BIGGAP;*/
	}
	debug("%s reason:0x%x\n", __func__, clmt.reason);
	return clmt.reason;
}
static void atc2603c_clmt_init(void)
{
	atc2603c_bat_chk_reason();
}
/**
 * atc2603c_bat_check_health - check whether if bat is healthy or not.
 * if the vol diff is more than 300mv between bat detect before and after detecting, means bat is shorted;
 * if bat vol is less ocv0 before or after detecting,means bat is over discharged.
 */
static int atc2603c_bat_check_health(struct atc2603c_battery *battery)
{
	int batv_pre;
	int batv_post;

	batv_pre = battery->batv_pre;
	batv_post = battery->batv_post;

	if (abs(batv_post - batv_pre) > 300 ||
		(batv_post < bat_info[0].ocv &&
		batv_pre < bat_info[0].ocv)) {
		battery->health = BAT_ABNORMAL_EVER;
		printf("health:abnormal ever\n");
	} else {
		battery->health = BAT_NORMAL;
		debug("health:normal\n");
	}

	return battery->health;
}
/**
 * bat_calc_shutdn_consumed_soc - calc consumed soc during powerdown.
 */
static int atc2603c_bat_calc_passing_soc(void)
{
	unsigned int shutdn_time_hour;
	unsigned long cur_time_hour;
	unsigned long cur_time_sec;
	long interval_hour;
	unsigned long clmb = 0;
	int consumed_soc;

	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME,
		&shutdn_time_hour);
	shutdn_time_hour &= 0x3fff;
	/*if reset system, PMU_OV_INT_EN will clean to 0*/
	if (!shutdn_time_hour)
		return 0;

	atc260x_rtc_tm_to_time(&cur_time_sec);
	cur_time_hour = cur_time_sec/3600;
	debug("%s, cur_time_hour:%ld\n", __func__, cur_time_hour);
	cur_time_hour &= 0x3fff;
	if (cur_time_hour < shutdn_time_hour)
		cur_time_hour += 1<<14;

	interval_hour = (cur_time_hour - shutdn_time_hour);
	debug("now_h(%ld),pd_h(%d)\n",
		cur_time_hour, shutdn_time_hour);
	if ((interval_hour < 0) || (interval_hour > MAX_SHUTDOWN_HOUR))
		return 0;

	/**
	 * cosume unit:uAh
	 */
	clmb = battery.items.shutdown_current * interval_hour;
	consumed_soc = (clmb * 100 /battery.items.capacity)/1000;
	printf("shutdown consumed clmb:%ldmAh, soc:%d\n", clmb/1000, consumed_soc);
	if ((consumed_soc < 0) || (consumed_soc > 100))
		consumed_soc = 0;
	debug("[%s, %d] passing soc:%d\n",__func__, __LINE__, consumed_soc);
	return consumed_soc*1000;
}
/**
 * atc2609a_bat_calc_soc - calc battery soc during booting.
 */
int atc2603c_bat_calc_soc(void)
{
	if (battery.detected) {
		debug("[%s %d], clmt.soc_cur:%d\n", __func__, __LINE__, clmt.soc_cur);
		return clmt.soc_cur;
	}

	/*recalc soc due to some reson*/
	if (clmt.reason) {
		if (!clmt.soc_cur || !clmt.soc_stored) {
			clmt.soc_cur = atc2603c_clmt_get_asoc_lookup();
			debug("[%s %d], clmt.soc_cur:%d\n", __func__, __LINE__, clmt.soc_cur);
			atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP, &clmt.soc_stored);
			debug("[%s %d], soc_get:%d\n", __func__, __LINE__, clmt.soc_stored);
		}

		if (clmt.reason & CALC_SOC_DUETO_CLEAR ||
			clmt.reason & CALC_SOC_DUETO_NOTINIT) {
//			atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP, clmt.soc_cur);
			debug("[%s, %d], soc_set:%d\n", __func__, __LINE__, clmt.soc_cur);
			return clmt.soc_cur;
		}

		if (clmt.reason & CALC_SOC_DUETO_RESET) {
			atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP, &clmt.soc_cur);
			debug("[%s, %d], soc_get:%d\n", __func__, __LINE__, clmt.soc_cur);
			if (clmt.soc_cur < 0)
				clmt.soc_cur = 0;
			if (clmt.soc_cur > FULL_CHARGE_SOC-1)
				clmt.soc_cur = FULL_CHARGE_SOC-1;
			return clmt.soc_cur;
		}

		if (clmt.reason & CALC_SOC_DUETO_LOWPWR ||
			clmt.reason & CALC_SOC_DUETO_BIGGAP) {
			if (clmt.soc_cur < 15000 ||
				(int)abs(clmt.soc_stored - clmt.soc_cur) < 15000)
				clmt.soc_cur = min(clmt.soc_stored, clmt.soc_cur);
			atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP, clmt.soc_cur);
			debug("[%s, %d], soc_set:%d\n\n", __func__, __LINE__, clmt.soc_cur);
			return clmt.soc_cur;
		}
	}

	/*boot normal, soc_cur=soc_sotred-shutdn_consumed_soc*/
	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP, &clmt.soc_stored);
	debug("[%s, %d],soc_get:%d\n",__func__, __LINE__, clmt.soc_stored);
	clmt.soc_cur = clmt.soc_stored - atc2603c_bat_calc_passing_soc();
	if (clmt.soc_cur < 0)
		clmt.soc_cur = 0;
	atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP, clmt.soc_cur);
	debug("[%s, %d], soc_set:%d\n", __func__, __LINE__, clmt.soc_cur);

	return clmt.soc_cur;
}

void atc2603c_bat_dump(void)
{
	printf("BAT:online(%d),vol(%dmv),soc(%d%%),reason(%s%s%s%s%s)\n",
		battery.online,
		battery.batv_post,
		clmt.soc_cur/1000,
		(clmt.reason & CALC_SOC_DUETO_CLEAR) ? "c" : "",
		(clmt.reason & CALC_SOC_DUETO_RESET) ? " r" : "",
		(clmt.reason & CALC_SOC_DUETO_LOWPWR) ? " l" : "",
		(clmt.reason & CALC_SOC_DUETO_BIGGAP) ? " b" : "",
		(clmt.reason & CALC_SOC_DUETO_NOTINIT) ? " n" : "");
}

static struct power_battery atc2603c_bat = {
	.measure_voltage = atc2603c_bat_measure_vol_avr,
	.chk_online = power_chk_bat_online_intermeddle,
	.calc_soc = atc2603c_bat_calc_soc,
};
/**
 * exec all the exported interface, return for next calling.
 */
int atc2603c_bat_init(const void *blob)
{
	int ret;
	ret = atc2603c_bat_cfg_init(&battery, blob);
	if (ret)
		return ret;
	rsense = atc260x_charger_get_rsense();
	atc2603c_bat_measure_vol_avr(&battery.batv_pre);
	battery.online = atc260x_chk_bat_online_intermeddle();
	atc2603c_bat_measure_vol_avr(&battery.batv_post);
	atc2603c_bat_set_uv(PMU_BAT_CTL0_BAT_UV_VOL_3100MV);
	atc2603c_bat_check_health(&battery);
	atc2603c_clmt_init();
	atc2603c_bat_calc_soc();
	atc260x_bat_register(&atc2603c_bat);
	battery.detected = 1;
	atc2603c_bat_dump();

	return 0;
}
