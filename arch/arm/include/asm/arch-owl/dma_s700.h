/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_DMA_S700_H__
#define __ASM_ARCH_DMA_S700_H__

#define OWL_MAX_DMA		10
#define IS_DEVICE_ADDR(x)	(((x) >= 0xe0000000ul) && ((x) < 0xe8000000ul))
#define IS_SHARERAM_ADDR(x)	(((x) >= 0xe4068000ul) && ((x) < 0xe4080000ul))

/* DMA DRQ ID */
#define DMA_DRQ_DCU				0
#define DMA_DRQ_SD0				2
#define DMA_DRQ_SD1				3
#define DMA_DRQ_SD2				4
#define DMA_DRQ_NANDDATA0			6
#define DMA_DRQ_I2S_TX				7
#define DMA_DRQ_I2S_RX				8
#define DMA_DRQ_PCM0_TX				9
#define DMA_DRQ_PCM0_RX				10
#define DMA_DRQ_PCM1_TX				11
#define DMA_DRQ_PCM1_RX				12
#define DMA_DRQ_SPDIF				13
#define DMA_DRQ_HDMI_AUDIO			14
#define DMA_DRQ_I2STX_SPDIF_HDMI	15
#define DMA_DRQ_UART0_TX			16
#define DMA_DRQ_UART0_RX			17
#define DMA_DRQ_UART1_TX			18
#define DMA_DRQ_UART1_RX			19
#define DMA_DRQ_UART2_TX			20
#define DMA_DRQ_UART2_RX			21
#define DMA_DRQ_UART3_TX			22
#define DMA_DRQ_UART3_RX			23
#define DMA_DRQ_UART4_TX			24
#define DMA_DRQ_UART4_RX			25
#define DMA_DRQ_UART5_TX			26
#define DMA_DRQ_UART5_RX			27
#define DMA_DRQ_SPI0_TX				28
#define DMA_DRQ_SPI0_RX				29
#define DMA_DRQ_SPI1_TX				30
#define DMA_DRQ_SPI1_RX				31
#define DMA_DRQ_SPI2_TX				32
#define DMA_DRQ_SPI2_RX				33
#define DMA_DRQ_SPI3_TX				34
#define DMA_DRQ_SPI3_RX				35
#define DMA_DRQ_DSI_TX				36
#define DMA_DRQ_DSI_RX				37
#define DMA_DRQ_HDCP20_TX			41
#define DMA_DRQ_HDCP20_RX			40
#define DMA_DRQ_UART6_TX			42
#define DMA_DRQ_UART6_RX			43
#define DMA_DRQ_NANDDATA1			45
#define DMA_DRQ_SD3				46

#endif	/* __ASM_ARCH_DMA_S700_H__ */
