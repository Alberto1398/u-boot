/*
* Actions ATC2609A PMIC charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc2609a_charger.h>
#include <power/power_battery.h>

#define ATC2609A_PMU_BASE			(0x00)
#define ATC2609A_PMU_WALLVADC                   (ATC2609A_PMU_BASE + 0x5d)
#define ATC2609A_PMU_VBUSVADC			(ATC2609A_PMU_BASE + 0x5F)

#define ATC2609A_PMU_OT_CTL			(ATC2609A_PMU_BASE + 0x52)
#define OT_CTL_OT				(1 << 15)
#define OT_CTL_OT_SHUTOFF_EN			(1 << 11)

#define ATC2609A_PMU_SYS_PENDING		(ATC2609A_PMU_BASE + 0x10)
#define SYS_PENDING_BAT_OV			(1 << 15)

#define ATC2609A_PMU_SWCHG_CTL0			(ATC2609A_PMU_BASE + 0x16)
#define SWCHG_CTL0_SWCHG_EN			(1 << 15)
#define SWCHG_CTL0_TRICKLEEN			(1 << 14)
#define SWCHG_CTL0_ICHG_REG_CHGISTK_SHIFT	(13)
#define SWCHG_CTL0_ICHG_REG_CHGISTK_MASK	(0x1 << SWCHG_CTL0_ICHG_REG_CHGISTK_SHIFT)
#define SWCHG_CTL0_ICHG_REG_CHGISTK_100MA	(0 << SWCHG_CTL0_ICHG_REG_CHGISTK_SHIFT)
#define SWCHG_CTL0_ICHG_REG_CHGISTK_200MA	(0x1 << SWCHG_CTL0_ICHG_REG_CHGISTK_SHIFT)
#define PMU_SWCHG_CTL0_RSENSEL_SHIFT		(12)
#define PMU_SWCHG_CTL0_RSENSEL_MASK		(0x1 << PMU_SWCHG_CTL0_RSENSEL_SHIFT)
#define PMU_SWCHG_CTL0_RSENSEL_20mohm		(0x0 << PMU_SWCHG_CTL0_RSENSEL_SHIFT)
#define PMU_SWCHG_CTL0_RSENSEL_10mohm		(0x1 << PMU_SWCHG_CTL0_RSENSEL_SHIFT)
#define SWCHG_CTL0_ICHG_REG_CC_SHIFT		(8)
#define SWCHG_CTL0_ICHG_REG_CC_MASK		(0xf << SWCHG_CTL0_ICHG_REG_CC_SHIFT)
#define SWCHG_CTL0_EN_CHG_TIME			(1 << 7)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT	(5)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK	(0x3 << SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_30MIN	(0 << SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_40MIN	(1 << SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_50MIN	(2 << SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_TRIKLE_TIMER_60MIN	(3 << SWCHG_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT	(3)
#define SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK	(0x3 << SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_CCCV_4H		(0 << SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_CCCV_6H		(1 << SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_CCCV_8H		(2 << SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define SWCHG_CTL0_CHARGE_CCCV_12H		(3 << SWCHG_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define SWCHG_CTL0_CHG_FORCE_OFF		(1 << 2)
#define SWCHG_CTL0_CHGAUTO_DETECT_EN		(1 << 1)
#define SWCHG_CTL0_DTSEL_SHIFT			(0)
#define SWCHG_CTL0_DTSEL_MASK			(0x1 << SWCHG_CTL0_DTSEL_SHIFT)
#define SWCHG_CTL0_DTSEL_12MIN			(0 << SWCHG_CTL0_DTSEL_SHIFT)
#define SWCHG_CTL0_DTSEL_20S			(1 << SWCHG_CTL0_DTSEL_SHIFT)

#define ATC2609A_PMU_SWCHG_CTL1			(ATC2609A_PMU_BASE + 0x17)
#define SWCHG_CTL1_EN_BAT_DET			(1 << 15)
#define SWCHG_CTL1_CHG_EN_CUR_RISE		(1 << 13)
#define SWCHG_CTL1_CV_SET_L_SHIFT		(10)
#define	SWCHG_CTL1_CV_SET_L			(1 << SWCHG_CTL1_CV_SET_L_SHIFT)
#define SWCHG_CTL1_CV_SET_MASK			(SWCHG_CTL1_CV_SET_L | SWCHG_CTL1_CV_SET_H)
#define SWCHG_CTL1_CV_SET_4200MV		(0)
#define SWCHG_CTL1_CV_SET_4250MV		(SWCHG_CTL1_CV_SET_L)
#define SWCHG_CTL1_CV_SET_4350MV		(SWCHG_CTL1_CV_SET_H)
#define SWCHG_CTL1_CV_SET_4400MV		(SWCHG_CTL1_CV_SET_L | SWCHG_CTL1_CV_SET_H)
#define SWCHG_CTL1_STOPV_SHIFT			(9)
#define SWCHG_CTL1_STOPV_MASK			(0x1 << SWCHG_CTL1_STOPV_SHIFT)
#define SWCHG_CTL1_STOPV_4160MV			(0 << SWCHG_CTL1_STOPV_SHIFT)
#define SWCHG_CTL1_STOPV_4180MV			(1 << SWCHG_CTL1_STOPV_SHIFT)
#define SWCHG_CTL1_CV_SET_H_SHIFT		(8)
#define SWCHG_CTL1_CV_SET_H			(1 << SWCHG_CTL1_CV_SET_H_SHIFT)
#define SWCHG_CTL1_CHGPWR_SET_SHIFT		(6)
#define SWCHG_CTL1_CHGPWR_SET_MASK		(0x3 << SWCHG_CTL1_CHGPWR_SET_SHIFT)
#define SWCHG_CTL1_CHGPWR_SET_60MV		(0 << SWCHG_CTL1_CHGPWR_SET_SHIFT)
#define SWCHG_CTL1_CHGPWR_SET_160MV		(1 << SWCHG_CTL1_CHGPWR_SET_SHIFT)
#define SWCHG_CTL1_CHGPWR_SET_264MV		(2 << SWCHG_CTL1_CHGPWR_SET_SHIFT)
#define SWCHG_CTL1_CHGPWR_SET_373MV		(3 << SWCHG_CTL1_CHGPWR_SET_SHIFT)
#define SWCHG_CTL1_CHG_SYSPWR			(1 << 5)
#define SWCHG_CTL1_EN_CHG_TEMP			(1 << 4)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT	(2)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_MASK	(0x3 << SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_LOWER	(0 << SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_LOW	(1 << SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_HIGH	(2 << SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT)
#define SWCHG_CTL1_CHG_SYSSTEADY_SET_HIGHER	(3 << SWCHG_CTL1_CHG_SYSSTEADY_SET_SHIFT)

#define ATC2609A_PMU_SWCHG_CTL2			(ATC2609A_PMU_BASE + 0x18)
#define SWCHG_CTL2_TM_EN2			(1 << 12)
#define SWCHG_CTL2_TM_EN			(1 << 10)
#define SWCHG_CTL2_EN_OCP			(1 << 4)
#define SWCHG_CTL2_ILIMITED_SHIFT		(3)
#define SWCHG_CTL2_ILIMITED_MASK		(1 << SWCHG_CTL2_ILIMITED_SHIFT)
#define SWCHG_CTL2_ILINITED_2500MA		(0 << SWCHG_CTL2_ILIMITED_SHIFT)
#define SWCHG_CTL2_ILINITED_3470MA		(1 << SWCHG_CTL2_ILIMITED_SHIFT)

#define ATC2609A_PMU_SWCHG_CTL3			(ATC2609A_PMU_BASE + 0x19)
#define SWCHG_CTL3_CHARGER_MODE_SEL_SHIFT	(15)
#define SWCHG_CTL3_CHARGER_MODE_SEL_MASK	(1 << SWCHG_CTL3_CHARGER_MODE_SEL_SHIFT)
#define SWCHG_CTL3_CHARGER_MODE_SEL_LINER	(0 << SWCHG_CTL3_CHARGER_MODE_SEL_SHIFT)
#define SWCHG_CTL3_CHARGER_MODE_SEL_SWITCH	(1 << SWCHG_CTL3_CHARGER_MODE_SEL_SHIFT)

#define ATC2609A_PMU_SWCHG_CTL4			(ATC2609A_PMU_BASE + 0x1A)
#define SWCHG_CTL4_PHASE_SHIFT			(11)
#define SWCHG_CTL4_PHASE_MASK			(0x3 << SWCHG_CTL4_PHASE_SHIFT)
#define SWCHG_CTL4_PHASE_PRECHARGE		(1 << SWCHG_CTL4_PHASE_SHIFT)
#define SWCHG_CTL4_PHASE_CONSTANT_CURRENT	(2 << SWCHG_CTL4_PHASE_SHIFT)
#define SWCHG_CTL4_PHASE_CONSTANT_VOLTAGE	(3 << SWCHG_CTL4_PHASE_SHIFT)
#define SWCHG_CTL4_BAT_EXT			(1 << 8)
#define SWCHG_CTL4_BAT_DT_OVER			(1 << 7)

#define ATC2609A_PMU_APDS_CTL0			(ATC2609A_PMU_BASE + 0x11)
#define APDS_CTL0_VBUSCONTROL_EN		(1 << 15)
#define APDS_CTL0_VBUS_CONTROL_SEL		(1 << 14)
#define APDS_CTL0_VBUS_CUR_LIMITED_SHIFT	(12)
#define APDS_CTL0_VBUS_CUR_LIMITED_MASK		(0x3 << APDS_CTL0_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_CUR_LIMITED_100MA	(0x0 << APDS_CTL0_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_CUR_LIMITED_300MA	(0x1 << APDS_CTL0_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_CUR_LIMITED_500MA	(0x2 << APDS_CTL0_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_CUR_LIMITED_800MA	(0x3 << APDS_CTL0_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_VOL_LIMITED_SHIFT	(10)
#define APDS_CTL0_VBUS_VOL_LIMITED_MASK		(0x3 << APDS_CTL0_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_VOL_LIMITED_4200MV	(0x0 << APDS_CTL0_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_VOL_LIMITED_4300MV	(0x1 << APDS_CTL0_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_VOL_LIMITED_4400MV	(0x2 << APDS_CTL0_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL0_VBUS_VOL_LIMITED_4500MV	(0x3 << APDS_CTL0_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL0_VBUSOTG			(1 << 9)
#define APDS_CTL0_VBUS_PD			(1 << 2)
#define APDS_CTL0_WALL_PD			(1 << 1)

#define ATC2609A_PMU_CHARGER_CTL		(ATC2609A_PMU_BASE + 0x14)
#define	CHARGER_CTL_TEMPTH1_SHIFT		(13)
#define	CHARGER_CTL_TEMPTH1_MASK		(0x3 << CHARGER_CTL_TEMPTH1_SHIFT)
#define CHARGER_CTL_TEMPTH1_65			(0 << CHARGER_CTL_TEMPTH1_SHIFT)
#define CHARGER_CTL_TEMPTH1_75			(1 << CHARGER_CTL_TEMPTH1_SHIFT)
#define CHARGER_CTL_TEMPTH1_85			(2 << CHARGER_CTL_TEMPTH1_SHIFT)
#define CHARGER_CTL_TEMPTH1_95			(3 << CHARGER_CTL_TEMPTH1_SHIFT)
#define	CHARGER_CTL_TEMPTH2_SHIFT		(11)
#define	CHARGER_CTL_TEMPTH2_MASK		(0x3 << CHARGER_CTL_TEMPTH2_SHIFT)
#define CHARGER_CTL_TEMPTH2_75			(0 << CHARGER_CTL_TEMPTH2_SHIFT)
#define	CHARGER_CTL_TEMPTH2_85			(1 << CHARGER_CTL_TEMPTH2_SHIFT)
#define CHARGER_CTL_TEMPTH2_95			(2 << CHARGER_CTL_TEMPTH2_SHIFT)
#define CHARGER_CTL_TEMPTH2_105			(3 << CHARGER_CTL_TEMPTH2_SHIFT)
#define	CHARGER_CTL_TEMPTH3_SHIFT		(9)
#define	CHARGER_CTL_TEMPTH3_MASK		(0x3 << CHARGER_CTL_TEMPTH3_SHIFT)
#define CHARGER_CTL_TEMPTH3_85			(0 << CHARGER_CTL_TEMPTH3_SHIFT)
#define CHARGER_CTL_TEMPTH3_95			(1 << CHARGER_CTL_TEMPTH3_SHIFT)
#define CHARGER_CTL_TEMPTH3_105			(2 << CHARGER_CTL_TEMPTH3_SHIFT)
#define CHARGER_CTL_TEMPTH3_115			(2 << CHARGER_CTL_TEMPTH3_SHIFT)

#define ATC2609A_PMU_ADC12B_V			(ATC2609A_PMU_BASE + 0x57)
#define ADC12B_V_MASK				(0xfff)

#define ADC_LSB_FOR_BATV			(732)
#define SHARE_VOL_FACTOR			(2)
#define CONST_ROUNDING				(5 * 100)
#define CONST_FACTOR				(1000)

enum CHARGER_MODE {
	CHARGER_MODE_LINER,
	CHARGER_MODE_SWITCH
};

enum TRICKLE_CURRENT {
	TRICKLE_CURRENT_100MA = 100,
	TRICKLE_CURRENT_200MA = 200
};

enum TRICKLE_TIMER {
	TRICKLE_TIMER_30MIN,
	TRICKLE_TIMER_40MIN,
	TRICKLE_TIMER_50MIN,
	TRICKLE_TIMER_60MIN,
};

enum VBUS_CTL_MODE {
	CANCEL_LIMITED,
	CURRENT_LIMITED,
	VOLTAGE_LIMITED
};

enum CONSTANT_CURRENT {
	CONSTANT_CURRENT_100MA,
	CONSTANT_CURRENT_200MA,
	CONSTANT_CURRENT_400MA,
	CONSTANT_CURRENT_600MA,
	CONSTANT_CURRENT_800MA,
	CONSTANT_CURRENT_1000MA,
	CONSTANT_CURRENT_1200MA,
	CONSTANT_CURRENT_1400MA,
	CONSTANT_CURRENT_1600MA,
	CONSTANT_CURRENT_1800MA,
	CONSTANT_CURRENT_2000MA,
	CONSTANT_CURRENT_2200MA,
	CONSTANT_CURRENT_2400MA,
	CONSTANT_CURRENT_2600MA,
	CONSTANT_CURRENT_2800MA,
	CONSTANT_CURRENT_3000MA,
	CONSTANT_CURRENT_MAX,
};

enum CONSTANT_VOLTAGE {
	CONSTANT_VOL_4200MV,
	CONSTANT_VOL_4250MV,
	CONSTANT_VOL_4350MV,
	CONSTANT_VOL_4400MV
};

enum CC_TIMER {
	CC_TIMER_4H,
	CC_TIMER_6H,
	CC_TIMER_8H,
	CC_TIMER_12H,
};

enum VBUS_CURRENT_LMT {
	VBUS_CURR_LIMT_100MA,
	VBUS_CURR_LIMT_300MA,
	VBUS_CURR_LIMT_500MA,
	VBUS_CURR_LIMT_800MA
};

enum VBUS_VOLTAGE_LMT {
	VBUS_VOL_LIMT_4200MV,
	VBUS_VOL_LIMT_4300MV,
	VBUS_VOL_LIMT_4400MV,
	VBUS_VOL_LIMT_4500MV
};

enum STOP_VOLTAGE {
	STOP_VOLTAGE_4160MV,
	STOP_VOLTAGE_4180MV,
};

enum ILINITED {
	ILINITED_2500MA,
	ILINITED_3470MA
};

enum BATTERY_TYPE {
	BAT_TYPE_4180MV,
	BAT_TYPE_4200MV,
	BAT_TYPE_4300MV,
	BAT_TYPE_4350MV,
};

void atc2609a_charger_set_onoff(int enable)
{
	int val;

	if (enable)
		val = SWCHG_CTL0_SWCHG_EN;
	else
		val = 0;

	atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_SWCHG_EN, val);
	printf("%s charger_ctl0(0x%x)\n",
		__func__, atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL0));
}

#ifdef UBOOT_CHARGE
static int atc2609a_charger_get_onoff(void)
{
	int onoff;

	onoff = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL0) &
				SWCHG_CTL0_SWCHG_EN;
	if (onoff)
		return 1;
	else
		return 0;
}

static int atc2609a_charger_get_trick_current(void)
{
	int data;

	data =  atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL0) &
				~SWCHG_CTL0_ICHG_REG_CHGISTK_MASK;

	if (data)
		return 200;
	else
		return 100;
}
#endif
static void atc2609a_charger_set_trick_current(enum TRICKLE_CURRENT value)
{
	if (value == TRICKLE_CURRENT_100MA)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_ICHG_REG_CHGISTK_MASK,
				SWCHG_CTL0_ICHG_REG_CHGISTK_100MA);
	else if (value == TRICKLE_CURRENT_200MA)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_ICHG_REG_CHGISTK_MASK,
				SWCHG_CTL0_ICHG_REG_CHGISTK_200MA);
	else
		printf("%s tricle current value invalid!\n", __func__);
}


static void atc2609a_charger_set_trick_timer(enum TRICKLE_TIMER timer)
{
	switch (timer) {
	case TRICKLE_TIMER_30MIN:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_30MIN);
		break;
	case TRICKLE_TIMER_40MIN:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_40MIN);
		break;
	case TRICKLE_TIMER_50MIN:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_50MIN);
		break;
	case TRICKLE_TIMER_60MIN:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_60MIN);
		break;
	default:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				SWCHG_CTL0_CHARGE_TRIKLE_TIMER_30MIN);
		break;
	}
}


static void atc2609a_charger_enable_trick(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_TRICKLEEN,
				SWCHG_CTL0_TRICKLEEN);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
				SWCHG_CTL0_TRICKLEEN,
				~SWCHG_CTL0_TRICKLEEN);
}

static void atc2609a_charger_set_mode(int mode)
{
	if (mode == CHARGER_MODE_LINER)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL3,
				SWCHG_CTL3_CHARGER_MODE_SEL_MASK,
				SWCHG_CTL3_CHARGER_MODE_SEL_LINER);
	else if (mode == CHARGER_MODE_SWITCH)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL3,
				SWCHG_CTL3_CHARGER_MODE_SEL_MASK,
				SWCHG_CTL3_CHARGER_MODE_SEL_SWITCH);
	else
		printf("%s charger mode value invalid\n", __func__);
}

static enum CHARGER_MODE atc2609a_charger_get_mode(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL3) &
				SWCHG_CTL3_CHARGER_MODE_SEL_MASK;

	if (data)
		return CHARGER_MODE_SWITCH;
	else
		return CHARGER_MODE_LINER;
}

#ifdef UBOOT_CHARGE
static int  atc2609a_charger_get_cc(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL0)
				& SWCHG_CTL0_ICHG_REG_CC_MASK;
	data >>= SWCHG_CTL0_ICHG_REG_CC_SHIFT;

	switch (data) {
	case CONSTANT_CURRENT_100MA:
		return 100;
	case CONSTANT_CURRENT_200MA:
		return 200;
	case CONSTANT_CURRENT_400MA:
		return 400;
	case CONSTANT_CURRENT_600MA:
		return 600;
	case CONSTANT_CURRENT_800MA:
		return 800;
	case CONSTANT_CURRENT_1000MA:
		return 1000;
	case CONSTANT_CURRENT_1200MA:
		return 1200;
	case CONSTANT_CURRENT_1400MA:
		return 1400;
	case CONSTANT_CURRENT_1600MA:
		return 1600;
	case CONSTANT_CURRENT_1800MA:
		return 1800;
	case CONSTANT_CURRENT_2000MA:
		return 2000;
	case CONSTANT_CURRENT_2200MA:
		return 2200;
	case CONSTANT_CURRENT_2400MA:
		return 2400;
	case CONSTANT_CURRENT_2600MA:
		return 2600;
	case CONSTANT_CURRENT_2800MA:
		return 2800;
	case CONSTANT_CURRENT_3000MA:
		return 3000;
	default:
		break;
	}

	return  -1;
}

static int atc2609a_charger_cc_filter(int value)
{
	enum CHARGER_MODE mode;
	int data;

	mode = atc2609a_charger_get_mode();

	if (mode == CHARGER_MODE_LINER)
		data = value * 2;
	else
		data = value;

	if (data < 100)
		data =  100;
	else if ((data >= 100) && (data < 200))
		data = 100;
	else if ((data >= 200) && (data < 400))
		data = 200;
	else if ((data >= 400) && (data < 600))
		data = 400;
	else if ((data >= 600) && (data < 800))
		data = 600;
	else if ((data >= 800) && (data < 1000))
		data = 800;
	else if ((data >= 1000) && (data < 1200))
		data = 1000;
	else if ((data >= 1200) && (data < 1400))
		data = 1200;
	else if ((data >= 1400) && (data < 1600))
		data = 1400;
	else
		data = 1600;

	if (mode == CHARGER_MODE_LINER)
		data = data / 2;

	return data;
}
#endif

static enum CONSTANT_CURRENT
	atc2609a_charger_cc2regval(enum CHARGER_MODE mode , int data)
{
	int value = 0;

	if (mode == CHARGER_MODE_LINER)
		value = data * 2;
	else
		value = data;

	if (value < 100)
		return CONSTANT_CURRENT_100MA;
	else if ((value >= 100) && (value < 200))
		return CONSTANT_CURRENT_100MA;
	else if ((value >= 200) && (value < 400))
		return CONSTANT_CURRENT_200MA;
	else if ((value >= 400) && (value < 600))
		return CONSTANT_CURRENT_400MA;
	else if ((value >= 600) && (value < 800))
		return CONSTANT_CURRENT_600MA;
	else if ((value >= 800) && (value < 1000))
		return CONSTANT_CURRENT_800MA;
	else if ((value >= 1000) && (value < 1200))
		return CONSTANT_CURRENT_1000MA;
	else if ((value >= 1200) && (value < 1400))
		return CONSTANT_CURRENT_1200MA;
	else if ((value >= 1400) && (value < 1600))
		return CONSTANT_CURRENT_1400MA;
	else
		return CONSTANT_CURRENT_1600MA;
}


static void atc2609a_charger_set_cc(int cc)
{
	enum CONSTANT_CURRENT reg;
	enum CHARGER_MODE mode;

	mode = atc2609a_charger_get_mode();
	reg = atc2609a_charger_cc2regval(mode, cc);

	atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_ICHG_REG_CC_MASK,
			reg << SWCHG_CTL0_ICHG_REG_CC_SHIFT);
}

static void atc2609a_charger_set_cc_timer(enum CC_TIMER timer)
{
	switch (timer) {

	case CC_TIMER_4H:
		atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK,
			SWCHG_CTL0_CHARGE_CCCV_4H);

		break;

	case CC_TIMER_6H:
		atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK,
			SWCHG_CTL0_CHARGE_CCCV_6H);
		break;
	case CC_TIMER_8H:
		atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK,
			SWCHG_CTL0_CHARGE_CCCV_8H);
		break;
	case CC_TIMER_12H:
		atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK,
			SWCHG_CTL0_CHARGE_CCCV_12H);
		break;
	default:
		printf("%s CCCV TIMER value invalid\n", __func__);
		atc260x_reg_setbits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHARGE_CCCV_TIMER_MASK,
			SWCHG_CTL0_CHARGE_CCCV_12H);
		break;
	}
}

static void atc2609a_vbus_set_vol_lmt(enum VBUS_VOLTAGE_LMT value)
{
	switch (value) {
	case VBUS_VOL_LIMT_4200MV:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_VOL_LIMITED_MASK,
			APDS_CTL0_VBUS_VOL_LIMITED_4200MV);
		break;
	case VBUS_VOL_LIMT_4300MV:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_VOL_LIMITED_MASK,
			APDS_CTL0_VBUS_VOL_LIMITED_4300MV);
		break;
	case VBUS_VOL_LIMT_4400MV:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_VOL_LIMITED_MASK,
			APDS_CTL0_VBUS_VOL_LIMITED_4400MV);
		break;
	case VBUS_VOL_LIMT_4500MV:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_VOL_LIMITED_MASK,
			APDS_CTL0_VBUS_VOL_LIMITED_4500MV);
		break;
	default:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_VOL_LIMITED_MASK,
			APDS_CTL0_VBUS_VOL_LIMITED_4300MV);
		break;
	}
}

#ifdef UBOOT_CHARGE
static int atc2609a_vbus_get_vol_lmt(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_APDS_CTL0);
	data = data & APDS_CTL0_VBUS_VOL_LIMITED_MASK;

	switch (data) {
	case APDS_CTL0_VBUS_VOL_LIMITED_4200MV:
		return 4200;
	case APDS_CTL0_VBUS_VOL_LIMITED_4300MV:
		return 4300;
	case APDS_CTL0_VBUS_VOL_LIMITED_4400MV:
		return 4400;
	case APDS_CTL0_VBUS_VOL_LIMITED_4500MV:
		return 4500;
	default:
		printf("%s get vbus voltage limited value err(%x)\n",
			__func__, data);
		return -1;
	}
}
#endif

static void atc2609a_vbus_set_current_lmt(enum VBUS_CURRENT_LMT value)
{
	switch (value) {
	case VBUS_CURR_LIMT_100MA:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CUR_LIMITED_MASK,
			APDS_CTL0_VBUS_CUR_LIMITED_100MA);
		break;
	case VBUS_CURR_LIMT_300MA:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CUR_LIMITED_MASK,
			APDS_CTL0_VBUS_CUR_LIMITED_300MA);
		break;
	case VBUS_CURR_LIMT_500MA:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CUR_LIMITED_MASK,
			APDS_CTL0_VBUS_CUR_LIMITED_500MA);
		break;
	case VBUS_CURR_LIMT_800MA:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CUR_LIMITED_MASK,
			APDS_CTL0_VBUS_CUR_LIMITED_800MA);
		break;
	default:
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CUR_LIMITED_MASK,
			APDS_CTL0_VBUS_CUR_LIMITED_500MA);
		break;
	}
}

static void atc2609a_vbus_set_ctlmode(enum VBUS_CTL_MODE vbus_control_mode)
{
	if (vbus_control_mode == VOLTAGE_LIMITED)
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CONTROL_SEL, 0);
	else if (vbus_control_mode == CURRENT_LIMITED)
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_CONTROL_SEL,
			APDS_CTL0_VBUS_CONTROL_SEL);
	else
		printf("%s vbus ctl mode value invalid\n", __func__);
}

static void atc2609a_vbus_set_ctl_en(int enable)
{
	if (enable)
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUSCONTROL_EN,
			APDS_CTL0_VBUSCONTROL_EN);
	else
		atc260x_reg_setbits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUSCONTROL_EN, 0);
}

#ifdef UBOOT_CHARGE
static int atc2609a_vbus_get_ctl_en(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_APDS_CTL0);
	if (data & APDS_CTL0_VBUSCONTROL_EN)
		return 1;
	else
		return 0;
}

static int atc2609a_vbus_get_onoff(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_APDS_CTL0);
	data &= APDS_CTL0_VBUSOTG;
	if (data)
		return 0;
	else
		return 1;
}
#endif

static void atc2609a_vbus_set_onoff(int enable)
{
	if (enable)
		/*shut off the path from vbus to vbat,
		 when support usb adaptor only.*/
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUSOTG, 0);
	else
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUSOTG, APDS_CTL0_VBUSOTG);
}

static void atc2609a_wall_set_pd(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_WALL_PD, APDS_CTL0_WALL_PD);
	else
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_WALL_PD, ~APDS_CTL0_WALL_PD);
}

#ifdef UBOOT_CHARGE
static void atc2609a_vbus_set_pd(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_PD, APDS_CTL0_VBUS_PD);
	else
		atc260x_set_bits(ATC2609A_PMU_APDS_CTL0,
			APDS_CTL0_VBUS_PD, ~APDS_CTL0_VBUS_PD);
}

static void atc2609a_charger_syspwr_steady(int enable)
{
	atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
		SWCHG_CTL1_CHGPWR_SET_MASK |
		SWCHG_CTL1_CHG_SYSSTEADY_SET_MASK,
		SWCHG_CTL1_CHGPWR_SET_160MV |
		SWCHG_CTL1_CHG_SYSSTEADY_SET_HIGH);

	if (!enable)
		atc260x_reg_write(ATC2609A_PMU_SWCHG_CTL1,
			atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL1) &
			~SWCHG_CTL1_CHG_SYSPWR);
	else
		atc260x_reg_write(ATC2609A_PMU_SWCHG_CTL1,
			atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL1) |
			SWCHG_CTL1_CHG_SYSPWR);
}
#endif
static void atc2609a_charger_adjust_current(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_CHG_EN_CUR_RISE,
			SWCHG_CTL1_CHG_EN_CUR_RISE);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_CHG_EN_CUR_RISE, 0);
}

static void atc2609a_charger_set_cv(enum BATTERY_TYPE type)
{
	switch (type) {
	case BAT_TYPE_4180MV:
	case BAT_TYPE_4200MV:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_CV_SET_MASK,
			SWCHG_CTL1_CV_SET_4250MV);
		break;
	case BAT_TYPE_4300MV:
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_CV_SET_MASK,
			SWCHG_CTL1_CV_SET_4350MV);
		break;
	default:
		printf("%s bat type invalid\n", __func__);
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_CV_SET_MASK,
			SWCHG_CTL1_CV_SET_4250MV);
		break;
	}
}

static void atc2609a_charger_ot_shutoff(int ot_shutoff_enable)
{
	if (!ot_shutoff_enable)
		atc260x_set_bits(ATC2609A_PMU_OT_CTL,
			OT_CTL_OT_SHUTOFF_EN, 0);
	else
		atc260x_set_bits(ATC2609A_PMU_OT_CTL,
			OT_CTL_OT_SHUTOFF_EN,
			OT_CTL_OT_SHUTOFF_EN);
}

static void atc2609a_charger_current_temp(int  change_current_temp)
{
	if (change_current_temp)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_EN_CHG_TEMP,
			SWCHG_CTL1_EN_CHG_TEMP);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_EN_CHG_TEMP, 0);
}

static void atc2609a_charger_set_stop_vol(enum STOP_VOLTAGE stopv)
{
	if (stopv == STOP_VOLTAGE_4160MV)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_STOPV_MASK,
			SWCHG_CTL1_STOPV_4160MV);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
			SWCHG_CTL1_STOPV_MASK,
			SWCHG_CTL1_STOPV_4180MV);
}

static void atc2609a_charger_op_offset_threshold(void)
{
	atc260x_set_bits(ATC2609A_PMU_CHARGER_CTL,
		CHARGER_CTL_TEMPTH1_MASK |
		CHARGER_CTL_TEMPTH2_MASK |
		CHARGER_CTL_TEMPTH3_MASK,
		CHARGER_CTL_TEMPTH1_95 |
		CHARGER_CTL_TEMPTH2_105 |
		CHARGER_CTL_TEMPTH3_115);
}

static void atc2609a_charger_adjust_op_offset(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_TM_EN2 | SWCHG_CTL2_TM_EN,
			SWCHG_CTL2_TM_EN2 | SWCHG_CTL2_TM_EN);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_TM_EN2 | SWCHG_CTL2_TM_EN, 0);
}

static void atc2609a_charger_set_ocp(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_EN_OCP,
			SWCHG_CTL2_EN_OCP);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_EN_OCP, 0);
}

static void atc2609a_charger_pick_current(enum ILINITED value)
{
	if (value == ILINITED_2500MA)
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_ILIMITED_MASK,
			SWCHG_CTL2_ILINITED_2500MA);
	else
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL2,
			SWCHG_CTL2_ILIMITED_MASK,
			SWCHG_CTL2_ILINITED_3470MA);
}

static void atc2609a_charger_autodet_timer(int timer)
{
	atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
		SWCHG_CTL0_DTSEL_MASK | SWCHG_CTL0_EN_CHG_TIME,
		SWCHG_CTL0_EN_CHG_TIME | timer);
}

static void atc2609a_charger_auto_stop(int enable)
{
	if (!enable) {
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHGAUTO_DETECT_EN |
			SWCHG_CTL0_CHG_FORCE_OFF, 0);
	} else {
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
			SWCHG_CTL0_CHGAUTO_DETECT_EN |
			SWCHG_CTL0_CHG_FORCE_OFF,
			SWCHG_CTL0_CHGAUTO_DETECT_EN |
			SWCHG_CTL0_CHG_FORCE_OFF);
		atc2609a_charger_autodet_timer(SWCHG_CTL0_DTSEL_12MIN);
	}
}

int atc2609a_charger_set_rsense(int rsense)
{
	if (rsense == RSENSE_10mohm) {
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
			PMU_SWCHG_CTL0_RSENSEL_MASK,
			PMU_SWCHG_CTL0_RSENSEL_10mohm);
	} else if (rsense == RSENSE_20mohm) {
		atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL0,
			PMU_SWCHG_CTL0_RSENSEL_MASK,
			PMU_SWCHG_CTL0_RSENSEL_20mohm);
	} else {
		printf("[%s] rsense invalid!", __func__);
		return -1;
	}

	return 0;
}

int atc2609a_charger_get_rsense(void)
{
	int data;

	data = atc260x_reg_read(ATC2609A_PMU_SWCHG_CTL0);
	data &= PMU_SWCHG_CTL0_RSENSEL_MASK;
	if (data == PMU_SWCHG_CTL0_RSENSEL_10mohm)
		return RSENSE_10mohm;
	else if (data == PMU_SWCHG_CTL0_RSENSEL_20mohm)
		return RSENSE_20mohm;
	else
		return -1;
}
/*in order to release battery protect*/
int atc2609a_charger_release_guard(void)
{
	int data = 0;

	/* dectect bit 0 > 1 to start dectecting */
	data = atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
		SWCHG_CTL1_EN_BAT_DET, SWCHG_CTL1_EN_BAT_DET);
	if (data < 0)
		return data;
	mdelay(120);

	/* clear battery detect bit, otherwise cannot changer */
	data = atc260x_set_bits(ATC2609A_PMU_SWCHG_CTL1,
		SWCHG_CTL1_EN_BAT_DET, 0);
	if (data < 0)
		return data;

	return data;
}

void atc2609a_charger_init(void)
{
	/*charger init*/
	atc2609a_charger_enable_trick(1);
	atc2609a_charger_set_trick_current(TRICKLE_CURRENT_200MA);
	atc2609a_charger_set_cc(200);
	atc2609a_charger_set_trick_timer(TRICKLE_TIMER_30MIN);
	atc2609a_charger_set_cc_timer(CC_TIMER_12H);
	atc2609a_charger_auto_stop(0);

	atc2609a_charger_adjust_current(0);
	atc2609a_charger_set_cv(CONSTANT_VOL_4250MV);
	atc2609a_charger_set_stop_vol(STOP_VOLTAGE_4160MV);
	atc2609a_charger_current_temp(1);

	atc2609a_charger_set_ocp(1);
	atc2609a_charger_pick_current(ILINITED_3470MA);

	atc2609a_charger_set_mode(CHARGER_MODE_SWITCH);

	atc2609a_charger_ot_shutoff(1);

	atc2609a_charger_op_offset_threshold();

	atc2609a_charger_adjust_op_offset(1);
	/* wall init*/
	atc2609a_wall_set_pd(1);
	/*vbus init*/
	atc2609a_vbus_set_onoff(1);
	atc2609a_vbus_set_ctl_en(1);
	atc2609a_vbus_set_ctlmode(CURRENT_LIMITED);
	atc2609a_vbus_set_current_lmt(VBUS_CURR_LIMT_500MA);
	atc2609a_vbus_set_vol_lmt(VBUS_VOL_LIMT_4300MV);
}
