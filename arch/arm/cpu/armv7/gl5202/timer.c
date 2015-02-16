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

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

DECLARE_GLOBAL_DATA_PTR;



int timer_init(void)
{
	/* enable timer clock */
	act_writel(act_readl(CMU_DEVCLKEN1) | (1U<<27), CMU_DEVCLKEN1);

	/* init T0 */
	act_writel(0, TWOHZ0_CTL);
	act_writel(1U, T0_CTL);
	act_writel(0, T0_VAL);
	act_readl(T0_CTL); // flush

	/*  */
	gd->arch.timer_rate_hz = CONFIG_SYS_HZ;
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	/* start T0 */
	act_writel((1U << 2), T0_CTL);

	return 0;
}

unsigned long long get_ticks(void)
{
	uint32_t cur_cnt, last_cnt, inc_ticks;

	cur_cnt = act_readl(T0_VAL);
	last_cnt = gd->arch.lastinc;
	gd->arch.lastinc = cur_cnt;

	inc_ticks = (cur_cnt - last_cnt) / (24000000U / CONFIG_SYS_HZ); /* ~178s before overflow */
	gd->arch.tbl += inc_ticks; /* ~49 days before overflow */
	return gd->arch.tbl;
}

ulong get_timer(ulong base)
{
	return (uint32_t)get_ticks() - base;
}

void __udelay(unsigned long usec)
{
	uint32_t need_cnt, accu_cnt, last_cntr, cur_cntr;

	//need_cnt = usec /1000000U * 24000000U;
	need_cnt = usec * 24U; /* usec should < 178s, or else overflow */

	last_cntr = act_readl(T0_VAL);
	accu_cnt = 0;
	while(accu_cnt < need_cnt)
	{
		cur_cntr = act_readl(T0_VAL);
		accu_cnt += cur_cntr - last_cntr;
		last_cntr = cur_cntr;
	}
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	/* 为避开tick定义混乱的问题, 这里一个tick等于  1/SYS_HZ */
	return CONFIG_SYS_HZ;
}

