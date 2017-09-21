/*
* Actions power battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <power/power_battery.h>
#ifdef CONFIG_POWER_BATTERY_ATC260X
#include "atc260x_battery.h"
#endif
#ifdef CONFIG_POWER_BATTERY_BQ27441
#include "bq27441_battery.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define BATTERY_LOW_SOC				(7*1000)
#define BAT_VOL_CRITIAL_THRESHOLD		(3300)

struct power_battery *pwr_bat;

void power_battery_register(struct power_battery *bat)
{
	pwr_bat = bat;
}

int power_battery_measure_voltage(int *batv)
{
	if (!pwr_bat || !pwr_bat->measure_voltage) {
		printf("[%s] pwr_bat ptr or measure_voltage err!\n", __func__);
		return -1;
	}

	return pwr_bat->measure_voltage(batv);
}

int power_battery_check_online(void)
{
	if (!pwr_bat || !pwr_bat->chk_online) {
		printf("[%s] pwr_bat ptr or chk_online err!\n", __func__);
		return -1;
	}

	return pwr_bat->chk_online();
}

static int power_battery_calc_soc(void)
{
	if (!pwr_bat || !pwr_bat->calc_soc) {
		printf("[%s] pwr_bat ptr or calc_soc err!\n", __func__);
		return -1;
	}

	return pwr_bat->calc_soc();
}

int power_battery_reset(void)
{
	if (!pwr_bat || !pwr_bat->reset) {
		printf("[%s] pwr_bat ptr or reset err!\n", __func__);
		return -1;
	}

	return pwr_bat->reset();
}

int power_battery_check_pwr(void)
{
	int bat_mv;
	int soc;
	int ret;

	ret = power_battery_measure_voltage(&bat_mv);
	if (ret < 0) {
		printf("[%s] get batv err!\n", __func__);
		return BAT_PWR_CRICIAL;
	}
	/*must calc soc, othersise could not get 1st soc in kernel*/
	soc = power_battery_calc_soc();
	if (soc < 0) {
		printf("======bat power err, soc(%d)=====\n", soc);
		return BAT_PWR_CRICIAL;
	}

	if (bat_mv <= BAT_VOL_CRITIAL_THRESHOLD) {
		debug("======critial(%dmv,%d%%) =====\n", bat_mv, soc);
		return BAT_PWR_CRICIAL;
	}

	if (soc >= BATTERY_LOW_SOC) {
		debug("======enouth, soc(%d)=====\n", soc);
		return BAT_PWR_HIGH;
	} else {
		debug("======low,soc(%d)=====\n", soc);
		return BAT_PWR_LOW;
	}
}

int power_battery_init(void)
{
	int ret = -1;

#if defined CONFIG_POWER_BATTERY_ATC260X
	ret = atc260x_bat_init(gd->fdt_blob);
	debug("gauge type : ATC260X_GAUGE\n");
#elif defined CONFIG_POWER_BATTERY_BQ27441
	ret = bq27441_battery_init(gd->fdt_blob);
	debug("gauge type : BQ2741_GAUGE\n");
#else
	debug("gauge type : undefined!!!\n");
	return -1;
#endif

	return ret;
}