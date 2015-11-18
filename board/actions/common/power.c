/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <power/power_battery.h>
#include <power/atc260x_charger.h>
#include <power/atc260x/owl_atc260x.h>
#include "board.h"

static int g_power_status = -1;

#ifdef CONFIG_POWER_BATTERY
static int __get_power_status(void)
{
	int charger_online;
	int bat_pwr;
	int bat_online;
	unsigned int value;

	bat_online = power_battery_check_online();
	if (bat_online < 0)
		return POWER_EXCEPTION;

	if (bat_online == 0)
		return POWER_NORMAL;

	charger_online = atc260x_charger_check_online();
	bat_pwr = power_battery_check_pwr();

	/* adapter is offline */
	if (charger_online == ADAPTER_TYPE_NO_PLUGIN) {
		if (bat_pwr == BAT_PWR_HIGH)
			return POWER_NORMAL;
		else if (bat_pwr == BAT_PWR_LOW)
			return POWER_LOW;
		else if (bat_pwr == BAT_PWR_CRICIAL)
			return POWER_CRITICAL_LOW;
		else
			return POWER_NORMAL;
	}

	/* adapter is online */
	atc260x_pstore_get(ATC260X_PSTORE_TAG_DIS_MCHRG, &value);
	if (value) {
		/*if onoff 3s restart, leave minicharge*/
		atc260x_pstore_set(ATC260X_PSTORE_TAG_DIS_MCHRG, 0);
		return POWER_NORMAL;
	}

	/* only usb plug in,but bat pwr is critial,
	 * goto minicharge but not light on screen
	 */
	if ((charger_online == ADAPTER_TYPE_USB_PLUGIN) &&
		(bat_pwr == BAT_PWR_CRICIAL))
			return POWER_CRITICAL_LOW_CHARGER;

	return POWER_NORMAL_CHARGER;
}
#else
static int __get_power_status(void)
{
	return POWER_NORMAL;
}
#endif

static int owl_init_power_status(void)
{
	g_power_status = __get_power_status();
	printf("%s: power_status: %d\n", __func__, g_power_status);

	return 0;
}

int owl_get_power_status(void)
{
	return g_power_status;
}

void board_poweroff(void)
{
	atc260x_misc_set_wakeup_src(ATC260X_WAKEUP_SRC_ALL,
				ATC260X_WAKEUP_SRC_RESET |
				ATC260X_WAKEUP_SRC_ONOFF_LONG |
				ATC260X_WAKEUP_SRC_IR |
				ATC260X_WAKEUP_SRC_WALL_IN |
				ATC260X_WAKEUP_SRC_VBUS_IN);
	atc260x_misc_pwrdown_machine(1);

	hang();
}

void owl_reset_to_adfu(void)
{
	printf("reset to adfu\n");
	atc260x_misc_reset_machine(OWL_PMIC_REBOOT_TGT_ADFU);
	hang();
}

#ifdef CONFIG_CHECK_POWER
int owl_check_power(void)
{
	int status;

	status = owl_get_power_status();
	if (status == BAT_PWR_LOW || status == POWER_CRITICAL_LOW) {
		printf("power is low, shutdown the machine!\n");

		/* show low power logo */
		if (status == BAT_PWR_LOW)
			mdelay(1000);

		board_poweroff();
	}

	return 0;
}
#endif

int owl_power_init(void)
{
	int ret;

	ret = owl_pmic_init();
	if (ret) {
		printf(" PMU Not Initilize\n ");
		return ret;
	}

#ifdef CONFIG_ATC260X_CHARGER
	ret = atc260x_charger_init();
	if (ret) {
		printf(" CHARGER Not Initilize\n ");
		return ret;
	}
#endif

#ifdef CONFIG_POWER_BATTERY
	ret = power_battery_init();
	if (ret) {
		printf(" BATTERY Not Initilize\n ");
		return ret;
	}
#endif

	owl_init_power_status();

	return 0;
}

#ifdef CONFIG_POWER_BATTERY
int owl_battery_reset(void)
{
	int reset;

	atc260x_pstore_get(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET,
		&reset);
	if (!reset) {
		power_battery_reset();
		atc260x_pstore_set(ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET, 1);
	}

	return 0;
}
#endif