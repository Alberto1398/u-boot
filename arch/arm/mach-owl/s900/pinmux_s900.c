/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>

/* Pin configurations */

/*0= P_PCM1_IN, P_PCM1_CLK, P_PCM1_SYNC, P_PCM1_OUT*/
/*1= P_SPI1_SCLK, P_SPI1_SS, P_SPI1_MISO, P_SPI1_MOSS*/
/*2= P_I2C3_SCLK, P_PWM4, P_PWM5, P_I2C3_SDATA*/
/*3= P_UART4_RTSB, P_PCM1_CTSB, -, -*/
#define MFP0_1_0_PCM1	0x0
#define MFP0_1_0_SPI1	0x1
#define MFP0_1_0_I2C3	0x2
#define MFP0_1_0_UART4	0x3
#define MFP0_1_0_F(x) (x)

/*0= */
/*1= */
#define MFP0_2_F(x) (x<<2)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_4_3_F(x) (x<<3)

/*0= */
/*1= */
#define MFP0_5_F(x) (x<<5)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_7_6_F(x) (x<<6)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_10_8_F(x) (x<<8)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_12_11_F(x) (x<<11)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_15_13_F(x) (x<<13)

/*0= */
/*1= */
/*2= */
/*3= */
#define MFP0_18_16_F(x) (x<<16)

/*0= */
/*1= */
/*2= */
/*3= */
/*4= */
/*5= */
#define MFP1_28_26_F(x) (x<<26)
#define MFP3_27_F(x) (x<<27)
#define MFP3_11_F(x) (x<<11)

/*0= */
/*1= */
/*2= */
/*3= */
/*4= */
/*5= */
/*6= */
#define MFP2_2_0_F(x) 	(x)

#define MFP2_6_5_F(x) 	(x<<5)
#define MFP2_8_7_F(x) 	(x<<7)
#define MFP2_13_11_F(x) (x<<11)
#define MFP2_16_14_F(x) (x<<14)
#define MFP2_19_17_F(x) (x<<17)

/*0= */
/*1= */
/*2= */
/*3= */
/*4= */
/*5= */
#define MFP3_21_19_F(x) (x<<19)
#define MFP3_18_16(x) (x<<16)

/* PAD_PULLCTL */
#define P_HI_Z		0
#define P_PULL_UP	1
#define P_PULL_DOWN	2
#define P_REPEATER	3

/* ====PAD_PULLCTL1 =====*/
#define P_31_30(x)	(x<<30)
#define P_29_28(x)	(x<<28)
#define P_27_26(x)	(x<<26)
#define P_25_24(x)	(x<<24)
#define P_23_22(x)	(x<<22)
#define P_21_20(x)	(x<<20)
#define P_19_18(x)	(x<<18)
#define P_17_16(x)	(x<<16)
#define P_15_14(x)	(x<<14)
#define P_13_12(x)	(x<<12)
#define P_11_10(x)	(x<<10)
#define P_9_8(x)	(x<<8)
#define P_7_6(x)	(x<<6)
#define P_5_4(x)	(x<<4)
#define P_3_2(x)	(x<<2)
#define P_1_0(x)	(x<<0)

/* ====PAD_PULLCTL2 =====*/
#define P_11(x)		(x<<11)
#define P_12(x)		(x<<12)
#define P_14(x)		(x<<14)
#define P_15(x)		(x<<15)

static const struct device_mfp_config s900_uart0_cfg = {
	.periph_id = PERIPH_ID_UART0,
	.mfp_cfg = {
		    {
		     .mfp = 0,
		     .reg_cfg = {
				 {MFP_CTL0, MFP0_12_11_F(3), MFP0_12_11_F(2)},
				 },
		     },
		    {
		     .mfp = 1,
		     .reg_cfg = {
				 {MFP_CTL0, MFP0_12_11_F(3), MFP0_12_11_F(2)},
				 },
		     },

		    },

};

static const struct device_mfp_config s900_i2c0_cfg = {
	.periph_id = PERIPH_ID_I2C0,
	.mfp_cfg = {
		{
			.mfp = 0,
			.reg_cfg = {
				{MFP_CTL3, MFP3_18_16(7), MFP3_18_16(0)},
				{PAD_PULLCTL0, P_9_8(3), P_9_8(P_HI_Z)},
				{PAD_PULLCTL0, P_11_10(3), P_11_10(P_HI_Z)},
				{PAD_DRV2, P_31_30(3), P_31_30(3)},
			},
		},
	},
};

static const struct device_mfp_config s900_i2c3_cfg = {
	.periph_id = PERIPH_ID_I2C3,
	.mfp_cfg = {
		    {
		     .mfp = 0,
		     .reg_cfg = {
				 {MFP_CTL0, MFP0_1_0_F(3), MFP0_1_0_F(2)},
				 {PAD_PULLCTL1, P_31_30(3), P_31_30(P_PULL_UP)},
				 {PAD_PULLCTL1, P_29_28(3), P_29_28(P_PULL_UP)},
				 },
		     },
		    },
};

static const struct device_mfp_config s900_i2c5_cfg = {
	.periph_id = PERIPH_ID_I2C5,
	.mfp_cfg = {
		    {
		     .mfp = 0,
		     .reg_cfg = {
				 {MFP_CTL2, MFP2_2_0_F(7), MFP2_2_0_F(3)},
                                 {MFP_CTL3, MFP3_21_19_F(7), MFP3_21_19_F(3)},
				 {PAD_PULLCTL1, P_5_4(3), P_5_4(P_PULL_UP)},
				 {PAD_PULLCTL1, P_3_2(3), P_3_2(P_PULL_UP)},
				 },
		     },
		    },
};


static const struct device_mfp_config s900_pwm2_cfg = {
	.periph_id = PERIPH_ID_PWM2,
	.mfp_cfg = {
		    {
		     .mfp = 0,
		     .reg_cfg = {
				 {MFP_CTL1, MFP1_28_26_F(0x7),
				  MFP1_28_26_F(0x3)},
				 },
		     },

		    },
};

static const struct device_mfp_config s900_mmc0_cfg = {
	.periph_id = PERIPH_ID_SDMMC0,
	.mfp_cfg = {
		    {
		     .mfp = 0,
		     .reg_cfg = {
				 {MFP_CTL2, MFP2_6_5_F(3), MFP2_6_5_F(0)},
				 {MFP_CTL2, MFP2_8_7_F(3), MFP2_8_7_F(0)},
				 {MFP_CTL2, MFP2_13_11_F(7), MFP2_13_11_F(0)},
				 {MFP_CTL2, MFP2_16_14_F(7), MFP2_16_14_F(0)},
				 {MFP_CTL2, MFP2_19_17_F(7), MFP2_19_17_F(0)},
				 {PAD_PULLCTL1, P_19_18(3), P_19_18(P_PULL_UP)},
				 {PAD_PULLCTL1, P_21_20(3), P_21_20(P_PULL_UP)},
				 {PAD_PULLCTL1, P_23_22(3), P_23_22(P_PULL_UP)},
				 {PAD_PULLCTL1, P_25_24(3), P_25_24(P_PULL_UP)},
				 {PAD_PULLCTL1, P_27_26(3), P_27_26(P_PULL_UP)},
				 {PAD_DRV1, P_21_20(3), P_21_20(2)},
				 {PAD_DRV1, P_17_16(3), P_17_16(2)},

				 },
		     },
		    {
		     .mfp = 1,
		     .reg_cfg = {
				 {MFP_CTL2, MFP2_19_17_F(7), MFP2_19_17_F(5)},
				 {MFP_CTL2, MFP2_16_14_F(7), MFP2_16_14_F(5)},
				 },
		     },

		    },
};

static const struct device_mfp_config s900_mmc2_cfg = {
	.periph_id = PERIPH_ID_SDMMC2,
	.mfp_cfg = {
		    {
			.mfp = 0,
			.reg_cfg = {
				 {MFP_CTL3, MFP3_27_F(1), MFP3_27_F(1)},
				 {PAD_PULLCTL2, P_14(1), P_14(P_PULL_UP)},
				 {PAD_PULLCTL2, P_15(1), P_15(P_PULL_UP)},
				 },
		     },

		    },
};

static const struct device_mfp_config s900_mmc3_cfg = {
	.periph_id = PERIPH_ID_SDMMC3,
	.mfp_cfg = {
		    {
			.mfp = 0,
			.reg_cfg = {
				 {MFP_CTL3, MFP3_11_F(1), MFP3_11_F(1)},
				 {PAD_PULLCTL2, P_12(1), P_12(P_PULL_UP)},
				 {PAD_PULLCTL2, P_15(1), P_15(P_PULL_UP)},
				 },
		     },

		    },
};
static const struct device_mfp_config *s900_dev_cfg[] = {
	&s900_uart0_cfg,
	&s900_i2c0_cfg,
	&s900_i2c3_cfg,
	&s900_i2c5_cfg,
	&s900_pwm2_cfg,
	&s900_mmc0_cfg,
	&s900_mmc2_cfg,
	&s900_mmc3_cfg,
	NULL,

};

int pin_funcmux_select(int periph_id, int mfp)
{
	const struct device_mfp_config *pdev;
	const struct periph_mfp_config *pmfp;
	const struct periph_reg_config *preg;
	int i;

	for (i = 0; (pdev = s900_dev_cfg[i]) != NULL; i++)
		if (pdev->periph_id == periph_id)
			break;

	if (pdev == NULL) {
		debug("%s: invalid periph_id %d\n", __func__, periph_id);
		return -1;
	}

	pmfp = NULL;
	for (i = 0; i < DEVICE_MAX_MFP; i++) {
		if (pdev->mfp_cfg[i].reg_cfg[0].reg == 0)
			break;
		if (pdev->mfp_cfg[i].mfp == mfp) {
			pmfp = &pdev->mfp_cfg[i];
			break;
		}
	}
	if (pmfp == NULL) {
		debug("periph_id %d, invalid mfp %d\n", periph_id, mfp);
		return -1;
	}

	for (i = 0; i < MFP_MAXREG_NUM; i++) {
		preg = &pmfp->reg_cfg[i];
		if (preg->reg == 0)
			break;
		clrsetbits_le32(preg->reg, preg->mask, preg->val);
	}

	return 0;
}
