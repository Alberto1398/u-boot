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


#ifndef __OWLXX_POWER_H_
#define __OWLXX_POWER_H_

#include <afinfo.h>

#ifdef CONFIG_ATC2603B_POWER
#define PMU_SWCHG_CTL0_SWCHGEN     					      (1 << 15)
#define PMU_SWCHG_CTL0_ENTKLE						      (1 << 14)
#define PMU_SWCHG_CTL0_CHGISTK_100MA				      (0<<13)
#define PMU_SWCHG_CTL0_CHGISTK_200MA				      (1<<13)
#define PMU_SWCHG_CTL0_RSENSEL_20					      (0<<12)
#define PMU_SWCHG_CTL0_RSENSEL_10					      (1<<12)
#define PMU_SWCHG_CTL0_CHGIS_SHIFT      				      (8)
#define PMU_SWCHG_CTL0_CHGIS_MASK       				      (0xf << PMU_SWCHG_CTL0_CHGIS_SHIFT)
#define PMU_SWCHG_CTL0_CHGIS_1000MA				      (5<<8)
#define PMU_SWCHG_CTL0_ENCHGTIME					      (1<<7)
#define PMU_SWCHG_CTL0_TKLT_30MIN					      (0<<5)
#define PMU_SWCHG_CTL0_TKLT_40MIN					      (1<<5)
#define PMU_SWCHG_CTL0_TKLT_50MIN					      (2<<5)
#define PMU_SWCHG_CTL0_TKLT_60MIN					      (3<<5)
#define PMU_SWCHG_CTL0_CHGT_4H						      (0<<3)
#define PMU_SWCHG_CTL0_CHGT_6H						      (1<<3)
#define PMU_SWCHG_CTL0_CHGT_8H						      (2<<3)
#define PMU_SWCHG_CTL0_CHGT_12H					      (3<<3)
#define PMU_SWCHG_CTL0_ENCHGFCOFF					      (1<<2)
#define PMU_SWCHG_CTL0_ENCHGATDT					      (1<<1)
#define PMU_SWCHG_CTL0_DTSEL_20S					      (1<<0)
#define PMU_SWCHG_CTL0_DTSEL_3MIN					      (0<<0)

#define PMU_SWCHG_CTL1_ENBATDT					              (1<<15)
#define PMU_SWCHG_CTL1_ENATDIS					              (1<<14)
#define PMU_SWCHG_CTL1_ENCURRISE					      (1<<13)
#define PMU_SWCHG_CTL1_ENFASTCHG					      (1<<10)
#define PMU_SWCHG_CTL1_STOPV_4160MV				      (0<<9)
#define PMU_SWCHG_CTL1_STOPV_4180MV				      (1<<9)
#define PMU_SWCHG_CTL1_CHGPWRSET_60MV				      (0<<6)
#define PMU_SWCHG_CTL1_CHGPWRSET_160MV			      (1<<6)
#define PMU_SWCHG_CTL1_ENSAMP						      (1<<5)
#define PMU_SWCHG_CTL1_ENCHGTEMP					      (1<<4)

#define PMU_SWCHG_CTL2_EN_OCP						      (1<<4) 
#define PMU_SWCHG_CTL2_IZSL 							      (5<<0)
#define PMU_BATVADC_BATVADC(val) 				      (val & 0xfff)

#define PMU_LDO0_CTL0_LDO0_EN				(1 << 0)
#endif

#ifdef CONFIG_ATC2603A_POWER
#define     PMU_CHARGER_CTL0_ENCH				(1 << 15)
#define     PMU_CHARGER_CTL1_BAT_EXIST_EN	(1 << 5)
#define	PMU_CHARGER_CTL1_BAT_EXIST		(1 << 10)

#define WAKEUP_SRC_RESET			(1 << 1)
#define WAKEUP_SRC_HDSW			(1 << 2)
#define WAKEUP_SRC_ALARM			(1 << 3)
#define WAKEUP_SRC_ONOFF_SHORT		(1 << 7)
#define WAKEUP_SRC_ONOFF_LONG		(1 << 8)
#define WAKEUP_SRC_WALL_IN			(1 << 9)
#define WAKEUP_SRC_VBUS_IN			(1 << 10)

#define PMU_SWITCH_CTL_SWITCH2_EN               (1 << 14)
#define PMU_SWITCH_CTL_SWITCH1_EN		(1 << 15)

/* gl5302_PMU_CHARGER_CTL0 */
#define     PMU_CHARGER_CTL0_CHGAUTO_DETECT_EN		(1 << 0)
#define     PMU_CHARGER_CTL0_CHGPWR_SET_MASK		(0x3 << 1)
#define     PMU_CHARGER_CTL0_CHGPWR_SET_100MV		(0 << 1)
#define     PMU_CHARGER_CTL0_CHGPWR_SET_200MV		(1 << 1)
#define     PMU_CHARGER_CTL0_CHGPWR_SET_300MV		(2 << 1)
#define     PMU_CHARGER_CTL0_CHGPWR_SET_400MV		(3 << 1)
#define     PMU_CHARGER_CTL0_CHG_CURRENT_TEMP		(1 << 3)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR_SET			(0x3 << 4)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR_SET_3810MV      (0 << 4)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR_SET_3960MV      (1 << 4)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR_SET_4250MV      (2 << 4)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR_SET_4400MV      (3 << 4)
#define     PMU_CHARGER_CTL0_CHG_SYSPWR		(1 << 6)
#define     PMU_CHARGER_CTL0_DTSEL_MASK		(0x1 << 7)
#define     PMU_CHARGER_CTL0_DTSEL_12MIN		(0 << 7)
#define     PMU_CHARGER_CTL0_DTSEL_20S		(1 << 7)
#define     PMU_CHARGER_CTL0_CHG_FORCE_OFF	(1 << 8)
#define     PMU_CHARGER_CTL0_TRICKLEEN		(1 << 9)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER2_MASK     (0x3 << 10)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER2_30MIN    (0 << 10)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER2_40MIN    (1 << 10)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER2_50MIN    (2 << 10)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER2_60MIN    (3 << 10)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER1_MASK	     (0x3 << 12)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER1_4H       (0 << 12)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER1_6H       (1 << 12)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER1_8H       (2 << 12)
#define     PMU_CHARGER_CTL0_CHARGE_TIMER1_12H	  (3 << 12)
#define     PMU_CHARGER_CTL0_CHGTIME			(1 << 14)
#define     PMU_CHARGER_CTL0_ENCH				(1 << 15)

/* gl5302_PMU_CHARGER_CTL1 */
#define PMU_CHARGER_CTL1_ICHG_REG_CC_MASK       (0xF)
#define PMU_CHARGER_CTL1_ICHG_REG_CC(i)         \
            (((i) / 100) & PMU_CHARGER_CTL1_ICHG_REG_CC_MASK)
#define     PMU_CHARGER_CTL1_BAT_EXIST_EN		(1 << 5)
#define     PMU_CHARGER_CTL1_CURRENT_SOFT_START	(1 << 6)
#define     PMU_CHARGER_CTL1_STOPV_MASK			(0x1 << 7)
#define     PMU_CHARGER_CTL1_STOPV_4180MV		(0 << 7)
#define     PMU_CHARGER_CTL1_STOPV_4160MV		(1 << 7)
#define     PMU_CHARGER_CTL1_CHARGER_TIMER_END	(1 << 8)
#define     PMU_CHARGER_CTL1_BAT_DT_OVER			(1 << 9)
#define     PMU_CHARGER_CTL1_BAT_EXIST			(1 << 10)
#define     PMU_CHARGER_CTL1_CUR_ZERO			(1 << 11)
#define     PMU_CHARGER_CTL1_CHGPWROK			(1 << 12)
#define     PMU_CHARGER_CTL1_PHASE_MASK			(0x3 << 13)
#define     PMU_CHARGER_CTL1_PHASE_PRECHARGE	(1 << 13)
#define     PMU_CHARGER_CTL1_PHASE_CONSTANT_CURRENT (2 << 13)
#define     PMU_CHARGER_CTL1_PHASE_CONSTANT_VOLTAGE (3 << 13)
#define     PMU_CHARGER_CTL1_CHGEND				(1 << 15)

/* gl5302_PMU_CHARGER_CTL2 */
#define     PMU_CHARGER_CTL2_ICHG_REG_T_MASK	(0x3)
#define     PMU_CHARGER_CTL2_ICHG_REG_T(i)          \
                    ((((i) / 100) & PMU_CHARGER_CTL2_ICHG_REG_T_MASK) << 4);
#define     PMU_CHARGER_CTL2_CV_SET                 (1 << 6)

/* gl5302_PMU_SYS_Pending */
#define     PMU_SYS_PENDING_BAT_OV                  (1 << 15)

/* gl5302_PMU_OT_CTL */
#define     PMU_OT_CTL_OT                           (1 << 15)

/* gl5302_APDS_CTL */
#define     APDS_CTL_VBUSOTG				(1 << 9)
#define     APDS_CTL_VBUS_PD				(1 << 2)
#define     APDS_CTL_WALL_PD				(1 << 1)
#define	APDS_CTL_VBUS_CUR_LMT_MASK	(0x3 << 12)
#define	APDS_CTL_VBUS_CUR_LMT_100		(0x0 << 12)
#define	APDS_CTL_VBUS_CUR_LMT_300		(0x1 << 12)
#define	APDS_CTL_VBUS_CUR_LMT_500		(0x2 << 12)
#define	APDS_CTL_VBUS_CUR_LMT_800		(0x3 << 12)
#define	APDS_CTL_VBUS_CONTROL_CUR	(0x1 << 14)
#define	APDS_CTL_VBUS_CONTROL_EN		(0x1 << 15)

#endif

#define PMU_SYS_CTL1_EN_S1              					     (1 << 0)
#define PMU_SYS_CTL3_EN_S3       					             (1 << 14)
#define PMU_SYS_CTL3_EN_S2            					     (1 << 15)
#define PMU_SYS_CTL3_FW_FLAG_S2					     (1 << 4)

#define CHARGE_STATUS_IN                		 		         	  1
#define CHARGE_STATUS_OUT               			 	          0

#define POWER_SLIGHT_LOW_LEVEL		3400   /*show logo before poweroff*/
#define POWER_SERIOUS_LOW_LEVEL	3500
#define POWER_VERY_LOW_LEVEL		3450   /*charger if usb plug*/
#define POWER_BAT_LOW_LEVEL			3600

enum {
	DDR_VOLTAGE_1550MV = 0,
	DDR_VOLTAGE_1600MV,
};

enum {
	NO_PLUG = 0,
	WALL_PLUG = 1,
	USB_ADPT = 2,
	USB_PC = 4,
};

enum {
	NORMAL_BOOT = 0,
	CHARGER_BOOT,
};

enum {
	BOOT_IMAGE_NO_PICTURE = 1,
	BOOT_IMAGE_BATTERY_LOW,
	BOOT_IMAGE_NORMAL,
	BOOT_IMAGE_RECOVERY,
};


int atc260x_sdvcc_enable(unsigned int enable);
int atc260x_reg_write(unsigned short reg , unsigned short value);
int atc260x_reg_read(unsigned short reg);
int atc260x_set_bits(unsigned int reg, unsigned short mask, unsigned short val);
int atc260x_get_vbat(void);
int atc260x_get_charge_plugin_status(void);
int low_power_dect(void);
int count_onoff_short_press(void);

void low_power_charger(void);
int get_battery_voltage(void);

#define OWLXX_PMU_LOW_BATTY_CAP   (afinfo->battery_low_cap)
#define OWLXX_PMU_GAUGE_ID   (afinfo->gauge_id)
#define OWLXX_PMU_POWER_SYTLE   (afinfo->power_style)
#define OWLXX_PMU_CHAEGER_MODE   (afinfo->charger_mode)

extern int bat_voltage;
extern int power_plug_status;
extern int charger_boot_mode;
extern int bat_cap;
extern int bat_exist;

/****************/
struct owlxx_regulator_ops {
	int (*enable)(int id);
	int (*disable)(int id);
	int (*set_voltage)(int id, unsigned voltage_mv);
};

struct owlxx_regulator {
	int id;
	int dev_node;
	struct owlxx_regulator_ops *ops;
};


#define ATC260X_REGULATOR_DCDC_BASE	1
#define ATC260X_REGULATOR_DCDC_NUM	4
#define ATC260X_REGULATOR_DCDC(x)	(ATC260X_REGULATOR_DCDC_BASE + (x - 1))

#define ATC260X_REGULATOR_LDO_BASE	(ATC260X_REGULATOR_DCDC_BASE + ATC260X_REGULATOR_DCDC_NUM)
#define ATC260X_REGULATOR_LDO_NUM	11
#define ATC260X_REGULATOR_LDO(x)	(ATC260X_REGULATOR_LDO_BASE + (x - 1))

#define ATC260X_REGULATOR_SWITCHLDO_BASE	(ATC260X_REGULATOR_LDO_BASE + ATC260X_REGULATOR_LDO_NUM)
#define ATC260X_REGULATOR_SWITCHLDO_NUM	2
#define ATC260X_REGULATOR_SWITCHLDO(x)	(ATC260X_REGULATOR_SWITCHLDO_BASE + (x - 1))

#define MAX_REGULATOR_NUM	(ATC260X_REGULATOR_SWITCHLDO_BASE + ATC260X_REGULATOR_SWITCHLDO_NUM - 1)

/*regulator consumer interfaces*/
int owlxx_regulator_enable(struct owlxx_regulator *regulator);
int owlxx_regulator_disable(struct owlxx_regulator *regulator);
struct owlxx_regulator *owlxx_regulator_get_by_node(int dev_node);
struct owlxx_regulator *owlxx_regulator_get_by_id(int dev_node);
struct owlxx_regulator *fdtdec_owlxx_regulator_get(const void *blob, int node,
		const char *prop_name);

/*regulator provider interfaces*/
int owlxx_regulator_add(int id, int dev_node, struct owlxx_regulator_ops *ops);



#endif
