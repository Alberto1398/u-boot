/*
 * actions_arch_funcs.h
 *
 *  Created on: 2014-6-19
 *      Author: liangzhixuan
 */

#ifndef __ACTIONS_ARCH_FUNCS_H__
#define __ACTIONS_ARCH_FUNCS_H__

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/afinfo.h>
#include <asm/arch/gpio_pre_cfg.h>


/* from arch -------------------------------------------------------- */

/* CMU */
void act_cmu_early_clk_config(void);
void act_cmu_middle_clk_config(void);
void act_cmu_latter_clk_config(uint dvfslevel, afinfo_t const *p_afinfo);
int act_cmu_boot_config_check(unsigned int dvfslevel);
void act_ext_pll_emi_config(void);
int act_cmu_adjust_max_cpufreq(uint32_t dvfslevel);
uint32_t act_cmu_get_corepll_clk(void);
uint32_t act_cmu_get_ddrpll_clk(void);
uint32_t act_cmu_get_dev_clk(void);
uint32_t act_cmu_get_cclk(void);
uint32_t act_cmu_get_hclk(void);
uint32_t act_cmu_get_pclk(void);
void act_cmu_reset_modules(uint32_t mod_rst_bit);
void act_cmu_module_clk_ctrl(uint32_t mod_clk_bit, uint val);

/* IRQ */
void act_init_exception_stacks(void);

/* DDR init */
int _DDR_init(ddr_param_t const *cfg, unsigned int boot_mode, unsigned int *p_total_cap);

/* dvfs */
int act_get_icversion(void);
int act_get_dvfslevel(void);

/* check sum */
uint32_t act_calc_checksum32(uint8_t *pbuf, uint32_t size, uint32_t init_val);
uint16_t act_calc_checksum16(uint8_t *pbuf, uint32_t size, uint16_t init_val);


/* from board ------------------------------------------------------- */
void act_spl_board_misc_init(afinfo_t const *p_afinfo);
void act_spl_init_gpio_cfg(void);
void act_spl_serial_init(void);
//
int act_spl_pmu_early_init(void);
void act_pmu_adjust_vdd_voltage(uint vdd_voltage);
uint32_t act_spl_get_S2_resum_addr(void);
int act_pmu_later_init(void);
void act_pmu_shutoff(void);
int act_pmu_helper_get_attached_spi_bus(void);
uint act_260x_reg_read(uint addr);
void act_260x_reg_write(uint addr, uint data);
void act_260x_reg_setbits(uint addr, uint mask, uint data);
void act_pmu_set_enter_adfu_rq_flag(uint value);
int act_pmu_chk_enter_adfu_rq_flag(uint clear);
int act_pmu_chk_dis_enter_charger_rq_flag(uint clear);
int act_pmu_chk_enter_recovery_rq_flag(uint clear);
int act_pmu_chk_enter_charger_rq_flag(void);
int act_pmu_chk_reboot_mode_rq_flag(void);
// batt
int act_pmu_check_power_state(void);

int act_key_scan(unsigned char scan_mode);

int act_boot_krnl_from_adfus(void *p_boot_img_hdr,
        void *p_xml_config, void *p_xml_mfps, void *p_xml_gpios);
int act_boot_krnl(const char *dev_ifname, const char *dev_part_str, uint fstype,
        const char *img_filename, uint disable_qb);

int act_xmlp_init(void);
struct tag * act_xmlp_generate_xml_tags(struct tag *p_tag_buf);
int act_xmlp_upgrade_parse_all(void *p_xml_config, void *p_xml_mfps, void *p_xml_gpios);
int act_xmlp_boot_parse_f(const char *dev_ifname, const char *dev_part_str, uint fstype);
int act_xmlp_boot_parse_r(const char *dev_ifname, const char *dev_part_str, uint fstype);
int act_xmlp_get_config(const char *key, char *buff, int len);
int act_gpio_get_pre_cfg(char *gpio_name, struct gpio_pre_cfg *m_gpio);

// pmem
int act_pmem_get_max_lcd_size(uint *max_lcd_size);
int act_pmem_init(void);

// boot_act_krnl
int act_append_to_bootarg_env(char const *p_new);


/* from drivers ----------------------------------------------------- */
int act_nandblk_init(void);
void act_nandblk_exit(void);

int act_mmchc_init(bd_t *bis);
void act_mmchc_exit(void);


#endif /* __ACTIONS_ARCH_FUNCS_H__ */
