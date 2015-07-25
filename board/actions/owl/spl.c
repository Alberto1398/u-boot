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

#include <common.h>
#include <spl.h>

u32 spl_boot_device(void)
{
#if defined(CONFIG_SPL_OWLXX_UPGRADE)
	return BOOT_DEVICE_UPGRADE;
#endif
#if defined(CONFIG_SPL_RAWMMC_SUPPORT)
	return BOOT_DEVICE_RAWMMC;
#endif
#if defined(CONFIG_SPL_RAWNAND_SUPPORT)
	return BOOT_DEVICE_RAWFLASH;
#endif
	puts("Unknown boot device, fallback to RAM\n");
	return BOOT_DEVICE_RAM;
}

#ifdef CONFIG_SPL_USE_IRQ
static inline int spl_interrupt_init(void)
{
	/* setup up stacks if necessary */
	IRQ_STACK_START = CONFIG_SPL_IRQ_STACK - 4;
	IRQ_STACK_START_IN = CONFIG_SPL_IRQ_STACK + 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;

	return arch_interrupt_init();
}
#else
#define spl_interrupt_init()
#define enable_interrupts()
#endif

void spl_board_init(void)
{
	spl_interrupt_init();
	enable_interrupts();
}
