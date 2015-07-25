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

#ifndef __ATM7059A_UPGRADE_H
#define __ATM7059A_UPGRADE_H

#include <configs/atm7059a_upgrade_spl.h>

/* DDR test in SPL stage */
#undef CONFIG_SPL_MTEST
#undef CONFIG_ENV_IS_IN_FAT

#define CONFIG_SPL_OWLXX_UPGRADE
#define CONFIG_ENV_IS_NOWHERE		1

#define CONFIG_EXTRA_ENV_SETTINGS				\
	CONFIG_COMMON_ENV_SETTINGS

#define CONFIG_ADFUBOOTCOMMAND				\
	"adfudec; bootm  0x7fc0 0x1ffffc0 ${fdtaddr}"

#define CONFIG_BOOTCOMMAND CONFIG_ADFUBOOTCOMMAND

#endif
