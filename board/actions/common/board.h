/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __OWL_BOARD_COMMON_H__
#define __OWL_BOARD_COMMON_H__

#define POWER_NORMAL			0
#define POWER_LOW			1
#define POWER_CRITICAL_LOW		2
#define POWER_CRITICAL_LOW_CHARGER	3
#define POWER_NORMAL_CHARGER		4
#define POWER_EXCEPTION			5

int owl_get_power_status(void);
int owl_power_init(void);
int owl_get_recovery_mode(void);
void owl_reset_to_adfu(void);
int owl_board_late_init(void);
int owl_check_enter_shell(void);

#ifdef CONFIG_POWER_BATTERY
int owl_battery_reset(void);
#endif

#ifdef CONFIG_CHECK_KEY
int owl_check_key(void);
#endif

#ifdef CONFIG_CHECK_POWER
int owl_check_power(void);
#endif

#endif	/* __OWL_BOARD_COMMON_H__ */
