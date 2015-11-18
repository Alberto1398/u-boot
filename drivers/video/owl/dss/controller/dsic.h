/*
 * OWL eDP(Embedded Display Port)  controller.
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __DSI_DSIC_H_
#define __DSI_DSIC_H_


#define     DSI_CTRL			(0x0000)
#define     DSI_SIZE			(0x0004)
#define     DSI_COLOR			(0x0008)
#define     DSI_VIDEO_CFG		(0x000C)
#define     DSI_RGBHT0			(0x0010)
#define     DSI_RGBHT1			(0x0014)
#define     DSI_RGBVT0			(0x0018)
#define     DSI_RGBVT1			(0x001c)
#define     DSI_TIMEOUT			(0x0020)
#define     DSI_TR_STA			(0x0024)
#define     DSI_INT_EN			(0x0028)
#define     DSI_ERROR_REPORT		(0x002c)
#define     DSI_FIFO_ODAT		(0x0030)
#define     DSI_FIFO_IDAT		(0x0034)
#define     DSI_IPACK			(0x0038)
#define     DSI_PACK_CFG		(0x0040)
#define     DSI_PACK_HEADER		(0x0044)
#define     DSI_TX_TRIGGER		(0x0048)
#define     DSI_RX_TRIGGER		(0x004c)
#define     DSI_LANE_CTRL		(0x0050)
#define     DSI_LANE_STA		(0x0054)
#define     DSI_PHY_T0			(0x0060)
#define     DSI_PHY_T1			(0x0064)
#define     DSI_PHY_T2			(0x0068)
#define     DSI_APHY_DEBUG0		(0x0070)
#define     DSI_APHY_DEBUG1		(0x0074)
#define     DSI_SELF_TEST		(0x0078)
#define     DSI_LANE_SWAP		(0x007c)
#define     DSI_PHY_CTRL		(0x0080)
#define     DSI_FT_TEST			(0x0088)


#endif	/* __DSI_DSIC_H_ */

