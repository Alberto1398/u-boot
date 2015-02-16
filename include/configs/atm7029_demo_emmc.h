/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_ATM7029_DEMO_UPGRADE_H__
#define __CONFIG_ATM7029_DEMO_UPGRADE_H__

#include <asm/mach-types.h>

#define CONFIG_ACTS_FOR_BOOT
#define CONFIG_ACTS_STORAGE_EMMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_ACTS_GL520X_MMC

#define CONFIG_EXTRA_ENV_SETTINGS CONFIG_ACT_COMMON_ENV_SETTINGS \
        ""

#define CONFIG_BOOTCOMMAND      "act_boot"

#define CONFIG_ACTS_CMD_ACTBOOT


#include <configs/atm7029_common.h>     /* place at last */

#endif /* __CONFIG_ATM7029_DEMO_UPGRADE_H__ */
