/*
 * Copyright 2013 Actions Semi Inc.
 * Author: Actions Semi, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <asm/armv7.h>
#include <asm/io.h>

#define OWLXX_PA_L2CC                0xB0022000
#define L2X0_CTL                    (OWLXX_PA_L2CC + 0x100)
#define L2X0_AUX_CTL                (OWLXX_PA_L2CC + 0x104)
#define L2X0_CACHE_SYNC             (OWLXX_PA_L2CC + 0x730)
#define L2X0_INV_LINE_PA            (OWLXX_PA_L2CC + 0x770)
#define L2X0_INV_WAY                (OWLXX_PA_L2CC + 0x77C)
#define L2X0_CLEAN_LINE_PA          (OWLXX_PA_L2CC + 0x7B0)
#define L2X0_CLEAN_WAY              (OWLXX_PA_L2CC + 0x7BC)

DECLARE_GLOBAL_DATA_PTR;

void v7_outer_cache_inval_all(void)
{
	u32 nr;
	/* get cpu number */
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (nr) : );
	nr = nr & 0xF;
	if (nr != 0)
		return;
	writel(0xffff, L2X0_INV_WAY);
	while (readl(L2X0_INV_WAY) & 0xffff)
		udelay(1);
	writel(0, L2X0_CACHE_SYNC);
}

void v7_outer_cache_flush_all(void)
{
	u32 nr;
	/* get cpu number */
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (nr) : );
	nr = nr & 0xF;
	if (nr != 0)
		return;
	writel(0xffff, L2X0_CLEAN_WAY);
	while (readl(L2X0_CLEAN_WAY) & 0xffff)
		udelay(1);
	writel(0, L2X0_CACHE_SYNC);
}

void v7_outer_cache_inval_range(u32 start, u32 stop)
{
	u32 mva;
	if (start >= gd->ram_size &&
	    stop < gd->ram_size + 1024 * 1024) {
		return;
	}
	for (mva = start; mva < stop; mva = mva + 32)
		writel(mva, L2X0_INV_LINE_PA);
}

void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	u32 mva;
	if (start >= gd->ram_size &&
	    stop < gd->ram_size + 1024 * 1024) {
		return;
	}
	for (mva = start; mva < stop; mva = mva + 32)
		writel(mva, L2X0_CLEAN_LINE_PA);
}

void v7_outer_cache_enable(void)
{
	u32 aux;

	/* Check if l2x0 controller is already enabled */
	if (readl(L2X0_CTL) & 1)
		return;

	printf("l2x0_enable\n");
	aux = readl(L2X0_AUX_CTL);
	aux &= 0xc0000fff;
	aux |= 0x3e050000;
	writel(aux, L2X0_AUX_CTL);
	v7_outer_cache_inval_all();

	/* enable l2x0 */
	writel(1, L2X0_CTL);
}

void v7_outer_cache_disable(void)
{
	if ((readl(L2X0_CTL) & 1) == 0)
		return;

	v7_outer_cache_flush_all();
	writel(0, L2X0_CTL);
	asm volatile("dsb");
}
