#ifndef __OWL_ATC260X_H__
#define __OWL_ATC260X_H__

#include <dm.h>

enum {
	/* DO NOT change the order! */
	ATC260X_ICTYPE_2603A = 0,
	ATC260X_ICTYPE_2603C,
	ATC260X_ICTYPE_2609A,
	ATC260X_ICTYPE_CNT
};

enum {
	ATC260X_ICVER_A = 0,
	ATC260X_ICVER_B,
	ATC260X_ICVER_C,
	ATC260X_ICVER_D,
	ATC260X_ICVER_E,
	ATC260X_ICVER_F,
	ATC260X_ICVER_G,
	ATC260X_ICVER_H,
};

struct atc260x_dev;
extern int atc260x_early_init(struct udevice *udev);
extern struct atc260x_dev g_atc260x_master_dev;
extern int atc260x_prepare_for_s2(struct atc260x_dev *atc260x,
				  unsigned int regu_shutdwn_bm);
#if ATC260X_SUPPORT_MULTI_DEVS
extern struct atc260x_dev g_atc260x_aux_dev;
#endif
extern int owl_pmic_init(void);
extern unsigned int atc260x_get_ic_ver(struct atc260x_dev *atc260x);	/* see ATC260X_ICVER_A ... */

/* register read/ write ----------------------------------------------------- */

extern int atc260x_m_reg_read(struct atc260x_dev *atc260x, unsigned int reg);
extern int atc260x_m_reg_write(struct atc260x_dev *atc260x, unsigned int reg,
			       unsigned short val);
extern int atc260x_m_reg_setbits(struct atc260x_dev *atc260x, unsigned int reg,
				 unsigned short mask, unsigned short val);

static inline int atc260x_m_set_bits(struct atc260x_dev *atc260x,
				     unsigned int reg, unsigned short mask,
				     unsigned short val)
{
	return atc260x_m_reg_setbits(atc260x, reg, mask, val);
}

static inline int atc260x_m_reg_wp_setbits(struct atc260x_dev *atc260x,
					   unsigned int reg,
					   unsigned short all_pnd_mask,
					   unsigned short mask,
					   unsigned short val)
{

	return atc260x_m_reg_setbits(atc260x, reg, (all_pnd_mask | mask),
				     (val & mask & ~all_pnd_mask));
}

static inline int atc260x_m_reg_wp_clrpnd(struct atc260x_dev *atc260x,
					  unsigned int reg,
					  unsigned short all_pnd_mask,
					  unsigned short clr_mask)
{
	unsigned int val;
	int ret = atc260x_m_reg_read(atc260x, reg);
	if (ret < 0)
		return ret;
	val = ((unsigned int)ret & ~all_pnd_mask) | clr_mask;
	return atc260x_m_reg_write(atc260x, reg, val);
}

extern int atc260x_reg_read(unsigned int reg);
extern int atc260x_reg_write(unsigned int reg, unsigned short val);
extern int atc260x_reg_setbits(unsigned int reg, unsigned short mask,
			       unsigned short val);
static inline int atc260x_set_bits(unsigned int reg, unsigned short mask,
				   unsigned short val)
{
	return atc260x_reg_setbits(reg, mask, val);
}

static inline int atc260x_reg_wp_setbits(unsigned int reg,
					 unsigned short all_pnd_mask,
					 unsigned short mask,
					 unsigned short val)
{
	return atc260x_reg_setbits(reg, (all_pnd_mask | mask),
				   (val & mask & ~all_pnd_mask));
}

static inline int atc260x_reg_wp_clrpnd(unsigned int reg,
					unsigned short all_pnd_mask,
					unsigned short clr_mask)
{
	unsigned int val;
	int ret = atc260x_reg_read(reg);
	if (ret < 0)
		return ret;
	val = ((unsigned int)ret & ~all_pnd_mask) | clr_mask;
	return atc260x_reg_write(reg, val);
}

/* for persistent storage --------------------------------------------------- */

enum {
	ATC260X_PSTORE_TAG_REBOOT_ADFU = 0,
	ATC260X_PSTORE_TAG_REBOOT_RECOVERY,
	ATC260X_PSTORE_TAG_FW_S2,
	ATC260X_PSTORE_TAG_DIS_MCHRG,
	ATC260X_PSTORE_TAG_SHDWN_NOT_DEEP,
	ATC260X_PSTORE_TAG_RTC_MSALM,
	ATC260X_PSTORE_TAG_RTC_HALM,
	ATC260X_PSTORE_TAG_RTC_YMDALM,
	ATC260X_PSTORE_TAG_GAUGE_CAP,
	ATC260X_PSTORE_TAG_GAUGE_BAT_RES,
	ATC260X_PSTORE_TAG_GAUGE_ICM_EXIST,
	ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME,
	ATC260X_PSTORE_TAG_GAUGE_S2_CONSUMP,
	ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET,
	ATC260X_PSTORE_TAG_RESUME_ADDR,
	ATC260X_PSTORE_TAG_BOOTLOADER,
	ATC260X_PSTORE_TAG_NUM
};
extern int atc260x_pstore_set(unsigned int tag, unsigned int value);
extern int atc260x_pstore_get(unsigned int tag, unsigned int *p_value);
extern int atc260x_pstore_reset_all(void);
struct atc260x_rtc_alarm_save {
	unsigned short msalm;
	unsigned short halm;
	unsigned short ymdalm;
};

struct atc260x_rtc_time {
	unsigned short year;
	unsigned char mon;
	unsigned char dom;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
};

extern int atc260x_rtc_get_time(struct atc260x_rtc_time *rtc_tm);
extern int atc260x_rtc_get_alarm_time(struct atc260x_rtc_time *rtc_tm);
extern int atc260x_rtc_setup_alarm(unsigned int seconds,
				   unsigned int safe_margin,
				   struct atc260x_rtc_alarm_save *p_old_alarm);
extern int atc260x_rtc_restore_alarm(struct atc260x_rtc_alarm_save
				     *p_old_alarm);
extern int atc260x_rtc_tm_to_time(unsigned long *time);

enum {
	ATC260X_AUXADC_BATV = 0,
	ATC260X_AUXADC_VBUSV,
	ATC260X_AUXADC_WALLV,
	ATC260X_AUXADC_SYSPWRV,
	ATC260X_AUXADC_REMCON,
	ATC260X_AUXADC_AUX0,
	ATC260X_AUXADC_AUX1,
	ATC260X_AUXADC_AUX2,
	ATC260X_AUXADC_AUX3,
	ATC260X_AUXADC_CNT,
};

extern int atc260x_m_auxadc_get_raw(struct atc260x_dev *atc260x,
				    unsigned int channel);

extern int atc260x_m_auxadc_get_translated(struct atc260x_dev *atc260x,
					   unsigned int channel,
					   signed int *p_tr_value);

static inline int atc260x_auxadc_get_raw(unsigned int channel)
{
	return atc260x_m_auxadc_get_raw(&g_atc260x_master_dev, channel);
}

static inline int atc260x_auxadc_get_translated(unsigned int channel,
						signed int *p_tr_value)
{
	return atc260x_m_auxadc_get_translated(&g_atc260x_master_dev, channel,
					       p_tr_value);
}

#define ATC260X_WAKEUP_SRC_IR               (1 << 0)
#define ATC260X_WAKEUP_SRC_RESET            (1 << 1)
#define ATC260X_WAKEUP_SRC_HDSW             (1 << 2)
#define ATC260X_WAKEUP_SRC_ALARM            (1 << 3)
#define ATC260X_WAKEUP_SRC_REMCON           (1 << 4)
/*#define ATC260X_WAKEUP_SRC_TP               (1 << 5) */
/*#define ATC260X_WAKEUP_SRC_WKIRQ            (1 << 6) */
#define ATC260X_WAKEUP_SRC_ONOFF_SHORT      (1 << 7)
#define ATC260X_WAKEUP_SRC_ONOFF_LONG       (1 << 8)
#define ATC260X_WAKEUP_SRC_WALL_IN          (1 << 9)
#define ATC260X_WAKEUP_SRC_VBUS_IN          (1 << 10)
#define ATC260X_WAKEUP_SRC_ALL              ((1 << 11) - 1U)

extern int atc260x_misc_set_wakeup_src(unsigned int wake_mask,
				       unsigned int wake_src);
extern int atc260x_misc_get_wakeup_src(void);
extern int atc260x_misc_get_wakeup_flag(void);
extern int atc260x_misc_enable_wakeup_detect(unsigned wake_src, int enable);
extern int atc260x_misc_pwrdown_machine(unsigned int deep_pwrdn);
extern int atc260x_misc_enter_S2(unsigned int regu_shutdwn_bm);

#define OWL_PMIC_REBOOT_TGT_NORMAL              (0) /* with charger_check etc. */
#define OWL_PMIC_REBOOT_TGT_SYS                 (1) /* no charger ... */
#define OWL_PMIC_REBOOT_TGT_ADFU                (2)
#define OWL_PMIC_REBOOT_TGT_RECOVERY            (3)
extern int atc260x_misc_reset_machine(unsigned int tgt);

#endif /* __OWL_ATC260X_H__ */
