/*
* Power battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __POWER_BATTERY__
#define __POWER_BATTERY__

struct power_battery {
	int (*measure_voltage)(int *batv);
	int (*calc_soc)(void);
	int (*chk_online)(void);
	int (*reset)(void);
};

/*
 * battery power status.
 * @BAT_PWR_CRICIAL : bat vol < 3.3v.
 * @BAT_PWR_LOW : bat soc <= 7% and bat vol >= 3.3v.
 * @BAT_PWR_HIGH : bat soc > 7% and bat vol >= 3.3v.
 */
enum BAT_PWR_STATUS {
	BAT_PWR_CRICIAL,
	BAT_PWR_LOW,
	BAT_PWR_HIGH,
};

void power_battery_register(struct power_battery *bat);
int power_battery_measure_voltage(int *batv);
int power_battery_check_online(void);
int power_battery_check_pwr(void);
int power_battery_init(void);
int power_battery_reset(void);
#endif
