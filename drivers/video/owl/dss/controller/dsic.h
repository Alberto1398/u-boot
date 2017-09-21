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
#define     DSI_PIN_MAP			(0x007c)
#define     DSI_PHY_CTRL		(0x0080)
#define     DSI_FT_TEST			(0x0088)
/*
 * dsi phy tx calculate
 * ***************************************************************************/
#define TLPX 100 /*in nano second, 0.000 000 001s*/
#define T_WAKEUP (1200000ul) /**in nano second, 0.000 000 001s*/

/*
 * config about dsi_phy_tx
 * */
#define CLK_PREPARE	5
#define CLK_ZERO	7
#define CLK_PRE		3
#define CLK_POST	7
#define CLK_TRAIL	5

#define HS_EXIT		7
#define HS_TRAIL	6
#define HS_ZERO		7
#define HS_PREPARE	5

uint16_t N_hs_prepare_cal_arra[5] = {0, 1, 8, 10, 14};
uint16_t N_hs_prepare_arra[6] = {0, 1, 2, 3, 4, 5};

uint16_t N_hs_zero_cal_arra[6] = {0, 16, 19, 28, 40, 56};
uint16_t N_hs_zero_arra[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint16_t N_hs_exit_cal_arra[6] = {0, 16, 19, 28, 40, 56};
uint16_t N_hs_exit_arra[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint16_t N_clk_prepare_cal_arra[5] = {0, 1, 8, 10, 14};
uint16_t N_clk_prepare_arra[6] = {0, 1, 2, 3, 4, 5};

uint16_t N_clk_zero_cal_arra[4] = {0, 64, 80, 112};
uint16_t N_clk_zero_arra[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint16_t N_clk_pre_cal_arra[2] = {0, 4};
uint16_t N_clk_pre_arra[4] = {0, 1, 2, 3};

uint16_t N_clk_post_cal_arra[1] = {7};
uint16_t N_clk_post_arra[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint16_t N_clk_trail_cal_arra[6] = {0, 3, 5, 7, 10, 14};
uint16_t N_clk_trail_arra[6] = {0, 1, 2, 3, 4, 5};


struct phy_tx_mode {
	uint16_t mode;
	uint16_t start;
	uint16_t end;
	uint16_t arra_prop;
};
/*hs_prepare*/
struct phy_tx_mode hs_prepare = {
		.mode	= HS_PREPARE,
		.start	= 1,
		.end	= 8,
		.arra_prop = 1,
};
/*hs_zero*/
struct phy_tx_mode hs_zero = {
		.mode	= HS_ZERO,
		.start	= 0,
		.end	= 16,
		.arra_prop = 2,
};
/*hs_exit*/
struct phy_tx_mode hs_exit = {
		.mode	= HS_EXIT,
		.start	= 0,
		.end	= 16,
		.arra_prop = 2,
};
/*clk_prepare*/
struct phy_tx_mode clk_prepar = {
		.mode	= CLK_PREPARE,
		.start	= 1,
		.end	= 8,
		.arra_prop = 1,
};
/*clk_zero*/
struct phy_tx_mode clk_zero = {
		.mode	= CLK_ZERO,
		.start	= 0,
		.end	= 64,
		.arra_prop = 4,
};
/*clk_pre*/
struct phy_tx_mode clk_pre = {
		.mode	= CLK_PRE,
		.start	= 0,
		.end	= 4,
		.arra_prop = 1,
};
/*clk_post*/
struct phy_tx_mode clk_post = {
		.mode = CLK_POST,
		.start = 0,
		.end  = 0,
		.arra_prop = 0,
};

/*clk_trail*/
struct phy_tx_mode clk_trail = {
		.mode = CLK_TRAIL,
		.start = 0,
		.end  = 0,
		.arra_prop = 0,
};
/*
 * Decimals up remainder function
 * */
static uint64_t dsic_ceil(uint64_t dividend, uint64_t divisor)
{
	return dividend / divisor + (dividend % divisor != 0 ? 1 : 0);
}

uint32_t get_val_from_cal(uint32_t cal, uint32_t mode)
{
	uint32_t val = 0;
	int i;
	if (mode == CLK_PRE) {
		val = cal - 1;
		return val;
	}
	for (i = 0; i < mode; i++) {
		val = (1 << i) * 10;
		if (cal <= val)
			return i;
	}
}
static uint16_t get_dsi_phy_tx(uint16_t cal, uint16_t *cal_arra,
				uint16_t cal_arra_length, uint16_t *arra,
					struct phy_tx_mode  *tx_mode)
{
	int i = 0, j = 0;
	uint16_t val;
	if (cal_arra_length >= 2) {
		for (i = 1, j = 0; i < cal_arra_length; i++) {
			if ((cal > cal_arra[i - 1] * 10) &&
				(cal <= cal_arra[i] * 10)) {
				if ((tx_mode->start == cal_arra[i - 1]) &&
					(tx_mode->end == cal_arra[i])) {
					val = get_val_from_cal(cal,
							tx_mode->mode);
					if ((tx_mode->mode == HS_ZERO) ||
					    (tx_mode->mode == HS_EXIT) ||
					    (tx_mode->mode == CLK_ZERO))
						return val - 1;
					else
						return val;
				} else {
					val = arra[j];
					return val;
				}
			} else if (cal > cal_arra[cal_arra_length - 1] * 10) {
				val = arra[j];
				return val;
			}
			if (tx_mode->start == j)
				j += tx_mode->arra_prop;
			j++;
		}
	} else if (1 == cal_arra_length) {
		if (cal >= cal_arra[0] * 10) {
			val = arra[tx_mode->mode];
			return val;
		} else {
			val = cal;
			return val;
		}
	} else
		return -1;
	return 0;
}



#endif	/* __DSI_DSIC_H_ */

