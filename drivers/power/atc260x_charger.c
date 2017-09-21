/*
* Actions ATC260X PMIC charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/power_charger.h>
#include "atc260x_charger.h"
#ifdef CONFIG_ATC2609A_CHARGER
#include "atc2609a_charger.h"
#endif
#ifdef CONFIG_ATC2603C_CHARGER
#include "atc2603c_charger.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

static struct atc260x_charger_info info = {.charger_online = ADAPTER_TYPE_NO_PLUGIN};

static struct power_charger_ops *charger_ops;
void atc260x_charger_register(struct power_charger_ops *ops)
{
	charger_ops = ops;
}

int atc260x_charger_check_online(void)
{
	if (charger_ops->chk_charger_online) {
		info.charger_online = charger_ops->chk_charger_online();
		return info.charger_online;
	} else
		printf("chk_charger_online undefined!\n");
	
	return -1;
}

int atc260x_charger_get_rsense(void)
{
	if (charger_ops->get_rsense)
		return charger_ops->get_rsense();
	else
		printf("get_rsense undefined!\n");
	
	return -1;
}

int atc260x_charger_release_guard(void)
{
	if (charger_ops->release_guard)
		return charger_ops->release_guard();
	else
		printf("release_guard undefined!\n");
	
	return -1;
}

int atc260x_chk_bat_online_intermeddle(void)
{	
	if (charger_ops->chk_bat_online_intermeddle)
		return charger_ops->chk_bat_online_intermeddle();
	else
		printf("check bat online intermeddle undefined!\n");
	
	return -1;
}

static struct power_charger_ops ops = {
	.get_rsense = atc260x_charger_get_rsense,
	.release_guard = atc260x_charger_release_guard,
	.chk_bat_online_intermeddle = atc260x_chk_bat_online_intermeddle,
	.chk_charger_online = atc260x_charger_check_online,
};

int atc260x_charger_init(const void *blob)
{
	int ret;

#if defined CONFIG_ATC2609A_CHARGER
	atc2609a_charger_init(blob);
#elif defined CONFIG_ATC2603C_CHARGER
	atc2603c_charger_init(blob);
#else
	
	printf("charger int undefined!\n ");
	return -1;
#endif
	atc260x_charger_check_online();
	power_charger_register(&ops);

	return 0;
}
