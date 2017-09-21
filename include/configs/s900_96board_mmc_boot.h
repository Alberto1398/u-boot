/*
 * Copyright (C) 2015 Actions Semi Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_S900_96BOARD_MMC_BOOT_H__
#define __CONFIG_S900_96BOARD_MMC_BOOT_H__
#include "s900_common.h"
#include "s900_96board.h"

#define CONFIG_IDENT_STRING	 "S900 96BOARD"

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_SETTINGS_COMMON \
					"devif=mmc\0"			 \
					"bootdisk=0\0"

#define CONFIG_BOOTCOMMAND		"run mmcboot;"
#define CONFIG_BOOTDELAY		1	/* autoboot after 1 seconds */


#define CONFIG_OWL_CARD_BURN

#endif /* __CONFIG_ATM9009_EVB_H */
