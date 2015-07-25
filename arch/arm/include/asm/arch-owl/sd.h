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

#ifndef __ASM_SDHC_H
#define __ASM_SDHC_H

/* SD0 */
#define SD0_BASE			0xB0230000
#define SD0_EN			(SD0_BASE + 0x0000)
#define SD0_CTL			(SD0_BASE + 0x0004)
#define SD0_STATE			(SD0_BASE + 0x0008)
#define SD0_CMD			(SD0_BASE + 0x000C)
#define SD0_ARG			(SD0_BASE + 0x0010)
#define SD0_RSPBUF0			(SD0_BASE + 0x0014)
#define SD0_RSPBUF1			(SD0_BASE + 0x0018)
#define SD0_RSPBUF2			(SD0_BASE + 0x001C)
#define SD0_RSPBUF3			(SD0_BASE + 0x0020)
#define SD0_RSPBUF4			(SD0_BASE + 0x0024)
#define SD0_DAT			(SD0_BASE + 0x0028)
#define SD0_BLK_SIZE			(SD0_BASE + 0x002C)
#define SD0_BLK_NUM			(SD0_BASE + 0x0030)
#define SD0_BUF_SIZE			(SD0_BASE + 0x0034)

/* SD1 */
#define SD1_BASE			0xB0234000
#define SD1_EN			(SD1_BASE + 0x0000)
#define SD1_CTL			(SD1_BASE + 0x0004)
#define SD1_STATE			(SD1_BASE + 0x0008)
#define SD1_CMD			(SD1_BASE + 0x000C)
#define SD1_ARG			(SD1_BASE + 0x0010)
#define SD1_RSPBUF0			(SD1_BASE + 0x0014)
#define SD1_RSPBUF1			(SD1_BASE + 0x0018)
#define SD1_RSPBUF2			(SD1_BASE + 0x001C)
#define SD1_RSPBUF3			(SD1_BASE + 0x0020)
#define SD1_RSPBUF4			(SD1_BASE + 0x0024)
#define SD1_DAT			(SD1_BASE + 0x0028)
#define SD1_BLK_SIZE			(SD1_BASE + 0x002C)
#define SD1_BLK_NUM			(SD1_BASE + 0x0030)
#define SD1_BUF_SIZE			(SD1_BASE + 0x0034)

/* SD2 */
#define SD2_BASE			0xB0238000
#define SD2_EN			(SD2_BASE + 0x0000)
#define SD2_CTL			(SD2_BASE + 0x0004)
#define SD2_STATE			(SD2_BASE + 0x0008)
#define SD2_CMD			(SD2_BASE + 0x000C)
#define SD2_ARG			(SD2_BASE + 0x0010)
#define SD2_RSPBUF0			(SD2_BASE + 0x0014)
#define SD2_RSPBUF1			(SD2_BASE + 0x0018)
#define SD2_RSPBUF2			(SD2_BASE + 0x001C)
#define SD2_RSPBUF3			(SD2_BASE + 0x0020)
#define SD2_RSPBUF4			(SD2_BASE + 0x0024)
#define SD2_DAT			(SD2_BASE + 0x0028)
#define SD2_BLK_SIZE			(SD2_BASE + 0x002C)
#define SD2_BLK_NUM			(SD2_BASE + 0x0030)
#define SD2_BUF_SIZE			(SD2_BASE + 0x0034)

#endif

