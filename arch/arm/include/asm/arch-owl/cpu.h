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

#ifndef __ASM_ARCH_CPU_H
#define __ASM_ARCH_CPU_H

#define TIMER_2HZ_BASE			0xB0168000
#define TWOHZ0_CTL			(TIMER_2HZ_BASE + 0x0000)
#define T0_CTL				(TIMER_2HZ_BASE + 0x0008)
#define T0_CMP				(TIMER_2HZ_BASE + 0x000C)
#define T0_VAL				(TIMER_2HZ_BASE + 0x0010)
#define T1_CTL				(TIMER_2HZ_BASE + 0x0014)
#define T1_CMP				(TIMER_2HZ_BASE + 0x0018)
#define T1_VAL				(TIMER_2HZ_BASE + 0x001C)
#define TWOHZ1_CTL			(TIMER_2HZ_BASE + 0x0020)
#define USB3_P0_CTL			(TIMER_2HZ_BASE + 0x0080)
#define USB3_P1_CTL			(TIMER_2HZ_BASE + 0x0084)
#define USB2_ECS			(TIMER_2HZ_BASE + 0x0088)

/* CHIPID */
#define CHIPID_BASE			0xB0150000
#define CHIPID_CTRL0			(CHIPID_BASE + 0x0000)
#define CHIPID_DAT_1			(CHIPID_BASE + 0x0004)
#define CHIPID_CTRL1			(CHIPID_BASE + 0x0008)
#define CHIPID_CTRL2			(CHIPID_BASE + 0x000C)

#define N_IRQS				(21)

#endif

