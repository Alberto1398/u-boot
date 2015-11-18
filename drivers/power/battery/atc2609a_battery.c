/*
* Actions ATC2609A PMIC battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <fdtdec.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x_charger.h>
#include <power/atc2609a_charger.h>
#include <power/power_battery.h>
#include "atc260x_battery.h"
#include "atc2609a_battery.h"

#define CONST_ROUNDING				(5 * 100)
#define CONST_FACTOR				(1000)
#define PERCENTAGE_1				(1000)
#define ADC_LSB_FOR_10mohm			(1144)
#define ADC_LSB_FOR_20mohm			(572)

#define SECONDS_PER_HOUR			(3600)
#define ADC_LSB_FOR_BATV			(732)
#define SHARE_VOL_FACTOR			(2) /*shared vol factor for batv*/
#define CONST_SOC_STOP				(1000)
#define FULL_CHARGE_SOC				(100000)
#define EMPTY_CHARGE_SOC			(0)
#define S2_CONSUME_DEFAULT_UA			(15000)
#define S4_CONSUME_DEFAULT_UA			(50)
/*current threshold(mA)*/
#define CHARGE_CURRENT_THRESHOLD		(60) /*if bati more than this, is charging*/
#define DISCHARGE_CURRENT_THRESHOLD		(30) /*if less than this, is discharging*/

#define ATC2609A_CAP_GAUGE_RESET		(1 << 0)

#define PMU_CHARGER_PHASE_PRECHARGE		(0)
#define PMU_CHARGER_PHASE_CONSTANT_CURRENT	(1)
#define PMU_CHARGER_PHASE_CONSTANT_VOLTAGE	(2)
#define CHARGE_CV_CURRENT_THRESHOLD             (50)

#define ATC2609A_PMU_BASE			(0x00)

#define ATC2609A_CLMT_CTL0			(ATC2609A_PMU_BASE + 0x82)
#define PMU_CLMT_CTL0_CLMT_EN			(1 << 0)
#define PMU_CLMT_CTL0_INIT_DATA_EN		(1 << 1)
#define PMU_CLMT_CTL0_U_STOP_SHIFT		(2)
#define PMU_CLMT_CTL0_U_STOP_MASK		(0xfff << PMU_CLMT_CTL0_U_STOP_SHIFT)
#define PMU_CLMT_CTL0_TIMER_SHIFT		(14)
#define PMU_CLMT_CTL0_TIMER_MASK		(0x3 << PMU_CLMT_CTL0_TIMER_SHIFT)
#define PMU_CLMT_CTL0_TIMER_1H			(0 << PMU_CLMT_CTL0_TIMER_SHIFT)
#define PMU_CLMT_CTL0_TIMER_3H			(1 << PMU_CLMT_CTL0_TIMER_SHIFT)
#define PMU_CLMT_CTL0_TIMER_5H			(2 << PMU_CLMT_CTL0_TIMER_SHIFT)
#define PMU_CLMT_CTL0_TIMER_7H			(3 << PMU_CLMT_CTL0_TIMER_SHIFT)

#define ATC2609A_PMU_SWCHG_CTL1			(ATC2609A_PMU_BASE + 0x17)
#define SWCHG_CTL1_EN_BAT_DET			(1 << 15)

#define ATC2609A_PMU_SWCHG_CTL4			(ATC2609A_PMU_BASE + 0x1A)
#define SWCHG_CTL4_BAT_EXT			(1 << 8)
#define SWCHG_CTL4_BAT_DT_OVER			(1 << 7)

#define ATC2609A_CLMT_DATA0			(ATC2609A_PMU_BASE + 0x83)
#define PMU_CLMT_DATA0_Q_MAX_SHIFT		(0)
#define PMU_CLMT_DATA0_Q_MAX_MASK		(0xffff << PMU_CLMT_DATA0_Q_MAX_SHIFT)

#define ATC2609A_CLMT_DATA1			(ATC2609A_PMU_BASE + 0x84)
#define PMU_CLMT_DATA1_SOC_R_SHIFT		(8)
#define PMU_CLMT_DATA1_SOC_R_MASK		(0x7f << PMU_CLMT_DATA1_SOC_R_SHIFT)
#define PMU_CLMT_DATA1_SOC_A_SHIFT		(0)
#define PMU_CLMT_DATA1_SOC_A_MASK		(0x7f << PMU_CLMT_DATA1_SOC_A_SHIFT)

#define ATC2609A_CLMT_DATA2			(ATC2609A_PMU_BASE + 0x85)
#define PMU_CLMT_DATA2_SOC_Q_R_SHIFT		(0)
#define PMU_CLMT_DATA2_SOC_Q_R_MASK		(0xffff << PMU_CLMT_DATA2_SOC_Q_R_SHIFT)

#define ATC2609A_CLMT_DATA3			(ATC2609A_PMU_BASE + 0x86)
#define PMU_CLMT_DATA2_SOC_Q_A_SHIFT		(0)
#define PMU_CLMT_DATA2_SOC_Q_A_MASK		(0xffff << PMU_CLMT_DATA2_SOC_Q_A_SHIFT)

#define ATC2609A_CLMT_ADD1			(ATC2609A_PMU_BASE + 0x88)
#define PMU_CLMT_ADD1_SIGN_BIT_SHIFT		(15)
#define PMU_CLMT_ADD1_SIGN_BIT_MASK		(1 << PMU_CLMT_ADD1_SIGN_BIT_SHIFT)
#define PMU_CLMT_ADD1_SIGN_BIT			(1 << PMU_CLMT_ADD1_SIGN_BIT_SHIFT)

#define ATC2609A_CLMT_OCV_TABLE			(ATC2609A_PMU_BASE + 0x89)
#define PMU_CLMT_OCV_TABLE_SOC_SEL_SHIFT	(12)
#define PMU_CLMT_OCV_TABLE_SOC_SEL_MASK		(0xf << PMU_CLMT_OCV_TABLE_SOC_SEL_SHIFT)
#define PMU_CLMT_OCV_TABLE_OCV_SET_SHIFT	(0)
#define PMU_CLMT_OCV_TABLE_OCV_SET_MASK		(0xfff << PMU_CLMT_OCV_TABLE_OCV_SET_SHIFT)

#define ATC2609A_CLMT_R_TABLE			(ATC2609A_PMU_BASE + 0x8A)
#define PMU_CLMT_R_TABLE_SOC_SEL_SHIFT		(12)
#define PMU_CLMT_R_TABLE_SOC_SEL_MASK		(0xf << PMU_CLMT_R_TABLE_SOC_SEL_SHIFT)
#define PMU_CLMT_R_TABLE_R_SET_SHIFT		(0)
#define PMU_CLMT_R_TABLE_R_SET_MASK		(0x3ff << PMU_CLMT_R_TABLE_R_SET_SHIFT)

#define ATC2609A_PMU_ADC12B_I			(ATC2609A_PMU_BASE + 0x56)
#define PMU_ADC12B_I_MASK			(0x3fff)
#define PMU_ADC12B_I_SIGN_BIT			(1 << 13)

#define ATC2609A_PMU_ADC12B_V			(ATC2609A_PMU_BASE + 0x57)
#define PMU_ADC12B_V_MASK			(0xfff)

#define ATC2609A_PMU_SWCHG_CTL1			(ATC2609A_PMU_BASE + 0x17)
#define PMU_SWCHG_CTL1_EN_BAT_DET		(1 << 15)

#define ATC2609A_PMU_SWCHG_CTL4			(ATC2609A_PMU_BASE + 0x1A)
#define PMU_SWCHG_CTL4_PHASE_SHIFT		(11)
#define PMU_SWCHG_CTL4_PHASE_MASK		(0x3 << PMU_SWCHG_CTL4_PHASE_SHIFT)
#define PMU_SWCHG_CTL4_PHASE_PRECHARGE		(1 << PMU_SWCHG_CTL4_PHASE_SHIFT)
#define PMU_SWCHG_CTL4_BAT_EXT			(1 << 8)
#define PMU_SWCHG_CTL4_BAT_DT_OVER		(1 << 7)
#define PMU_RTC_CTL_VERI                        (1 << 10)

#define ATC2609A_PMU_SYS_PENDING		(ATC2609A_PMU_BASE + 0x10)
#define PMU_SYS_PENDING_BAT_OV_STATUS           (1 << 15)
#define PMU_SYS_PENDING_BAT_UV_STATUS           (1 << 14)
#define PMU_SYS_PENDING_BAT_OC_STATUS           (1 << 13)
#define PMU_SYS_PENDING_BAT_CLR_STATUS          (1 << 0)

#define ATC2609A_PMU_BAT_CTL0			(ATC2609A_PMU_BASE + 0x0A)
#define PMU_BAT_CTL0_BAT_UV_VOL_SHIFT		(14)
#define PMU_BAT_CTL0_BAT_UV_VOL_MASK		(0x3 << PMU_BAT_CTL0_BAT_UV_VOL_SHIFT)
#define PMU_BAT_CTL0_BAT_UV_VOL_3100MV		(0)
#define PMU_BAT_CTL0_BAT_UV_VOL_3300MV		(1)
#define PMU_BAT_CTL0_BAT_UV_VOL_3400MV		(2)
#define PMU_BAT_CTL0_BAT_UV_VOL_3500MV		(3)


/*the min over-charged protect voltage of battery circuit*/
#define MIN_OVER_CHARGERD_VOL                   (4275)

#define CLMT_PRE_AMPLIFIER_COMSUMP_UA		(300)
#define CLMT_ADC_COMSUMP_UA			(700)

/*the reson why re-calc soc*/
#define CALC_SOC_DUETO_CLEAR			(1 << 0)
#define CALC_SOC_DUETO_RESET			(1 << 1)
#define CALC_SOC_DUETO_LOWPWR			(1 << 2)
#define CALC_SOC_DUETO_BIGGAP			(1 << 3)
#define CALC_SOC_DUETO_NOTINIT			(1 << 4)

#define BATTERY_INVALID_SOC			(0xff)

enum POWER_SUPPLY_STATUS {
	POWER_SUPPLY_STATUS_UNKNOWN = 0,
	POWER_SUPPLY_STATUS_CHARGING,
	POWER_SUPPLY_STATUS_DISCHARGING,
	POWER_SUPPLY_STATUS_NOT_CHARGING,
	POWER_SUPPLY_STATUS_FULL,
};

enum CLMT_TABLE {
	CLMT_OCV_TABLE,
	CLMT_R_TABLE
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
	int shutdn_current_pre;
	int shutdown_current;
};

/**
 * atc2609a_clmt - atc2609a clmt information.
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
struct atc2609a_clmt {
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
 * atc2609a_battery - atc2609a battery information.
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
 * @clmt : the 2609a gauge.
 */
struct atc2609a_battery {
	int charging;
	int online;
	int batv_pre;
	int batv_post;
	int health;
	int bat_vol;
	int bat_cur;
	int chg_type;
	int bat_temp;
	struct bat_dts_items items;
	struct atc2609a_clmt *clmt;
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
static struct atc2609a_battery battery;
static struct atc2609a_clmt clmt;
struct battery_data fake_bat_info[16];
struct battery_data bat_info[16] = {
	{0, 3000, 400}, {4, 3620, 400}, {8, 3680, 200}, {12, 3693, 120},
	{16, 3715, 120}, {20, 3734, 120}, {28, 3763, 120}, {36, 3787, 120},
	{44, 3806, 120}, {52, 3832, 120}, {60, 3866, 120}, {68, 3920, 120},
	{76, 3973, 120}, {84, 4028, 120}, {92, 4090, 120}, {100, 4169, 120}
};

static int atc2609a_bat_get_pending_flag(void)
{
	int flag = 0;
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_SYS_PENDING);
	if (data  & PMU_SYS_PENDING_BAT_OV_STATUS)
		flag |= PMU_SYS_PENDING_BAT_OV_STATUS;
	if (data  & PMU_SYS_PENDING_BAT_UV_STATUS)
		flag |= PMU_SYS_PENDING_BAT_UV_STATUS;
	if (data  & PMU_SYS_PENDING_BAT_OC_STATUS)
		flag |= PMU_SYS_PENDING_BAT_OC_STATUS;

	return flag;
}

static void atc2609a_bat_clr_pending_flag(void)
{
	atc260x_reg_setbits(ATC2609A_PMU_SYS_PENDING,
		PMU_SYS_PENDING_BAT_CLR_STATUS,
		PMU_SYS_PENDING_BAT_CLR_STATUS);
	printf("%s PMU_SYS_PENDING(0x%x)\n",
		__func__, atc260x_reg_read(ATC2609A_PMU_SYS_PENDING));
}

int atc2609a_bat_check_online(void)
{
	int adp_type;
	int data;
	int count = 0;

	adp_type = atc260x_charger_check_online();
	if (adp_type == ADAPTER_TYPE_NO_PLUGIN) {
		battery.online = 1;
		printf("bat online\n");

		return battery.online;
	}

	/* dectect bit 0 > 1 to start dectecting */
	data = atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
		SWCHG_CTL1_EN_BAT_DET, SWCHG_CTL1_EN_BAT_DET);
	if (data < 0)
		return data;

	/* wait bat detect over */
	do {
		mdelay(70);
		data = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL4) &
			SWCHG_CTL4_BAT_DT_OVER;
		debug("%s wait battery detect over,data:0x%x\n",
			__func__, data);
		count += 70;
		if (count >= 300)
			break;
	} while (!data);

	data = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL4);
	if (data < 0)
		return data;
	if (data & SWCHG_CTL4_BAT_EXT)
		battery.online = 1;
	else
		battery.online = 0;

	/* clear battery detect bit, otherwise cannot changer */
	data = atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
		SWCHG_CTL1_EN_BAT_DET, 0);
	if (data < 0)
		return data;

	printf("%s battery exist:%d\n", __func__, battery.online);

	return battery.online;
}

static void atc2609a_bat_get_uv(void)
{
	int data = atc260x_reg_read(ATC2609A_PMU_BAT_CTL0);
	data &= PMU_BAT_CTL0_BAT_UV_VOL_MASK;

	switch (data) {
	case PMU_BAT_CTL0_BAT_UV_VOL_3100MV:
		debug("%s bat uv:3100mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3300MV:
		debug("%s bat uv:3100mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3400MV:
		debug("%s bat uv:3100mv\n", __func__);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3500MV:
		debug("%s bat uv:3100mv\n", __func__);
		break;
	default:
		printf("%s invalid bat uv threshold!", __func__);
		break;
	}

}

static void atc2609a_bat_set_uv(int vol)
{
	switch (vol) {
	case PMU_BAT_CTL0_BAT_UV_VOL_3100MV:
		atc260x_reg_setbits(ATC2609A_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3100MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3300MV:
		atc260x_reg_setbits(ATC2609A_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3300MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3400MV:
		atc260x_reg_setbits(ATC2609A_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3400MV);
		break;
	case PMU_BAT_CTL0_BAT_UV_VOL_3500MV:
		atc260x_reg_setbits(ATC2609A_PMU_BAT_CTL0,
			PMU_BAT_CTL0_BAT_UV_VOL_MASK,
			PMU_BAT_CTL0_BAT_UV_VOL_3500MV);
		break;
	default:
		printf("%s invalid bat uv threshold!", __func__);
		break;
	}

	atc2609a_bat_get_uv();

}

static void atc2609a_clmt_enable(bool enable)
{

	if (enable)
		atc260x_reg_setbits(ATC2609A_CLMT_CTL0,
			PMU_CLMT_CTL0_CLMT_EN, PMU_CLMT_CTL0_CLMT_EN);
	else
		atc260x_reg_setbits(ATC2609A_CLMT_CTL0,
			PMU_CLMT_CTL0_CLMT_EN, 0);
}

static void atc2609a_clmt_get_ocv_table (void)
{
	int i;
	int data;

	for (i = 0; i < 16; i++) {
		/* Fistr of all, set ocv to  zero, then write into  reg together with soc value.
		  * Next to read this reg.
		 */
		atc260x_reg_write(ATC2609A_CLMT_OCV_TABLE,
			i <<  PMU_CLMT_OCV_TABLE_SOC_SEL_SHIFT);
		data = atc260x_reg_read(ATC2609A_CLMT_OCV_TABLE)
			& PMU_CLMT_OCV_TABLE_OCV_SET_MASK;
		data = (data * ADC_LSB_FOR_BATV * 2 + CONST_ROUNDING) /
			CONST_FACTOR;
		debug("%s, (%d)~(%dmv)\n", __func__, bat_info[i].soc, data);
	}
}

static void atc2609a_clmt_set_ocv_table (struct battery_data *bat_info)
{
	int i;
	int ocv;

	for (i = 0; i < 16; i++) {
		ocv = (bat_info[i].ocv * CONST_FACTOR + CONST_ROUNDING)
			/ADC_LSB_FOR_BATV / 2;
		atc260x_reg_write(ATC2609A_CLMT_OCV_TABLE,
			ocv | (i << PMU_CLMT_OCV_TABLE_SOC_SEL_SHIFT));
	}

	atc2609a_clmt_get_ocv_table();
}

static int atc2609a_clmt_get_fcc(void)
{
	int fcc;

	fcc =  atc260x_reg_read(ATC2609A_CLMT_DATA0);

	if (rsense == RSENSE_10mohm) {
		fcc = (fcc * ADC_LSB_FOR_10mohm + CONST_ROUNDING) /
			CONST_FACTOR;
	} else if (rsense == RSENSE_20mohm) {
		fcc = (fcc * ADC_LSB_FOR_20mohm + CONST_ROUNDING) /
			CONST_FACTOR;
	} else {
		printf("[%s] rsense invalid!", __func__);
		return -1;
	}
	return fcc;
}

/**
 * atc2609a_clmt_set_fcc - set full charger coulomb, norminal capacity initialized.
 */
static int  atc2609a_clmt_set_fcc(int fcc)
{
	int data = 0;

	if (rsense == RSENSE_10mohm) {
		data |= (fcc * CONST_FACTOR + CONST_ROUNDING) /
			ADC_LSB_FOR_10mohm;
	} else if (rsense == RSENSE_20mohm) {
		data |= (fcc * CONST_FACTOR + CONST_ROUNDING) /
			ADC_LSB_FOR_20mohm;
	} else {
		printf("[%s] rsense invalid!", __func__);
		return -1;
	}

	atc260x_reg_write(ATC2609A_CLMT_DATA0, data);

	return 0;
}


static int atc2609a_bat_measure_vol(void)
{
	int data;

	data =  atc260x_reg_read(ATC2609A_PMU_ADC12B_V);
	data = data & PMU_ADC12B_V_MASK;
	data = (data * ADC_LSB_FOR_BATV * 2 + CONST_ROUNDING) /
		CONST_FACTOR;
	debug("[%s] batv:%d\n", __func__, data);

	return data;
}

int atc2609a_bat_measure_vol_avr(int *batv)
{
	int data;
	int sum = 0;
	int i;
	for (i = 0; i < 5; i++) {
		sum += atc2609a_bat_measure_vol();
		udelay(2000);
	}
	data = sum / 5;
	debug("[%s]: average batv = %d /%d = %dmv\n",
		__func__, sum, 5, data);

	*batv = data;
	debug("enter %s, bat:%d\n", __func__, data);
	return 0;

}

static int  atc2609a_bat_measure_current(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_ADC12B_I) &
		PMU_ADC12B_I_MASK;

	if (PMU_ADC12B_I_SIGN_BIT & data) {
		if (rsense == RSENSE_10mohm) {
			data =  ((data ^ PMU_ADC12B_I_MASK) & PMU_ADC12B_I_MASK) + 1;
			data = -(data * ADC_LSB_FOR_10mohm + CONST_ROUNDING)  /
				CONST_FACTOR;
		} else if (rsense == RSENSE_20mohm) {
			data =  ((data ^ PMU_ADC12B_I_MASK) & PMU_ADC12B_I_MASK) + 1;
			data = -(data * ADC_LSB_FOR_20mohm + CONST_ROUNDING)  /
				CONST_FACTOR;
		} else {
			printf("[%s] rsense invalid!", __func__);
			return -1;
		 }
	} else {
		if (rsense == RSENSE_10mohm) {
			data = (data * ADC_LSB_FOR_10mohm + CONST_ROUNDING) /
				CONST_FACTOR;
		} else if (rsense == RSENSE_20mohm) {
			data = (data * ADC_LSB_FOR_20mohm + CONST_ROUNDING) /
				CONST_FACTOR;
		} else {
			printf("[%s] rsense invalid!", __func__);
			return -1;
		}
	}

	if ((data >= 0) &&
		(data <= CHARGE_CURRENT_THRESHOLD))
		return 0;

	if ((data <= 0) &&
		(abs(data) <= DISCHARGE_CURRENT_THRESHOLD))
		return 0;

	return data;
}

static void atc2609a_bat_measure_current_avr(int *bati)
{
	int data = 0;
	int sum = 0;
	int count = 0;
	int i;

	for (i = 0; i < 10; i++) {
		data = atc2609a_bat_measure_current();
		if (bati >= 0) {
			sum += data;
			count++;
		}
		udelay(2000);
	}

	*bati = data / count;
}


static void atc2609a_bat_get_charge_status(int *status)
{
	int data;

	atc2609a_bat_measure_current_avr(&data);

	if (data < 0) {
		*status =  POWER_SUPPLY_STATUS_DISCHARGING;
	} else if (data > 0) {
		*status = POWER_SUPPLY_STATUS_CHARGING;
	} else {
		*status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	}

	printf("charge status:%d\n", *status);

}
/**
 * atc2609a_clmt_get_asoc_lookup - lookup soc from ocv-soc table by ocv.
 * ocv = batv + ir,  (charging);
 * ocv = batv - ir,  (discharging)
 */
static int atc2609a_clmt_get_asoc_lookup(void)
{
	int ocv;
	int batv;
	int bati;
	int status;
	int soc;
	int i;

	atc2609a_bat_measure_vol_avr(&batv);
	atc2609a_bat_measure_current_avr(&bati);

	atc2609a_bat_get_charge_status(&status);
	if (status == POWER_SUPPLY_STATUS_CHARGING)
		ocv = batv - bati * bat_info[15].resistor / 1000;
	else if (status == POWER_SUPPLY_STATUS_DISCHARGING)
		ocv = batv + bati * bat_info[15].resistor / 1000;
	else
		ocv = batv;

	debug("%s ocv:%dmv\n", __func__, ocv);

	for (i = 0; i < 16; i++)
		if (ocv < bat_info[i].ocv)
			break;

	if (i == 0)
		return 0;
	else if (i > 15)
		return 100;
	else {
		soc = (bat_info[i].soc - bat_info[i - 1].soc) * 1000 /
			(bat_info[i].ocv - bat_info[i - 1].ocv);
		soc = soc * (ocv - bat_info[i - 1].ocv);
		soc = (soc + CONST_ROUNDING) / 1000 + bat_info[i - 1].soc;
	}

	return soc;
}

static void atc2609a_clmt_construct_fake_ocv_table(void)
{
	int record = 0;
	int i;

	for (i = 15; i >= 0; i--) {
		if (battery.batv_post >= bat_info[i].ocv) {
			record = i;
			break;
		}
	}

	 for (i = 0; i < 16; i++) {
		fake_bat_info[i].ocv =
			bat_info[i].ocv * battery.batv_pre / bat_info[record].ocv;
		debug("%s:fake ocv table(%d)~(%dmv)\n",
			__func__, i, fake_bat_info[i].ocv);
	 }
}
static int atc2609a_bat_cfg_init(struct atc2609a_battery *battery,
	const void *blob)
{
	struct bat_dts_items *items = &battery->items;
	int node;
	int error = 0;

	node = fdt_node_offset_by_compatible(blob, 0,
		"actions,atc2609a-battery");

	if (node < 0) {
		printf("%s no match in dts\n", __func__);
		return -1;
	}

	items->capacity = fdtdec_get_int(blob, node,
		"capacity", 0);
	error |= (items->capacity == -1);
	items->shutdown_current = fdtdec_get_int(blob, node,
		"shutdown_current", 0);
	error |= (items->shutdown_current == -1);
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

/**
 * atc2609a_clmt_check_fcc - check whether if fcc is normal.
 * if fcc is abnormal, then reset fcc to norminal capacity.
 */
static void atc2609a_clmt_check_fcc(void)
{
	int diff_val;
	int fcc;

	fcc = atc2609a_clmt_get_fcc();
	diff_val = abs(battery.items.capacity - fcc);
	if (diff_val >= fcc / 2)
		atc2609a_clmt_set_fcc(battery.items.capacity);
}
/*check the reason why re-calc soc*/
static unsigned int atc2609a_bat_chk_reason(void)
{
	unsigned int clmt_reset;
	unsigned int wakeup_flag;

	clmt.soc_cur = atc2609a_clmt_get_asoc_lookup();
	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP,
				&clmt.soc_stored);
	debug("%s soc_real:%d,soc_stored:%d\n",
		__func__, clmt.soc_cur, clmt.soc_stored);
	wakeup_flag = atc260x_misc_get_wakeup_flag();
	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET,
		&clmt_reset);

	/*clmt_reset is 0 means upgrade or pmu revive from power down(s5)*/
	if (!clmt_reset) {
		printf("RECALC SOC DUETO CLEAR!\n");
		atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP, BATTERY_INVALID_SOC);
		atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET, 1);
		clmt.reason |= CALC_SOC_DUETO_CLEAR;
	}
	/*the stored soc not init*/
	if (clmt.soc_stored == BATTERY_INVALID_SOC) {
		printf("RECALC SOC DUETO NOTINIT!\n");
		clmt.reason |= CALC_SOC_DUETO_NOTINIT;
	}
	/*the machine was reset by onoff8s or reset key*/
	if (wakeup_flag & ATC260X_WAKEUP_SRC_RESET) {
		printf("RECALC SOC DUETO RESET!\n");
		clmt.reason |= CALC_SOC_DUETO_RESET;
	}
	/*the mathine entered into s4 due to battery vol is low*/
	if (atc2609a_bat_get_pending_flag() &
		PMU_SYS_PENDING_BAT_UV_STATUS) {
		printf("RECALC SOC DUETO LOWPWR!\n");
		atc2609a_bat_clr_pending_flag();
		clmt.reason |= CALC_SOC_DUETO_LOWPWR;
	}
	/*big gap between the stored soc and calced soc due to leakage,etc.*/
	if ((int)abs(clmt.soc_cur - clmt.soc_stored) > 30 &&
		clmt.soc_stored != -1) {
		printf("RECALC SOC DUETO BIGGAP!\n");
		clmt.reason |= CALC_SOC_DUETO_BIGGAP;
	}

	return clmt.reason;
}

static void atc2609a_clmt_init(void)
{
	atc2609a_clmt_enable(true);

	atc2609a_clmt_check_fcc();

	if (battery.health == BAT_NORMAL) {
		atc2609a_clmt_set_ocv_table (bat_info);
	} else {
		atc2609a_clmt_construct_fake_ocv_table();
		atc2609a_clmt_set_ocv_table (fake_bat_info);
	}

	atc2609a_bat_chk_reason();

}
/**
 * atc2609a_bat_check_health - check whether if bat is healthy or not.
 * if the vol diff is more than 300mv between bat detect before and after detecting, means bat is shorted;
 * if bat vol is less ocv0 before or after detecting,means bat is over discharged.
 */
static int atc2609a_bat_check_health(struct atc2609a_battery *battery)
{
	int batv_pre;
	int batv_post;

	batv_pre = battery->batv_pre;
	batv_post = battery->batv_post;

	if (abs(batv_post - batv_pre) > 300 ||
		(batv_post < bat_info[0].ocv &&
		batv_pre < bat_info[0].ocv)) {
		battery->health = BAT_ABNORMAL_EVER;
		printf("battery abnormal ever\n");
	} else {
		battery->health = BAT_NORMAL;
		printf("battery normal\n");
	}

	return battery->health;
}
/**
 * bat_calc_shutdn_consumed_soc - calc consumed soc during powerdown.
 */
static int atc2609a_bat_calc_passing_soc(void)
{
	unsigned int shutdn_time_sec;
	unsigned long cur_time_sec;
	long interval_sec;
	unsigned long clmb = 0;
	int consumed_soc;
	int fcc;

	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME,
		&shutdn_time_sec);
	if (!shutdn_time_sec)
		return 0;

	atc260x_rtc_tm_to_time(&cur_time_sec);
	interval_sec = cur_time_sec - shutdn_time_sec;
	printf("%s cur_time_sec(%ld),shutdn_time_sec(%d)\n",
		__func__,
		cur_time_sec, shutdn_time_sec);
	if (interval_sec < 0)
		return 0;

	/**
	 * clmt is full load during 1 hour after suspend, laterly, the comsume could be down.
	 * cosume unit:uAh
	 */
	if (interval_sec <= 3600) {
		/*uAh*/
		clmb = battery.items.shutdn_current_pre * interval_sec / 3600;
		debug("%s shutdn interval less than 1 hour(%lduAh)\n",
			__func__, clmb);
	} else {
		clmb = battery.items.shutdn_current_pre * 1;
		debug("%s shutdn interval more than 1 hour,precious 1h:%lduAh\n",
			__func__, clmb);
		clmb = clmb + battery.items.shutdown_current *
			(interval_sec - 3600) / 3600;
		debug("%s clmb=clmb_1h+shutdn_consume*(interval_sec-3600)/3600=\n" \
			 "clmb_1h+%d*(%ld-3600)/3600=%ld\n",
			 __func__,
			 battery.items.shutdown_current,
			 interval_sec, clmb);
	}

	fcc = atc2609a_clmt_get_fcc();
	/*clmb unit:uAh, fcc unit:mAh, due to we  use soc*1000, so not devided by 1000,here  */
	consumed_soc = clmb * 100 / fcc;
	consumed_soc = (consumed_soc + CONST_ROUNDING) / 1000;
	printf("%s consumed_soc:%d\n", __func__, consumed_soc);

	return consumed_soc;
}
/**
 * atc2609a_bat_calc_soc - calc battery soc during booting.
 */
int atc2609a_bat_calc_soc(void)
{
	/*recalc soc due to some reson*/
	if (clmt.reason) {
		if (!clmt.soc_cur || !clmt.soc_stored) {
			clmt.soc_cur = atc2609a_clmt_get_asoc_lookup();
			atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP,
				&clmt.soc_stored);
			printf("%s soc_real:%d,soc_stored:%d\n",
				__func__, clmt.soc_cur, clmt.soc_stored);
		}

		if (clmt.reason & CALC_SOC_DUETO_CLEAR ||
			clmt.reason & CALC_SOC_DUETO_NOTINIT) {
			atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP,
				clmt.soc_cur);
			printf("%s soc_cur:%d\n", __func__, clmt.soc_cur);
			return clmt.soc_cur;
		}

		if (clmt.reason & CALC_SOC_DUETO_LOWPWR ||
			clmt.reason & CALC_SOC_DUETO_RESET ||
			clmt.reason & CALC_SOC_DUETO_BIGGAP) {
			if (clmt.soc_cur < 15 ||
				(int)abs(clmt.soc_stored - clmt.soc_cur) < 15)
				clmt.soc_cur = min(clmt.soc_stored, clmt.soc_cur);

			atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP,
				clmt.soc_cur);
			printf("[%s] soc_cur:%d\n", __func__, clmt.soc_cur);
			return clmt.soc_cur;
		}

	}

	/*boot normal, soc_cur=soc_sotred-shutdn_consumed_soc*/
	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CAP,
		&clmt.soc_stored);
	clmt.soc_cur = clmt.soc_stored -
		atc2609a_bat_calc_passing_soc();
	if (clmt.soc_cur < 0)
		clmt.soc_cur = 0;
	atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CAP,
		clmt.soc_cur);
	printf("%s, soc_stored:%d, soc_cur:%d\n",
		__func__, clmt.soc_stored, clmt.soc_cur);

	return clmt.soc_cur;
}

static struct power_battery atc2609a_bat = {
	.measure_voltage = atc2609a_bat_measure_vol_avr,
	.chk_online = atc2609a_bat_check_online,
	.calc_soc = atc2609a_bat_calc_soc,
};

int atc2609a_bat_init(const void *blob)
{
	int ret;
	ret = atc2609a_bat_cfg_init(&battery, blob);
	if (ret)
		return ret;
	rsense = atc2609a_charger_get_rsense();
	atc2609a_bat_measure_vol_avr(&battery.batv_pre);
	atc260x_charger_release_guard();
	atc2609a_bat_measure_vol_avr(&battery.batv_post);
	atc2609a_bat_check_online();
	atc2609a_bat_set_uv(PMU_BAT_CTL0_BAT_UV_VOL_3100MV);
	atc2609a_bat_check_health(&battery);
	atc2609a_clmt_init();
	atc260x_bat_register(&atc2609a_bat);

	return 0;
}
