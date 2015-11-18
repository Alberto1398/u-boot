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

#define ADAPTER_TYPE_NO_PLUGIN		0
#define ADAPTER_TYPE_WALL_PLUGIN	(1 << 0)
#define ADAPTER_TYPE_USB_PLUGIN		(1 << 1)

int atc260x_charger_init(void);
int atc260x_charger_release_guard(void);
int atc260x_charger_check_online(void);
#endif