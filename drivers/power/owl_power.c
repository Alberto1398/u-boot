/*
 * owlxx_power.c - OWL PMIC driver
 *
 * Copyright (C) 2012, Actions Semiconductor Co. LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/pmu.h>
#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>
#include <power/owlxx_power.h>
#include <power/gauge_eg2801.h>
#include <power/gauge_bq27441.h>
#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu.h>
#include <linux/input.h>

#define DBG(fmt, arg...) do {} while(0)

DECLARE_GLOBAL_DATA_PTR;

int bat_voltage = 1;
int power_plug_status = 1;
int charger_boot_mode = 1;
int bat_cap = 1;
int bat_exist = 1;
int low_bat_cap_flag = 1;
int splash_image_type;
static int guage_type =0;
int bat_low_flag = 0;
static int bat_vol_record[5] = {0,0,0,0,0};
static int bat_vol_record_point = 0;
static int bat_vol_record_sum;
static int bat_vol = 0;
static int chg_cycle;

#define ALARM_WK_FLAG			(1<<8)
#define USB_WK_FLAG				(1<<14)
#define WALL_WK_FLAG			(1<<15)
#define ONOFF_INT_EN				(1 << 12)
#define ONOFF_LONG_PRESS		(1 << 13)
#define ONOFF_SHORT_PRESS		(1 << 14)
#define ONOFF_PRESS              (1 << 15)


#define GAUGE_SOFTCAP			0
#define GAUGE_EG2801				1
#define GAUGE_BQ27441			2
#define PMU_SYS_CTL9		0x9

#define     gl5302_CVER			(0x704)

static const unsigned int charger_current_D[16] ={
	   67,   133,   267,  400,
	 533,   667,   800,  933,
	1067, 1200, 1333, 1467,
	1600, 1733, 1867, 2000,
};
static unsigned int bl_off_current_usb_pc;
static unsigned int bl_off_current_usb_adp;
static unsigned int bl_off_current_wall_adp;
static int pmic_ver = 0;
static struct owlxx_fdt_gpio_state extchg_gpio;
static struct owlxx_fdt_gpio_state wallctl_gpio;
static int ext_charger;
static int support_adaptor_type;

static unsigned int config_curve_arg0 = 416000;
static unsigned int config_curve_arg1 = 3727;  //0.3

static unsigned int table_bat_percentage[11][2] =
{
	{3850,0},
	{3910,10},
	{3940,20},
	{3980,30},
	{4020,40},
	{4060,50},
	{4090,60},
	{4130,70},
	{4170,80},
	{4190,90},
	{4210,95},
};  

extern int kbd_tstc(void);
extern int kbd_getc(void);

int get_battery_voltage(void);


#ifdef CONFIG_ATC2603A_POWER
static int gl5302_get_bat_voltage(int *volt)
{
	*volt = 4100; // TESTCODE
	return 0;
#if 0
	int ret =  0;

	/*BATVADC have no enable bit, just read the register*/
	ret = atc260x_reg_read(gl5302_PMU_BATVADC);
	if (ret < 0)
		return ret;

	*volt = (ret & 0x3ff) * 2930 * 2 / 1000;

	return 0;
#endif
}

static int gl5302_check_bat_online(void)
{
	return 1; // TESTCODE
#if 0
	int ret =  0;
	int exist;

	/* dectect bit 0 > 1 to start dectecting */
	ret = atc260x_set_bits(gl5302_PMU_CHARGER_CTL1,
			PMU_CHARGER_CTL1_BAT_EXIST_EN,
			PMU_CHARGER_CTL1_BAT_EXIST_EN);
	if (ret < 0)
		return ret;

	/* wait bat detect over */
	mdelay(300);

	ret = gl5302_reg_read(gl5302_PMU_CHARGER_CTL1);
	if (ret < 0)
		return ret;

	exist = ret & PMU_CHARGER_CTL1_BAT_EXIST;

	/* cleare battery detect bit, otherwise cannot changer */
	ret = atc260x_set_bits(gl5302_PMU_CHARGER_CTL1,
			PMU_CHARGER_CTL1_BAT_EXIST_EN, 0);
	if (ret < 0)
		return ret;

	if (exist)
		return 1;

	return 0;
#endif
}

static int get_usb_plug_type(void)
{
	return 0; // TESTCODE
#if 0
	int ret = 0;
	int reg_bk;
	int reg;
	
	reg_bk = readl(USB3_P0_CTL);
	reg = reg_bk;
	writel(reg | 0x0000f000, USB3_P0_CTL);   /* dp up enable, dp down disable */
	mdelay(100);
	reg = readl(USB3_P0_CTL);
	if((reg & 0x00000018) != 0) {           /* bit 3, bit 4 not 0, this is usb_adapter */
		//printf("----- current connect is USB_ADPT! \n");
		ret = 1;
	} else {
		//printf("----- current connect is USB_PC! \n");
		ret = 0;
	}

	writel(reg_bk, USB3_P0_CTL);            /*restore the USB3_P0_CTL */
	return ret;
#endif
}

static int gl5302_get_charge_plugin_status(void)
{
	return USB_PC; //TESTCODE
#if 0
	int ret = 0, plug_stat = 0;
	int wall_mv = 0, vbus_mv = 0;

	ret = gl5302_reg_read(gl5302_PMU_WALLVADC);
	if (ret >= 0) {
		ret *= 2930;		/* 2.93mv * 2.5 */
		ret = ret * 2 + ret / 2;
		ret /= 1000;
		wall_mv = ret;
		if (wall_mv > 3000)
			plug_stat = WALL_PLUG;
	}

	ret = gl5302_reg_read(gl5302_PMU_VBUSVADC);
	if (ret >= 0) {
		ret *= 2930;            /* 2.93mv * 2.5 */
		ret = ret * 2 + ret / 2;
		ret /= 1000;
		vbus_mv = ret;
		if (vbus_mv  > 3000) {
			ret = get_usb_plug_type();
			if (ret) {
				plug_stat |= USB_ADPT;
				printf("----- current connect is USB_ADPT! \n");
			} else {
				printf("----- current connect is USB_PC! \n");
				plug_stat |= USB_PC;
			}
		}
	}

	printf("----- current plug_stat is 0x%x\n", plug_stat);
	return plug_stat;
#endif
}
#if defined(CONFIG_OWLXX_NAND) || defined(CONFIG_OWLXX_MMC)
static void gl5302_open_bat_charger(void)
{
#if 0   //TESTCODE
	int val;

	/*Vbus charger, 500mA limit*/
	val = gl5302_reg_read(gl5302_PMU_APDS_CTL);
	val &= ~ (0xf<<12);
	val |= 0xe <<12;
	gl5302_reg_write(gl5302_PMU_APDS_CTL, val);

	/*enable  charger*/
	val = gl5302_reg_read(gl5302_PMU_CHARGER_CTL1);
	val &= ~0xf;
	val |= 0x5;
	gl5302_reg_write(gl5302_PMU_CHARGER_CTL1, val);

	val = gl5302_reg_read(gl5302_PMU_CHARGER_CTL0);
	val |= 0x1 <<15;
	gl5302_reg_write(gl5302_PMU_CHARGER_CTL0, val);

	return;
#endif
}

#ifdef CONFIG_SPL_BUILD
static void gl5302_close_wall(void)
{
	uint gpio;
	int value;

	if (2 == OWLXX_PMU_POWER_SYTLE) {
		/*close wall to system*/
		gpio = afinfo->wallctl_gpio;
		value = afinfo->wallctl_gpio_flag;
		printf("gl5302_close_wall: gpio=%d, value=%d\n", gpio, value);
		owlxx_gpio_set_value(gpio, value);
	}

	return;

}

static void gl5302_open_wall(void)
{
	uint gpio;
	int value;

	if (2 == OWLXX_PMU_POWER_SYTLE) {
		/*close wall to system*/
		gpio = afinfo->wallctl_gpio;
		value = afinfo->wallctl_gpio_flag;
		printf("gl5302_open_wall: gpio=%d, !value=%d\n", gpio, !value);
		owlxx_gpio_set_value(gpio, !value);
	}

	return;

}

static void gl5302_close_extcharger(void)
{
	uint gpio;
	int value;

	if (afinfo->ext_charger) {
		gpio = afinfo->extchg_gpio;
		value = afinfo->extchg_gpio_flag;
		printf("gl5302_close_extcharger: gpio=%d, !value=%d\n", gpio, !value);
		owlxx_gpio_set_value(gpio, !value);
	}

	return;

}

static void close_otgvbus(void)
{
	uint gpio;
	gpio = afinfo->otg_gpio;

	printf("close_otgvbus: %d \n ",gpio);
	owlxx_gpio_set_value(gpio, 0);	 /* close the otgvbus */
	return;

}

static int find_guage(void)
{
	int ret = afinfo->gauge_id;
	printf("find_guage: %d \n ", ret);
	return ret;
}

static int owlxx_in_recovery(void)
{
	return 0;
}
#else
static void gl5302_close_wall(void)
{
	int dev_node;
	struct owlxx_fdt_gpio_state power_gpio;

	if (2 == OWLXX_PMU_POWER_SYTLE) {
		/*close wall to system*/
		dev_node = fdtdec_next_compatible(
			gd->fdt_blob, 0,  COMPAT_ACTIONS_OWLXX_POWER);
		owlxx_fdtdec_decode_gpio(
			gd->fdt_blob, dev_node, "wall_ctrl_gpio", &power_gpio);
		owlxx_gpio_generic_direction_output(power_gpio.chip, power_gpio.gpio, power_gpio.flags);
	}

	return;
}

static void gl5302_close_extcharger(void)
{
	int dev_node, ret;
	struct owlxx_fdt_gpio_state power_gpio;

	dev_node = fdtdec_next_compatible(
		gd->fdt_blob, 0,  COMPAT_ACTIONS_OWLXX_POWER);

	ret = fdtdec_get_int(gd->fdt_blob, dev_node, "ext_charger", -1);

	if (ret) {
		owlxx_fdtdec_decode_gpio(
			gd->fdt_blob, dev_node, "ext_chg_ctrl_gpio", &power_gpio);
		owlxx_gpio_generic_direction_output(power_gpio.chip, power_gpio.gpio, !(power_gpio.flags));
	}

	return;
}

static void close_otgvbus(void)
{
	int devnode;
	struct owlxx_fdt_gpio_state vbus_gpio;
	
	devnode = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_ACTIONS_OWLXX_USB3);
	if (devnode < 0) {
		debug("%s: Cannot find device tree node\n", __func__);
		return -1;
	}
	
	owlxx_fdtdec_decode_gpio(gd->fdt_blob, devnode, "gpios", &vbus_gpio);
	printf("------ uboot close otgvbus: %d \n ", vbus_gpio.gpio);
	owlxx_gpio_generic_direction_output(vbus_gpio.chip, vbus_gpio.gpio, 0);	 /* close the otgvbus */
	return;
}

static int find_guage(void)
{
	int devnode;
	devnode = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_ACTIONS_BQ27441);
	if (devnode < 0) {
		devnode = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_ACTIONS_SOFTCAP);
		if (devnode < 0) {
			printf("gauge is  softcap\n");
			return GAUGE_SOFTCAP;
		} else {
			printf("error !gauge is  not find!\n");
			return -1;
		}
	} else {
		printf("gauge is bq27441\n");
		return GAUGE_BQ27441;
	}
}
#endif

#else
static void gl5302_open_bat_charger(void)
{
	return;
}

static void gl5302_close_wall(void)
{
	return;
}

static void gl5302_close_extcharger(void)
{
	return;
}

static void close_otgvbus(void)
{
	return;
}

static int  find_guage(void)
{
	return 0;
}

#endif

static void gl5302_close_bat_charger(void)
{
#if 0 // TESTCODE
	int val;
	val = gl5302_reg_read(gl5302_PMU_CHARGER_CTL0);
	val &= ~(0x1 <<15);
	gl5302_reg_write(gl5302_PMU_CHARGER_CTL0, val);

	return;
#endif
}

static void gl5302_shutoff(void){
	while(1); // TESTCODE
#if 0 // TESTCODE
	atc260x_set_bits(
		gl5302_PMU_SYS_CTL3,
		PMU_SYS_CTL3_EN_S2 | PMU_SYS_CTL3_EN_S3,
		0);
	atc260x_set_bits(gl5302_PMU_SYS_CTL1, PMU_SYS_CTL1_EN_S1, 0);
	return;
#endif
}

#endif

#ifdef CONFIG_ATC2603B_POWER
static int gl5303_get_bat_voltage(int *volt)
{
	*volt = 4100;  //TESTCODE
	return 0;
#if 0
	int ret =  0;
	int break_charge = 0;

	ret = atc260x_reg_read(gl5303_PMU_SWCHG_CTL0);

	if (ret & PMU_SWCHG_CTL0_SWCHGEN)
		break_charge = 1;

	if (break_charge) {
		atc260x_set_bits(
			gl5303_PMU_SWCHG_CTL0,
			PMU_SWCHG_CTL0_SWCHGEN,
			0);
		/* wait 100ms */
		mdelay(100);
	}

	/*BATVADC have no enable bit, just read the register*/
	ret = atc260x_reg_read(gl5303_PMU_BATVADC);
	if (ret < 0)
		return ret;

	*volt = PMU_BATVADC_BATVADC(ret) * 732 * 2 / 1000;

	if (break_charge) {
		atc260x_set_bits(
			gl5303_PMU_SWCHG_CTL0,
			PMU_SWCHG_CTL0_SWCHGEN,
			PMU_SWCHG_CTL0_SWCHGEN);
	}

	return 0;
#endif
}

static static int gl5303_check_bat_online(void)
{
	return 1;
}

static int gl5303_get_charge_plugin_status(void)
{
	int ret = 0, plug_stat = 0;
	int wall_mv = 0, vbus_mv = 0;

	ret = atc260x_reg_read(gl5303_PMU_AuxADC_CTL0);
	if (ret < 0)
		return ret;

	/* if current is disabled, enable it and wait 1ms for translate.*/
	if ((ret & (2 << 10)) == 0) {
		ret = atc260x_reg_write(gl5303_PMU_AuxADC_CTL0, 2 << 10);
		if (ret < 0)
			return ret;
		mdelay(1);
	}

	ret = atc260x_reg_read(gl5303_PMU_WALLVADC);
	wall_mv = (ret * 732 * 2 + ret * 732 / 2) / 1000;
	if (wall_mv > 4200)
		plug_stat = WALL_PLUG;

	ret = atc260x_reg_read(gl5303_PMU_VBUSVADC);
	vbus_mv = (ret * 732 * 2 + ret * 732 / 2) / 1000;
	if (vbus_mv  > 3000)
		plug_stat |= WALL_PLUG;

	return plug_stat;
}

static void gl5303_open_bat_charger(void)
{
	return;
}

static void gl5303_close_bat_charger(void)
{
	return;
}

static void gl5303_shutoff(void)
{
	atc260x_set_bits(
		gl5303_PMU_SYS_CTL3,
		PMU_SYS_CTL3_EN_S2 | PMU_SYS_CTL3_EN_S3,
		0);
	atc260x_set_bits(gl5303_PMU_SYS_CTL1, PMU_SYS_CTL1_EN_S1, 0);
	return;
}

#endif
#if 0
int atc260x_reg_read(unsigned short reg)
{
	int ret = 0;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		ret = gl5303_reg_read(reg);
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		ret = gl5302_reg_read(reg);
#endif
	return ret;
}

int atc260x_reg_write(unsigned short reg , unsigned short value)
{
	int ret = 0;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		ret = gl5303_reg_write(reg, value);
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		ret = gl5302_reg_write(reg, value);
#endif
	return ret;
}
#endif
/*
int atc260x_set_bits(unsigned int reg, unsigned short mask, unsigned short val)
{
	int ret = 0;
	unsigned short r = 0;

	r = atc260x_reg_read(reg);

	if (r < 0)
		return r;

	r &= ~mask;
	r |= val;

	ret = atc260x_reg_write(reg, r);

	return ret;
} */

#ifndef CONFIG_SPL_OWLXX_UPGRADE

static void atc260x_open_charger(void)
{
#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		gl5303_open_bat_charger();
#endif
									
#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		gl5302_open_bat_charger();
#endif
	return;
}

static void atc260x_close_charger(void)
{
#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		gl5303_close_bat_charger();
#endif
												
#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		gl5302_close_bat_charger();
#endif
	return;
}

int atc260x_get_vbat(void)
{
	int bat_v;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		gl5303_get_bat_voltage(&bat_v);
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		gl5302_get_bat_voltage(&bat_v);
#endif

	return bat_v;
}

static void atc260x_shutoff(void)
{
#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		gl5303_shutoff();
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		gl5302_shutoff();
#endif
	return;
}

static int check_battery_exist_hw(void)
{
	int ret = 0, vol;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		ret = gl5303_check_bat_online();
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		ret = gl5302_check_bat_online();
#endif

	vol = atc260x_get_vbat();

	if (ret > 0 && vol < 2000)
		return 0;

	return ret;
}

int atc260x_get_charge_plugin_status(void)
{
	int ret =  0;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID)
		ret = gl5303_get_charge_plugin_status();
#endif

#ifdef CONFIG_ATC2603A_POWER
	if (0 == OWLXX_PMU_ID)
		ret = gl5302_get_charge_plugin_status();
#endif
	return ret;
}

static void charger_bat(void)
{
	int charger_en = 0;

	power_plug_status = atc260x_get_charge_plugin_status();
	if (0 == power_plug_status)
		return;

	do {
		bat_voltage = get_battery_voltage();

		if (bat_voltage < POWER_VERY_LOW_LEVEL) {
			power_plug_status = atc260x_get_charge_plugin_status();
			if (((0 == OWLXX_PMU_POWER_SYTLE) && (power_plug_status == USB_PC)) || 
				(OWLXX_PMU_POWER_SYTLE && (power_plug_status & USB_PC))) {
				if (0 == charger_en) {
					atc260x_open_charger();
					charger_en = 1;
				}

				printf("Bat is very low(%dV) when plug usb-PC,chagring!\n", bat_voltage);
				mdelay(10000);
			} else if (0 == power_plug_status) {
				printf("Unplug when bat_v is too low (<3.45V), shut down!\n");
				atc260x_shutoff();
			} else
				break;
		} else
			break;
	} while(1);

	if (charger_en)
		atc260x_close_charger();

	return;
}

static int boot_mode_dect(void)
{
	int val, boot_choice = 0;

	power_plug_status = atc260x_get_charge_plugin_status();

	printf ("OWLXX_PMU_CHAEGER_MODE: %ld\n", OWLXX_PMU_CHAEGER_MODE);
	if ((0 == bat_exist) ||(0 == OWLXX_PMU_CHAEGER_MODE) || (0 == power_plug_status)) {
		return NORMAL_BOOT;
	}

	/*this bit will clear in driver*/
#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID) {
		val = atc260x_reg_read(gl5303_PMU_UV_INT_EN);
		boot_choice = val & 0x1;
	}
#endif

#if 0 //#ifdef CONFIG_ATC2603A_POWER  // TESTCODE
	if (0 == OWLXX_PMU_ID) {
		val = atc260x_reg_read(gl5302_PMU_SYS_CTL1);
		if (val & ALARM_WK_FLAG) {
			printf("Normal boot when alarm wake\n");
			return NORMAL_BOOT;
		}

		val = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
		boot_choice = val & 0x1;
	}
#endif

	printf("boot_choice: %d\n", boot_choice);
	//if (0 == boot_choice) { // TESTCODE
	//	return CHARGER_BOOT;
	//} else {
		return NORMAL_BOOT;
	//}
}

#if 0
int atc260x_sdvcc_enable(unsigned int enable)
{
	int ret =  0;

#ifdef CONFIG_ATC2603B_POWER
	if (1 == OWLXX_PMU_ID) {
		if (enable)
			ret = atc260x_set_bits(gl5303_PMU_LDO0_CTL0,
					PMU_LDO0_CTL0_LDO0_EN,
					PMU_LDO0_CTL0_LDO0_EN);
		else
			ret = atc260x_set_bits(gl5303_PMU_LDO0_CTL0,
					PMU_LDO0_CTL0_LDO0_EN,
					0);
	}
#endif

#if 0 //#ifdef CONFIG_ATC2603A_POWER //TESTCODE
	if (0 == OWLXX_PMU_ID) {
		if (enable)
			ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
					PMU_SWITCH_CTL_SWITCH1_EN,
					gl5302_PMU_SWITCH_CTL);
		else
			ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
					PMU_SWITCH_CTL_SWITCH1_EN,
					0);
	}
#endif
	return ret;
}
#endif

int get_battery_voltage(void)
{
	int ret;

	switch (guage_type) {
	case GAUGE_EG2801:
		//ret = eg2801_gasgauge_get_voltage();
		break;
	case GAUGE_BQ27441:
		ret = bq27441_gasgauge_get_voltage();
		if (ret > 4300 || ret < 0) {
			printf("bq27441 is crazy (%d), use PMU\n", ret);
			ret = atc260x_get_vbat();
		}
		break;

	case GAUGE_SOFTCAP:
	default:
		ret = atc260x_get_vbat();
		break;
	}

	return ret;
}

int get_battery_cap(void)
{
	int ret, pmu_reg;

	switch (guage_type) {
	case GAUGE_EG2801:
		//ret = eg2801_gasgauge_get_capacity();
		break;
	case GAUGE_BQ27441:
		ret = bq27441_gasgauge_get_capacity();
		break;

	//case GAUGE_SOFTCAP:
	default:
		ret = -1;
		break;
	}

	if (ret < 0 || ret > 100) {
		pmu_reg = atc260x_reg_read(PMU_SYS_CTL9);
		if (pmu_reg & 0x8000) {
			ret = (pmu_reg >> 8) & 0x7f; 
			printf("store cap: %d, PMU_SYS_CTL9=0x%x\n", ret, pmu_reg);
		}
	}

	return ret;
}

int low_power_dect(void)
{
	int ret = 0;

	close_otgvbus();
	gl5302_close_extcharger();
	mdelay(50);

	printf("OWLXX_PMU_GAUGE_ID : %ld\n", afinfo->gauge_id);
	printf("OWLXX_GAUGE_BUS_ID : %ld\n", afinfo->gauge_bus_id);
	guage_type = find_guage();

	bat_voltage = atc260x_get_vbat();

	if (bat_voltage < 3000) {
		debug(" battery hw detect~\n");
		ret = check_battery_exist_hw();
		if (ret <= 0) {
			printf("No battery, boot normally!\n");
			bat_exist = 0;
		} else {
			bat_exist = 1;
		}
	} else {
		bat_exist = 1;
	}

	printf("bat_exist: %d\n", bat_exist) ;

#ifdef CONFIG_SPL_BUILD
	if (bat_exist && (0 == OWLXX_PRODUCT_STATE))
		charger_bat();
#endif

#ifndef CONFIG_SPL_BUILD
	charger_boot_mode  = boot_mode_dect();
	printf("charger_boot_mode %d\n", charger_boot_mode);
#endif
	return 0;
}

void detect_quick_plug(void)
{
#if 0  // TESTCODE
	int ret;
	power_plug_status = atc260x_get_charge_plugin_status();

	ret = atc260x_reg_read(gl5302_PMU_SYS_CTL1);
	printf("last gl5302_PMU_SYS_CTL1=0x%x\n", ret);
	if ((0 == power_plug_status) && (ret & (USB_WK_FLAG |WALL_WK_FLAG))) {
		printf("last, why quick plug in and plug out, U S B? shut down!\n");
		atc260x_shutoff();	
	}
#endif
}

void low_power_charger(void)
{
	charger_boot_mode  = NORMAL_BOOT;  // TESTCODE
	return;
#if 0
	int ret, cap, old_val, usb_pc_plug = 0;

	ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
	printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);

	if (ret & 0x2) {
		printf("show charger picture in normal boot !\n");
		cap = get_bat_cap();
		/*show picture by cap*/
		charge_animation_display(cap);
		atc260x_shutoff();	
	}

	if (1 == OWLXX_PRODUCT_STATE) {
		charger_boot_mode  = NORMAL_BOOT;
		return;
	}

	if (0 == bat_exist) {
		owlxx_dss_enable();
	}

	power_plug_status = atc260x_get_charge_plugin_status();
	usb_pc_plug = ((0 == OWLXX_PMU_POWER_SYTLE) && (power_plug_status == USB_PC)) || 
				(OWLXX_PMU_POWER_SYTLE && (power_plug_status & USB_PC));

	ret = atc260x_reg_read(gl5302_PMU_SYS_CTL1);
	printf("gl5302_PMU_SYS_CTL1=0x%x\n", ret);
	if ((0 == power_plug_status) && (ret & (USB_WK_FLAG |WALL_WK_FLAG))) {
		printf("why quick plug in and plug out, U S B? shut down!\n");
		atc260x_shutoff();	
	}

	if (bat_exist && usb_pc_plug) {
		bat_voltage =  get_battery_voltage();
		if (bat_voltage < POWER_SERIOUS_LOW_LEVEL) {
			DBG("1 DDD dss en\n");
			owlxx_dss_enable();
			mdelay(2000);
			DBG("1b DDD dss rev\n");
			owlxx_dss_remove();
			old_val = atc260x_reg_read(gl5302_PMU_SYS_CTL2);
			atc260x_reg_write(gl5302_PMU_SYS_CTL2, old_val |ONOFF_INT_EN);
			atc260x_set_bits(gl5302_PMU_SYS_CTL2, 
		            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS, 
		            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);

			atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x2, 0x2);
			printf("goto lower power pc charger!\n");
			ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
			printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
			atc260x_shutoff();	
		}

		DBG("4 DDD bmode %d\n", charger_boot_mode);
		if ((1 == charger_boot_mode) && (owlxx_in_recovery() == 0)) {
			DBG("3b DDD dss rev\n");
			owlxx_dss_remove();
		} else {
			DBG("3. DDD dss en\n");
			owlxx_dss_enable();
		}
	}else if (0 == power_plug_status) {
		if (bat_low_flag) {
			DBG("4. DDD dss en\n");
			owlxx_dss_enable();
			mdelay(2000);
			DBG("4b DDD dss rev\n");
			owlxx_dss_remove();
			printf("Bat cap is too low, shut down!\n");
			mdelay(2000);
			atc260x_shutoff();
			while(1)
				;
		}

		if (splash_image_type == BOOT_IMAGE_NO_PICTURE) {
			owlxx_dss_remove();
		} else {
			DBG("5. DDD dss en\n");
			owlxx_dss_enable();
		}
	} else {
		if ((1 == charger_boot_mode) && (owlxx_in_recovery() == 0)) {
			DBG("5b DDD dss rev\n");
			owlxx_dss_remove();
		} else {
			DBG("6. DDD dss en\n");
			owlxx_dss_enable();
		}
	}

	/* 1) not in recovery mode && 2) charger is plugged in */
	if ((1 == charger_boot_mode) && (owlxx_in_recovery() == 0)) {
		printf("charger_boot_mode : %d\n", charger_boot_mode);
		atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x2, 0x2);
		printf("goto minicharger!\n");
		ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
		printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
		cap = get_bat_cap();
		charge_animation_display(cap);
		atc260x_shutoff();	
	}

	return;
#endif
}

int soft_gasgauge_get_capacity()
{
	return 100; // TESTCODE
#if 0
	int i, bat_cap, bat_v, bat_vsum, ret, bat_curr, save_cap;
	unsigned int pmu_sys_ctl9;

	ret =  atc260x_reg_read(gl5302_PMU_BATVADC);
	if (ret) {
		ret *= 2930 * 2;
		bat_v= ret/1000;
	}

	printf("soft_cap get bat_v = %d\n", bat_v);

	for (i=0; i < 11; i++) {
		if (bat_v < table_bat_percentage[i][0]) {
			bat_cap = table_bat_percentage[i][1];
			break;
		}
	}

	if (11 == i)
		bat_cap = 95;

	if (95 == bat_cap) {
		bat_curr = atc260x_reg_read(gl5302_PMU_CHGIADC);
		if (bat_curr < 50)
			bat_cap = 100;
	}

	pmu_sys_ctl9 = atc260x_reg_read(gl5302_PMU_SYS_CTL9);
	if (pmu_sys_ctl9 & 0x8000) {
		save_cap = (pmu_sys_ctl9 >> 8) & 0x7f; 
		printf("store cap: %d\n", save_cap);
		if (bat_cap > save_cap) {
			pmu_sys_ctl9 = atc260x_reg_read(gl5302_PMU_SYS_CTL9);
			pmu_sys_ctl9 &= 0xff;
			pmu_sys_ctl9 |= bat_cap << 8;
			pmu_sys_ctl9 |= 0x8000;
			atc260x_reg_write(gl5302_PMU_SYS_CTL9, pmu_sys_ctl9);
		}
	}	

	return bat_cap;
#endif
}

int get_bat_cap(void)
{
	int bat_cap;
	bat_cap = bq27441_gasgauge_get_capacity();
	if (bat_cap<0 || bat_cap > 100) {
		bat_cap = soft_gasgauge_get_capacity();
		printf("use soft cap, bat_cap: %d\n", bat_cap);
	}
	return bat_cap;
}

#ifdef CONFIG_SPL_BUILD
int low_power_pcchg(void)
{
	return 0; // TESTCODE
#if 0
	int ret, key_stat;
	int charger_en = 0, usb_pc_plug = 0;

	power_plug_status = atc260x_get_charge_plugin_status();
	usb_pc_plug = ((0 == OWLXX_PMU_POWER_SYTLE) && (power_plug_status == USB_PC)) || 
				(OWLXX_PMU_POWER_SYTLE && (power_plug_status & USB_PC));

	if (bat_exist && usb_pc_plug) {
		do {
			bat_voltage = get_battery_voltage();
			if ((bat_voltage < POWER_SERIOUS_LOW_LEVEL) && (bat_voltage >= POWER_SLIGHT_LOW_LEVEL)) {
				power_plug_status = atc260x_get_charge_plugin_status();
				DBG("3 DDD plug 0x%x\n", power_plug_status);
				if (power_plug_status) {
					if (0 == charger_en) {
						atc260x_open_charger();
						charger_en = 1;
					}
		
					ret = atc260x_reg_read(gl5302_PMU_SYS_CTL2); 
					key_stat = ret & (ONOFF_SHORT_PRESS |ONOFF_LONG_PRESS);
					printf("key_stat = 0x%x\n", key_stat);
					if (key_stat) {
						if (bat_voltage >3400) {
							printf("show low bat picture when use pc charger!\n");
							atc260x_set_bits(gl5302_PMU_SYS_CTL2,
								ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS,
								ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
							return -1;
						}
					}
		
					printf("Bat is low(%dV) when plug usb-PC,chagring!\n", bat_voltage);
				} else {
					printf("Unplug USB-PC when Bat cap is too low(<3.5V), shut down!\n");
					atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x3, 0x0);
					ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
					printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
					atc260x_shutoff();
				}
			} else if (bat_voltage < POWER_SLIGHT_LOW_LEVEL) {
				printf("diacharger below 3400 when plug usb_pc, shut down!\n");
				atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x3, 0x0);
				ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
				printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
				atc260x_shutoff();
			} else
				break;
		}while(1);
	}

	return 0;
#endif
}

/*
 * get chip version of atc260x
 */
/*
int atc260x_get_version(void)
{
	int ret;

	ret = atc260x_reg_read(gl5302_CVER);
	if (ret < 0) {
		printf("Failed to read version data: %d\n", ret);
		return ret;
	}

	switch(ret & 0x7)
	{
		case 0:
			return 0;
		case 1:
			return 1;
		case 3:
			return 2;
		case 7:
			return 3;
		default:
			return (ret & 0x7);
	}
	return -1; 
} */

static void get_cc_version_d(unsigned int *value)
{
	int i;
	unsigned int cc = *value;

	for(i = 0; i < 16; i++) {
		if(charger_current_D[i] >= cc)
			break;
	}

	if (0 == i) {
		*value = 0;
		return;
	}

	if (16 == i) {
		*value = 5;
		return;
	}

	if ((cc - charger_current_D[i-1]) >
			(charger_current_D[i] - cc))
		*value = i;
	else
		*value = i - 1;

	return;
}

static int atc260x_bat_charger_init(void)
{
	return 0; // TESTCODE
#if 0
	int constant_current, trick_current, stop_v, ret;

	constant_current = 8;
	trick_current = 100;
	stop_v = PMU_CHARGER_CTL1_STOPV_4160MV;

	/*
	 * charge power set ( > SYSPWR): 0.1v
	 * charge SYSPWR set (stable voltage): 4.4v
	 *
	 * 0x33f9
	 */
	atc260x_reg_write(gl5302_PMU_CHARGER_CTL0,
		PMU_CHARGER_CTL0_CHGAUTO_DETECT_EN |
		PMU_CHARGER_CTL0_CHGPWR_SET_100MV |
		PMU_CHARGER_CTL0_CHG_CURRENT_TEMP |
		PMU_CHARGER_CTL0_CHG_SYSPWR_SET_4250MV |
		PMU_CHARGER_CTL0_CHG_SYSPWR |
		PMU_CHARGER_CTL0_DTSEL_12MIN |
		PMU_CHARGER_CTL0_CHG_FORCE_OFF |
		PMU_CHARGER_CTL0_TRICKLEEN |
		PMU_CHARGER_CTL0_CHARGE_TIMER2_30MIN |
		PMU_CHARGER_CTL0_CHARGE_TIMER1_12H |
		PMU_CHARGER_CTL0_CHGTIME);

	/*disable charger detect, force end and timer because them are not precise*/
	atc260x_set_bits(gl5302_PMU_CHARGER_CTL0,
	    	PMU_CHARGER_CTL0_CHGAUTO_DETECT_EN |
	    	PMU_CHARGER_CTL0_CHG_FORCE_OFF |
		PMU_CHARGER_CTL0_CHGTIME |
		PMU_CHARGER_CTL0_CHG_SYSPWR, 0);

	atc260x_reg_write(gl5302_PMU_CHARGER_CTL1,
		  PMU_CHARGER_CTL1_CURRENT_SOFT_START |stop_v |constant_current);

	/*
	 * trickle charge current: 100mA
	 * fast charge: enable
	 */
	ret = PMU_CHARGER_CTL2_ICHG_REG_T(trick_current);
	atc260x_reg_write(gl5302_PMU_CHARGER_CTL2,
	                  PMU_CHARGER_CTL2_CV_SET | ret);

	atc260x_set_bits(gl5302_PMU_APDS_CTL,
		APDS_CTL_WALL_PD|APDS_CTL_VBUS_PD,APDS_CTL_WALL_PD|APDS_CTL_VBUS_PD);

	/*init charger status*/
	printf("CHARGER_CTL0:%x, CHARGER_CTL1:%x, CHARGER_CTL2:%x\n",
		atc260x_reg_read(gl5302_PMU_CHARGER_CTL0),
		atc260x_reg_read(gl5302_PMU_CHARGER_CTL1),
		atc260x_reg_read(gl5302_PMU_CHARGER_CTL2));
	return 0;
#endif
}

int atc260x_vbus_current_limit(int on, int current_ma)
{
	return 0; // TESTCODE
#if 0
	unsigned int pmu_adps_ctl_val;
	int ret;

	pmu_adps_ctl_val = atc260x_reg_read(gl5302_PMU_APDS_CTL);

	if(on){
		pmu_adps_ctl_val &= (~APDS_CTL_VBUS_CUR_LMT_MASK);
		switch(current_ma){
		case 100:
			pmu_adps_ctl_val |= (APDS_CTL_VBUS_CUR_LMT_100);
			break;
		case 300:
			pmu_adps_ctl_val |= (APDS_CTL_VBUS_CUR_LMT_300);
			break;
		case 500:
			pmu_adps_ctl_val |= (APDS_CTL_VBUS_CUR_LMT_500);
			break;
		case 800:
			pmu_adps_ctl_val |= (APDS_CTL_VBUS_CUR_LMT_800);
			break;
		default:
			return -1;
		}
		/*PMU_APDS_CTL's 15bit is discribe wrong in spec. 0-disable 1-enable */
		pmu_adps_ctl_val |= (APDS_CTL_VBUS_CONTROL_EN);
		pmu_adps_ctl_val |= (APDS_CTL_VBUS_CONTROL_CUR);
	} else {
		pmu_adps_ctl_val &= (~APDS_CTL_VBUS_CONTROL_EN);
	}

	ret = atc260x_reg_write(gl5302_PMU_APDS_CTL, pmu_adps_ctl_val);

	printf("gl5302_PMU_APDS_CTL = 0x%x\n", atc260x_reg_read(gl5302_PMU_APDS_CTL));

	return ret;
#endif
}

void charger_init(void)
{
	return  ; // TESTCODE
#if 0
	int i, len, ret, type, plug_stat, dev_node;

	printf("charger_init\n");

	/*(chenbo)-config pwm3 for ext dcdc*/
#if defined(CONFIG_ATM7059TC) || defined(CONFIG_ATM7059A)
	ret = readl(CMU_DEVCLKEN1);
	ret |= 1<<26;
	writel(ret, CMU_DEVCLKEN1);

	writel(0x1000, CMU_PWM3CLK);

	ret = readl(MFP_CTL1);
	ret &= ~0x1c000000;
	ret |= 0x0c000000;
	writel(ret, MFP_CTL1);
#elif defined(CONFIG_ATM7039C)
		ret = readl(CMU_DEVCLKEN1);
		ret |= 1<<25;
		writel(ret, CMU_DEVCLKEN1);
	
		writel(0x1000, CMU_PWM2CLK);
	
		ret = readl(MFP_CTL1);
		ret &= ~0x03800000;
		ret |= 0x01800000;
		writel(ret, MFP_CTL1);
#endif
	printf("GPIO_BOUTEN;%x, GPIO_BINEN;%x, GPIO_BDAT:%x,  MFP_CTL1:%x,PWM_CTL2:%x, PWM_CTL3:%x,CMU_DEVCLKEN1:%x, CMU_PWM2CLK:%x, CMU_PWM3CLK:%x\n",
		readl(GPIO_BOUTEN),
		readl(GPIO_BINEN),
		readl(GPIO_BDAT),
		readl(MFP_CTL1),
		readl(PWM_CTL2),
		readl(PWM_CTL3),
		readl(CMU_DEVCLKEN1),
		readl(CMU_PWM2CLK),
		readl(CMU_PWM3CLK));


	pmic_ver = atc260x_get_version();
	if(pmic_ver < 0) {
		printf(" %s atc260x_get_version failed\n",__FUNCTION__);
		return pmic_ver;
	}
	printf("PMU version: %c\n", ('A' + pmic_ver));

#ifdef CONFIG_SPL_BUILD
	bl_off_current_usb_pc = (afinfo->bl_off_current_usb_pc) * 100;
	bl_off_current_usb_adp = (afinfo->bl_off_current_usb_adp) * 100;
	bl_off_current_wall_adp = (afinfo->bl_off_current_wall_adp) * 100;
#else
	dev_node = fdtdec_next_compatible(
		gd->fdt_blob, 0,  COMPAT_ACTIONS_OWLXX_POWER);
	bl_off_current_usb_pc = fdtdec_get_int(gd->fdt_blob, dev_node, "bl_off_current_usb_pc", 500);
	bl_off_current_usb_adp = fdtdec_get_int(gd->fdt_blob, dev_node, "bl_off_current_usb_adp", 800);
	bl_off_current_wall_adp = fdtdec_get_int(gd->fdt_blob, dev_node, "bl_off_current_wall_adp", 1000);
#endif

	if(pmic_ver < 3) {
		bl_off_current_usb_pc /= 100;
		bl_off_current_usb_adp /= 100;
		bl_off_current_wall_adp /= 100;
	}else {
		get_cc_version_d(&bl_off_current_usb_pc);
		get_cc_version_d(&bl_off_current_usb_adp);
		get_cc_version_d(&bl_off_current_wall_adp);
	}

	printf("bl_off_current_usb_pc: %d \n", bl_off_current_usb_pc);
	printf("bl_off_current_usb_adp: %d \n", bl_off_current_usb_adp);
	printf("bl_off_current_wall_adp: %d \n", bl_off_current_wall_adp);

#ifdef CONFIG_SPL_BUILD
	ext_charger = afinfo->ext_charger;
	printf("ext_charger: %d\n ", ext_charger);

	if (ext_charger)
		gl5302_close_extcharger();

	support_adaptor_type= afinfo->support_adaptor_type;
	printf("support_adaptor_type: %d\n ", support_adaptor_type);
#else
	ext_charger = fdtdec_get_int(gd->fdt_blob, dev_node, "ext_charger", 0);
	if (ext_charger) {
		owlxx_fdtdec_decode_gpio(
			gd->fdt_blob, dev_node, "ext_chg_ctrl_gpio", &extchg_gpio);
		owlxx_gpio_generic_direction_output(extchg_gpio.chip, extchg_gpio.gpio, !(extchg_gpio.flags));
		printf("extchg_gpio = %d, flag = %d\n", extchg_gpio.gpio, extchg_gpio.flags);
	} 

	support_adaptor_type = fdtdec_get_int(gd->fdt_blob, dev_node, "support_adaptor_type", 0);
	printf("support_adaptor_type: %d\n ", support_adaptor_type);

	if (2 == support_adaptor_type) {
		owlxx_fdtdec_decode_gpio(
					gd->fdt_blob, dev_node, "wall_ctrl_gpio", &wallctl_gpio);
		printf("wall_ctrl_gpio: %d, flags=%d\n ", wallctl_gpio.gpio, wallctl_gpio.flags);
	}
#endif
	if (1 == support_adaptor_type) {
		atc260x_set_bits(gl5302_PMU_APDS_CTL, APDS_CTL_VBUSOTG, APDS_CTL_VBUSOTG);
	}

	ret = atc260x_bat_charger_init();

	plug_stat = atc260x_get_charge_plugin_status();

	if ((plug_stat & USB_PC) || (0 == plug_stat)) {
		printf("POWER: VBUS limit to 700, close wall!\n");
		atc260x_vbus_current_limit(1,800);
	
#ifdef CONFIG_SPL_BUILD
		gl5302_close_wall();
#else
		if (2 == support_adaptor_type)
			owlxx_gpio_generic_direction_output(wallctl_gpio.chip, wallctl_gpio.gpio, wallctl_gpio.flags);
#endif
		ret =  atc260x_reg_read(gl5302_PMU_CHARGER_CTL1);	
		ret &= ~0xf;
		ret |= bl_off_current_usb_pc;
		atc260x_reg_write(gl5302_PMU_CHARGER_CTL1, ret);
	} else {
		printf("POWER: ADPT plug in\n");
#ifdef CONFIG_SPL_BUILD
		gl5302_open_wall();
#else
		owlxx_gpio_generic_direction_output(wallctl_gpio.chip, wallctl_gpio.gpio, !(wallctl_gpio.flags));
#endif
		ret =  atc260x_reg_read(gl5302_PMU_CHARGER_CTL1);	
		ret &= ~0xf;
		ret |= bl_off_current_usb_adp;
		atc260x_reg_write(gl5302_PMU_CHARGER_CTL1, ret);
	}
#endif
}

static void atc260x_charger_turn_on(void)
{
#if 0 //TESTCODE
	printf("atc260x_charger_turn_on\n");

	atc260x_set_bits(gl5302_PMU_CHARGER_CTL0,
			PMU_CHARGER_CTL0_ENCH, 0);
	mdelay(500);

	if (afinfo->ext_charger) {
		printf("ext charger on~!\n");
#ifdef CONFIG_SPL_BUILD
		owlxx_gpio_set_value( (uint)(afinfo->extchg_gpio), (int)(afinfo->extchg_gpio_flag));
#else
		owlxx_gpio_generic_direction_output(extchg_gpio.chip, extchg_gpio.gpio,  extchg_gpio.flags);
#endif
	}
	mdelay(500);

	atc260x_set_bits(gl5302_PMU_CHARGER_CTL0,
		PMU_CHARGER_CTL0_ENCH, PMU_CHARGER_CTL0_ENCH);
	printf("inter charger on~!\n");

	atc260x_set_bits(gl5302_PMU_CHARGER_CTL2,
		PMU_CHARGER_CTL2_CV_SET, PMU_CHARGER_CTL2_CV_SET);
#endif
}

static void atc260x_charger_turn_off(void)
{
#if 0 // TESTOCDE
	printf("atc260x_charger_turn_off\n");

	if (ext_charger) {
#ifdef CONFIG_SPL_BUILD
		owlxx_gpio_set_value( (uint)(afinfo->extchg_gpio), !(afinfo->extchg_gpio_flag));
#else
		owlxx_gpio_generic_direction_output(extchg_gpio.chip, extchg_gpio.gpio,  !(extchg_gpio.flags));
#endif
	}

	atc260x_set_bits(gl5302_PMU_CHARGER_CTL0, PMU_CHARGER_CTL0_ENCH, 0);
	atc260x_set_bits(gl5302_PMU_CHARGER_CTL2, PMU_CHARGER_CTL2_CV_SET, 0);
#endif
}

static unsigned int get_pwm_level(unsigned int curve_arg0, unsigned int curve_arg1, unsigned int vbat, unsigned int bat_added)
{
#define PWM_MAX_LEVEL 63
	unsigned int level = 0;

	if(vbat<=(curve_arg1-bat_added)) {
		return 0;
	}

	level = (curve_arg0*(vbat+bat_added-curve_arg1))/(1000*10000);
	if(level > PWM_MAX_LEVEL) {
		level = PWM_MAX_LEVEL;
	}
	return level;
}   


int charger_monitor(void)
{
	return 0; // TESTCODE
#if 0
	int ret, bat_cap, key_stat, plug_stat, bat_v, bat_curr, charger_flag = 0, charger_en = 0, guaue_type;
	int pwm_level;

	charger_init();

	bat_cap = bq27441_gasgauge_get_capacity();
	if (bat_cap < 0 || bat_cap > 100) {
		guaue_type = 0;
		bat_v = get_battery_voltage();
		printf("bat_v: %d\n", bat_v);
		if (bat_v < 4200)
			charger_flag = 1;
	} else {
		guaue_type = 1;
		if (bat_cap < 100)
			charger_flag = 1;
	}

	atc260x_set_bits(gl5302_PMU_SYS_CTL2, (0x3 << 10), (0x2 << 10));

	do {
		if (charger_flag && (0 == charger_en)) {
			atc260x_charger_turn_on();
			charger_en = 1;
		}

		ret = atc260x_reg_read(gl5302_PMU_SYS_CTL2); 
		key_stat = ret & (ONOFF_SHORT_PRESS |ONOFF_LONG_PRESS);
		if (key_stat == 0x4000) {
			printf("1.key_stat = 0x%x\n", key_stat);
			if (charger_en)
				atc260x_set_bits(gl5302_PMU_CHARGER_CTL0,
					PMU_CHARGER_CTL0_ENCH, 0);
			/*show charger picture*/	
			printf("show picture when minicharger\n");
			if (charger_en)
				atc260x_set_bits(gl5302_PMU_CHARGER_CTL0,
					PMU_CHARGER_CTL0_ENCH, PMU_CHARGER_CTL0_ENCH);	
			atc260x_set_bits(gl5302_PMU_SYS_CTL2,
				ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS,
				ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
			return -1;
		} else if (key_stat == 0x2000) {
			printf("~~restart, key_stat = 0x%x\n", key_stat);
			atc260x_set_bits(gl5302_PMU_SYS_CTL2,
				ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS,
				ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
			atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x3, 0x1);
			ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
			printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
			mdelay(500);
			atc260x_shutoff();
		}

		if (++chg_cycle > 10) {
			/*(chenbo)-pwm adjust*/
			pwm_level = get_pwm_level(config_curve_arg0, config_curve_arg1, bat_v, 400);
#ifdef CONFIG_ATM7059TC
			writel(pwm_level, PWM_CTL3);
			printf("PWM_CTL3:%x\n", readl(PWM_CTL3));
#endif
#ifdef CONFIG_ATM7039C
			writel(pwm_level, PWM_CTL2);
			printf("PWM_CTL2:%x\n", readl(PWM_CTL2));
#endif
			
			
			if (0 == guaue_type) {
				bat_cap = soft_gasgauge_get_capacity();
				ret =  atc260x_reg_read(gl5302_PMU_BATVADC);
				if (ret) {
					ret *= 2930 * 2;
					bat_v = ret/1000;
				}

				ret = atc260x_reg_read(gl5302_PMU_CHGIADC);
				if (ret) {
					bat_curr = (ret * 1500 / 1024);
				}
#if 0
				bat_vol_record[bat_vol_record_point++] = bat_v;
				bat_vol_record_sum += bat_v;
				if (5 == bat_vol_record_point) {
					bat_vol = bat_vol_record_sum/5;

					bat_vol_record_sum -= bat_vol_record[0];
					bat_vol_record[0] = bat_vol_record[1];
					bat_vol_record[1] = bat_vol_record[2];
					bat_vol_record[2] = bat_vol_record[3];
					bat_vol_record[3] = bat_vol_record[4];
					bat_vol_record_point = 4;
				}
#endif
			} else if (1 == guaue_type) {
				bat_cap = bq27441_gasgauge_get_capacity();
				bat_v = bq27441_gasgauge_get_voltage();
				bat_curr = bq27441_gasgauge_get_curr();
			} else 
				printf("guaue_type is error\n");

			printf("bat_cap=%d, bat_v=%d, bat_curr=%d\n", bat_cap, bat_v, bat_curr);

			plug_stat = atc260x_get_charge_plugin_status();
			if (0 == plug_stat) {
				if (1 == charger_en)
					atc260x_charger_turn_off();
				printf("unplug when chargering, shut dowm!\n");
				atc260x_set_bits(gl5302_PMU_UV_INT_EN, 0x3, 0x0);
				ret = atc260x_reg_read(gl5302_PMU_UV_INT_EN);
				 printf("gl5302_PMU_UV_INT_EN: 0x%x\n", ret);
				atc260x_shutoff();
			}

			chg_cycle = 0;
		}
		
		if (100 == bat_cap ) {
			if (1 == guaue_type) {
				if (bat_curr < 350 && bat_v > 4180) {
					atc260x_charger_turn_off();
					charger_flag = charger_en = 0;
				}
			}

			if (0 == guaue_type) {
				if (bat_curr < 50 && bat_v > 4180) {
					atc260x_charger_turn_off();
					charger_flag = charger_en = 0;
				}
			}
		} else {
			charger_flag = 1;
		}

		mdelay(500);

	} while(1);

	return 0;
#endif
}
#endif
#else
int low_power_dect(void)
{
	return 0;
}
#endif

/***********************/

struct owlxx_regulator regulator_list[MAX_REGULATOR_NUM];

int owlxx_regulator_enable(struct owlxx_regulator *regulator)
{
	int ret = 0;

	if (regulator && regulator->ops->enable)
		ret = regulator->ops->enable(regulator->id);

	return ret;
}

int owlxx_regulator_disable(struct owlxx_regulator *regulator)
{
	int ret = 0;

	if (regulator && regulator->ops->disable)
		ret = regulator->ops->disable(regulator->id);

	return ret;
}

struct owlxx_regulator *owlxx_regulator_get_by_node(int dev_node)
{
	int i;

	for (i = 0; i < MAX_REGULATOR_NUM; i++) {
		if (regulator_list[i].dev_node == dev_node)
			return &regulator_list[i];
	}

	return NULL;
}

struct owlxx_regulator *owlxx_regulator_get_by_id(int dev_node)
{
	int i;

	for (i = 0; i < MAX_REGULATOR_NUM; i++) {
		if (regulator_list[i].dev_node == dev_node)
			return &regulator_list[i];
	}

	return NULL;
}

struct owlxx_regulator *fdtdec_owlxx_regulator_get(const void *blob, int node,
		const char *prop_name)
{
	int regulator_node = 0;

	regulator_node = fdtdec_lookup_phandle(blob, node, prop_name);

	return owlxx_regulator_get_by_node(regulator_node);
}



int owlxx_regulator_add(int id, int dev_node, struct owlxx_regulator_ops *ops)
{
	int i;
	int find_slot = 0;
	struct owlxx_regulator *regulator;

	for (i = 0; i < MAX_REGULATOR_NUM; i++) {
		if (regulator_list[i].id == 0) {
			find_slot = 1;
			break;
		}
	}

	if (find_slot) {
		regulator = &regulator_list[i];
		regulator->id = id;
		regulator->dev_node = dev_node;
		regulator->ops = ops;
	} else {
		return -1;
	}

	return 0;
}

int atc260x_switchldo_enable(int id)
{
	return 0;

#if 0
	int ret = -1;

	if (id == ATC260X_REGULATOR_SWITCHLDO(1)) {
		ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
			PMU_SWITCH_CTL_SWITCH1_EN,
			PMU_SWITCH_CTL_SWITCH1_EN);

	} else if (id == ATC260X_REGULATOR_SWITCHLDO(2)) {
		ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
				PMU_SWITCH_CTL_SWITCH2_EN,
				PMU_SWITCH_CTL_SWITCH2_EN);
	}

	return ret;
#endif
}

int atc260x_switchldo_disable(int id)
{
#if 0 // TESTCODE
	int ret = -1;

	if (id == ATC260X_REGULATOR_SWITCHLDO(1)) {
		ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
			PMU_SWITCH_CTL_SWITCH1_EN,
			0);

	} else if (id == ATC260X_REGULATOR_SWITCHLDO(2)) {
		ret = atc260x_set_bits(gl5302_PMU_SWITCH_CTL,
				PMU_SWITCH_CTL_SWITCH2_EN,
				0);
	}

	return ret;
#endif
}

struct owlxx_regulator_ops atc260x_switchldo_ops = {
	.enable = atc260x_switchldo_enable,
	.disable = atc260x_switchldo_disable,
};

int atc260x_regulators_init(void)
{
	int node;
	int depth;
	int i;

	node = fdtdec_next_compatible(gd->fdt_blob,
		0, COMPAT_ACTIONS_ATC260X);
	if (node <= 0) {
		debug("Can't get ATC260X device node\n");
		return -1;
	}

	depth = 0;
	for (i = 1; ; i++) {
		node = fdtdec_next_compatible_subnode(gd->fdt_blob, node,
				COMPAT_ACTIONS_ATC260X_SWITCHLDO, &depth);
		if (node < 0)
			break;

		owlxx_regulator_add(ATC260X_REGULATOR_SWITCHLDO(i), node,
					&atc260x_switchldo_ops);
	}



}
