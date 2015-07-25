/*
* pmu.c - OWL PMIC driver
*
* Copyright (C) 2012, Actions Semiconductor Co. LTD.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published
* by the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

/* No way to use pstore here, SPL will overflow... */

#include <common.h>
#include <asm/io.h>
#include <spi.h>
#include <asm/arch/spi.h>
#include <i2c.h>
#include <asm/arch/pmu.h>
#include <power/owlxx_power.h>
#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>

#define OWLXX_PMU_BUS_ID       (afinfo->bus_id)

#define OWLXX_PMU_DCDC_EN_BM   (afinfo->pmu_dcdc_en_bm)
#define OWLXX_PMU_DCDC_CFGS(A) ((uint)(afinfo->pmu_dcdc_cfg[(A)]))
#define OWLXX_PMU_LDO_EN_BM    (afinfo->pmu_ldo_en_bm)
#define OWLXX_PMU_LDO_CFGS(A)  ((uint)(afinfo->pmu_ldo_cfg[(A)]))

#define OWLXX_PMU_PWM1_DCDC_CONFIG   (afinfo->pwm1_dcdc_config)
#define OWLXX_PMU_PWM2_DCDC_CONFIG   (afinfo->pwm2_dcdc_config)



int s2_resume;
void (*cpu_resume_fn)(void);
int alarm_wakeup;


/* atc2603a ----------------------------------------------------------------- */

struct spi_slave g_atc260x_spi_slave = {
	.bus = 1,
	.cs = 0,
	.max_write_size = 32,
};

static int _atc2603a_reg_read(unsigned short reg)
{
	unsigned short dout[2];
	unsigned int din;
	int ret;

	dout[0] = ((reg & 0x0fff) << 3) & 0x7fff;
	dout[1] = 0;

	ret = spi_xfer(&g_atc260x_spi_slave, 16, dout, &din, SPI_XFER_BEGIN | SPI_XFER_END);
	return ret < 0 ? ret : (unsigned short)(din & 0xffff);
}

static int _atc2603a_reg_write(unsigned short reg, unsigned short val)
{
	unsigned short dout[2];
	int ret;

	dout[0] = ((reg & 0x0fff) << 3) | 0x8000;
	dout[1] = val;

	ret = spi_xfer(&g_atc260x_spi_slave, 16, dout, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	return ret < 0 ? ret : 0;
}

static int _atc2603a_init(void)
{
	int dat, val, wakeup_flag;
	int rtc_h, rtc_ms;
	unsigned int ver = 0;
	unsigned long cpu_resume_fn_addr;

	/* update bandgap from BDG_VOL */
	val = _atc2603a_reg_read(ATC2603A_PMU_BDG_CTL);
	val |= 1 << 11;
	_atc2603a_reg_write(ATC2603A_PMU_BDG_CTL, val);

	/*close charger*/
	val = _atc2603a_reg_read(ATC2603A_PMU_CHARGER_CTL0);
	val &= ~(1 << 15);
	_atc2603a_reg_write(ATC2603A_PMU_CHARGER_CTL0, val);

	/* dcdc1 VDD=1.0V , pwm mode*/
	_atc2603a_reg_read(ATC2603A_PMU_DC1_CTL0);
	val = 0x804f;
	val |= OWLXX_PMU_DCDC_CFGS(1) << 7;
	_atc2603a_reg_write(ATC2603A_PMU_DC1_CTL0, val);

	_atc2603a_reg_read(ATC2603A_PMU_DC1_CTL1);
	_atc2603a_reg_write(ATC2603A_PMU_DC1_CTL1, 0x6cae);

	_atc2603a_reg_read(ATC2603A_PMU_DC1_CTL2);
	_atc2603a_reg_write(ATC2603A_PMU_DC1_CTL2, 0x334b);

	/*dcdc2 1.7v, auto mode*/
	_atc2603a_reg_read(ATC2603A_PMU_DC2_CTL0);
	val = 0x80af;
	val |= OWLXX_PMU_DCDC_CFGS(2) << 8;
	_atc2603a_reg_write(ATC2603A_PMU_DC2_CTL0, val);

	_atc2603a_reg_read(ATC2603A_PMU_DC2_CTL1);
	_atc2603a_reg_write(ATC2603A_PMU_DC2_CTL1, 0xecae);

	_atc2603a_reg_read(ATC2603A_PMU_DC2_CTL2);
	_atc2603a_reg_write(ATC2603A_PMU_DC2_CTL2, 0x334b);

	/* dcdc3 3.1v, PWM */
	_atc2603a_reg_read(ATC2603A_PMU_DC3_CTL0);
	_atc2603a_reg_write(ATC2603A_PMU_DC3_CTL0, 0x8acf);

	_atc2603a_reg_read(ATC2603A_PMU_DC3_CTL1);
	_atc2603a_reg_write(ATC2603A_PMU_DC3_CTL1, 0x254c);

	_atc2603a_reg_read(ATC2603A_PMU_DC3_CTL2);
	_atc2603a_reg_write(ATC2603A_PMU_DC3_CTL2, 0x338a);

	/* enable phy clock */
	dat = _atc2603a_reg_read(ATC2603A_CMU_DEVRST);
	_atc2603a_reg_write(ATC2603A_CMU_DEVRST, dat | (0x1 << 9));

	/* ethenet phy power down */
	dat = _atc2603a_reg_read(ATC2603A_PHY_CONFIG);
	_atc2603a_reg_write(ATC2603A_PHY_CONFIG, (dat & ~(0x3 << 12)) | (0x2 << 12));

	/* disable phy clock */
	dat = _atc2603a_reg_read(ATC2603A_CMU_DEVRST);
	_atc2603a_reg_write(ATC2603A_CMU_DEVRST, dat & ~(0x1 << 9));

	/*enable sd power to use uart5*/
#if 1
	dat = _atc2603a_reg_read(ATC2603A_PMU_SWITCH_CTL);
	ver = (_atc2603a_reg_read(ATC2603A_CVER)&0x7);
    
	if(ver == 0x7)
	{
		/*ver D,
		enable is 0, disable is 1
		switch is 1, LDO is 0
		*/
		dat &= ~0x8038;
		dat |= 0x8;
	}
	else
	{
		/*ver ABC,
		enable is 1, disable is 0
		switch is 0, LDO is 1
		*/
		dat &= ~0x38;
		dat |= 0x8028;    /* enable switch 1, LDO, 3.1v */
	}
	_atc2603a_reg_write(ATC2603A_PMU_SWITCH_CTL, dat);
#endif

	/* ldo6 avdd 1.2v */
	dat = _atc2603a_reg_read(ATC2603A_PMU_LDO6_CTL);
	dat &= ~0xf800;
	dat |= 0x8000;
	_atc2603a_reg_write(ATC2603A_PMU_LDO6_CTL, dat);

	s2_resume = 0;
	cpu_resume_fn = NULL;
	alarm_wakeup = 0;
	/* check suspend to S2 flag */
	dat = _atc2603a_reg_read(ATC2603A_PMU_SYS_CTL3);
	if (dat & PMU_SYS_CTL3_FW_FLAG_S2) {
		cpu_resume_fn_addr = (_atc2603a_reg_read(ATC2603A_PMU_SYS_CTL8) & 0xffff) |
			((_atc2603a_reg_read(ATC2603A_PMU_SYS_CTL9) & 0xff) << 16) |
			(((_atc2603a_reg_read(ATC2603A_PMU_SYS_CTL3) & 0x3c00) >> 6) << 24) |
			(((_atc2603a_reg_read(ATC2603A_PMU_OC_STATUS) & 0x3c) >> 2) << 28);
		rtc_h = _atc2603a_reg_read(ATC2603A_RTC_H);
		rtc_ms = _atc2603a_reg_read(ATC2603A_RTC_MS);
		cpu_resume_fn = (typeof(cpu_resume_fn))cpu_resume_fn_addr;

		wakeup_flag = _atc2603a_reg_read(ATC2603A_PMU_SYS_CTL1);
		printf("%02d:%02d:%02d cpu_resume = %x wakeup flag = %x\n",
			rtc_h,
			(rtc_ms & (0x3f << 6)) >> 6,
			(rtc_ms & 0x3f),
			(u32)cpu_resume_fn,
			wakeup_flag);
		if ((wakeup_flag & (1 << 8)) && ((wakeup_flag & 0xf000) == 0))
			alarm_wakeup = 1;
		if ((alarm_wakeup == 0) || (((u32)cpu_resume_fn & 0x80000000) == 0)) {
			/* clear the S2 flag anyway */
			val = dat & (~PMU_SYS_CTL3_FW_FLAG_S2);
			_atc2603a_reg_write(ATC2603A_PMU_SYS_CTL3, val);
		} else if ((u32)cpu_resume_fn & 0x80000000) {
			/* clear the highest bit of cpu_resume_fn */
			val = _atc2603a_reg_read(ATC2603A_PMU_OC_STATUS);
			val &= 0xffdf;
			_atc2603a_reg_write(ATC2603A_PMU_OC_STATUS, val);
		}
		s2_resume = 1;

		/* check if wakeup from S2 */
		if (dat & 0x8000) {
			/*wakeup from S2, return directly*/
			return 1;
		}
	}

	dat = _atc2603a_reg_read(ATC2603A_PMU_SYS_CTL0);
	dat &= (~(0x7ff<<5));
	dat |= ((WAKEUP_SRC_RESET | WAKEUP_SRC_HDSW |
				 WAKEUP_SRC_ONOFF_LONG |
				 WAKEUP_SRC_WALL_IN |
				 WAKEUP_SRC_VBUS_IN)<<5);
	_atc2603a_reg_write(ATC2603A_PMU_SYS_CTL0, dat);
	
	return 0;
}

static int _atc2603a_probe(void)
{
	int ret = _atc2603a_reg_read(ATC2603A_CMU_HOSCCTL);
	if (ret < 0) {
		return -1;
	}
	if ((ret & ~(1U<<15)) != 0x2aab) {
		return -1;
	}
	return 0;
}

static void _atc2603a_prepare_for_s2(void)
{
	uint ver;

	/* close all LDO opened by _init */
	ver = (_atc2603a_reg_read(ATC2603A_CVER)&0x7);
	if(ver == 0x7)
	{
		/*ver D,
		enable is 0, disable is 1
		switch is 1, LDO is 0
		*/
		atc260x_set_bits(ATC2603A_PMU_SWITCH_CTL, (3U<<14), (2U<<14));
	}
	else
	{
		/*ver ABC,
		enable is 1, disable is 0
		switch is 0, LDO is 1
		*/
		atc260x_set_bits(ATC2603A_PMU_SWITCH_CTL, (3U<<14), (0U<<14));
	}
}



/* atc2603c ----------------------------------------------------------------- */

#define ATC2603C_ADDR	 0x65 /*7 bit slaveaddress, 0xCA >> 1*/

static int _atc2603c_reg_read(unsigned int reg)
{
	unsigned char buf[2] = { 0 };
	u32 ret_val = 0;

	I2C_SET_BUS(OWLXX_PMU_BUS_ID);
	if (i2c_read(ATC2603C_ADDR, reg, 1, buf, 2))
		return -1;

 	ret_val = ((u32)(buf[0])<<8) | buf[1]; //msb
	return ret_val;
}

static int _atc2603c_reg_write(unsigned int reg , unsigned short value)
{
	unsigned char buf[2] = { 0 };

	buf[0] = (value >> 8) & 0xffU;
	buf[1] = value  & 0xffU;

	I2C_SET_BUS(OWLXX_PMU_BUS_ID);
	if (i2c_write(ATC2603C_ADDR, reg , 1 , buf , 2))
		return  -1;

	return 0;
}

#define CHK_DBG_MODE	(0)
#if (CHK_DBG_MODE)
#define CHK_VERIFY_REG(x)	do{printf("%s 0x%x\n", #x, _atc2603c_reg_read(x));} while(0);
#else
#define CHK_VERIFY_REG(x)	do{} while(0);
#endif //(CHK_DBG_MODE)

static int _atc2603c_init(void)
{
	int dat, val, wakeup_flag;
	int rtc_h, rtc_ms;
	unsigned long cpu_resume_fn_addr;

	/* update bandgap from BDG_VOL */
	val = _atc2603c_reg_read(ATC2603C_PMU_BDG_CTL);
	val |= 1 << 11;
	_atc2603c_reg_write(ATC2603C_PMU_BDG_CTL, val);
	CHK_VERIFY_REG(ATC2603C_PMU_BDG_CTL);

	//why close the charger here?
	/*close charger*/
	val = _atc2603c_reg_read(ATC2603C_PMU_CHARGER_CTL0);
	val &= ~(1 << 15);
	_atc2603c_reg_write(ATC2603C_PMU_CHARGER_CTL0, val);
	CHK_VERIFY_REG(ATC2603C_PMU_CHARGER_CTL0);
	
	/* dcdc1 VDD=1.0V , pwm mode*/
	val = 0xe04f;
	val |= OWLXX_PMU_DCDC_CFGS(1) << 7;
	_atc2603c_reg_write(ATC2603C_PMU_DC1_CTL0, val);
	CHK_VERIFY_REG(ATC2603C_PMU_DC1_CTL0);

	_atc2603c_reg_write(ATC2603C_PMU_DC1_CTL1, 0x6cae);
	CHK_VERIFY_REG(ATC2603C_PMU_DC1_CTL1);

	_atc2603c_reg_write(ATC2603C_PMU_DC1_CTL2, 0x334b);
	CHK_VERIFY_REG(ATC2603C_PMU_DC1_CTL2);
	
	/*dcdc2 1.7v, auto mode*/
	val = 0x80af;
	val |= OWLXX_PMU_DCDC_CFGS(2) << 8;
	_atc2603c_reg_write(ATC2603C_PMU_DC2_CTL0, val);
	CHK_VERIFY_REG(ATC2603C_PMU_DC2_CTL0);

	_atc2603c_reg_write(ATC2603C_PMU_DC2_CTL1, 0xecae);
	CHK_VERIFY_REG(ATC2603C_PMU_DC2_CTL1);

	_atc2603c_reg_write(ATC2603C_PMU_DC2_CTL2, 0x334b);
	CHK_VERIFY_REG(ATC2603C_PMU_DC2_CTL2);

	/* dcdc3 3.1v, PWM */
	val = 0x80cf;
	val |= OWLXX_PMU_DCDC_CFGS(3) << 9;
	_atc2603c_reg_write(ATC2603C_PMU_DC3_CTL0, val);
	CHK_VERIFY_REG(ATC2603C_PMU_DC3_CTL0);

	_atc2603c_reg_write(ATC2603C_PMU_DC3_CTL1, 0x254c);
	CHK_VERIFY_REG(ATC2603C_PMU_DC3_CTL1);

	_atc2603c_reg_write(ATC2603C_PMU_DC3_CTL2, 0x338a);
	CHK_VERIFY_REG(ATC2603C_PMU_DC3_CTL2);

	/* LDO 1 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 1)) {
		_atc2603c_reg_write(ATC2603C_PMU_LDO1_CTL, OWLXX_PMU_LDO_CFGS(1)<<13);
	}

	/* LDO 2 & 3  (avcc & avdd1v8) fixed usage */
	_atc2603c_reg_write(ATC2603C_PMU_LDO2_CTL, 5<<13);
	_atc2603c_reg_write(ATC2603C_PMU_LDO3_CTL, 3<<13);

	/* LDO 5 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 5)) {
		_atc2603c_reg_write(ATC2603C_PMU_LDO5_CTL, (OWLXX_PMU_LDO_CFGS(5)<<13) | 1);
	}

	/* ldo6 avdd 1.1v  fixed usage */
	_atc2603c_reg_write(ATC2603C_PMU_LDO6_CTL, 0x8000); /* 0xa000 : 1.2v */

	/* LDO 7 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 7)) {
		_atc2603c_reg_write(ATC2603C_PMU_LDO7_CTL, (OWLXX_PMU_LDO_CFGS(7)<<13) | 1);
	}

	/* LDO 8 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 8)) {
		_atc2603c_reg_write(ATC2603C_PMU_LDO8_CTL, (OWLXX_PMU_LDO_CFGS(8)<<12) | 1);
	}

	/* SWITCH 1, sd power */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 0)) {
		dat = _atc2603c_reg_read(ATC2603C_PMU_SWITCH_CTL);
		dat &= ~0x803a;  /* LDO mode */
		dat |= ((3 & OWLXX_PMU_LDO_CFGS(0)) << 3) | (0 << 15);
		_atc2603c_reg_write(ATC2603C_PMU_SWITCH_CTL, dat);
	}

	//--------------------------------------------------
	// S2 resume here.
	//--------------------------------------------------
	
	s2_resume = 0;
	cpu_resume_fn = NULL;
	alarm_wakeup = 0;
	/* check suspend to S2 flag */
	dat = _atc2603c_reg_read(ATC2603C_PMU_SYS_CTL3);
	if (dat & PMU_SYS_CTL3_FW_FLAG_S2) {

		cpu_resume_fn_addr = (_atc2603c_reg_read(ATC2603C_PMU_SYS_CTL8) & 0xffff) |
			(_atc2603c_reg_read(ATC2603C_PMU_FW_USE0 & 0xffff) << 16);
		rtc_h = _atc2603c_reg_read(ATC2603C_RTC_H);
		rtc_ms = _atc2603c_reg_read(ATC2603C_RTC_MS);
		cpu_resume_fn = (typeof(cpu_resume_fn))cpu_resume_fn_addr;

		wakeup_flag = _atc2603c_reg_read(ATC2603C_PMU_SYS_CTL1);
		printf("%02d:%02d:%02d cpu_resume = %x wakeup flag = %x\n",
			rtc_h,
			(rtc_ms & (0x3f << 6)) >> 6,
			(rtc_ms & 0x3f),
			(u32)cpu_resume_fn,
			wakeup_flag);
		//find the wakeup source
		if ((wakeup_flag & (1 << 8)) && ((wakeup_flag & 0xf000) == 0))
			alarm_wakeup = 1;
		if ((alarm_wakeup == 0) || (((u32)cpu_resume_fn & 0x80000000) == 0)) {
			/* clear the S2 flag anyway */
			val = dat & (~PMU_SYS_CTL3_FW_FLAG_S2);
			_atc2603c_reg_write(ATC2603C_PMU_SYS_CTL3, val);
		} else if ((u32)cpu_resume_fn & 0x80000000) {
			/* clear the highest bit of cpu_resume_fn */
			val = _atc2603c_reg_read(ATC2603C_PMU_FW_USE0);
			val &= ~(1U << 15);
			_atc2603c_reg_write(ATC2603C_PMU_FW_USE0, val);
		}
		s2_resume = 1;

		/* check if wakeup from S2 */
		if (dat & 0x8000) {
			/*wakeup from S2, return directly*/
			return 1;
		}
	}
	//setup wakup source.
	dat = _atc2603c_reg_read(ATC2603C_PMU_SYS_CTL0);
	dat &= (~(0x7ff<<5));
	dat |= ((WAKEUP_SRC_RESET | WAKEUP_SRC_HDSW |
				 WAKEUP_SRC_ONOFF_LONG |
				 WAKEUP_SRC_WALL_IN |
				 WAKEUP_SRC_VBUS_IN)<<5);
	_atc2603c_reg_write(ATC2603C_PMU_SYS_CTL0, dat);

	//for BUG00187912, enable adc data
	if(_atc2603c_reg_read(ATC2603C_CHIP_VER) == 0) {
		/* for revA */
		dat = _atc2603c_reg_read(ATC2603C_PMU_AUXADC_CTL1);
		if (dat & (1<<3))
			_atc2603c_reg_write(ATC2603C_PMU_AUXADC_CTL1, dat & (~(1<< 3)));
		CHK_VERIFY_REG(ATC2603C_PMU_AUXADC_CTL1);
	}

	return 0;
}

static int _atc2603c_probe(void)
{
	int ret = _atc2603c_reg_read(ATC2603C_PMU_OC_INT_EN);
	if (ret < 0) {
		return -1;
	}
	if (ret != 0x1bc0) {
		return -1;
	}
	return 0;
}

static void _atc2603c_prepare_for_s2(void)
{
	atc260x_set_bits(ATC2603C_PMU_LDO5_CTL, (1U<<0), 0);
	atc260x_set_bits(ATC2603C_PMU_LDO7_CTL, (1U<<0), 0);
	atc260x_set_bits(ATC2603C_PMU_LDO8_CTL, (1U<<0), 0);
	atc260x_set_bits(ATC2603C_PMU_SWITCH_CTL, (1U<<15), (1U<<15));
}


/* atc2603b/atc2609a -------------------------------------------------------- */

#define ATC2609A_ADDR	0x65		/*7 bit slaveaddress*/

static int _atc2609a_reg_read(unsigned int reg)
{
	unsigned char buf[2] = { 0 };
	u32 ret_val = 0;

	I2C_SET_BUS(OWLXX_PMU_BUS_ID);
	if (i2c_read(ATC2609A_ADDR, reg, 1, buf, 2))
		return -1;

	ret_val = ((u32)(buf[0])<<8) | buf[1]; // msb
	return ret_val;
}

static int _atc2609a_reg_write(unsigned int reg , unsigned short value)
{
	unsigned char buf[2] = { 0 };

	buf[0] = (value >> 8) & 0xffU;
	buf[1] = value  & 0xffU;

	I2C_SET_BUS(OWLXX_PMU_BUS_ID);
	if (i2c_write(ATC2609A_ADDR, reg , 1 , buf , 2))
		return  -1;
	return 0;
}

static int _atc2609a_init(void)
{
	uint dat, val, wakeup_flag, pmic_ver;
	uint rtc_h, rtc_ms;
	unsigned long cpu_resume_fn_addr;

	pmic_ver = atc260x_get_version();

	/* update bandgap from BDG_VOL */
	val = _atc2609a_reg_read(ATC2609A_PMU_BDG_CTL);
	val |= 1 << 14;
	_atc2609a_reg_write(ATC2609A_PMU_BDG_CTL, val);

	/*close charger*/
	val = _atc2609a_reg_read(ATC2609A_PMU_SWCHG_CTL0);
	val &= ~(1 << 15);
	_atc2609a_reg_write(ATC2609A_PMU_SWCHG_CTL0, val);

	/* DCDC init */
	/* dcdc0 : VDD-GPU
	 * dcdc1 : VDD-CPU
	 * dcdc2 : VDDR
	 * dcdc3 : VCC
	 * dcdc4 : VDD_CORE */
	if(pmic_ver <= 3) {
		/* for rev D */
		/* dcdc0  */
		_atc2609a_reg_write(ATC2609A_PMU_DC0_CTL1, 0x6c0e);
		_atc2609a_reg_write(ATC2609A_PMU_DC0_CTL6, 0x3285);
		_atc2609a_reg_write(ATC2609A_PMU_DC0_CTL0, (OWLXX_PMU_DCDC_CFGS(0) << 8));

		/* dcdc1 */
		_atc2609a_reg_write(ATC2609A_PMU_DC1_CTL1, 0x6c0e);
		_atc2609a_reg_write(ATC2609A_PMU_DC1_CTL6, 0x3285);
		_atc2609a_reg_write(ATC2609A_PMU_DC1_CTL0, (OWLXX_PMU_DCDC_CFGS(1) << 8));

		/* dcdc2 */
		_atc2609a_reg_write(ATC2609A_PMU_DC2_CTL1, 0x6c0e);
		_atc2609a_reg_write(ATC2609A_PMU_DC2_CTL6, 0x3285);
		_atc2609a_reg_write(ATC2609A_PMU_DC2_CTL0, (OWLXX_PMU_DCDC_CFGS(2) << 8));

		/* dcdc3 */
		_atc2609a_reg_write(ATC2609A_PMU_DC3_CTL1, 0x2c0e);
		_atc2609a_reg_write(ATC2609A_PMU_DC3_CTL6, 0xac85);
		_atc2609a_reg_write(ATC2609A_PMU_DC3_CTL0, (OWLXX_PMU_DCDC_CFGS(3) << 8));

		/* dcdc4 */
		_atc2609a_reg_write(ATC2609A_PMU_DC4_CTL1, 0x6c0e);
		_atc2609a_reg_write(ATC2609A_PMU_DC4_CTL6, 0x3285);
		_atc2609a_reg_write(ATC2609A_PMU_DC4_CTL0, (OWLXX_PMU_DCDC_CFGS(4) << 8));

		/* all dcdc on. lower OSC */
		_atc2609a_reg_write(ATC2609A_PMU_DC_OSC, 0Xabfb);
	} else {
		// TODO
		printf("TODO 5303 rev E\n");
		while(1);
	}

	/* ldo-0 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 0)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO0_CTL0, (OWLXX_PMU_LDO_CFGS(0)<<2) | 1);
	}

	/* ldo-1 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 1)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO1_CTL0, (OWLXX_PMU_LDO_CFGS(1)<<2) | 1);
	}

	/* ldo-2, SOC avcc, 3.1v  fixed usage */
	_atc2609a_reg_write(ATC2609A_PMU_LDO2_CTL0, 0x0021);
	/* ldo-3, SOC avcc, 1.8v  fixed usage */
	_atc2609a_reg_write(ATC2609A_PMU_LDO3_CTL0, 0x0017);

	/* ldo-4 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 4)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO4_CTL0, (OWLXX_PMU_LDO_CFGS(4)<<1) | 1);
	}
	/* ldo-5 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 5)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO5_CTL0, (0xfU & OWLXX_PMU_LDO_CFGS(5)<<1) | 1);
	}

	/* ldo-6, SOC avdd, 1.2v  fixed usage */
	if(pmic_ver <= 3)
		_atc2609a_reg_write(ATC2609A_PMU_LDO6_CTL0, 0x001b);
	else
		while(1); //_atc2609a_reg_write(ATC2609A_PMU_LDO6_CTL0, 0x0007);

	/* ldo-7 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 7)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO7_CTL0, (OWLXX_PMU_LDO_CFGS(7)<<1) | 1);
	}
	/* ldo-8 */
	if(OWLXX_PMU_LDO_EN_BM & (1 << 8)) {
		_atc2609a_reg_write(ATC2609A_PMU_LDO8_CTL0, (OWLXX_PMU_LDO_CFGS(8)<<1) | 1);
	}

	/* handle resume */
	s2_resume = 0;
	cpu_resume_fn = NULL;
	alarm_wakeup = 0;
	/* check suspend to S2 flag */
	dat = _atc2609a_reg_read(ATC2609A_PMU_SYS_CTL3);
	if (dat & PMU_SYS_CTL3_FW_FLAG_S2) {

		cpu_resume_fn_addr = (_atc2609a_reg_read(ATC2609A_PMU_SYS_CTL8) & 0xffff) |
			(_atc2609a_reg_read(ATC2609A_PMU_SYS_CTL7 & 0xffff) << 16);
		rtc_h = _atc2609a_reg_read(ATC2609A_RTC_H);
		rtc_ms = _atc2609a_reg_read(ATC2609A_RTC_MS);
		cpu_resume_fn = (typeof(cpu_resume_fn))cpu_resume_fn_addr;

		wakeup_flag = _atc2609a_reg_read(ATC2609A_PMU_SYS_CTL1);
		printf("%02d:%02d:%02d cpu_resume = %x wakeup flag = %x\n",
			rtc_h,
			(rtc_ms & (0x3f << 6)) >> 6,
			(rtc_ms & 0x3f),
			(u32)cpu_resume_fn,
			wakeup_flag);
		//find the wakeup source
		if ((wakeup_flag & (1 << 8)) && ((wakeup_flag & 0xf000) == 0))
			alarm_wakeup = 1;
		if ((alarm_wakeup == 0) || (((u32)cpu_resume_fn & 0x80000000) == 0)) {
			/* clear the S2 flag anyway */
			val = dat & (~PMU_SYS_CTL3_FW_FLAG_S2);
			_atc2609a_reg_write(ATC2609A_PMU_SYS_CTL3, val);
		} else if ((u32)cpu_resume_fn & 0x80000000) {
			/* clear the highest bit of cpu_resume_fn */
			val = _atc2609a_reg_read(ATC2609A_PMU_SYS_CTL7);
			val &= ~(1U << 15);
			_atc2609a_reg_write(ATC2609A_PMU_SYS_CTL7, val);
		}
		s2_resume = 1;

		/* check if wakeup from S2 */
		if (dat & 0x8000) {
			/*wakeup from S2, return directly*/
			return 1;
		}
	}

	/* setup wakup source (reset/hdsw/onoff_l/wall/usb) */
	dat = _atc2609a_reg_read(ATC2609A_PMU_SYS_CTL0);
	dat &= (~(0x7ff<<5));
	dat |= (1U<<15)|(1U<<14)|(1U<<13)|(1U<<7)|(1U<<6);
	_atc2609a_reg_write(ATC2609A_PMU_SYS_CTL0, dat);

	return 0;
}

static int _atc2609a_probe(void)
{
	int ret = _atc2609a_reg_read(ATC2609A_PMU_OC_INT_EN);
	if (ret < 0) {
		return -1;
	}
	if (ret != 0x0ff8) {
		return -1;
	}
	return 0;
}

static void _atc2609a_prepare_for_s2(void)
{
	atc260x_set_bits(ATC2609A_PMU_LDO0_CTL0, (1U<<0), 0);
	atc260x_set_bits(ATC2609A_PMU_LDO1_CTL0, (1U<<0), 0);
	atc260x_set_bits(ATC2609A_PMU_LDO4_CTL0, (1U<<0), 0);
	atc260x_set_bits(ATC2609A_PMU_LDO5_CTL0, (1U<<0), 0);
	atc260x_set_bits(ATC2609A_PMU_LDO7_CTL0, (1U<<0), 0);
	atc260x_set_bits(ATC2609A_PMU_LDO8_CTL0, (1U<<0), 0);
}


/* common ------------------------------------------------------------------- */

int atc260x_reg_read(unsigned short reg)
{
	int ret = -1;
	switch(OWLXX_PMU_ID) {
	case OWLXX_PMU_ID_ATC2603A:
		ret = _atc2603a_reg_read(reg);
		break;
	case OWLXX_PMU_ID_ATC2603B:
		ret = _atc2609a_reg_read(reg);
		break;
	case OWLXX_PMU_ID_ATC2603C:
		ret = _atc2603c_reg_read(reg);
		break;
	}
	return ret;
}

int atc260x_reg_write(unsigned short reg , unsigned short value)
{
	int ret = -1;
	switch(OWLXX_PMU_ID) {
	case OWLXX_PMU_ID_ATC2603A:
		ret = _atc2603a_reg_write(reg, value);
		break;
	case OWLXX_PMU_ID_ATC2603B:
		ret = _atc2609a_reg_write(reg, value);
		break;
	case OWLXX_PMU_ID_ATC2603C:
		ret = _atc2603c_reg_write(reg, value);
		break;
	}
	return ret;
}

int atc260x_set_bits(unsigned int reg, unsigned short mask, unsigned short val)
{
	int ret, ret2 = 0;
	unsigned short r, r2;
	ret = atc260x_reg_read(reg);
	if (ret < 0)
		return ret;
	r = r2 = ret;
	r &= ~mask;
	r |= (val & mask);
	if(r != r2) {
		ret2 = atc260x_reg_write(reg, r);
	}
	return ret2;
}

/*
 * get chip version of atc260x
 */
int atc260x_get_version(void)
{
	static const u16 sc_ver_reg_addr_tbl[OWLXX_PMU_ID_CNT] = {
		[OWLXX_PMU_ID_ATC2603A] = ATC2603A_CVER,
		[OWLXX_PMU_ID_ATC2603C] = ATC2603C_CHIP_VER,
		[OWLXX_PMU_ID_ATC2603B] = ATC2609A_CHIP_VER
	};
	int ret;

	ret = atc260x_reg_read(sc_ver_reg_addr_tbl[OWLXX_PMU_ID]);
	if (ret < 0) {
		printf("Failed to read version data: %d\n", ret);
		return ret;
	}
	switch(ret & 0x1f)
	{
		case 0:
			return 0;
		case 1:
			return 1;
		case 3:
			return 2;
		case 7:
			return 3;
		case 15:
			return 4;
		case 31:
			return 5;
	}
	return -1; 
}


/*  
    config 24M output to pmu 
    debug_port:
        0---clko;
        1---P_KS_OUT2
*/
static void switch_24m_output(int debug_port)
{
	uint reg_val;

    if (debug_port) {
        /* disable CLKO_24M output */
        reg_val = readl(MFP_CTL3);
        reg_val &= ~(0x1<<30);
        writel(reg_val, MFP_CTL3);

        /* output 24M clk from P_KS_OUT2 Debug29 port */
    	reg_val = readl(CMU_ETHERNETPLL);
    	if((reg_val & 1U) == 0) {
    		setbits_le32(CMU_ETHERNETPLL, 0x1);	//i2c clock source.
    		udelay(300);
    	}
    
        reg_val = readl(PAD_DRV1) & (~(0x3<<30));
        writel(reg_val | (0x2<<30) , PAD_DRV1);	/*	set ksout2 drv level to highest	*/
    
    	writel(0x8000002D, CMU_DIGITALDEBUG);	/*	Select timer_clk	*/
    	writel(0x10, DEBUG_SEL);	/*	CMU debug select	*/	
    	writel(0x20000000, DEBUG_OEN0); /*	P_KS_OUT2: Debug29	*/
    	readl(DEBUG_OEN0);

        /* enable CLKO_24M output for bubble gum ethernet*/
        reg_val = readl(MFP_CTL3);
		reg_val |= (0x1<<30);
        writel(reg_val, MFP_CTL3);    	
    	
    } else {
        /* disable output 24M clk from P_KS_OUT2 Debug29 port */
        writel(0x00000000, DEBUG_OEN0);
        
        /* enable CLKO_24M output */
        reg_val = readl(MFP_CTL3);
		reg_val |= (0x1<<30);
        writel(reg_val, MFP_CTL3);
        writel(readl(CMU_ANALOGDEBUG) | (0x1f), CMU_ANALOGDEBUG);   
    }
}

static int _pmu_init_hosc_clk(void)
{
    static const struct {
        u16 reg;
        u16 mask;
        u16 val;
    } magicnum_reg_tbl[OWLXX_PMU_ID_CNT] = {
		[OWLXX_PMU_ID_ATC2603A] = {.reg = ATC2603A_CMU_HOSCCTL, .mask = 0x7fff, .val = 0x2aab}, 
		[OWLXX_PMU_ID_ATC2603C] = {.reg = ATC2603C_SADDR, .mask = 0xffff, .val = 0xca},
		[OWLXX_PMU_ID_ATC2603B] = {.reg = ATC2609A_SADDR, .mask = 0xffff, .val = 0xca},
	};
	int dat, debug_port_24m = 1;
    
    switch_24m_output(1);
retry:
	dat = atc260x_reg_read(magicnum_reg_tbl[OWLXX_PMU_ID].reg) & magicnum_reg_tbl[OWLXX_PMU_ID].mask;
    if (dat != magicnum_reg_tbl[OWLXX_PMU_ID].val) {
        if (debug_port_24m == 1) {
            /* switch 24m to debug port and try again */
            switch_24m_output(0);
            debug_port_24m = 0;
            goto retry;
        }
        
        printf("not find HOSC\n");
        return -1;
    }
    
    return 0;
}

/* probe PMU */
void pmu_early_init(void)
{
	uint bus_id = OWLXX_PMU_BUS_ID;
	uint pmu_type = OWLXX_PMU_ID;
	int ret = -1;

	_pmu_init_hosc_clk();

	switch(pmu_type) {
	case OWLXX_PMU_ID_ATC2603A:
		g_atc260x_spi_slave.bus = bus_id;
		spi_init();
		ret = _atc2603a_probe();
		break;
	case OWLXX_PMU_ID_ATC2603B:
		I2C_SET_BUS(bus_id);
		ret = i2c_probe(ATC2609A_ADDR);
		if(ret != 0)
			break;
		ret = _atc2609a_probe();
		break;
	case OWLXX_PMU_ID_ATC2603C:
		I2C_SET_BUS(bus_id);
		ret = i2c_probe(ATC2603C_ADDR);
		if(ret != 0)
			break;
		ret = _atc2603c_probe();
		break;
	}
	if(ret != 0) {
		printf("%s() Can't find PMU, type=%u bus_id=%u\n",
			__func__, pmu_type, bus_id);
		while(1);
	}
}


#if 0 /* not used */
void set_ddr_voltage(int add_flag)
{
	uint pmu_type = OWLXX_PMU_ID;
	uint reg_addr;
	
	switch(pmu_type) {
	case OWLXX_PMU_ID_ATC2603A:
	case OWLXX_PMU_ID_ATC2603C:
		reg_addr = (pmu_type == OWLXX_PMU_ID_ATC2603A) ?
				ATC2603A_PMU_DC2_CTL0 : ATC2603C_PMU_DC2_CTL0;
		if (add_flag) {
			atc260x_set_bits(reg_addr, (7U<<12), (6U<<12));
		} else {
			atc260x_set_bits(reg_addr, (7U<<12), (5U<<12));
		}
		break;
	case OWLXX_PMU_ID_ATC2603B:
		printf("%s %u\n", __func__, __LINE__);
		while(1); // TODO : ATC2603B
		break;
	}
}
#endif

void vdd_cpu_init(void)
{
	int ret, need_pad=0;

	if(OWLXX_PMU_PWM1_DCDC_CONFIG != 0) {
		printf("pwm1_dcdc_cfg=0x%lx\n", OWLXX_PMU_PWM1_DCDC_CONFIG);

		ret = readl(CMU_DEVCLKEN1);
		ret |= 1<<24;
		writel(ret, CMU_DEVCLKEN1);
		writel(0x1000, CMU_PWM1CLK);

		writel(OWLXX_PMU_PWM1_DCDC_CONFIG, PWM_CTL1);
		readl(PWM_CTL1);

		need_pad = 1;
	} else {
		ret = readl(GPIO_BINEN);
		ret |= 1<<4;
		writel(ret, GPIO_BINEN);
	}

	if(OWLXX_PMU_PWM2_DCDC_CONFIG != 0) {
		printf("pwm2_dcdc_cfg=0x%lx\n", OWLXX_PMU_PWM2_DCDC_CONFIG);

		ret = readl(CMU_DEVCLKEN1);
		ret |= 1<<25;
		writel(ret, CMU_DEVCLKEN1);
		writel(0x1000, CMU_PWM2CLK);

		writel(OWLXX_PMU_PWM2_DCDC_CONFIG, PWM_CTL2);
		readl(PWM_CTL2);

		need_pad = 1;
	} else {
		ret = readl(GPIO_BINEN);
		ret |= 1<<7;
		writel(ret, GPIO_BINEN);
	}

	if(need_pad) {
		/* switch PAD only after PWM is ready! */
		ret = readl(MFP_CTL1);
		ret &= ~0x1c000000;
		ret |= 0x0c000000;
		writel(ret, MFP_CTL1);
		setbits_le32(PAD_CTL, (1 << 1));
	}
}

static int vdd_cpu_voltage_read(void)
{
	uint pmu_type = OWLXX_PMU_ID;
	int result = -1;

	switch(pmu_type) {
	case OWLXX_PMU_ID_ATC2603A:
	case OWLXX_PMU_ID_ATC2603C:
		{
			uint data_reg_addr;
			int ret;
			data_reg_addr = (pmu_type == OWLXX_PMU_ID_ATC2603A) ?
				ATC2603A_PMU_AUXADC0 : ATC2603C_PMU_AUXADC0;
			ret = atc260x_reg_read(data_reg_addr);
			if(ret >= 0)
				result = (uint)ret * 2930 / 1000;	/*mv*/
		}
		break;
	case OWLXX_PMU_ID_ATC2603B:
		printf("%s %u\n", __func__, __LINE__);
		while(1); // TODO : ATC2603B
		break;
	}
	return result;
}

void vdd_cpu_voltage_scan(void)
{
#define SAMPLE_COUNT 10

	int pwm_val;
	int cpu_vol;
	u32 pwm_ctl_bak;
	int i;
	int sum;
	int table_idx = 0;

	pwm_ctl_bak = readl(PWM_CTL1);

	for (pwm_val = 0x3; pwm_val <= 0xf; pwm_val++) {
		clrsetbits_le32(PWM_CTL1, 0x3f, pwm_val);
		mdelay(10);

		debug("pwm ctl1 = 0x%x\n", readl(PWM_CTL1));
		sum = 0;
		for (i = 0; i < SAMPLE_COUNT; i++) {
			cpu_vol = vdd_cpu_voltage_read();
			sum += cpu_vol;
			debug("pwm val = %d, cpu vol = %d\n", pwm_val, cpu_vol);
		}
		cpu_vol = sum / SAMPLE_COUNT;
		debug("pwm val = %d, avg cpu vol = %d\n", pwm_val, cpu_vol);

		afinfo->cpu_pwm_volt_tb[table_idx].pwm_val = pwm_val;
		afinfo->cpu_pwm_volt_tb[table_idx].voltage_mv = cpu_vol;
		table_idx++;
	}

	afinfo->cpu_pwm_volt_tb_len = table_idx;

	writel(pwm_ctl_bak, PWM_CTL1);
}

void vdd_cpu_voltage_store(void)
{
	struct cpu_pwm_volt_info *info;
	int len;
	int i;

	len = afinfo->cpu_pwm_volt_tb_len;
	info = &kinfo->cpu_pwm_volt;
	info->cpu_pwm_volt_tb_len = len;

	for (i = 0; i < len; i++) {
		info->cpu_pwm_volt_tb[i].pwm_val = 
				afinfo->cpu_pwm_volt_tb[i].pwm_val;

		info->cpu_pwm_volt_tb[i].voltage_mv = 
				afinfo->cpu_pwm_volt_tb[i].voltage_mv;
	}
}

int pmu_init(void)
{
	uint pmu_type;
	int ret = -1;

	pmu_early_init();
	vdd_cpu_init();

	pmu_type = OWLXX_PMU_ID;
	switch(pmu_type) {
	case OWLXX_PMU_ID_ATC2603A:
		ret = _atc2603a_init();
		break;
	case OWLXX_PMU_ID_ATC2603B:
		ret = _atc2609a_init();
		break;
	case OWLXX_PMU_ID_ATC2603C:
		ret = _atc2603c_init();
		break;
	}
	if(ret < 0) {
		printf("%s() Can't init PMU, ret=%d type=%u\n",
			__func__, ret, pmu_type);
		while(1);
		return ret;
	}

	return 0;
}

#ifndef CONFIG_SPL_OWLXX_UPGRADE	
void pmu_prepare_for_s2(void)
{
	uint pmu_type;

	pmu_type = OWLXX_PMU_ID;
	switch(pmu_type) {
	case OWLXX_PMU_ID_ATC2603A:
		_atc2603a_prepare_for_s2();
		break;
	case OWLXX_PMU_ID_ATC2603B:
		_atc2609a_prepare_for_s2();
		break;
	case OWLXX_PMU_ID_ATC2603C:
		_atc2603c_prepare_for_s2();
		break;
	}
}
#endif