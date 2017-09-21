/*
* Actions ATC2603C PMIC charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*#define DEBUG*/
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/power_battery.h>
#include <power/power_charger.h>
#include <adfu.h>
#include "atc260x_charger.h"

#define ATC2603C_PMU_BASE			(0x00)

#define ATC2603C_PMU_OT_CTL			(ATC2603C_PMU_BASE + 0x38)
#define OT_CTL_OT_SHUTOFF_EN			(1 << 11)

#define ATC2603C_PMU_SYS_PENDING		(ATC2603C_PMU_BASE + 0x10)
#define SYS_PENDING_BAT_OV			(1 << 15)

#define ATC2603C_PMU_CHARGER_CTL0		(ATC2603C_PMU_BASE + 0x39)
#define CHARGER_CTL0_CHARGER_EN			(1 << 15)
#define CHARGER_CTL0_EN_CHG_TIME		(1 << 14)
#define CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT	(12)
#define CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK	(0x3 << CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_CCCV_4H		(0 << CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_CCCV_6H		(1 << CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_CCCV_8H		(2 << CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_CCCV_12H		(3 << CHARGER_CTL0_CHARGE_CCCV_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT	(10)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK	(0x3 << CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_30MIN	(0 << CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_40MIN	(1 << CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_50MIN	(2 << CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define CHARGER_CTL0_CHARGE_TRIKLE_TIMER_60MIN	(3 << CHARGER_CTL0_CHARGE_TRIKLE_TIMER_SHIFT)
#define CHARGER_CTL0_TRICKLEEN			(1 << 9)
#define CHARGER_CTL0_CHG_FORCE_OFF		(1 << 8)
#define CHARGER_CTL0_DTSEL_SHIFT		(7)
#define CHARGER_CTL0_DTSEL_MASK			(0x1 << CHARGER_CTL0_DTSEL_SHIFT)
#define CHARGER_CTL0_DTSEL_12MIN		(0 << CHARGER_CTL0_DTSEL_SHIFT)
#define CHARGER_CTL0_DTSEL_20S			(1 << CHARGER_CTL0_DTSEL_SHIFT)
#define CHARGER_CTL0_CHG_SYSPWR			(1 << 6)
#define CHARGER_CTL0_CHG_SYSPWR_SET_SHIFT	(4)
#define CHARGER_CTL0_CHG_SYSPWR_SET_MASK	(0x3 << CHARGER_CTL0_CHG_SYSPWR_SET_SHIFT)
#define CHARGER_CTL0_CHG_SYSPWR_SET_3810_MV	(0)
#define CHARGER_CTL0_CHG_SYSPWR_SET_3960_MV	(1 << CHARGER_CTL0_CHG_SYSPWR_SET_SHIFT)
#define CHARGER_CTL0_CHG_SYSPWR_SET_4250_MV	(2 << CHARGER_CTL0_CHG_SYSPWR_SET_SHIFT)
#define CHARGER_CTL0_CHG_SYSPWR_SET_4400_MV	(3 << CHARGER_CTL0_CHG_SYSPWR_SET_SHIFT)
#define CHARGER_CTL0_EN_CHG_TEMP		(1 << 3)
#define CHARGER_CTL0_CHGPWR_SET_SHIFT		(1)
#define CHARGER_CTL0_CHGPWR_SET_MASK		(0x3 << CHARGER_CTL0_CHGPWR_SET_SHIFT)
#define CHARGER_CTL0_CHGPWR_SET_10MV		(0 << CHARGER_CTL0_CHGPWR_SET_SHIFT)
#define CHARGER_CTL0_CHGPWR_SET_20MV		(1 << CHARGER_CTL0_CHGPWR_SET_SHIFT)
#define CHARGER_CTL0_CHGPWR_SET_30MV		(2 << CHARGER_CTL0_CHGPWR_SET_SHIFT)
#define CHARGER_CTL0_CHGPWR_SET_40MV		(3 << CHARGER_CTL0_CHGPWR_SET_SHIFT)
#define CHARGER_CTL0_CHGAUTO_DETECT_EN		(1 << 0)

#define ATC2603C_PMU_CHARGER_CTL1		(ATC2603C_PMU_BASE + 0x3a)
#define CHARGER_CTL1_CHG_END			(1 << 15)
#define CHARGER_CTL1_PHASE_SHIFT		(13)
#define CHARGER_CTL1_PHASE_MASK			(0x3 << CHARGER_CTL1_PHASE_SHIFT)
#define CHARGER_CTL1_PHASE_PRECHARGE		(1 << CHARGER_CTL1_PHASE_SHIFT)
#define CHARGER_CTL1_PHASE_CONSTANT_CURRENT	(2 << CHARGER_CTL1_PHASE_SHIFT)
#define CHARGER_CTL1_PHASE_CONSTANT_VOLTAGE	(3 << CHARGER_CTL1_PHASE_SHIFT)
#define CHARGER_CTL1_CHG_PWROK			(1 << 12)
#define CHARGER_CTL1_CUR_ZERO			(1 << 11)
#define CHARGER_CTL1_BAT_EXT			(1 << 10)
#define CHARGER_CTL1_BAT_DT_OVER		(1 << 9)
#define CHARGER_CTL1_CHARGER_TIMER_END		(1 << 8)
#define CHARGER_CTL1_STOPV_SHIFT		(7)
#define CHARGER_CTL1_STOPV_MASK			(0x1 << CHARGER_CTL1_STOPV_SHIFT)
#define CHARGER_CTL1_STOPV_4160MV		(0 << CHARGER_CTL1_STOPV_SHIFT)
#define CHARGER_CTL1_STOPV_4180MV		(1 << CHARGER_CTL1_STOPV_SHIFT)
#define CHARGER_CTL1_SOFT_START			(1 << 6)
#define CHARGER_CTL1_EN_BAT_DET			(1 << 5)
#define CHARGER_CTL1_CHARGER_AUTO_CLOSE_EN	(1 << 4)
#define CHARGER_CTL1_ICHG_REG_CC_MASK		(0x7)


#define ATC2603C_PMU_CHARGER_CTL2		(ATC2603C_PMU_BASE + 0x3B)
#define	CHARGER_CTL2_TEMPTH1_SHIFT		(13)
#define	CHARGER_CTL2_TEMPTH1_MASK		(0x3 << CHARGER_CTL2_TEMPTH1_SHIFT)
#define CHARGER_CTL2_TEMPTH1_75			(0)
#define CHARGER_CTL2_TEMPTH1_90			(1 << CHARGER_CTL2_TEMPTH1_SHIFT)
#define CHARGER_CTL2_TEMPTH1_105		(2 << CHARGER_CTL2_TEMPTH1_SHIFT)
#define CHARGER_CTL2_TEMPTH1_115		(3 << CHARGER_CTL2_TEMPTH1_SHIFT)
#define	CHARGER_CTL2_TEMPTH2_SHIFT		(11)
#define	CHARGER_CTL2_TEMPTH2_MASK		(0x3 << CHARGER_CTL2_TEMPTH2_SHIFT)
#define CHARGER_CTL2_TEMPTH2_90			(0)
#define	CHARGER_CTL2_TEMPTH2_105		(1 << CHARGER_CTL2_TEMPTH2_SHIFT)
#define CHARGER_CTL2_TEMPTH2_120		(2 << CHARGER_CTL2_TEMPTH2_SHIFT)
#define CHARGER_CTL2_TEMPTH2_135		(3 << CHARGER_CTL2_TEMPTH2_SHIFT)
#define	CHARGER_CTL2_TEMPTH3_SHIFT		(9)
#define	CHARGER_CTL2_TEMPTH3_MASK		(0x3 << CHARGER_CTL2_TEMPTH3_SHIFT)
#define CHARGER_CTL2_TEMPTH3_100		(0)
#define CHARGER_CTL2_TEMPTH3_120		(1 << CHARGER_CTL2_TEMPTH3_SHIFT)
#define CHARGER_CTL2_TEMPTH3_130		(2 << CHARGER_CTL2_TEMPTH3_SHIFT)
#define CHARGER_CTL2_TEMPTH3_140		(3 << CHARGER_CTL2_TEMPTH3_SHIFT)
#define	CHARGER_CTL2_TIMESTEP_SHIFT		(7)
#define	CHARGER_CTL2_TIMESTEP_MASK		(0x3 << CHARGER_CTL2_TIMESTEP_SHIFT)
#define CHARGER_CTL2_TIMESTEP_0_5S		(0)
#define CHARGER_CTL2_TIMESTEP_1_S		(1 << CHARGER_CTL2_TIMESTEP_SHIFT)
#define CHARGER_CTL2_TIMESTEP_2_S		(2 << CHARGER_CTL2_TIMESTEP_SHIFT)
#define CHARGER_CTL2_TIMESTEP_4_S		(3 << CHARGER_CTL2_TIMESTEP_SHIFT)
#define	CHARGER_CTL2_ICHG_REG_T_SHIFT		(4)
#define	CHARGER_CTL2_ICHG_REG_T_MASK		(0x3 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_ICHG_REG_T_50MA		(0)
#define CHARGER_CTL2_ICHG_REG_T_100MA		(1 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_ICHG_REG_T_200MA		(2 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_ICHG_REG_T_300MA		(3 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define	CHARGER_CTL2_CV_SET_SHIFT		(1)
#define	CHARGER_CTL2_CV_SET_MASK		(0x7 << CHARGER_CTL2_CV_SET_SHIFT)
#define CHARGER_CTL2_CV_SET_4200MV		(0)
#define CHARGER_CTL2_CV_SET_4250MV		(1 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_CV_SET_4300MV		(2 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_CV_SET_4350MV		(3 << CHARGER_CTL2_ICHG_REG_T_SHIFT)
#define CHARGER_CTL2_CV_SET_4400MV		(4 << CHARGER_CTL2_ICHG_REG_T_SHIFT)

#define ATC2603C_PMU_APDS_CTL			(ATC2603C_PMU_BASE + 0x3D)
#define APDS_CTL_VBUSCONTROL_EN			(1 << 15)
#define APDS_CTL_VBUS_CONTROL_SEL		(1 << 14)
#define APDS_CTL_VBUS_CUR_LIMITED_SHIFT		(12)
#define APDS_CTL_VBUS_CUR_LIMITED_MASK		(0x3 << APDS_CTL_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL_VBUS_CUR_LIMITED_100MA		(0x0 << APDS_CTL_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL_VBUS_CUR_LIMITED_300MA		(0x1 << APDS_CTL_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL_VBUS_CUR_LIMITED_500MA		(0x2 << APDS_CTL_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL_VBUS_CUR_LIMITED_800MA		(0x3 << APDS_CTL_VBUS_CUR_LIMITED_SHIFT)
#define APDS_CTL_VBUS_VOL_LIMITED_SHIFT		(10)
#define APDS_CTL_VBUS_VOL_LIMITED_MASK		(0x3 << APDS_CTL_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL_VBUS_VOL_LIMITED_4200MV	(0x0 << APDS_CTL_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL_VBUS_VOL_LIMITED_4300MV	(0x1 << APDS_CTL_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL_VBUS_VOL_LIMITED_4400MV	(0x2 << APDS_CTL_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL_VBUS_VOL_LIMITED_4500MV	(0x3 << APDS_CTL_VBUS_VOL_LIMITED_SHIFT)
#define APDS_CTL_VBUSOTG			(1 << 9)
#define APDS_CTL_VBUS_PD			(1 << 2)
#define APDS_CTL_WALL_PD			(1 << 1)

#define ATC2603C_PMU_AUXADC_CTL1		(ATC2603C_PMU_BASE + 0x3F)
#define PMU_AUXADC_CTL1_RSENSEL_SHIFT		(4)
#define PMU_AUXADC_CTL1_RSENSEL_MASK		(0x1 << PMU_AUXADC_CTL1_RSENSEL_SHIFT)
#define PMU_AUXADC_CTL1_RSENSEL_20mohm		(0x0 << PMU_AUXADC_CTL1_RSENSEL_SHIFT)
#define PMU_AUXADC_CTL1_RSENSEL_10mohm		(0x1 << PMU_AUXADC_CTL1_RSENSEL_SHIFT)

enum TRICKLE_CURRENT {
	TRICKLE_CURRENT_50MA = 50,
	TRICKLE_CURRENT_100MA = 100,
	TRICKLE_CURRENT_200MA = 200,
	TRICKLE_CURRENT_300MA = 300,

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
	CONSTANT_CURRENT_50MA,
	CONSTANT_CURRENT_100MA,
	CONSTANT_CURRENT_200MA,
	CONSTANT_CURRENT_400MA,
	CONSTANT_CURRENT_500MA,
	CONSTANT_CURRENT_600MA,
	CONSTANT_CURRENT_800MA,
	CONSTANT_CURRENT_900MA,
	CONSTANT_CURRENT_1000MA,
	CONSTANT_CURRENT_1200MA,
	CONSTANT_CURRENT_1300MA,
	CONSTANT_CURRENT_1400MA,
	CONSTANT_CURRENT_1600MA,
	CONSTANT_CURRENT_1700MA,
	CONSTANT_CURRENT_1800MA,
	CONSTANT_CURRENT_2000MA,
	CONSTANT_CURRENT_MAX,
};

enum CONSTANT_VOLTAGE {
	CONSTANT_VOL_4200MV,
	CONSTANT_VOL_4250MV,
	CONSTANT_VOL_4300MV,
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

enum BATTERY_TYPE {
	BAT_TYPE_4180MV,
	BAT_TYPE_4200MV,
	BAT_TYPE_4300MV,
	BAT_TYPE_4350MV,
};
struct atc260x_charger_info atc2603c_charger_info;
static void atc2603c_charger_set_onoff(int enable)
{
	int val;

	if (enable)
		val = CHARGER_CTL0_CHARGER_EN;
	else
		val = 0;

	atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGER_EN, val);
	printf("%s charger_ctl0(0x%x)\n",
		__func__, atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL0));
}

#ifdef UBOOT_CHARGE
static int atc2603c_charger_get_onoff(void)
{
	int onoff;

	onoff = atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL0) &
				CHARGER_CTL0_CHARGER_EN;
	if (onoff)
		return 1;
	else
		return 0;
}

static int atc2603c_charger_get_trick_current(void)
{
	int data;

	data =  atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL2) &
				~CHARGER_CTL2_ICHG_REG_T_MASK;

	if (data == CHARGER_CTL2_ICHG_REG_T_50MA)
		return 50;
	else if (data == CHARGER_CTL2_ICHG_REG_T_100MA)
		return 100;
	else if (data == CHARGER_CTL2_ICHG_REG_T_200MA)
		return 200;
	else
		return 300;
}
#endif
static void atc2603c_charger_set_trick_current(enum TRICKLE_CURRENT value)
{
	if (value == TRICKLE_CURRENT_50MA)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
				CHARGER_CTL2_ICHG_REG_T_MASK,
				CHARGER_CTL2_ICHG_REG_T_50MA);
	else if (value == TRICKLE_CURRENT_100MA)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
				CHARGER_CTL2_ICHG_REG_T_MASK,
				CHARGER_CTL2_ICHG_REG_T_100MA);
	else if (value == TRICKLE_CURRENT_200MA)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
				CHARGER_CTL2_ICHG_REG_T_MASK,
				CHARGER_CTL2_ICHG_REG_T_200MA);
	else if (value == TRICKLE_CURRENT_300MA)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
				CHARGER_CTL2_ICHG_REG_T_MASK,
				CHARGER_CTL2_ICHG_REG_T_300MA);
	else
		printf("%s tricle current value invalid!\n", __func__);
}


static void atc2603c_charger_set_trick_timer(enum TRICKLE_TIMER timer)
{
	switch (timer) {
	case TRICKLE_TIMER_30MIN:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_30MIN);
		break;
	case TRICKLE_TIMER_40MIN:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_40MIN);
		break;
	case TRICKLE_TIMER_50MIN:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_50MIN);
		break;
	case TRICKLE_TIMER_60MIN:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_60MIN);
		break;
	default:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_MASK,
				CHARGER_CTL0_CHARGE_TRIKLE_TIMER_30MIN);
		break;
	}
}


static void atc2603c_charger_enable_trick(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_TRICKLEEN,
				CHARGER_CTL0_TRICKLEEN);
	else
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
				CHARGER_CTL0_TRICKLEEN,
				~CHARGER_CTL0_TRICKLEEN);
}

#ifdef UBOOT_CHARGE
static int  atc2603c_charger_get_cc(void)
{
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL1)
				& CHARGER_CTL1_ICHG_REG_CC_MASK;
	switch (data) {
	case CONSTANT_CURRENT_50MA:
		return 50;
	case CONSTANT_CURRENT_100MA:
		return 100;
	case CONSTANT_CURRENT_200MA:
		return 200;
	case CONSTANT_CURRENT_400MA:
		return 400;
	case CONSTANT_CURRENT_500MA:
		return 500;
	case CONSTANT_CURRENT_600MA:
		return 600;
	case CONSTANT_CURRENT_800MA:
		return 800;
	case CONSTANT_CURRENT_900MA:
		return 900;
	case CONSTANT_CURRENT_1000MA:
		return 1000;
	case CONSTANT_CURRENT_1200MA:
		return 1200;
	case CONSTANT_CURRENT_1300MA:
		return 1300;
	case CONSTANT_CURRENT_1400MA:
		return 1400;
	case CONSTANT_CURRENT_1600MA:
		return 1600;
	case CONSTANT_CURRENT_1700MA:
		return 1700;
	case CONSTANT_CURRENT_1800MA:
		return 1800;
	case CONSTANT_CURRENT_2000MA:
		return 2000;
	default:
		break;
	}

	return  -1;
}

static int atc2603c_charger_cc_filter(int value)
{
	int data;

	if (value < 50)
		data = 50;
	if (value >= 50) && (value < 100)
		data =  50;
	else if ((value >= 100) && (value < 200))
		data = 100;
	else if ((value >= 200) && (value < 400))
		data = 200;
	else if ((value >= 400) && (value < 500))
		data = 400;
	else if ((value >= 500) && (value < 600))
		data = 500;
	else if ((value >= 600) && (value < 800))
		data = 600;
	else if ((value >= 800) && (value < 900))
		data = 800;
	else if ((value >= 900) && (value < 1000))
		data = 900;
	else if ((value >= 1000) && (value < 1200))
		data = 1000;
	else if ((value >= 1200) && (value < 1300))
		data = 1200;
	else if ((value >= 1300) && (value < 1400))
		data = 1300;
	else if ((value >= 1400) && (value < 1600))
		data = 1400;
	else if ((value >= 1600) && (value < 1700))
		data = 1600;
	else if ((value>= 1700) && (value < 1800))
		data = 1700;
	else if ((value >= 1800) && (value < 2000))
		data = 1800;
	else
		data = 2000;

	return data;
}
#endif

static enum CONSTANT_CURRENT
	atc2603c_charger_cc2regval(int value)
{

	if (value < 50)
		return CONSTANT_CURRENT_50MA;
	else if ((value >= 50) && (value < 100))
		return CONSTANT_CURRENT_50MA;
	else if ((value >= 100) && (value < 200))
		return CONSTANT_CURRENT_100MA;
	else if ((value >= 200) && (value < 400))
		return CONSTANT_CURRENT_200MA;
	else if ((value >= 400) && (value < 500))
		return CONSTANT_CURRENT_400MA;
	else if ((value >= 500) && (value < 600))
		return CONSTANT_CURRENT_500MA;
	else if ((value >= 600) && (value < 800))
		return CONSTANT_CURRENT_600MA;
	else if ((value >= 800) && (value < 900))
		return CONSTANT_CURRENT_800MA;
	else if ((value >= 900) && (value < 1000))
		return CONSTANT_CURRENT_900MA;
	else if ((value >= 1000) && (value < 1200))
		return CONSTANT_CURRENT_1000MA;
	else if ((value >= 1200) && (value < 1300))
		return CONSTANT_CURRENT_1200MA;
	else if ((value >= 1300) && (value < 1400))
		return CONSTANT_CURRENT_1300MA;
	else if ((value >= 1400) && (value < 1600))
		return CONSTANT_CURRENT_1400MA;
	else if ((value >= 1600) && (value < 1700))
		return CONSTANT_CURRENT_1600MA;
	else if ((value >= 1700) && (value < 1800))
		return CONSTANT_CURRENT_1700MA;
	else if ((value >= 1800) && (value < 2000))
		return CONSTANT_CURRENT_1800MA;
	else
		return CONSTANT_CURRENT_2000MA;
}


static void atc2603c_charger_set_cc(int cc)
{
	enum CONSTANT_CURRENT reg;

	reg = atc2603c_charger_cc2regval(cc);

	atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL1,
			CHARGER_CTL1_ICHG_REG_CC_MASK,
			reg);
}

static void atc2603c_charger_set_cc_timer(enum CC_TIMER timer)
{
	switch (timer) {

	case CC_TIMER_4H:
		atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK,
			CHARGER_CTL0_CHARGE_CCCV_4H);

		break;

	case CC_TIMER_6H:
		atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK,
			CHARGER_CTL0_CHARGE_CCCV_6H);
		break;
	case CC_TIMER_8H:
		atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK,
			CHARGER_CTL0_CHARGE_CCCV_8H);
		break;
	case CC_TIMER_12H:
		atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK,
			CHARGER_CTL0_CHARGE_CCCV_12H);
		break;
	default:
		printf("%s CCCV TIMER value invalid\n", __func__);
		atc260x_reg_setbits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHARGE_CCCV_TIMER_MASK,
			CHARGER_CTL0_CHARGE_CCCV_12H);
		break;
	}
}

static void atc2603c_vbus_set_vol_lmt(enum VBUS_VOLTAGE_LMT value)
{
	switch (value) {
	case VBUS_VOL_LIMT_4200MV:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_VOL_LIMITED_MASK,
			APDS_CTL_VBUS_VOL_LIMITED_4200MV);
		break;
	case VBUS_VOL_LIMT_4300MV:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_VOL_LIMITED_MASK,
			APDS_CTL_VBUS_VOL_LIMITED_4300MV);
		break;
	case VBUS_VOL_LIMT_4400MV:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_VOL_LIMITED_MASK,
			APDS_CTL_VBUS_VOL_LIMITED_4400MV);
		break;
	case VBUS_VOL_LIMT_4500MV:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_VOL_LIMITED_MASK,
			APDS_CTL_VBUS_VOL_LIMITED_4500MV);
		break;
	default:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_VOL_LIMITED_MASK,
			APDS_CTL_VBUS_VOL_LIMITED_4300MV);
		break;
	}
}

#ifdef UBOOT_CHARGE
static int atc2603c_vbus_get_vol_lmt(void)
{
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_APDS_CTL);
	data = data & APDS_CTL_VBUS_VOL_LIMITED_MASK;

	switch (data) {
	case APDS_CTL_VBUS_VOL_LIMITED_4200MV:
		return 4200;
	case APDS_CTL_VBUS_VOL_LIMITED_4300MV:
		return 4300;
	case APDS_CTL_VBUS_VOL_LIMITED_4400MV:
		return 4400;
	case APDS_CTL_VBUS_VOL_LIMITED_4500MV:
		return 4500;
	default:
		printf("%s get vbus voltage limited value err(%x)\n",
			__func__, data);
		return -1;
	}
}
#endif

static void atc2603c_vbus_set_current_lmt(enum VBUS_CURRENT_LMT value)
{
	switch (value) {
	case VBUS_CURR_LIMT_100MA:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CUR_LIMITED_MASK,
			APDS_CTL_VBUS_CUR_LIMITED_100MA);
		break;
	case VBUS_CURR_LIMT_300MA:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CUR_LIMITED_MASK,
			APDS_CTL_VBUS_CUR_LIMITED_300MA);
		break;
	case VBUS_CURR_LIMT_500MA:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CUR_LIMITED_MASK,
			APDS_CTL_VBUS_CUR_LIMITED_500MA);
		break;
	case VBUS_CURR_LIMT_800MA:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CUR_LIMITED_MASK,
			APDS_CTL_VBUS_CUR_LIMITED_800MA);
		break;
	default:
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CUR_LIMITED_MASK,
			APDS_CTL_VBUS_CUR_LIMITED_500MA);
		break;
	}
}

static void atc2603c_vbus_set_ctlmode(enum VBUS_CTL_MODE vbus_control_mode)
{
	if (vbus_control_mode == VOLTAGE_LIMITED)
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CONTROL_SEL, 0);
	else if (vbus_control_mode == CURRENT_LIMITED)
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_CONTROL_SEL,
			APDS_CTL_VBUS_CONTROL_SEL);
	else
		printf("%s vbus ctl mode value invalid\n", __func__);
}

static void atc2603c_vbus_set_ctl_en(int enable)
{
	if (enable)
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUSCONTROL_EN,
			APDS_CTL_VBUSCONTROL_EN);
	else
		atc260x_reg_setbits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUSCONTROL_EN, 0);
}

#ifdef UBOOT_CHARGE
static int atc2603c_vbus_get_ctl_en(void)
{
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_APDS_CTL);
	if (data & APDS_CTL_VBUSCONTROL_EN)
		return 1;
	else
		return 0;
}

static int atc2603c_vbus_get_onoff(void)
{
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_APDS_CTL);
	data &= APDS_CTL_VBUSOTG;
	if (data)
		return 0;
	else
		return 1;
}
#endif

static void atc2603c_vbus_set_onoff(int enable)
{
	if (enable)
		/*shut off the path from vbus to vbat,
		 when support usb adaptor only.*/
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUSOTG, 0);
	else
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUSOTG, APDS_CTL_VBUSOTG);
}

static void atc2603c_wall_set_pd(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_WALL_PD, APDS_CTL_WALL_PD);
	else
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_WALL_PD, ~APDS_CTL_WALL_PD);
}

#ifdef UBOOT_CHARGE
static void atc2603c_vbus_set_pd(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_PD, APDS_CTL_VBUS_PD);
	else
		atc260x_set_bits(ATC2603C_PMU_APDS_CTL,
			APDS_CTL_VBUS_PD, ~APDS_CTL_VBUS_PD);
}

static void atc2603c_charger_syspwr_steady(int enable)
{
	atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
		CHARGER_CTL0_CHGPWR_SET_MASK |
		CHARGER_CTL0_CHG_SYSPWR_SET_MASK,
		CHARGER_CTL0_CHGPWR_SET_20MV |
		CHARGER_CTL0_CHG_SYSPWR_SET_3960_MV);

	if (!enable)
		atc260x_reg_write(ATC2603C_PMU_CHARGER_CTL0,
			atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL0) &
			~CHARGER_CTL0_CHG_SYSPWR);
	else
		atc260x_reg_write(ATC2603C_PMU_CHARGER_CTL0,
			atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL0) |
			CHARGER_CTL0_CHG_SYSPWR);
}
#endif
static void atc2603c_charger_adjust_current(int enable)
{
	if (enable)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
			CHARGER_CTL1_SOFT_START,
			CHARGER_CTL1_SOFT_START);
	else
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
			CHARGER_CTL1_SOFT_START, 0);
}

static void atc2603c_charger_set_cv(enum BATTERY_TYPE type)
{
	switch (type) {
	case BAT_TYPE_4180MV:
	case BAT_TYPE_4200MV:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
			CHARGER_CTL2_CV_SET_MASK,
			CHARGER_CTL2_CV_SET_4250MV);
		break;
	case BAT_TYPE_4300MV:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
			CHARGER_CTL2_CV_SET_MASK,
			CHARGER_CTL2_CV_SET_4350MV);
		break;
	case BAT_TYPE_4350MV:
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
			CHARGER_CTL2_CV_SET_MASK,
			CHARGER_CTL2_CV_SET_4400MV);
		break;
	default:
		printf("%s bat type invalid\n", __func__);
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
			CHARGER_CTL2_CV_SET_MASK,
			CHARGER_CTL2_CV_SET_4250MV);
		break;
	}
}

static void atc2603c_charger_ot_shutoff(int ot_shutoff_enable)
{
	if (!ot_shutoff_enable)
		atc260x_set_bits(ATC2603C_PMU_OT_CTL,
			OT_CTL_OT_SHUTOFF_EN, 0);
	else
		atc260x_set_bits(ATC2603C_PMU_OT_CTL,
			OT_CTL_OT_SHUTOFF_EN,
			OT_CTL_OT_SHUTOFF_EN);
}

static void atc2603c_charger_current_temp(int  change_current_temp)
{
	if (change_current_temp)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_EN_CHG_TEMP,
			CHARGER_CTL0_EN_CHG_TEMP);
	else
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_EN_CHG_TEMP, 0);
}

static void atc2603c_charger_set_stop_vol(enum STOP_VOLTAGE stopv)
{
	if (stopv == STOP_VOLTAGE_4160MV)
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
			CHARGER_CTL1_STOPV_MASK,
			CHARGER_CTL1_STOPV_4160MV);
	else
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
			CHARGER_CTL1_STOPV_MASK,
			CHARGER_CTL1_STOPV_4180MV);
}

static void atc2603c_charger_op_offset_threshold(void)
{
	atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL2,
		CHARGER_CTL2_TEMPTH1_MASK |
		CHARGER_CTL2_TEMPTH2_MASK |
		CHARGER_CTL2_TEMPTH3_MASK,
		CHARGER_CTL2_TEMPTH1_90 |
		CHARGER_CTL2_TEMPTH2_105 |
		CHARGER_CTL2_TEMPTH3_120);
}

static void atc2603c_charger_autodet_timer(int timer)
{
	atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
		CHARGER_CTL0_DTSEL_MASK | CHARGER_CTL0_EN_CHG_TIME,
		CHARGER_CTL0_EN_CHG_TIME | timer);
}

static void atc2603c_charger_auto_stop(int enable)
{
	if (!enable) {
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHGAUTO_DETECT_EN |
			CHARGER_CTL0_CHG_FORCE_OFF, 0);
	} else {
		atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL0,
			CHARGER_CTL0_CHGAUTO_DETECT_EN |
			CHARGER_CTL0_CHG_FORCE_OFF,
			CHARGER_CTL0_CHGAUTO_DETECT_EN |
			CHARGER_CTL0_CHG_FORCE_OFF);
		atc2603c_charger_autodet_timer(CHARGER_CTL0_DTSEL_12MIN);
	}
}

static int atc2603c_charger_set_rsense(int rsense)
{
	if (rsense == RSENSE_10mohm) {
		atc260x_set_bits(ATC2603C_PMU_AUXADC_CTL1,
			PMU_AUXADC_CTL1_RSENSEL_MASK,
			PMU_AUXADC_CTL1_RSENSEL_10mohm);
	} else if (rsense == RSENSE_20mohm) {
		atc260x_set_bits(ATC2603C_PMU_AUXADC_CTL1,
			PMU_AUXADC_CTL1_RSENSEL_MASK,
			PMU_AUXADC_CTL1_RSENSEL_20mohm);
	} else {
		printf("[%s] rsense invalid!", __func__);
		return -1;
	}

	return 0;
}

int atc2603c_charger_get_rsense(void)
{
	int data;

	data = atc260x_reg_read(ATC2603C_PMU_AUXADC_CTL1);
	data &= PMU_AUXADC_CTL1_RSENSEL_MASK;
	if (data == PMU_AUXADC_CTL1_RSENSEL_10mohm)
		return RSENSE_10mohm;
	else if (data == PMU_AUXADC_CTL1_RSENSEL_20mohm)
		return RSENSE_20mohm;
	else
		return -1;
}
/*in order to release battery protect*/
static int atc2603c_charger_release_guard(void)
{
	int data = 0;

	/* dectect bit 0 > 1 to start dectecting */
	data = atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
		CHARGER_CTL1_EN_BAT_DET, CHARGER_CTL1_EN_BAT_DET);
	if (data < 0)
		return data;
	mdelay(120);

	/* clear battery detect bit, otherwise cannot changer */
	data = atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
		CHARGER_CTL1_EN_BAT_DET, 0);
	if (data < 0)
		return data;

	return data;
}

int atc2603c_chk_bat_online_intermeddle(void)
{
	int data;
	int count = 0;

	if (atc2603c_charger_info.bat_detected)
		return atc2603c_charger_info.bat_online;

	atc2603c_charger_info.bat_detected = 1;

	/* dectect bit 0 > 1 to start dectecting */
	data = atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
		CHARGER_CTL1_EN_BAT_DET, CHARGER_CTL1_EN_BAT_DET);
	if (data < 0)
		return data;

	/* wait bat detect over */
	do {
		mdelay(70);
		data = atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL1) &
			CHARGER_CTL1_BAT_DT_OVER;
		debug("%s wait battery detect over,data:0x%x\n",
			__func__, data);
		count += 70;
		if (count >= 300)
			break;
	} while (!data);

	data = atc260x_reg_read(ATC2603C_PMU_CHARGER_CTL1);
	if (data < 0)
		return data;

	if (data & CHARGER_CTL1_BAT_EXT)
		atc2603c_charger_info.bat_online = 1;
	else
		atc2603c_charger_info.bat_online = 0;

	/* clear battery detect bit, otherwise cannot changer */
	data = atc260x_set_bits(ATC2603C_PMU_CHARGER_CTL1,
		CHARGER_CTL1_EN_BAT_DET, 0);
	if (data < 0)
		return data;

	debug("%s battery online:%d\n", __func__, atc2603c_charger_info.bat_online);

	return atc2603c_charger_info.bat_online;
}

static int atc2603c_charger_check_wall_online(void)
{
	int wallv;
	int ret;
	int i;

	for (i = 0; i < 5; i++) {
		ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_WALLV, &wallv);
		if (ret) {
			printf("[%s] get wallv auxadc err!\n", __func__);
			wallv = 0;
			continue;
		}

		if (wallv > ATC260X_WALL_VOL_THRESHOLD) {
			debug("WALL ONLINE\n");
			return ADAPTER_TYPE_WALL_PLUGIN;
		}
		mdelay(2);
		debug("%s the %d time\n", __func__, i);
	}

	debug("WALL OFFLINE(%dmv)\n", wallv);

	return ADAPTER_TYPE_NO_PLUGIN;
}

static int atc2603c_charger_check_usb_online(void)
{
	struct atc260x_charger_data *data = &atc2603c_charger_info.data;
	int vbusv;
	int ret;
	int i;

	if (data->support_adaptor_type == SUPPORT_DCIN_ONLY) {
		atc260x_misc_enable_wakeup_detect(ATC260X_WAKEUP_SRC_VBUS_IN, 0);
		return ADAPTER_TYPE_NO_PLUGIN;
	} else {
		atc260x_misc_enable_wakeup_detect(ATC260X_WAKEUP_SRC_VBUS_IN, 1);
	}

	for (i = 0; i < 5; i++) {
		ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_VBUSV, &vbusv);
		if (ret) {
			printf("[%s] get vbusv auxadc err!\n", __func__);
			vbusv = 0;
			continue;
		}

		if (vbusv > ATC260X_VBUS_VOL_THRESHOLD) {
			debug("USB ONLINE\n");
			return ADAPTER_TYPE_USB_PLUGIN;
		}
		mdelay(2);
		debug("%s the %d time\n", __func__, i);
	}

	debug("USB OFFLINE(%dmv)\n", vbusv);

	return ADAPTER_TYPE_NO_PLUGIN;
}


static int atc2603c_charger_check_online(void)
{
	if (atc2603c_charger_info.charge_detected)
		return atc2603c_charger_info.charger_online;
	if (atc2603c_charger_info.data.support_adaptor_type != SUPPORT_USB_ONLY)
		atc2603c_charger_info.charger_online |= atc2603c_charger_check_wall_online();
	if (atc2603c_charger_info.data.support_adaptor_type != SUPPORT_DCIN_ONLY)
		atc2603c_charger_info.charger_online |= atc2603c_charger_check_usb_online();
	atc2603c_charger_info.charge_detected = 1;

	return atc2603c_charger_info.charger_online;
}

static void atc2603c_charger_init_base(void)
{
	/*charger init*/
	atc2603c_charger_set_onoff(0);
	atc2603c_charger_enable_trick(1);
	atc2603c_charger_set_trick_current(TRICKLE_CURRENT_200MA);
	atc2603c_charger_set_cc(200);
	atc2603c_charger_set_trick_timer(TRICKLE_TIMER_30MIN);
	atc2603c_charger_set_cc_timer(CC_TIMER_12H);
	atc2603c_charger_auto_stop(0);

	atc2603c_charger_adjust_current(0);
	atc2603c_charger_set_cv(CONSTANT_VOL_4250MV);
	atc2603c_charger_set_stop_vol(STOP_VOLTAGE_4160MV);
	atc2603c_charger_current_temp(1);

	atc2603c_charger_ot_shutoff(1);

	atc2603c_charger_op_offset_threshold();
	/* wall init*/
	atc2603c_wall_set_pd(1);
	/*vbus init*/
	atc2603c_vbus_set_onoff(1);
	if (atc2603c_charger_info.data.usb_pc_ctl_mode == 0)
		atc2603c_vbus_set_ctl_en(0);
	else if (atc2603c_charger_info.data.usb_pc_ctl_mode == 2) {
		atc2603c_vbus_set_ctl_en(1);
		atc2603c_vbus_set_ctlmode(VOLTAGE_LIMITED);
		atc2603c_vbus_set_vol_lmt(VBUS_VOL_LIMT_4300MV);
	} else {
		atc2603c_vbus_set_ctl_en(1);
		atc2603c_vbus_set_ctlmode(CURRENT_LIMITED);
		atc2603c_vbus_set_current_lmt(VBUS_CURR_LIMT_500MA);
	}
}

static struct power_charger_ops ops = {
	.get_rsense = atc2603c_charger_get_rsense,
	.release_guard = atc2603c_charger_release_guard,
	.chk_bat_online_intermeddle = atc2603c_chk_bat_online_intermeddle,
	.chk_charger_online = atc2603c_charger_check_online,
};

static int atc2603c_charger_cfg_init(struct atc260x_charger_info *info,
	const void *blob, const char *compatible)
{
	int node;
	int error = 0;

	node = fdt_node_offset_by_compatible(blob, 0, compatible);
	if (node < 0) {
		printf("%s no match in dts, node:%d\n", __func__, node);
		return -1;
	}

	info->data.rsense = fdtdec_get_int(blob, node,
		"rsense", 0);
	error |= (info->data.rsense == -1);
	info->data.support_adaptor_type = fdtdec_get_int(blob, node,
		"support_adaptor_type", 2);
	error |= (info->data.support_adaptor_type == -1);
	info->data.usb_pc_ctl_mode = fdtdec_get_int(blob, node,
		"usb_pc_ctl_mode", 2);
	error |= (info->data.usb_pc_ctl_mode == -1);
	gpio_request_by_name_nodev(blob, node, "wall_switch", 0,
				       &info->wall_switch, GPIOD_IS_OUT);

	if (error) {
		printf("%s get dts value failed\n", __func__);
		return -1;
	}

	debug("%s rsense(%d)\n",
		__func__, info->data.rsense);
	debug("%s support_adaptor_type(%d)\n",
		__func__, info->data.support_adaptor_type);
	debug("%s usb_pc_ctl_mode(%d)\n",
		__func__, info->data.usb_pc_ctl_mode);

	return 0;
}

int atc2603c_charger_init(const void *blob)
{
	int ret;

	ret = atc2603c_charger_cfg_init(&atc2603c_charger_info, blob,
		"actions,atc2603c-charger");
	if (ret)
		return ret;
	atc2603c_charger_init_base();
	atc2603c_charger_set_rsense(atc2603c_charger_info.data.rsense);
	atc260x_charger_register(&ops);
	/*如果有usb/adaptor插入则检测当前是否是adaptor供电，如果是adaptor供电则打开vbus->wall开关*/
	if (atc2603c_charger_check_online() & ADAPTER_TYPE_USB_PLUGIN) {
		/*返回值为1：PC；返回值为2：usb adapter*/
		ret = owl_usb_get_connect_type();
		printf("CHARGER:usb type:%s\n", (ret == 2) ? "usb adaptor" : "usb pc");
		if (ret == 2) {
				if (dm_gpio_is_valid(&(atc2603c_charger_info.wall_switch)))
					dm_gpio_set_value(&(atc2603c_charger_info.wall_switch), 1);
		}
	}

	return 0;
}
