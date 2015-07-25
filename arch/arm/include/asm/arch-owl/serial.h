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

#ifndef __ASM_SERIAL_H
#define __ASM_SERIAL_H

#define UART0_BASE			0xB0120000
#define UART0_CTL			(UART0_BASE + 0x0000)
#define UART0_RXDAT			(UART0_BASE + 0x0004)
#define UART0_TXDAT			(UART0_BASE + 0x0008)
#define UART0_STAT			(UART0_BASE + 0x000c)

#define UART1_BASE			0xB0122000
#define UART1_CTL			(UART1_BASE + 0x0000)
#define UART1_RXDAT			(UART1_BASE + 0x0004)
#define UART1_TXDAT			(UART1_BASE + 0x0008)
#define UART1_STAT			(UART1_BASE + 0x000c)

#define UART2_BASE			0xB0124000
#define UART2_CTL			(UART2_BASE + 0x0000)
#define UART2_RXDAT			(UART2_BASE + 0x0004)
#define UART2_TXDAT			(UART2_BASE + 0x0008)
#define UART2_STAT			(UART2_BASE + 0x000c)

#define UART3_BASE			0xB0126000
#define UART3_CTL			(UART3_BASE + 0x0000)
#define UART3_RXDAT			(UART3_BASE + 0x0004)
#define UART3_TXDAT			(UART3_BASE + 0x0008)
#define UART3_STAT			(UART3_BASE + 0x000c)

#define UART4_BASE			0xB0128000
#define UART4_CTL			(UART4_BASE + 0x0000)
#define UART4_RXDAT			(UART4_BASE + 0x0004)
#define UART4_TXDAT			(UART4_BASE + 0x0008)
#define UART4_STAT			(UART4_BASE + 0x000c)

#define UART5_BASE			0xB012a000
#define UART5_CTL			(UART5_BASE + 0x0000)
#define UART5_RXDAT			(UART5_BASE + 0x0004)
#define UART5_TXDAT			(UART5_BASE + 0x0008)
#define UART5_STAT			(UART5_BASE + 0x000c)

#define UART6_BASE			0xB012c000
#define UART6_CTL			(UART6_BASE + 0x0000)
#define UART6_RXDAT			(UART6_BASE + 0x0004)
#define UART6_TXDAT			(UART6_BASE + 0x0008)
#define UART6_STAT			(UART6_BASE + 0x000c)

#endif

