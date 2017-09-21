/*
 * Copyright (C) 2015 Actions Semi Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_S900_UPGRADE_H__
#define __CONFIG_S900_UPGRADE_H__
#include "s900_common.h"

#define CONFIG_IDENT_STRING	 "S900 upgrade"

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_SETTINGS_COMMON

#define CONFIG_BOOTCOMMAND		"run ramboot;"
#define CONFIG_BOOTDELAY		0	/* autoboot after 1 seconds */

#endif /* __CONFIG_ATM9009_EVB_H */
