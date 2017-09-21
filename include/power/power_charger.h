/*
* POWER charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __POWER_CHARGER_H__
#define __POWER_CHARGER_H__

/*resistor sensor value, unit : mohm*/
enum RSENSE_VALUE {
	RSENSE_10mohm = 10,
	RSENSE_20mohm = 20
};

struct power_charger_ops {
	int (*get_rsense)(void);
	int (*release_guard)(void);
	int (*chk_bat_online_intermeddle)(void);
	int (*chk_charger_online)(void);
};

#define ADAPTER_TYPE_NO_PLUGIN		0
#define ADAPTER_TYPE_WALL_PLUGIN	(1 << 0)
#define ADAPTER_TYPE_USB_PLUGIN		(1 << 1)

void power_charger_register(struct power_charger_ops *ops);
int power_charger_init(void);
int power_charger_get_rsense(void);
int power_charger_release_guard(void);
int power_charger_check_online(void);
int power_chk_bat_online_intermeddle(void);
#endif
