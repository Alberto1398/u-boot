/*
 * OWL GEP(generic EDP Panel)
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
#define DEBUGX
#define pr_fmt(fmt) "owl_panel_tc358860: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <i2c.h>
#include <asm-generic/errno.h>
#include <asm/io.h>
#include <malloc.h>

#include <dss.h>

#define EDP2DSI_ADDR		0x1c >> 1 /*TODO*/

struct tc358860_i2c_dev {
	int			bus_id;
};

struct panel_tc358860_data {

	/* data for panel contorl */
	struct gpio_desc	reset_gpio;
	struct gpio_desc	reset1_gpio;

	struct gpio_desc	power_gpio;
	struct gpio_desc	power1_gpio;
	struct gpio_desc	power2_gpio;

	struct tc358860_i2c_dev	i2c_dev;
};

static int panel_tc358860_power_on(struct owl_panel *panel)
{
	int val;
	struct  panel_tc358860_data *tc358860 = panel->pdata;

	debug("%s, start!\n", __func__);

	/* tc358860 IC deassert */
	if (dm_gpio_is_valid(&tc358860->reset_gpio))
		dm_gpio_set_value(&tc358860->reset_gpio, 0);
	mdelay(10);
	/* tc358860 IC power on */
	if (dm_gpio_is_valid(&tc358860->power_gpio))
		dm_gpio_set_value(&tc358860->power_gpio, 1);
	else
		debug("%s, set tc358860 power gpio en failed!\n", __func__);
	mdelay(10);

	/* tc358860 IC assert */
	if (dm_gpio_is_valid(&tc358860->reset_gpio))
		dm_gpio_set_value(&tc358860->reset_gpio, 1);
	else
		debug("%s, set tc358860 reset gpio failed!\n", __func__);
	mdelay(10);
	/* tc358860 IC deassert */
	if (dm_gpio_is_valid(&tc358860->reset_gpio))
		dm_gpio_set_value(&tc358860->reset_gpio, 0);

	/* panel assert */
	if (dm_gpio_is_valid(&tc358860->reset1_gpio))
		dm_gpio_set_value(&tc358860->reset1_gpio, 1);
	else
		debug("%s, set panel reste gpio failed!\n", __func__);
	/* pull low level, panel vddio */
	if (dm_gpio_is_valid(&tc358860->power1_gpio))
		dm_gpio_set_value(&tc358860->power1_gpio, 0);
	else
		debug("%s, set vddio en gpio failed!\n", __func__);
	/* pull low level, panel vsp */
	if (dm_gpio_is_valid(&tc358860->power2_gpio))
		dm_gpio_set_value(&tc358860->power2_gpio, 0);
	else
		debug("%s, set vsp gpio en failed!\n", __func__);
	mdelay(10);
	/* pull hight level, panel vddio */
	if (dm_gpio_is_valid(&tc358860->power1_gpio))
		dm_gpio_set_value(&tc358860->power1_gpio, 1);
	else
		debug("%s, set vddio en gpio failed!\n", __func__);
	mdelay(200);
	/* pull hight level, panel vsp */
	if (dm_gpio_is_valid(&tc358860->power2_gpio))
		dm_gpio_set_value(&tc358860->power2_gpio, 1);
	else
		debug("%s, set vsp en gpio failed!\n", __func__);
	mdelay(10);
	/* panel deassert */
	if (dm_gpio_is_valid(&tc358860->reset1_gpio))
		dm_gpio_set_value(&tc358860->reset1_gpio, 0);
	else
		debug("%s, set panel reset gpio failed!\n", __func__);

	debug("%s, end!\n", __func__);
	return 0;
}

static int panel_tc358860_power_off(struct owl_panel *panel)
{
	struct  panel_tc358860_data *tc358860 = panel->pdata;

	dm_gpio_set_value(&tc358860->power_gpio, 0);

	return 0;
}

void tc358860_i2c_write(struct panel_tc358860_data *tc358860,
			u32 addr, u32 cnt, u64 dat)
{
	u32 tmp,i;
	int ret;
	char buffer[6];

	debug("%s, addr 0x%x, cnt %d, data 0x%lx\n", __func__, addr, cnt, dat);

	for (i = 0; i < cnt; i++) {
		buffer[i] = (dat >> (8 * (cnt - i - 1))) & 0xff;
		debug("buffer[%d] 0x%x\n", i, buffer[i]);
	}

	i2c_set_bus_num(tc358860->i2c_dev.bus_id);
	ret = i2c_write(EDP2DSI_ADDR, addr, 2, (uint8_t *)&buffer, cnt);
}

void tc358860_write32(struct panel_tc358860_data *tc358860,
				u32 addr, u32 dat)
{
	u64 dat_tmp, tmp[4] = {0};

	tmp[0] = dat & 0xff;
	tmp[1] = (dat>>8) & 0xff;
	tmp[2] = (dat>>16) & 0xff;
	tmp[3] = (dat>>24) & 0xff;

	dat = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | (tmp[3]);

	tc358860_i2c_write(tc358860, addr, 0x04, dat);
}

void tc358860_write8(struct panel_tc358860_data *tc358860, u32 addr, u32 dat)
{
	u64 dat_tmp;
	dat_tmp = (addr<<8) | dat;

	tc358860_i2c_write(tc358860, addr, 0x01, dat);
}

u32 tc358860_i2c_read(struct panel_tc358860_data *tc358860,
			u32 addr, u32 cnt)
{
	u32 tmp, dat, i;
	int ret;

	unsigned int buffer;
	char *pbuffer;

	i2c_set_bus_num(tc358860->i2c_dev.bus_id);

	ret = i2c_read(EDP2DSI_ADDR, addr, 2, &dat, cnt);

	if (ret < 0) {
		debug("%s, fail to read edp i2c data(%d)\n", __func__, ret);
	}
#if 0
	pbuffer = &buffer;
	for (i = 0; i < cnt; i++) {
		pbuffer[i] = (dat >> (8 * (cnt - i - 1))) & 0xff;
		debug("pbuffer[%d] 0x%x\n", i, pbuffer[i]);
	}
#endif
	return dat;
}

u32 tc358860_read32(struct panel_tc358860_data *tc358860, u32 addr)
{
	u32 tmp;

	tmp = tc358860_i2c_read(tc358860, addr, 0x4);

	return tmp;
}

u32 tc358860_read8(struct panel_tc358860_data *tc358860, u32 addr)
{
	u32 tmp;

	tmp = tc358860_i2c_read(tc358860, addr,0x1);

	return tmp;
}

void owl_tc358860_colorbar(struct panel_tc358860_data *tc358860)
{
	unsigned long l_val;

	debug("%s, start!\n", __func__);

	/* IO Voltahge Setting	*/
	tc358860_write32(tc358860, 0x0800,0x00000001); /* IOB_CTRL1 */
	/* Boot Settings */
	tc358860_write32(tc358860, 0x1000,0x00006978); /* BootWaitCount */
	tc358860_write32(tc358860, 0x1004,0x00040907); /* Boot Set0 */
	tc358860_write32(tc358860, 0x1008,0x0366000C); /* Boot Set1 */
	tc358860_write32(tc358860, 0x100C,0x130002D5); /* Boot Set2 */
	tc358860_write32(tc358860, 0x1010,0x00640020); /* Boot Set3 */
	tc358860_write32(tc358860, 0x1014,0x00000005); /* Boot Ctrl */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x1018))!=0x00000002){;} /* Check if 0x1018<bit2:0> is expected value */
	/* Internal PCLK Setting for Non Preset or REFCLK=26MHz	*/
	tc358860_write8(tc358860, 0xB005,0x06); /* SET CG_VIDPLL_CTRL1 */
	tc358860_write8(tc358860, 0xB006,0x00); /* SET CG_VIDPLL_CTRL2 */
	tc358860_write8(tc358860, 0xB007,0x09); /* SET CG_VIDPLL_CTRL3 */
	tc358860_write8(tc358860, 0xB008,0x00); /* SET CG_VIDPLL_CTRL4 */
	tc358860_write8(tc358860, 0xB009,0x21); /* SET CG_VIDPLL_CTRL5 */
	tc358860_write8(tc358860, 0xB00A,0x06); /* SET CG_VIDPLL_CTRL6 */
	tc358860_write32(tc358860, 0x1014,0x00000007); /* Boot Ctrl */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x1018))!=0x00000007){;} /* Check if 0x1018<bit2:0> is expected value */
	tc358860_write32(tc358860, 0x4158,0x00240008); /* PPI_DPHY_TCLK_HEADERCNT */
	tc358860_write32(tc358860, 0x4160,0x000E0007); /* PPI_DPHY_THS_HEADERCNT */
	tc358860_write32(tc358860, 0x4164,0x00002134); /* PPI_DPHY_TWAKEUPCNT */
	tc358860_write32(tc358860, 0x4168,0x0000000D); /* PPI_DPHY_TCLK_POSTCNT	*/
	/* DSI Start */
	tc358860_write32(tc358860, 0x407C,0x00000081); /* DSI_DSITX_MODE */
	tc358860_write32(tc358860, 0x4050,0x00000000); /* DSI_FUNC_MODE	*/
	tc358860_write32(tc358860, 0x401C,0x00000001); /* DSI_DSITX_START */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4060))!=0x00000003){;} /* Check if 0x2060/4060<bit1:0>=11b. */
	/* GPIO setting for LCD.  (Depends on LCD specification and System configuration) */
	tc358860_write32(tc358860, 0x0804,0x00000000); /* IOB_CTRL2 */
	tc358860_write32(tc358860, 0x0080,0x0000000F); /* GPIOC	*/
	tc358860_write32(tc358860, 0x0084,0x0000000F); /* GPIOO	*/
	tc358860_write32(tc358860, 0x0084,0x00000000); /* GPIOO	*/
	tc358860_write32(tc358860, 0x0084,0x0000000F); /* GPIOO	*/
	mdelay(50);
	/* DSI Hs Clock Mode */
	tc358860_write32(tc358860, 0x4050,0x00000020); /* DSI_FUNC_MODE	*/
	mdelay(100);
	/* Command Transmission Before Video Start. (Depeds on LCD specification) */
	/* LCD Initialization */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000000B0); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000001D6); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000018B3); /* DSIG_CQ_PAYLOAD */
	mdelay(1);

	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x0000FF51); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x00000C53); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x00000035); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000003B0); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x81002905); /* DSIG_CQ_HEADER */
	mdelay(200);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x81001105); /* DSIG_CQ_HEADER */
	mdelay(200);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001){;} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x2A10,0x80040010); /* DSI0_CQMODE */
	tc358860_write32(tc358860, 0x3A10,0x80040010); /* DSI1_CQMODE */
	tc358860_write32(tc358860, 0x2A04,0x00000001); /* DSI0_VideoSTART */
	tc358860_write32(tc358860, 0x3A04,0x00000001); /* DSI1_VideoSTART */
	/* Color Bar Setting */
	tc358860_write32(tc358860, 0x0300,0x003C003C); /* CBR00_HTIM1 */
	tc358860_write32(tc358860, 0x0304,0x00B405A0); /* CBR00_HTIM2 */
	tc358860_write32(tc358860, 0x0308,0x00040004); /* CBR00_VTIM1 */
	tc358860_write32(tc358860, 0x030C,0x000C0A00); /* CBR00_VTIM2 */
	tc358860_write32(tc358860, 0x0310,0x00000001); /* CBR00_MODE */
	tc358860_write32(tc358860, 0x0314,0x00FF0000); /* CBR00_COLOR */
	tc358860_write32(tc358860, 0x0318,0x00000001); /* CBR00_ENABLE */
	tc358860_write32(tc358860, 0x031C,0x00000001); /* CBR00_START */
	/* Command Transmission After Video Start. (Depends on LCD specification) */
	mdelay(1000);

	debug("%s, end\n", __func__);
}

/*
 * eDP timing
 * pclk= 250MHz.
 * hsync_width = 32
 * hbp = 80
 * hfp = 80
 * vsync_width= 2
 * vbp= 6
 * vfp = 8 
 * pclk = 250  2560*1440
 * */
void panel_tc358860_init_cmd(struct  panel_tc358860_data *tc358860)
{
	unsigned int temp;
	unsigned char c_val;
	unsigned long l_val;

	printf("%s, start!\n", __func__);
	/* IO Voltahge Setting */
	// tc358860_write32(tc358860, 0x0800,0x00000001); /* IOB_CTRL1 */
	tc358860_write32(tc358860, 0x0800,0x00000000); /* IOB_CTRL1 */

	/* Boot Settings */
	tc358860_write32(tc358860, 0x1000,0x00006978); /* BootWaitCount */
	tc358860_write32(tc358860, 0x1004,0x00040907); /* Boot Set0 */
	tc358860_write32(tc358860, 0x1008,0x03610008); /* Boot Set1 */
	tc358860_write32(tc358860, 0x100C,0x23000332); /* Boot Set2 */
	tc358860_write32(tc358860, 0x1010,0x00E60020); /* Boot Set3 */
	tc358860_write32(tc358860, 0x1014,0x00000001); /* Boot Ctrl */
	mdelay(1);
	while ((l_val = tc358860_read32(tc358860, 0x1018)) != 0x00000002) {
		;
	} /* Check if 0x1018<bit2:0> is expected value */

	/* Internal PCLK Setting for Non Preset or REFCLK=26MHz */
	tc358860_write8(tc358860, 0xB005,0x06); /* SET CG_VIDPLL_CTRL1 */
	tc358860_write8(tc358860, 0xB006,0x04); /* SET CG_VIDPLL_CTRL2 */
	tc358860_write8(tc358860, 0xB007,0x3A); /* SET CG_VIDPLL_CTRL3 */
	tc358860_write8(tc358860, 0xB008,0x00); /* SET CG_VIDPLL_CTRL4 */
	tc358860_write8(tc358860, 0xB009,0x21); /* SET CG_VIDPLL_CTRL5 */
	tc358860_write8(tc358860, 0xB00A,0x08); /* SET CG_VIDPLL_CTRL6 */

	tc358860_write32(tc358860, 0x1014,0x00000003); /* Boot Ctrl */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x1018))!=0x00000006) {
		;
	} /* Check if 0x1018<bit2:0> is expected value */

	/* Additional Setting for eDP */
	tc358860_write8(tc358860, 0x8003,0x41); /* Max Downspread */
	tc358860_write8(tc358860, 0xB400,0x0D); /* AL Mode Control Link */
	/* DPRX CAD Register Setting */
	tc358860_write8(tc358860, 0xB88E,0xFF); /* Set CR_OPT_WCNT0 */
	tc358860_write8(tc358860, 0xB88F,0xFF); /* Set CR_OPT_WCNT1 */
	tc358860_write8(tc358860, 0xB89A,0xFF); /* Set CR_OPT_WCNT2 */
	tc358860_write8(tc358860, 0xB89B,0xFF); /* Set CR_OPT_WCNT3 */
	tc358860_write8(tc358860, 0xB800,0x0E); /* Set CDR_PHASE_LP_EN */
	tc358860_write8(tc358860, 0xBB26,0x02); /* RX_VREG_VALUE */
	tc358860_write8(tc358860, 0xBB01,0x20); /* RX_VREG_ENABLE	*/
	tc358860_write8(tc358860, 0xB8C0,0xF1); /* RX_CDR_LUT1 */
	tc358860_write8(tc358860, 0xB8C1,0xF1); /* RX_CDR_LUT2 */
	tc358860_write8(tc358860, 0xB8C2,0xF0); /* RX_CDR_LUT3 */
	tc358860_write8(tc358860, 0xB8C3,0xF0); /* RX_CDR_LUT4 */
	tc358860_write8(tc358860, 0xB8C4,0xF0); /* RX_CDR_LUT5 */
	tc358860_write8(tc358860, 0xB8C5,0xF0); /* RX_CDR_LUT6 */
	tc358860_write8(tc358860, 0xB8C6,0xF0); /* RX_CDR_LUT7 */
	tc358860_write8(tc358860, 0xB8C7,0xF0); /* RX_CDR_LUT8 */
	tc358860_write8(tc358860, 0xB80B,0x00); /* PLL_CP1P1 */
	tc358860_write8(tc358860, 0xB833,0x00); /* PLL_CP1P2 */
	tc358860_write8(tc358860, 0xB85B,0x00); /* PLL_CP1P3 */
	tc358860_write8(tc358860, 0xB810,0x00); /* PLL_CP2P1 */
	tc358860_write8(tc358860, 0xB838,0x00); /* PLL_CP2P2 */
	tc358860_write8(tc358860, 0xB860,0x00); /* PLL_CP2P3 */
	tc358860_write8(tc358860, 0xB815,0x00); /* PLL_CP2P4 */
	tc358860_write8(tc358860, 0xB83D,0x00); /* PLL_CP2P5 */
	tc358860_write8(tc358860, 0xB865,0x00); /* PLL_CP2P6 */
	tc358860_write8(tc358860, 0xB81A,0x00); /* PLL_CP2P7 */
	tc358860_write8(tc358860, 0xB842,0x00); /* PLL_CP2P8 */
	tc358860_write8(tc358860, 0xB86A,0x00); /* PLL_CP2P9 */
	tc358860_write8(tc358860, 0xB81F,0x00); /* PLL_CP3P1 */
	tc358860_write8(tc358860, 0xB847,0x00); /* PLL_CP3P2 */
	tc358860_write8(tc358860, 0xB86F,0x00); /* PLL_CP3P3 */
	tc358860_write8(tc358860, 0xB824,0x00); /* PLL_CP4P1 */
	tc358860_write8(tc358860, 0xB84C,0x00); /* PLL_CP4P2 */
	tc358860_write8(tc358860, 0xB874,0x00); /* PLL_CP4P3 */
	tc358860_write8(tc358860, 0xB829,0x00); /* PLL_CP4P4 */
	tc358860_write8(tc358860, 0xB851,0x00); /* PLL_CP4P5 */
	tc358860_write8(tc358860, 0xB879,0x00); /* PLL_CP4P6 */
	tc358860_write8(tc358860, 0xB82E,0x00); /* PLL_CP5P7 */
	tc358860_write8(tc358860, 0xB856,0x00); /* PLL_CP5P2 */
	tc358860_write8(tc358860, 0xB87E,0x00); /* PLL_CP5P3 */
	tc358860_write8(tc358860, 0xBB90,0x10); /* ctle_em_data_rate_control_0[7:0] */
	tc358860_write8(tc358860, 0xBB91,0x0F); /* ctle_em_data_rate_control_1[7:0] */
	tc358860_write8(tc358860, 0xBB92,0xF6); /* ctle_em_data_rate_control_2[7:0] */
	tc358860_write8(tc358860, 0xBB93,0x10); /* ctle_em_data_rate_control_3[7:0] */
	tc358860_write8(tc358860, 0xBB94,0x0F); /* ctle_em_data_rate_control_4[7:0] */
	tc358860_write8(tc358860, 0xBB95,0xF6); /* ctle_em_data_rate_control_5[7:0] */
	tc358860_write8(tc358860, 0xBB96,0x10); /* ctle_em_data_rate_control_6[7:0] */
	tc358860_write8(tc358860, 0xBB97,0x0F); /* ctle_em_data_rate_control_7[7:0] */
	tc358860_write8(tc358860, 0xBB98,0xF6); /* ctle_em_data_rate_control_8[7:0] */
	tc358860_write8(tc358860, 0xBB99,0x10); /* ctle_em_data_rate_control_A[7:0] */
	tc358860_write8(tc358860, 0xBB9A,0x0F); /* ctle_em_data_rate_control_B[7:0] */
	tc358860_write8(tc358860, 0xBB9B,0xF6); /* ctle_em_data_rate_control_0[7:0] */
	tc358860_write8(tc358860, 0xB88A,0x03); /* CR_OPT_CTRL */
	tc358860_write8(tc358860, 0xB896,0x03); /* EQ_OPT_CTRL */
	tc358860_write8(tc358860, 0xBBD1,0x07); /* ctle_em_contro_1 */
	tc358860_write8(tc358860, 0xBBB0,0x07); /* eye_configuration_0 */
	tc358860_write8(tc358860, 0xB88B,0x04); /* CR_OPT_MIN_EYE_VALID */
	tc358860_write8(tc358860, 0xB88C,0x45); /* CR_OPT_WCNT0_EYE */
	tc358860_write8(tc358860, 0xB88D,0x05); /* CT_OPT_WCNT1_EYE */
	tc358860_write8(tc358860, 0xB897,0x04); /* EQ_OPT_MIN_EYE_VALID */
	tc358860_write8(tc358860, 0xB898,0xE0); /* EQ_OPT_WCNT0_FEQ */
	tc358860_write8(tc358860, 0xB899,0x2E); /* EQ_OPT_WCNT1_FEQ */
	tc358860_write8(tc358860, 0x800E,0x00); /* TRAINING_AUX_RD_INTERVAL */
	tc358860_write32(tc358860, 0x1014,0x00000007); /* Boot Ctrl */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x1018))!=0x00000007) {
		;
	} /* Check if 0x1018<bit2:0> is expected value */

	/* eDP Settings for Link Training */
	while ((c_val = tc358860_read8(tc358860, 0xB631))!=0x01) {
		;
	} /* Check if 0xB631<bit1:0>=01b. */

	tc358860_write8(tc358860, 0x8000,0x11); /* DPCD Rev */
	tc358860_write8(tc358860, 0x8001,0x0A); /* Max Link Rate */
	tc358860_write8(tc358860, 0x8002,0x04); /* Max Lane Count	*/
	tc358860_write8(tc358860, 0xB608,0x0B); /* Set AUXTXHSEN	*/
	tc358860_write8(tc358860, 0xB800,0x1E); /* Set CDR_PHASE_LP_EN */
	tc358860_write8(tc358860, 0x8700,0x00); /* DPCD 0700h */
	tc358860_write32(tc358860, 0x5010,0x009D0000); /* Monitor Signal Selection */
	tc358860_write32(tc358860, 0x008C,0x00000040); /* GPIOOUTMODE */
	tc358860_write32(tc358860, 0x0080,0x00000002); /* GPIOC */

	/* DSI Transition Time Setting for Non Preset */

	tc358860_write32(tc358860,0x4158,0x00280009); // PPI_DPHY_TCLK_HEADERCNT

	tc358860_write32(tc358860,0x4160,0x000F0007); // PPI_DPHY_THS_HEADERCNT
	tc358860_write32(tc358860,0x4164,0x00002328); // PPI_DPHY_TWAKEUPCNT
	tc358860_write32(tc358860,0x4168,0x0000000E); // PPI_DPHY_TCLK_POSTCNT

	/* DSI Start */
	tc358860_write32(tc358860, 0x407C,0x00000081); /* DSI_DSITX_MODE */
	tc358860_write32(tc358860, 0x4050,0x00000000); /* DSI_FUNC_MODE */
	tc358860_write32(tc358860, 0x401C,0x00000001); /* DSI_DSITX_START */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4060))!=0x00000003) {
		;
	} /* Check if 0x2060/4060<bit1:0>=11b. */

	/* GPIO setting for LCD control.  (Depends on LCD specification and System configuration) */
	tc358860_write32(tc358860, 0x0804,0x00000000); /* IOB_CTRL2 */
	tc358860_write32(tc358860, 0x0080,0x0000000F); /* GPIOC */
	tc358860_write32(tc358860, 0x0084,0x0000000F); /* GPIOO */
	tc358860_write32(tc358860, 0x0084,0x00000000); /* GPIOO */
	tc358860_write32(tc358860, 0x0084,0x0000000F); /* GPIOO */
	mdelay(50);
	/* DSI Hs Clock Mode */

	/* Command Transmission Before Video Start. (Depeds on LCD specification) */
	/* LCD Initialization */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000000B0); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000001D6); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000018B3); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x0000FF51); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x00000C53); /* DSIG_CQ_PAYLOAD */
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x83000239); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x00000035); /* DSIG_CQ_PAYLOAD	*/
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	/* change color */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000004c1); /* DSIG_CQ_PAYLOAD	*/
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */
	tc358860_write32(tc358860, 0x42FC,0x83000229); /* DSIG_CQ_HEADER */
	tc358860_write32(tc358860, 0x4300,0x000003B0); /* DSIG_CQ_PAYLOAD	*/
	mdelay(1);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x81002905); /* DSIG_CQ_HEADER	*/
	mdelay(200);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1 */

	tc358860_write32(tc358860, 0x42FC,0x81001105); /* DSIG_CQ_HEADER */
	mdelay(200);
	while ((l_val=tc358860_read32(tc358860, 0x4200))!=0x00000001) {
		;
	} /* Check if <bit0>=1	*/

	tc358860_write32(tc358860, 0x2A10,0x80040010); /* DSI0_CQMODE */
	tc358860_write32(tc358860, 0x3A10,0x80040010); /* DSI1_CQMODE */
	tc358860_write32(tc358860, 0x2A04,0x00000001); /* DSI0_VideoSTART */
	tc358860_write32(tc358860, 0x3A04,0x00000001); /* DSI1_VideoSTART */
	/* Check if eDP video is coming */
	tc358860_write32(tc358860, 0x0154,0x00000001); /* Set_DPVideoEn */
	/* Command Transmission After Video Start. (Depends on LCD specification) */
	mdelay(100);

	printf("%s, end!\n", __func__);
}
static int panel_tc358860_enable(struct owl_panel *panel)
{
	uint32_t val;
	struct owl_display_ctrl *ctrl = panel->ctrl;
	struct  panel_tc358860_data *tc358860 = panel->pdata;

	debug("%s, start!\n", __func__);

	panel_tc358860_init_cmd(tc358860);

	/* sharp tc358860 colorbar for teset */
	#if 0
	owl_tc358860_colorbar(tc358860);
	#endif

	/* edp controller aux training  TODO */
	if (ctrl->ops && ctrl->ops->aux_write)
		ctrl->ops->aux_write(ctrl, NULL, 0);

	debug("%s, end\n", __func__);

	return 0;
}

static int panel_tc358860_disable(struct owl_panel *panel)
{

	return 0;
}

struct owl_panel_ops owl_panel_tc358860_ops = {
	.power_on = panel_tc358860_power_on,
	.power_off = panel_tc358860_power_off,

	.enable = panel_tc358860_enable,
	.disable = panel_tc358860_disable,
};

static struct owl_panel owl_panel_tc358860 = {
	.desc = {
		.name = "panel_tc358860",
		.type = OWL_DISPLAY_TYPE_EDP,
		.ops = &owl_panel_tc358860_ops,
	},
};


int owl_edp2dsi_i2c_probe(struct panel_tc358860_data *tc358860,
				const void *blob)
{
	int ret, node, parent, tmp;
	struct udevice *dev;

	debug("%s\n", __func__);

	tc358860->i2c_dev.bus_id = -1;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,edp-i2c-init");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	/* get bus ID */
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		error("%s, Cannot find node parent\n", __func__);
		return -1;
	}

	tc358860->i2c_dev.bus_id = i2c_get_bus_num_fdt(parent);
	if (tc358860->i2c_dev.bus_id < 0) {
		error("can not get bus id\n");
		return -EINVAL;
	}
	debug("%s, bus id %d\n", __func__, tc358860->i2c_dev.bus_id);

	/*
	 * I2C probe
	 */
	i2c_set_bus_num(tc358860->i2c_dev.bus_id);

	ret = i2c_get_chip_for_busnum(tc358860->i2c_dev.bus_id,
			EDP2DSI_ADDR, 1, &dev);
	if (ret < 0)
		debug("i2c_get_chip_for_busnum failed(%d)!\n", ret);

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret < 0)
		debug("i2c_set_chip_offset_len failed(%d)!\n", ret);

	return 0;
}

int owl_panel_tc358860_init(const void *blob)
{
	int node;
	int ret = 0;
	struct  panel_tc358860_data *tc358860;

	debug("%s\n", __func__);

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-tc358860");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}

	tc358860 = malloc(sizeof(*tc358860));
	if (!tc358860) {
		error("malloc tc358860 data failed\n");
		return 0;
	}

	ret = owl_panel_parse_panel_info(blob, node, &owl_panel_tc358860);
	if (ret < 0) {
		error("failed to parse timings\n");
		return ret;
	}

	/* parse power gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power-gpio", 0,
				&tc358860->power_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power-gpio failed\n", __func__);
	mdelay(10);
	/* parse power1 gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power1-gpio", 0,
				&tc358860->power1_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power1-gpio failed\n", __func__);
	/* parse power2 gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "power2-gpio", 0,
				&tc358860->power2_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_power2-gpio failed\n", __func__);
	/* parse reset gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "reset-gpio", 0,
				&tc358860->reset_gpio, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE) < 0)
		debug("%s: fdtdec_decode_reset-gpio failed\n", __func__);
	/* parse reset1 gpio ... */
	if (gpio_request_by_name_nodev(blob, node, "reset1-gpio", 0,
				&tc358860->reset1_gpio, GPIOD_IS_OUT) < 0)
		debug("%s: fdtdec_decode_reset1-gpio failed\n", __func__);

	ret = owl_panel_register(&owl_panel_tc358860);
	if (ret < 0) {
		error("failed to register panel\n");
		return ret;
	}
	owl_panel_tc358860.pdata = tc358860;

	/* initial sharp tc358860 edp2dsi i2c device */
	owl_edp2dsi_i2c_probe(tc358860, blob);

	return 0;
}
