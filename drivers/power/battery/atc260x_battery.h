/*
* Actions ATC260X PMIC battery driver
*
* Copyright (c) 2015 Actions Semiconductor Co., Ltd.
* Terry Chen chenbo@actions-semi.com
*
* SPDX-License-Identifier:	GPL-2.0+
*/
#ifndef __ATC260X_BATTERY_H__
#define __ATC260X_BATTERY_H__

void atc260x_bat_register(struct power_battery *bat);
int atc260x_bat_init(const void *blob);

#endif

