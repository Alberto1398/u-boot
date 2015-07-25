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

#ifndef __ASM_I2C_H
#define __ASM_I2C_H

/* I2C0 */
#define I2C0_BASE			0xB0170000
#define I2C0_CTL			(I2C0_BASE + 0x0000)
#define I2C0_CLKDIV			(I2C0_BASE + 0x0004)
#define I2C0_STAT			(I2C0_BASE + 0x0008)
#define I2C0_ADDR			(I2C0_BASE + 0x000C)
#define I2C0_TXDAT			(I2C0_BASE + 0x0010)
#define I2C0_RXDAT			(I2C0_BASE + 0x0014)
#define I2C0_CMD			(I2C0_BASE + 0x0018)
#define I2C0_FIFOCTL			(I2C0_BASE + 0x001C)
#define I2C0_FIFOSTAT			(I2C0_BASE + 0x0020)
#define I2C0_DATCNT			(I2C0_BASE + 0x0024)
#define I2C0_RCNT			(I2C0_BASE + 0x0028)

/* I2C1 */
#define I2C1_BASE			0xB0174000
#define I2C1_CTL			(I2C1_BASE + 0x0000)
#define I2C1_CLKDIV			(I2C1_BASE + 0x0004)
#define I2C1_STAT			(I2C1_BASE + 0x0008)
#define I2C1_ADDR			(I2C1_BASE + 0x000C)
#define I2C1_TXDAT			(I2C1_BASE + 0x0010)
#define I2C1_RXDAT			(I2C1_BASE + 0x0014)
#define I2C1_CMD			(I2C1_BASE + 0x0018)
#define I2C1_FIFOCTL			(I2C1_BASE + 0x001C)
#define I2C1_FIFOSTAT			(I2C1_BASE + 0x0020)
#define I2C1_DATCNT			(I2C1_BASE + 0x0024)
#define I2C1_RCNT			(I2C1_BASE + 0x0028)

/* I2C2 */
#define I2C2_BASE			0xB0178000
#define I2C2_CTL			(I2C2_BASE + 0x0000)
#define I2C2_CLKDIV			(I2C2_BASE + 0x0004)
#define I2C2_STAT			(I2C2_BASE + 0x0008)
#define I2C2_ADDR			(I2C2_BASE + 0x000C)
#define I2C2_TXDAT			(I2C2_BASE + 0x0010)
#define I2C2_RXDAT			(I2C2_BASE + 0x0014)
#define I2C2_CMD			(I2C2_BASE + 0x0018)
#define I2C2_FIFOCTL			(I2C2_BASE + 0x001C)
#define I2C2_FIFOSTAT			(I2C2_BASE + 0x0020)
#define I2C2_DATCNT			(I2C2_BASE + 0x0024)
#define I2C2_RCNT			(I2C2_BASE + 0x0028)

/* I2C3 */
#define I2C3_BASE			0xB017C000
#define I2C3_CTL			(I2C3_BASE + 0x0000)
#define I2C3_CLKDIV			(I2C3_BASE + 0x0004)
#define I2C3_STAT			(I2C3_BASE + 0x0008)
#define I2C3_ADDR			(I2C3_BASE + 0x000C)
#define I2C3_TXDAT			(I2C3_BASE + 0x0010)
#define I2C3_RXDAT			(I2C3_BASE + 0x0014)
#define I2C3_CMD			(I2C3_BASE + 0x0018)
#define I2C3_FIFOCTL			(I2C3_BASE + 0x001C)
#define I2C3_FIFOSTAT			(I2C3_BASE + 0x0020)
#define I2C3_DATCNT			(I2C3_BASE + 0x0024)
#define I2C3_RCNT			(I2C3_BASE + 0x0028)

#endif

