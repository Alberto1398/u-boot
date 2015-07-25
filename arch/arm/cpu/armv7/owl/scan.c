#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>
#include <asm/io.h>
#include <asm/arch/sps.h>
#include <common.h>


#ifdef CONFIG_OWLXX_SCAN_DDR
extern unsigned short atc260x_read(unsigned short reg);

extern int atc260x_write(unsigned short reg, unsigned short val);



#define CDC_OFFSET 0x12

#define PERIPHCLK 400
#define OWLXX_PA_TWD                 (0xB0020600)
#define TWD_WDOG_LOAD			    (OWLXX_PA_TWD + 0x20)
#define TWD_WDOG_COUNTER		    (OWLXX_PA_TWD + 0x24)
#define TWD_WDOG_CONTROL		    (OWLXX_PA_TWD + 0x28)
#define TWD_WDOG_INTSTAT		    (OWLXX_PA_TWD + 0x2C)
#define TWD_WDOG_RESETSTAT		    (OWLXX_PA_TWD + 0x30)
#define TWD_WDOG_DISABLE		    (OWLXX_PA_TWD + 0x34)

#define WDOG_CTL_ENABLE	            (1 << 0)
#define WDOG_CTL_ONESHOT	        (0 << 1)
#define WDOG_CTL_PERIODIC	        (1 << 1)
#define WDOG_CTL_IT_ENABLE	        (1 << 2)
#define WDOG_CTL_MODE_TIMER		(0 << 3)
#define WDOG_CTL_MODE_WDLOG	        (1 << 3)
#define WDOG_CTL_PRESCALER(div)	    ((div) << 8)

struct cdc_info {
	unsigned short magic;
#if 0
	unsigned short adfu_timeout;
	unsigned short mbrc_timeout;
#endif
	unsigned short test_start;
	unsigned short test_end;
	unsigned char dll_begin;
	unsigned char dll_end;
	unsigned char dqs_begin;
	unsigned char dqs_end;
	unsigned char step;
};

ddr_param_t new_info;

void enteradfu(void)
{
}

static int setup_watchdog(unsigned int load_counter, unsigned int div)
{
	/* enbable cpu0 watch reset all cpu */
	writel(readl(SPS_RST_CTL) | 0x2, SPS_RST_CTL);

	/* set initial load counter */
	writel(load_counter, TWD_WDOG_LOAD);

	/* enable the watch dog */
	writel(WDOG_CTL_ENABLE | WDOG_CTL_ONESHOT | WDOG_CTL_MODE_WDLOG |
	WDOG_CTL_PRESCALER(div), TWD_WDOG_CONTROL);
}

void disable_wd(void)
{
	writel(0, TWD_WDOG_CONTROL);
}

void enable_wd(unsigned int second)
{
	int periphclk_mhz = PERIPHCLK;
	int load_counter;
	int div = 100;

	load_counter = (second  * 1000 * 1000 / div) * periphclk_mhz;
	setup_watchdog(load_counter, div - 1);
}

void set_scan_param(struct cdc_info *cdc_info)
{
	unsigned short val, freq, dll, dqs, magic, step;

	val = atc260x_read(0x9);
	magic = (val & 0x800) >> 11;
	if (magic == 0) {
		magic = 1;
		freq = cdc_info->test_start;
		step = 0;
		dll = cdc_info->dll_begin;
		dqs = cdc_info->dqs_begin;
	} else {
		dll = (val & 0xf0) >> 4;
		dqs = (val & 0x700) >> 8;
		step = val & 0xf;

		dqs += cdc_info->step;
		if (dqs >= cdc_info->dqs_end) {
			dqs = cdc_info->dqs_begin;
			dll += cdc_info->step;
			if (dll >= cdc_info->dll_end) {
				dll = cdc_info->dll_begin;
				step++;
				freq = cdc_info->test_start + 12 * step;
				if (freq >= cdc_info->test_end)
					enteradfu();
			}
		}
		freq = cdc_info->test_start + 12 * step;
	}
	new_info.ddr_freq = freq;
	new_info.dll = dll;
	new_info.dqs = dqs;
	val = step | (dll << 4) | (dqs << 8) | (magic << 11);
	atc260x_write(0x9, val);

}

ddr_param_t *scan_ddr_prepare(int mode)
{
	struct cdc_info *cdc_info =
		(struct cdc_info *)(ddr_afinfo + CDC_OFFSET);

	if (mode != 0)
		return ddr_afinfo;
	memcpy(&new_info, ddr_afinfo, sizeof(ddr_param_t));
	if (cdc_info->magic == 0xcdc) {
		set_scan_param(cdc_info);
		enable_wd(5); /*set 5s*/
	}
	return &new_info;
}
#else
ddr_param_t *scan_ddr_prepare(int mode)
{
	return &afinfo->ddr_param;
}
#endif
/*
int enter_lp(void)
{
	act_writel((1 << 28), PCTL0_MCFG1);
	udelay(1);
	Chg_Mode(CFG,0);
	act_setl(0x1, PCTL0_SCFG);
	act_writel(0, PCTL0_MCFG1);
	act_writel(0, PCTL0_MCFG);
	act_setl((0x20<<16), PCTL0_SCFG);
	Chg_Mode(LP,0);
}
*/

DECLARE_GLOBAL_DATA_PTR;

#define M_SIZE	(1024 * 1024)


int dram_init(void)
{
	const void *blob = gd->fdt_blob;
	int node, addr;
	u32 size;

	size = get_memory_size();
	if (fdt_fixup_memory(gd->fdt_blob, 0, size) < 0) {
		printf("set memory %dMB failed\n", size / M_SIZE);
	}

#ifdef CONFIG_OWLXX_QUICKBOOT
	/* reserve for quickboot */
	gd->ram_size = size - 1024 * 1024;
#else
	gd->ram_size = size;
#endif

	return 0;
}
