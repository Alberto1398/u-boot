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

#ifndef __OWLXX_SYS_PROTO_H
#define __OWLXX_SYS_PROTO_H

#include <afinfo.h>

#define OWLXX_PERIPH_BASE (0xB0020000)

struct owlxx_timer {
	u32 twohz0_ctl;
	u32 pad;
	u32 t0_ctl;
	u32 t0_cmp;
	u32 t0_val;
};

int dcu_dev_timeout_init(void);

void prcm_init(void);

int pmic_init(unsigned int bus);

int acts_mmc_init(int dev_index, uint host_caps_mask, uint f_max);

void adfusever_main(void);

void check_recovery_mode(void);

int irq_printf(const char *fmt, ...);

/*pinctrl stuff*/
int pinctrl_init_f(void);
int pinctrl_init_r(void);

/*if you cannot parse devicetree yet, use these 3 interfaces*/
int owlxx_pinctrl_group_set_configs(const char *pinctrl_dev_name,
				const char *group_name,
				unsigned long *configs,
				int num_configs);
int owlxx_pinctrl_pin_set_configs(const char *pinctrl_dev_name,
				const char *pin_name,
				unsigned long *configs,
				int num_configs);
int owlxx_pinctrl_set_function(const char *pinctrl_dev_name,
				const char *group_name,
				const char *function_name);

int owlxx_device_fdtdec_set_pinctrl_default(int dev_offset);
int owlxx_fdtdec_set_pinctrl(int offset);

extern afinfo_t *afinfo;
extern int charger_boot_mode;
extern int splash_image_type;
extern unsigned int owlxx_adc_val;

int owlxx_get_devpll_rate(void);
int owlxx_get_devclk_rate(void);
int owlxx_get_nic_clk_rate(void);
int owlxx_get_ddr_clk_rate(void);
int owlxx_get_displaypll_rate(void);
int owlxx_set_displaypll_rate(u32 target_hz);
int owlxx_gpio_init(void);
int owlxx_enter_adfu(void);
void owlxx_enable_jtag(void);
void boot_append_remove_args(char *append, char *remove);
int dual_logo_cfgval_get(void);


int atc260x_regulators_init(void);

int adckey_scan(void);
void check_key(void);
#ifdef CONFIG_GENERIC_MMC
void mmc_sd_init(void);
void check_burn_recovery (void);
#endif
#if !defined(CONFIG_SPL_BUILD)
const char *get_defif(void);
#endif


int splash_image_init(void);
int spl_mtest(ulong start, ulong end,
		ulong pattern, int iteration_limit);

void fdt_get_checksum(int index);

int owlxx_speaker_init(void);
#define OWLXX_PRODUCT_STATE   (afinfo->is_product)

#if !(defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_OWLXX_UPGRADE))
int charge_animation_display(int bat_cap);
int owlxx_dss_enable(void);
int owlxx_dss_remove(void);
#else
static inline int charge_animation_display(int bat_cap)
{
	return 0;
}

static inline int owlxx_dss_enable(void)
{
	return 0;
}

static inline int owlxx_dss_remove(void)
{
	return 0;
}


#endif

#endif
