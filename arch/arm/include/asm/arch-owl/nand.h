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

#ifndef __ASM_NAND_H
#define __ASM_NAND_H

#define NAND_BASE			0xB0210000
#define NAND_CTL			(NAND_BASE + 0x0000)
#define NAND_STATUS			(NAND_BASE + 0X0004)
#define NAND_CONFIG			(NAND_BASE + 0X0008)
#define NAND_BC				(NAND_BASE + 0X000c)
#define NAND_COLADDR			(NAND_BASE + 0X0010)
#define NAND_ROWADDR0			(NAND_BASE + 0X0014)
#define NAND_ROWADDR1			(NAND_BASE + 0X0018)
#define NAND_CMD_FSM0			(NAND_BASE + 0X001c)
#define NAND_CMD_FSM_CTL0		(NAND_BASE + 0X0020)
#define NAND_CMD_FSM1			(NAND_BASE + 0x0024)
#define NAND_CMD_FSM_CTL1		(NAND_BASE + 0X0028)
#define NAND_CMD_FSM2			(NAND_BASE + 0x002c)
#define NAND_CMD_FSM_CTL2		(NAND_BASE + 0X0030)
#define NAND_ECCST0			(NAND_BASE + 0x0034)
#define NAND_ECCST1			(NAND_BASE + 0x0038)
#define NAND_ECCST2			(NAND_BASE + 0X003c)
#define NAND_ECCST3			(NAND_BASE + 0X0040)
#define NAND_UDATAC0			(NAND_BASE + 0X0044)
#define NAND_UDATAC1			(NAND_BASE + 0X0048)
#define NAND_UDATAC2			(NAND_BASE + 0X004c)
#define NAND_UDATAC3			(NAND_BASE + 0X0050)
#define NAND_FSM_START			(NAND_BASE + 0X0054)
#define NAND_DATA_ENTRY			(NAND_BASE + 0x0080)
#define NAND_CTL_STATUS			(NAND_BASE + 0xa0)
#define NAND_TIMING			(NAND_BASE + 0Xa4)
#define NAND_ANALOG_CTL			(NAND_BASE + 0xa8)
#define NAND_CHCMD			(NAND_BASE + 0xac)
#define NAND_CHCMD_INTERVAL		(NAND_BASE + 0xb0)

#endif

