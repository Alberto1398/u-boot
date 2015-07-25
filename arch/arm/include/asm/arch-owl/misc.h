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

#ifndef __ASM_MISC_H
#define __ASM_MISC_H

/* DMM */
#define DMM_BASE			0xB0290000
#define DMM_SCH_CTRL			(DMM_BASE + 0x0000)
#define DMM_TO_CPU_BISP			(DMM_BASE + 0x0004)
#define DMM_TO_VDE			(DMM_BASE + 0x0008)
#define DMM_TO_ETHERNET_DE			(DMM_BASE + 0x000C)
#define DMM_TO_DE_CRITICAL_USB2			(DMM_BASE + 0x0010)
#define DMM_TO_USB3_VCE			(DMM_BASE + 0x0014)
#define DMM_TO_GPU3D_DAP			(DMM_BASE + 0x0018)
#define DMM_TO_DMA_DMA_CRITICAL			(DMM_BASE + 0x001C)
#define DMM_INTERLEAVE_BASEADDR			(DMM_BASE + 0x0020)
#define DMM_INTERLEAVE_FRAGMENT			(DMM_BASE + 0x0024)
#define DMM_INTERLEAVE_CONFIG			(DMM_BASE + 0x0028)
#define DMM_AXI_PRIORITY			(DMM_BASE + 0x002C)
#define DMM_WATCH_ADDR			(DMM_BASE + 0x0030)
#define DMM_WATCH_ADDR_MASK			(DMM_BASE + 0x0034)
#define DMM_WATCH_DATA			(DMM_BASE + 0x0038)
#define DMM_WATCH_DATA_MASK			(DMM_BASE + 0x003C)
#define DMM_WATCH_CTRL			(DMM_BASE + 0x0040)
#define DMM_MONITOR_CTRL			(DMM_BASE + 0x0044)
#define DMM_PM_CTRL			(DMM_BASE + 0x0048)
#define DMM_PC0			(DMM_BASE + 0x004c)
#define DMM_PC1			(DMM_BASE + 0x0050)

/* SRAMI */
#define SRAMI_BASE			0xB0240000
#define SRAMI_CTL			(SRAMI_BASE + 0x00)

/* SHARESRAM_CRTL */
#define SHARESRAM_CRTL_BASE			0xB0240000
#define SHARESRAM_CTL			(SHARESRAM_CRTL_BASE + 0x04)
/* IRC */
#define IRC_BASE			0xB0120050
#define IR_CTL			(IRC_BASE + 0x00)
#define IR_STAT			(IRC_BASE + 0x04)
#define IR_CC			(IRC_BASE + 0x08)
#define IR_KDC			(IRC_BASE + 0x0C)
#define IR_TCOUNTER			(IRC_BASE + 0x10)
#define IR_RCC			(IRC_BASE + 0x14)

/* LCD */
#define LCD_BASE			0xB02A0000
#define LCD0_CTL			(LCD_BASE + 0x0000)
#define LCD0_SIZE			(LCD_BASE + 0x0004)
#define LCD0_STATUS			(LCD_BASE + 0x0008)
#define LCD0_TIM0			(LCD_BASE + 0x000C)
#define LCD0_TIM1			(LCD_BASE + 0x0010)
#define LCD0_TIM2			(LCD_BASE + 0x0014)
#define LCD0_COLOR			(LCD_BASE + 0x0018)
#define LCD0_CPU_CTL			(LCD_BASE + 0x001c)
#define LCD0_CPU_CMD			(LCD_BASE + 0x0020)
#define LCD0_TEST_P0			(LCD_BASE + 0x0024)
#define LCD0_TEST_P1			(LCD_BASE + 0x0028)
#define LCD0_IMG_XPOS			(LCD_BASE + 0x002c)
#define LCD0_IMG_YPOS			(LCD_BASE + 0x0030)
#define LCD1_CTL			(LCD_BASE + 0x0100)
#define LCD1_SIZE			(LCD_BASE + 0x0104)
#define LCD1_TIM1			(LCD_BASE + 0x0108)
#define LCD1_TIM2			(LCD_BASE + 0x010c)
#define LCD1_COLOR			(LCD_BASE + 0x0110)
#define LCD1_CPU_CTL			(LCD_BASE + 0x0114)
#define LCD1_CPU_CMD			(LCD_BASE + 0x0118)
#define LCD1_IMG_XPOS			(LCD_BASE + 0x011C)
#define LCD1_IMG_YPOS			(LCD_BASE + 0x0120)

/* LVDS */
#define LVDS_BASE			0xB02A0200
#define LVDS_CTL			(LVDS_BASE + 0x0000)
#define LVDS_ALG_CTL0			(LVDS_BASE + 0x0004)
#define LVDS_DEBUG			(LVDS_BASE + 0x0008)

/* CSI */
#define CSI_BASE			0xB02D0000
#define CSI_CTRL			(CSI_BASE + 0x00)
#define SHORT_PACKET			(CSI_BASE + 0x04)
#define ERROR_PENDING			(CSI_BASE + 0x08)
#define STATUS_PENDING			(CSI_BASE + 0x0c)
#define LANE_STATUS			(CSI_BASE + 0x10)
#define CSI_PHY_T0			(CSI_BASE + 0x14)
#define CSI_PHY_T1			(CSI_BASE + 0x18)
#define CSI_PHY_T2			(CSI_BASE + 0x1c)
#define CSI_ANALOG_PHY			(CSI_BASE + 0x20)
#define CONTEXT0_CFG			(CSI_BASE + 0x100)
#define CONTEXT0_STATUS			(CSI_BASE + 0x104)
#define CONTEXT1_CFG			(CSI_BASE + 0x120)
#define CONTEXT1_STATUS			(CSI_BASE + 0x124)

/* DSI */
#define DSI_BASE			0xB0220000
#define DSI_CTRL			(DSI_BASE + 0x00)
#define DSI_SIZE			(DSI_BASE + 0x04)
#define DSI_COLOR			(DSI_BASE + 0x08)
#define DSI_VIDEO_CFG			(DSI_BASE + 0x0C)
#define DSI_RGBHT0			(DSI_BASE + 0x10)
#define DSI_RGBHT1			(DSI_BASE + 0x14)
#define DSI_RGBVT0			(DSI_BASE + 0x18)
#define DSI_RGBVT1			(DSI_BASE + 0x1C)
#define DSI_TIMEOUT			(DSI_BASE + 0x20)
#define DSI_TR_STA			(DSI_BASE + 0x24)
#define DSI_INT_EN			(DSI_BASE + 0x28)
#define DSI_ERROR_REPORT		(DSI_BASE + 0x2C)
#define DSI_FIFO_ODAT			(DSI_BASE + 0x30)
#define DSI_FIFO_IDAT			(DSI_BASE + 0x34)
#define DSI_IPACK			(DSI_BASE + 0x38)
#define DSI_PACK_CFG			(DSI_BASE + 0x40)
#define DSI_PACK_HEADER			(DSI_BASE + 0x44)
#define DSI_TX_TRIGGER			(DSI_BASE + 0x48)
#define DSI_RX_TRIGGER			(DSI_BASE + 0x4C)
#define DSI_LANE_CTRL			(DSI_BASE + 0x50)
#define DSI_LANE_STA			(DSI_BASE + 0x54)
#define DSI_PHY_T0			(DSI_BASE + 0x60)
#define DSI_PHY_T1			(DSI_BASE + 0x64)
#define DSI_PHY_T2			(DSI_BASE + 0x68)
#define DSI_APHY_DEBUG0			(DSI_BASE + 0x70)
#define DSI_APHY_DEBUG1			(DSI_BASE + 0x74)
#define DSI_SELF_TEST			(DSI_BASE + 0x78)
#define DSI_PIN_MAP			(DSI_BASE + 0x7C)
#define DSI_PHY_CTRL			(DSI_BASE + 0x80)
#define DSI_FT_TEST			(DSI_BASE + 0x88)

/* TVOUT */
#define TVOUT_BASE			0xB02B0000
#define TVOUT_EN			(TVOUT_BASE + 0x0000)
#define TVOUT_OCR			(TVOUT_BASE + 0x0004)
#define TVOUT_STA			(TVOUT_BASE + 0x0008)
#define TVOUT_CCR			(TVOUT_BASE + 0x000C)
#define TVOUT_BCR			(TVOUT_BASE + 0x0010)
#define TVOUT_CSCR			(TVOUT_BASE + 0x0014)
#define TVOUT_PRL			(TVOUT_BASE + 0x0018)
#define TVOUT_VFALD			(TVOUT_BASE + 0x001C)
#define CVBS_MSR			(TVOUT_BASE + 0x0020)
#define CVBS_AL_SEPO			(TVOUT_BASE + 0x0024)
#define CVBS_AL_SEPE			(TVOUT_BASE + 0x0028)
#define CVBS_AD_SEP			(TVOUT_BASE + 0x002C)
#define CVBS_HUECR			(TVOUT_BASE + 0x0030)
#define CVBS_SCPCR			(TVOUT_BASE + 0x0034)
#define CVBS_SCFCR			(TVOUT_BASE + 0x0038)
#define CVBS_CBACR			(TVOUT_BASE + 0x003C)
#define CVBS_SACR			(TVOUT_BASE + 0x0040)
#define BT_MSR0				(TVOUT_BASE + 0x0100)
#define BT_MSR1				(TVOUT_BASE + 0x0104)
#define BT_AL_SEPO			(TVOUT_BASE + 0x0108)
#define BT_AL_SEPE			(TVOUT_BASE + 0x010C)
#define BT_AP_SEP			(TVOUT_BASE + 0x0110)
#define TVOUT_DCR			(TVOUT_BASE + 0x0070)
#define TVOUT_DDCR			(TVOUT_BASE + 0x0074)
#define TVOUT_DCORCTL			(TVOUT_BASE + 0x0078)
#define TVOUT_DRCR			(TVOUT_BASE + 0x007C)

/* HDMI */
#define HDMI_BASE			0xB02C0000
#define HDMI_VICTL			(HDMI_BASE + 0x0000)
#define HDMI_VIVSYNC			(HDMI_BASE + 0x0004)
#define HDMI_VIVHSYNC			(HDMI_BASE + 0x0008)
#define HDMI_VIALSEOF			(HDMI_BASE + 0x000C)
#define HDMI_VIALSEEF			(HDMI_BASE + 0x0010)
#define HDMI_VIADLSE			(HDMI_BASE + 0x0014)
#define HDMI_AIFRAMEC			(HDMI_BASE + 0x0020)
#define HDMI_AICHSTABYTE0TO3		(HDMI_BASE + 0x0024)
#define HDMI_AICHSTABYTE4TO7		(HDMI_BASE + 0x0028)
#define HDMI_AICHSTABYTE8TO11		(HDMI_BASE + 0x002C)
#define HDMI_AICHSTABYTE12TO15		(HDMI_BASE + 0x0030)
#define HDMI_AICHSTABYTE16TO19		(HDMI_BASE + 0x0034)
#define HDMI_AICHSTABYTE20TO23		(HDMI_BASE + 0x0038)
#define HDMI_AICHSTASCN			(HDMI_BASE + 0x003C)
#define HDMI_VR				(HDMI_BASE + 0x0050)
#define HDMI_CR				(HDMI_BASE + 0x0054)
#define HDMI_SCHCR			(HDMI_BASE + 0x0058)
#define HDMI_ICR			(HDMI_BASE + 0x005C)
#define HDMI_SCR			(HDMI_BASE + 0x0060)
#define HDMI_LPCR			(HDMI_BASE + 0x0064)
#define HDCP_CR				(HDMI_BASE + 0x0068)
#define HDCP_SR				(HDMI_BASE + 0x006C)
#define HDCP_ANLR			(HDMI_BASE + 0x0070)
#define HDCP_ANMR			(HDMI_BASE + 0x0074)
#define HDCP_ANILR			(HDMI_BASE + 0x0078)
#define HDCP_ANIMR			(HDMI_BASE + 0x007C)
#define HDCP_DPKLR			(HDMI_BASE + 0x0080)
#define HDCP_DPKMR			(HDMI_BASE + 0x0084)
#define HDCP_LIR			(HDMI_BASE + 0x0088)
#define HDCP_SHACR			(HDMI_BASE + 0x008C)
#define HDCP_SHADR			(HDMI_BASE + 0x0090)
#define HDCP_ICR			(HDMI_BASE + 0x0094)
#define HDCP_KMMR			(HDMI_BASE + 0x0098)
#define HDCP_KMLR			(HDMI_BASE + 0x009C)
#define HDCP_MILR			(HDMI_BASE + 0x00A0)
#define HDCP_MIMR			(HDMI_BASE + 0x00A4)
#define HDCP_KOWR			(HDMI_BASE + 0x00A8)
#define HDCP_OWR			(HDMI_BASE + 0x00AC)
#define TMDS_STR0			(HDMI_BASE + 0x00B8)
#define TMDS_STR1			(HDMI_BASE + 0x00BC)
#define TMDS_EODR0			(HDMI_BASE + 0x00C0)
#define TMDS_EODR1			(HDMI_BASE + 0x00C4)
#define HDMI_ASPCR			(HDMI_BASE + 0x00D0)
#define HDMI_ACACR			(HDMI_BASE + 0x00D4)
#define HDMI_ACRPCR			(HDMI_BASE + 0x00D8)
#define HDMI_ACRPCTSR			(HDMI_BASE + 0x00DC)
#define HDMI_ACRPPR			(HDMI_BASE + 0x00E0)
#define HDMI_GCPCR			(HDMI_BASE + 0x00E4)
#define HDMI_RPCR			(HDMI_BASE + 0x00E8)
#define HDMI_RPRBDR			(HDMI_BASE + 0x00EC)
#define HDMI_OPCR			(HDMI_BASE + 0x00F0)
#define HDMI_DIPCCR			(HDMI_BASE + 0x00F4)
#define HDMI_ORP6PH			(HDMI_BASE + 0x00F8)
#define HDMI_ORSP6W0			(HDMI_BASE + 0x00FC)
#define HDMI_ORSP6W1			(HDMI_BASE + 0x0100)
#define HDMI_ORSP6W2			(HDMI_BASE + 0x0104)
#define HDMI_ORSP6W3			(HDMI_BASE + 0x0108)
#define HDMI_ORSP6W4			(HDMI_BASE + 0x010C)
#define HDMI_ORSP6W5			(HDMI_BASE + 0x0110)
#define HDMI_ORSP6W6			(HDMI_BASE + 0x0114)
#define HDMI_ORSP6W7			(HDMI_BASE + 0x0118)
#define HDMI_CECCR			(HDMI_BASE + 0x011C)
#define HDMI_CECRTCR			(HDMI_BASE + 0x0120)
#define HDMI_CECRXCR			(HDMI_BASE + 0x0124)
#define HDMI_CECTXCR			(HDMI_BASE + 0x0128)
#define HDMI_CECTXDR			(HDMI_BASE + 0x012C)
#define HDMI_CECRXDR			(HDMI_BASE + 0x0130)
#define HDMI_CECRXTCR			(HDMI_BASE + 0x0134)
#define HDMI_CECTXTCR0			(HDMI_BASE + 0x0138)
#define HDMI_CECTXTCR1			(HDMI_BASE + 0x013C)
#define HDMI_CRCCR			(HDMI_BASE + 0x0140)
#define HDMI_CRCDOR			(HDMI_BASE + 0x0144)
#define HDMI_TX_1			(HDMI_BASE + 0x0154)
#define HDMI_TX_2			(HDMI_BASE + 0x0158)
#define CEC_DDC_HPD			(HDMI_BASE + 0x015C)

/* I2S_SPDIF */
#define I2S_SPDIF_BASE			0xB0100000
#define I2S_CTL				(I2S_SPDIF_BASE + 0x0000)
#define I2S_FIFOCTL			(I2S_SPDIF_BASE + 0x0004)
#define I2STX_DAT			(I2S_SPDIF_BASE + 0x0008)
#define I2SRX_DAT			(I2S_SPDIF_BASE + 0x000c)
#define SPDIF_HDMI_CTL			(I2S_SPDIF_BASE + 0x0010)
#define SPDIF_DAT			(I2S_SPDIF_BASE + 0x0014)
#define SPDIF_CLSTAT			(I2S_SPDIF_BASE + 0x0018)
#define SPDIF_CHSTAT			(I2S_SPDIF_BASE + 0x001c)
#define HDMI_DAT			(I2S_SPDIF_BASE + 0x0020)
#define I2STX_DAT_DBG			(I2S_SPDIF_BASE + 0x0024)
#define I2SRX_DAT_DBG			(I2S_SPDIF_BASE + 0x0028)
#define I2STX_SPDIF_HDMI_CTL		(I2S_SPDIF_BASE + 0x002c)
#define I2STX_SPDIF_HDMI_DAT		(I2S_SPDIF_BASE + 0x0030)

/* PCM0 */
#define PCM0_BASE			0xB0110000
#define PCM0_CTL			(PCM0_BASE + 0x0000)
#define PCM0_STAT			(PCM0_BASE + 0x0004)
#define PCM0_RXDAT			(PCM0_BASE + 0x0008)
#define PCM0_TXDAT			(PCM0_BASE + 0x000C)

/* PCM1 */
#define PCM1_BASE			0xB0118000
#define PCM1_CTL			(PCM1_BASE + 0x0000)
#define PCM1_STAT			(PCM1_BASE + 0x0004)
#define PCM1_RXDAT			(PCM1_BASE + 0x0008)
#define PCM1_TXDAT			(PCM1_BASE + 0x000C)

/* KEY */
#define KEY_BASE			0xB01A0000
#define KEY_CTL				(KEY_BASE + 0x0000)
#define KEY_DAT0			(KEY_BASE + 0x0004)
#define KEY_DAT1			(KEY_BASE + 0x0008)
#define KEY_DAT2			(KEY_BASE + 0x000C)
#define KEY_DAT3			(KEY_BASE + 0x0010)
#define KEY_DAT4			(KEY_BASE + 0x0014)
#define KEY_DAT5			(KEY_BASE + 0x0018)
#define KEY_DAT6			(KEY_BASE + 0x001C)
#define KEY_DAT7			(KEY_BASE + 0x0020)

/* LENSC */
#define LENSC_BASE			0xB0140000
#define ST1_MOTOR_CTL1			(LENSC_BASE + 0x0)
#define ST1_MOTOR_CTL2			(LENSC_BASE + 0x4)
#define ST1_MOTOR_SPEED			(LENSC_BASE + 0x8)
#define ST1_MOTOR_TRDATA		(LENSC_BASE + 0xC)
#define ST1_MOTOR_CMD_CTL		(LENSC_BASE + 0x10)
#define ST2_MOTOR_CTL1			(LENSC_BASE + 0x20)
#define ST2_MOTOR_CTL2			(LENSC_BASE + 0x24)
#define ST2_MOTOR_SPEED			(LENSC_BASE + 0x28)
#define ST2_MOTOR_TRDATA		(LENSC_BASE + 0x2C)
#define ST2_MOTOR_CMD_CTL		(LENSC_BASE + 0x30)
#define ST3_MOTOR_CTL1			(LENSC_BASE + 0x40)
#define ST3_MOTOR_CTL2			(LENSC_BASE + 0x44)
#define ST3_MOTOR_SPEED			(LENSC_BASE + 0x48)
#define ST3_MOTOR_TRDATA		(LENSC_BASE + 0x4C)
#define ST3_MOTOR_CMD_CTL		(LENSC_BASE + 0x50)
#define DC_MOTOR_CTL1			(LENSC_BASE + 0x60)
#define DC_MOTOR_CTL2			(LENSC_BASE + 0x64)
#define MOTOR_MFP_CTL			(LENSC_BASE + 0x70)
#define MOTOR_PWM_CTL			(LENSC_BASE + 0x74)
#define MOTOR_VD_CTL			(LENSC_BASE + 0x78)

/* NIC */
#define NIC_BASE			0xB0500000

/* VDE */
#define VDE_BASE			0xB0280000

/* VCE */
#define VCE_BASE			0xB0288000

/* G3D */
#define G3D_BASE			0xB0300000

/* DDR_Upctrl/Pub */
#define UPCTL0_BASE			0xB01C0000
#define UPCTL1_BASE			0xB01D0000
#define PUB0_BASE			0xB01C8000
#define PUB1_BASE			0xB01D8000

#endif
