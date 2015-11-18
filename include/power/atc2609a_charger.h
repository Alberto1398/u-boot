/*
* Actions ATC2609A PMIC charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ATC2609A_CHARGER_H__
#define __ATC2609A_CHARGER_H__
/*resistor sensor value, unit : mohm*/
enum RSENSE_VALUE {
	RSENSE_10mohm = 10,
	RSENSE_20mohm = 20
};

enum CHANNEL{
	WALLV,
	VBUSV,
};

/***************************ATC2609A****************************/
void atc2609a_charger_init(void);
int atc2609a_charger_release_guard(void);
int atc2609a_charger_get_rsense(void);
int atc2609a_charger_set_rsense(int rsense);
void atc2609a_charger_set_onoff(int enable);
#endif
