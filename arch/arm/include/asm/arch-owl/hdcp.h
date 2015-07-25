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

#ifndef __ASM_HDCP2TX_H
#define __ASM_HDCP2TX_H

/*HDCP2Tx*/
#define HDCP2TX_BASE			0xB0250000
#define HDCP2TX_CR			(HDCP2TX_BASE + 0x0000)
#define HDCP2TX_SR			(HDCP2TX_BASE + 0x0004)
#define HDCP2TX_PDOFIFO			(HDCP2TX_BASE + 0x0008)
#define HDCP2TX_HMACID			(HDCP2TX_BASE + 0x000C)
#define HDCP2TX_RTXMR			(HDCP2TX_BASE + 0x0010)
#define HDCP2TX_RTXLR			(HDCP2TX_BASE + 0x0014)
#define HDCP2TX_KMD3			(HDCP2TX_BASE + 0x0018)
#define HDCP2TX_KMD2			(HDCP2TX_BASE + 0x001C)
#define HDCP2TX_KMD1			(HDCP2TX_BASE + 0x0020)
#define HDCP2TX_KMD0			(HDCP2TX_BASE + 0x0024)
#define HDCP2TX_RRXMR			(HDCP2TX_BASE + 0x0028)
#define HDCP2TX_RRXLR			(HDCP2TX_BASE + 0x002C)
#define HDCP2TX_RNMR			(HDCP2TX_BASE + 0x0030)
#define HDCP2TX_RNLR			(HDCP2TX_BASE + 0x0034)
#define HDCP2TX_RIVMR			(HDCP2TX_BASE + 0x0038)
#define HDCP2TX_RIVLR			(HDCP2TX_BASE + 0x003C)
#define HDCP2TX_EKSD3			(HDCP2TX_BASE + 0x0040)
#define HDCP2TX_EKSD2			(HDCP2TX_BASE + 0x0044)
#define HDCP2TX_EKSD1			(HDCP2TX_BASE + 0x0048)
#define HDCP2TX_EKSD0			(HDCP2TX_BASE + 0x004C)
#define HDCP2TX_KSD3			(HDCP2TX_BASE + 0x0050)
#define HDCP2TX_KSD2			(HDCP2TX_BASE + 0x0054)
#define HDCP2TX_KSD1			(HDCP2TX_BASE + 0x0058)
#define HDCP2TX_KSD0			(HDCP2TX_BASE + 0x005C)
#define HDCP2TX_GC3			(HDCP2TX_BASE + 0x0060)
#define HDCP2TX_GC2			(HDCP2TX_BASE + 0x0064)
#define HDCP2TX_GC1			(HDCP2TX_BASE + 0x0068)
#define HDCP2TX_GC0			(HDCP2TX_BASE + 0x006C)
#define HDCP2TX_STREAMCTR		(HDCP2TX_BASE + 0x0070)
#define HDCP2TX_INPUTCTRMR		(HDCP2TX_BASE + 0x0074)
#define HDCP2TX_INPUTCTRLR		(HDCP2TX_BASE + 0x0078)
#define HDCP2TX_TSPH			(HDCP2TX_BASE + 0x007C)
#define HDCP2TX_TSSP			(HDCP2TX_BASE + 0x0080)
#define HDCP2TX_INOUTFIFOCR		(HDCP2TX_BASE + 0x0084)
#define HDCP2TX_INRFIFOD		(HDCP2TX_BASE + 0x0088)
#define HDCP2TX_INLENGTH		(HDCP2TX_BASE + 0x008C)
#define HDCP2TX_OUTEFIFOD		(HDCP2TX_BASE + 0x0090)
#define HDCP2TX_OUTLENGTH		(HDCP2TX_BASE + 0x0094)

#endif

