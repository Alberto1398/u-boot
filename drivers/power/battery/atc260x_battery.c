/*
* Actions ATC260X PMIC battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/power_battery.h>
#include "atc2609a_battery.h"

static struct power_battery *battery;

void atc260x_bat_register(struct power_battery *bat)
{
	battery = bat;
}

static int atc260x_bat_measure_voltage(int *batv)
{
	if (!battery || !battery->measure_voltage) {
		printf("[%s] battery ptr or measure_voltage err!\n",
			__func__);
		return -1;
	}

	return battery->measure_voltage(batv);
}

static int atc260x_bat_calc_soc(void)
{
	if (!battery || !battery->calc_soc) {
		printf("[%s] battery ptr or calc_soc err!\n",
			__func__);
		return -1;
	}

	return battery->calc_soc();
}

static int atc260x_bat_check_online(void)
{
	if (!battery || !battery->chk_online) {
		printf("[%s] battery ptr or chk_online err!\n",
			__func__);
		return -1;
	}

	return battery->chk_online();
}


static struct power_battery atc260x_bat = {
	.measure_voltage = atc260x_bat_measure_voltage,
	.chk_online = atc260x_bat_check_online,
	.calc_soc = atc260x_bat_calc_soc,
};

int atc260x_bat_init(const void *blob)
{
	int ret = -1;

#ifdef CONFIG_POWER_BATTERY_ATC2609A
	ret = atc2609a_bat_init(blob);
#endif
	if (!ret)
		power_battery_register(&atc260x_bat);

	return ret;
}


