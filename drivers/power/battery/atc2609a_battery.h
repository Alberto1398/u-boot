/*
* Actions ATC2609A PMIC battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ATC2609A_BATTERY_H__
#define __ATC2609A_BATTERY_H__

int atc2609a_bat_init(const void *blob);
int atc2609a_bat_check_online(void);
int atc2609a_bat_calc_soc(void);
int atc2609a_bat_measure_vol_avr(int *batv);

#endif
