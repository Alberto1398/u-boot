/*
 * gauge_eg2801.c - EG2801 driver
 *
 * Copyright (C) 2012, Actions Semiconductor Co. LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gmp.h>
#include <i2c.h>
#include <power/gauge_eg2801.h>

#define OWLXX_GAUGE_BUS_ID   (afinfo->gauge_bus_id)

static int sda_shift, sck_shift;

static int gague_read_reg_byte(uchar i2c_addr, uchar i2c_reg)
{
	unsigned char val;

	if (i2c_read(i2c_addr, i2c_reg, 1, &val, 1))
		return -1;

	return val;
}

static inline void i2c_clk(int bus_id, int num)
{
	int i, tmp;

	if (0 == bus_id) {
		/*SDA: GPIOC26  SCK:GPIOC27*/
		sda_shift = 26;
		sck_shift = 27;
	} else if (1 == bus_id) {
		/*SDA: GPIOC29  SCK:GPIOC28*/
		sda_shift = 29;
		sck_shift = 28;
	} /*else if (2 == bus_id)
		//SDA:GPIO E3  SCK:GPIOE2
		sda_shift = 3;
		sck_shift = 2;
	} */
	else if (3 == bus_id) {
		/*SDA: GPIOC25  SCK:GPIOC22*/
		sda_shift = 25;
		sck_shift = 22;
	} else {
		return;
	}

	tmp = readl(GPIO_COUTEN);
	tmp |= 1<<sck_shift;
	writel(tmp, GPIO_COUTEN);

	tmp = readl(GPIO_CDAT);
	tmp |= 1<<sck_shift;
	writel(tmp, GPIO_CDAT);

	udelay(2000);

	for (i = 0; i < num; i++) {
		tmp = readl(GPIO_CDAT);
		tmp &= ~(1<<sck_shift);
		writel(tmp, GPIO_CDAT);
		udelay(1);

		tmp = readl(GPIO_CDAT);
		tmp |= 1<<sck_shift;
		writel(tmp, GPIO_CDAT);
		udelay(1);
	}
	udelay(1000);

	tmp = readl(GPIO_CDAT);
	tmp &= ~(1<<sck_shift);
	writel(tmp, GPIO_CDAT);

	tmp = readl(GPIO_COUTEN);
	tmp &= ~(1<<sck_shift);
	writel(tmp, GPIO_COUTEN);

	tmp = readl(GPIO_CINEN);
	tmp &= ~(1<<sck_shift);
	writel(tmp, GPIO_CINEN);

	udelay(1000);
}

static void vcc_up_set(void)
{
	int tmp = 0;

	/*set_clk_down*/
	tmp = readl(GPIO_COUTEN);
	tmp |= 1<<sda_shift;
	tmp |= 1<<sck_shift;
	writel(tmp, GPIO_COUTEN);

	tmp = readl(GPIO_CDAT);
	tmp &= ~(1<<sck_shift);
	tmp &= ~(1<<sda_shift);
	writel(tmp, GPIO_CDAT);

	udelay(1000);

	/*set_clk_up*/
	tmp = readl(GPIO_CDAT);
	tmp = 1<<sck_shift;
	writel(tmp, GPIO_CDAT);
	udelay(1);

	/*set_dat_up*/
	tmp = readl(GPIO_CDAT);
	tmp = 1<<sda_shift;
	writel(tmp, GPIO_CDAT);
	udelay(1000);

	tmp = readl(GPIO_CDAT);
	tmp &= ~(1<<sck_shift);
	tmp &= ~(1<<sda_shift);
	writel(tmp, GPIO_CDAT);

	tmp = readl(GPIO_COUTEN);
	tmp &= ~(1<<sck_shift);
	tmp &= ~(1<<sda_shift);
	writel(tmp, GPIO_COUTEN);

	tmp = readl(GPIO_CINEN);
	tmp &= ~(1<<sck_shift);
	tmp &= ~(1<<sda_shift);
	writel(tmp, GPIO_CINEN);
	udelay(500);

}


int eg2801_gasgauge_get_capacity(void)
{
	int num = 9;
	int capacity_h = 0;
	int capacity_l = 0;
	int tmp;

	/*set_mfp*/
	tmp = readl(MFP_CTL1);
	tmp |= 1 << 5;
	tmp &= ~(1 << 4);
	writel(tmp, MFP_CTL1);

	i2c_clk(OWLXX_GAUGE_BUS_ID, num);
	vcc_up_set();

	udelay(5000);
	/*now clk is 100K. zhu hai set i2c_SCL clock,24M/(16*4)=375K*/
	I2C_SET_BUS(OWLXX_GAUGE_BUS_ID);

	capacity_h = gague_read_reg_byte(EG2801_I2C_ADDR, REG_ChargeState_H);

	printf("[eg2801]cap h:%d\n", capacity_h);
	return capacity_h;
}

