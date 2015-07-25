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
#include <asm/io.h>
#include <common.h>
#include <asm/arch/clocks.h>

#define TIMER_CLK_24M		24
#define TIMER_CLOCK		(TIMER_CLK_24M * 1000 * 1000)
#define COUNT_TO_USEC(x)	((x) / TIMER_CLK_24M)
#define USEC_TO_COUNT(x)	((x) * TIMER_CLK_24M)
#define TICKS_PER_HZ		(TIMER_CLOCK / CONFIG_SYS_HZ)
#define TICKS_TO_HZ(x)		((x) / TICKS_PER_HZ)
#define TIMER_LOAD_VAL		0xffffffff

DECLARE_GLOBAL_DATA_PTR;

static struct owlxx_timer *t =
	(struct owlxx_timer *) TIMER_2HZ_BASE;
int timer_init(void)
{
    
	setbits_le32(CMU_DEVCLKEN1, DEVCLKEN_TIMER);
	writel(0x0, &t->twohz0_ctl);
	/* reset timer */
	writel(0x1, &t->t0_ctl);
	writel(0x0, &t->t0_val);

	writel(0x0, &t->t0_cmp);
	/* enable timer */
	writel(0x5, &t->t0_ctl);

	return 0;
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = TICKS_TO_HZ(readl(&t->t0_val));

	if (now >= gd->arch.lastinc)	/* normal (non rollover) */
		gd->arch.tbl += (now - gd->arch.lastinc);
	else			/* rollover */
		gd->arch.tbl += (TICKS_TO_HZ(TIMER_LOAD_VAL) - gd->arch.lastinc) + now;
	gd->arch.lastinc = now;
	return gd->arch.tbl;
}

void __udelay(ulong usec)
{
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	ulong now, last = readl(&t->t0_val);

	while (tmo > 0) {
		now = readl(&t->t0_val);
		if (now > last)	/* normal (non rollover) */
			tmo -= now - last;
		else		/* rollover */
			tmo -= TIMER_LOAD_VAL - last + now;
		last = now;
	}
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void time1_init(void)
{
		/*T1*/
	writel(0x1, T1_CTL);
	writel(0x0, T1_VAL);

	writel(0x0, T1_CMP);
	/* enable timer */
	writel(0x5, T1_CTL);
}
ulong get_timer1(void)
{
	return  TICKS_TO_HZ(readl(T1_VAL));
}



unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
