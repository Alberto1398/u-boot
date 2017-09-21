/*
* Actions POWER charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <power/power_charger.h>
#include "atc260x_charger.h"

DECLARE_GLOBAL_DATA_PTR;

static struct power_charger_ops *charger_ops;
static int online;
void power_charger_register(struct power_charger_ops *ops)
{
	charger_ops = ops;
}

int power_charger_check_online(void)
{
	if (charger_ops->chk_charger_online) {
		online = charger_ops->chk_charger_online();
		return online;
	} else
		printf("%s chk_charger_online undefined!\n", __func__);
	
	return -1;
}

int power_charger_get_rsense(void)
{
	if (charger_ops->get_rsense)
		return charger_ops->get_rsense();
	else
		printf("%s get_rsense undefined!\n", __func__);
	
	return -1;
}

int power_charger_release_guard(void)
{
	if (charger_ops->release_guard)
		return charger_ops->release_guard();
	else
		printf("%s release_guard undefined!\n", __func__);
	
	return -1;
}

int power_chk_bat_online_intermeddle(void)
{	
	if (charger_ops->chk_bat_online_intermeddle)
		return charger_ops->chk_bat_online_intermeddle();
	else
		printf("%s check bat online intermeddle undefined!\n", __func__);
	
	return -1;
}

static void power_charger_dump(void)
{
	printf("CHARGER:(%s %s)online\n",
		(online & ADAPTER_TYPE_WALL_PLUGIN) ? "AC" : "noAC",
		(online & ADAPTER_TYPE_USB_PLUGIN) ? " USB" : "noUSB");
}
int power_charger_init(void)
{

#if defined CONFIG_ATC260X_CHARGER
	atc260x_charger_init(gd->fdt_blob);
#else
	printf("charger int undefined!\n ");
	return -1;
#endif
	power_charger_check_online();
	power_charger_dump();
	return 0;
}
