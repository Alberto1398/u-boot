/*
* ATC260X PMIC battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ATC260X_CHARGER_H__
#define __ATC260X_CHARGER_H__
#include <asm-generic/gpio.h>
struct power_charger_ops;

#define ATC260X_VBUS_VOL_THRESHOLD	(3900)
#define ATC260X_WALL_VOL_THRESHOLD	(3400)

enum CHANNEL{
	WALLV,
	VBUSV,
};

enum  SUPPORT_ADAPTER {
	SUPPORT_DCIN_ONLY = 0x1,
	SUPPORT_USB_ONLY = 0x2,
	SUPPORT_DCIN_USB = 0x3
};

struct atc260x_charger_data {
	int rsense;
	int support_adaptor_type;
	int usb_pc_ctl_mode;
};

struct atc260x_charger_info {
	struct atc260x_charger_data data;
	struct gpio_desc wall_switch;
	int charger_online;
	int bat_online;
	int charge_detected;
	int bat_detected;
};

void atc260x_charger_register(struct power_charger_ops *ops);
int atc260x_charger_init(const void *blob);
int atc260x_charger_get_rsense(void);
int atc260x_charger_release_guard(void);
int atc260x_charger_check_online(void);
int atc260x_chk_bat_online_intermeddle(void);
#endif
