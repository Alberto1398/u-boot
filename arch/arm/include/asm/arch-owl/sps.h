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

#ifndef __ASM_SPS_H
#define __ASM_SPS_H

#define SPS_PG_BASE			0xB01B0100
#define SPS_PG_CTL			(SPS_PG_BASE + 0x0000)
#define SPS_RST_CTL			(SPS_PG_BASE + 0x0004)
#define SPS_LDO_CTL			(SPS_PG_BASE + 0x0008)
#define SPS_BDG_CTL			(SPS_PG_BASE + 0x000c)
#define SPS_TEMP_CTL			(SPS_PG_BASE + 0x0010)

#define CMU_PWR_CTL			(SPS_PG_BASE + 0x0028)

#endif

