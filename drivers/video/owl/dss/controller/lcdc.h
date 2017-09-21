/*
 * OWL LCD/LVDS controller
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
#ifndef __LCD_LCDC_H_
#define __LCD_LCDC_H_

/* should consider other platforms, TODO */
#define LCDC_CTL			(0x0000)
#define LCDC_SIZE			(0x0004)
#define LCDC_STATUS			(0x0008)
#define LCDC_TIM0			(0x000C)
#define LCDC_TIM1			(0x0010)
#define LCDC_TIM2			(0x0014)
#define LCDC_COLOR			(0x0018)

#define LCDC_IMG_XPOS			(0x001c)
#define LCDC_IMG_YPOS			(0x0020)

#define LCDC_LVDS_CTL			(0x0200)
#define LCDC_LVDS_ALG_CTL0		(0x0204)
#define LCDC_LVDS_DEBUG			(0x0208)

#endif
