#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <common.h>
#include <dm.h>
#include <power/atc260x/actions_reg_atc2609a.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <common.h>
#include <dm.h>
#include <power/atc260x/actions_reg_atc2609a.h>

static const u16 sc_atc260x_regtbl_sysctl0 = ATC2609A_PMU_SYS_CTL0;

static const u16 sc_atc260x_regtbl_sysctl1 = ATC2609A_PMU_SYS_CTL1;

static const u16 sc_atc260x_regtbl_sysctl3 = ATC2609A_PMU_SYS_CTL3;

static const u16 sc_atc260x_regtbl_sysctl4 = ATC2609A_PMU_SYS_CTL5;

int atc260x_misc_enable_remcon_wakeup_detect(int enable)
{
	uint reg_sysctl4;

	reg_sysctl4 = sc_atc260x_regtbl_sysctl4;

	atc260x_set_bits(reg_sysctl4, 1U << 10, enable << 10);

	return 0;

}

int atc260x_misc_enable_wall_wakeup_detect(int enable)
{
	uint reg_sysctl4;

	reg_sysctl4 = sc_atc260x_regtbl_sysctl4;

	atc260x_set_bits(reg_sysctl4, 1U << 7, enable << 7);

	return 0;
}

int atc260x_misc_enable_vbus_wakeup_detect(int enable)
{
	uint reg_sysctl4;

	reg_sysctl4 = sc_atc260x_regtbl_sysctl4;

	atc260x_set_bits(reg_sysctl4, 1U << 8, enable << 8);

	return 0;
}

int atc260x_misc_enable_wakeup_detect(uint wake_src, int enable)
{
	switch (wake_src) {
	case ATC260X_WAKEUP_SRC_REMCON:
		atc260x_misc_enable_remcon_wakeup_detect(enable);
		break;
	case ATC260X_WAKEUP_SRC_WALL_IN:
		atc260x_misc_enable_wall_wakeup_detect(enable);
		break;
	case ATC260X_WAKEUP_SRC_VBUS_IN:
		atc260x_misc_enable_vbus_wakeup_detect(enable);
		debug("ATC2609A_PMU_SYS_CTL5:%d\n",
			atc260x_reg_read(ATC2609A_PMU_SYS_CTL5));
		break;
	default:
		pr_err("%s() wake_src err", __func__);
		return -1;

	}

	return 0;
}

int atc260x_misc_set_wakeup_src(uint wake_mask, uint wake_src)
{
	uint reg_sysctl0, reg_mask, reg_val;
	int ret;

	if ((~ATC260X_WAKEUP_SRC_ALL & wake_mask) ||
	    (~ATC260X_WAKEUP_SRC_ALL & wake_src) || (wake_src & ~wake_mask)) {
		return -1;
	}

	reg_sysctl0 = sc_atc260x_regtbl_sysctl0;
	reg_mask = wake_mask << 5;
	reg_val = wake_src << 5;

	ret = atc260x_reg_setbits(reg_sysctl0, reg_mask, reg_val);
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
	}
	return ret;
}

/*get which src can wakeup the machine*/
int atc260x_misc_get_wakeup_src(void)
{
	uint reg_sysctl0;
	reg_sysctl0 = sc_atc260x_regtbl_sysctl0;
	return atc260x_reg_read(reg_sysctl0) >> 5;
}

/*get which src has wakeup the machine*/
int atc260x_misc_get_wakeup_flag(void)
{
	uint reg_sysctl1;
	reg_sysctl1 = sc_atc260x_regtbl_sysctl1;
	return atc260x_reg_read(reg_sysctl1) >> 5;
}

int atc260x_misc_pwrdown_machine(uint deep_pwrdn)
{
	uint reg_sysctl0, reg_sysctl1, reg_sysctl3, reg_mask, reg_val;
	int ret;

	pr_info("%s() enter, deep_pwrdn=%u\n", __func__, deep_pwrdn);

	reg_sysctl0 = sc_atc260x_regtbl_sysctl0;
	reg_sysctl1 = sc_atc260x_regtbl_sysctl1;
	reg_sysctl3 = sc_atc260x_regtbl_sysctl3;

	/* set onoff_long wakeup */
	ret = atc260x_reg_setbits(reg_sysctl0, (1U << 13), (1U << 13));
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
		return ret;
	}

	/* set S2 & S3 */
	reg_mask = (1U << 15) | (1U << 14);
	reg_val = deep_pwrdn ? 0 : (1U << 14);
	ret = atc260x_reg_setbits(reg_sysctl3, reg_mask, reg_val);
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
		return ret;
	}

	mdelay(2);		/* wait for UART print... */

	ret = atc260x_reg_setbits(reg_sysctl1, (1U << 0), 0);
	while (ret == 0)
	;
	/* wait for power down if success. */
	return ret;
}

static int _atc2603a_pm_do_reboot(void)
{
	int ret;

	/* 旧的5302没有软件触发的reset功能, 只能用PowerDown+Alarm来模承 */

	/* disable WALL/VBUS wakeup, enable alarm wakeup. */
	ret = atc260x_reg_setbits(sc_atc260x_regtbl_sysctl0,
				  (1U << 15) | (1U << 14) | (1U << 8),
				  (1U << 8));
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
		return ret;
	}

	/* set alarm */
	ret = atc260x_rtc_setup_alarm(3, 8, NULL);
	if (ret) {
		pr_err("%s() setup alarm err, ret=%d\n", __func__, ret);
		return ret;
	}

	/* enter S3 */
	ret = atc260x_misc_pwrdown_machine(0);
	if (ret) {
		pr_err("%s() pwrdown err, ret=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int _atc2603c_9a_pm_do_reboot(void)
{
	static const u8 sc_reg_bit_tbl = 10;
	uint discharge;
	uint reg, bit;
	int ret;

	/*if dis_minich set 1, there will not enter minich when reboot */
	atc260x_pstore_set(ATC260X_PSTORE_TAG_DIS_MCHRG, 1);
	/* 新的IC带软件触发的reset功能, 直接使用即可. */
	/* wait atc260x_pstore_set complete */
	mdelay(10);
	/*if dis_minich set failed, then set again */
	atc260x_pstore_get(ATC260X_PSTORE_TAG_DIS_MCHRG, &discharge);
	if (!discharge) {
		atc260x_pstore_set(ATC260X_PSTORE_TAG_DIS_MCHRG, 1);
		mdelay(10);
	}
	pr_info("%s discharge(%d)\n", __func__, discharge);

	reg = sc_atc260x_regtbl_sysctl0;
	bit = sc_reg_bit_tbl;

	pr_info("%s() enter, reg %d is 0x%x\n", __func__,
		reg, atc260x_reg_read(reg));

	while (1) {
		ret = atc260x_reg_setbits(reg, (1U << bit), (1U << bit));

		/* wait reboot */
		mdelay(2);

		pr_info("%s(): fail to reboot, ret %d bit %d, reg %d is 0x%x\n",
			__func__, ret, bit, reg, atc260x_reg_read(reg));
	}

	/* never run to here */
	return ret;
}

int atc260x_misc_reset_machine(u32 tgt)
{
	uint flag_adfu, flag_recovery, flag_dis_mchrg;
	int ret;

	pr_err("%s() tgt %d\n", __func__, tgt);

	flag_adfu = flag_recovery = flag_dis_mchrg = 0;
	switch (tgt) {
	case OWL_PMIC_REBOOT_TGT_NORMAL:
		flag_adfu = flag_recovery = flag_dis_mchrg = 0;
		break;
	case OWL_PMIC_REBOOT_TGT_SYS:
		flag_adfu = flag_recovery = 0;
		flag_dis_mchrg = 1;
		break;
	case OWL_PMIC_REBOOT_TGT_ADFU:
		flag_adfu = flag_dis_mchrg = 1;
		flag_recovery = 0;
		break;
	case OWL_PMIC_REBOOT_TGT_RECOVERY:
		flag_recovery = flag_dis_mchrg = 1;
		flag_adfu = 0;
		break;
	}
	ret = atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_ADFU, flag_adfu);
	ret |=
	    atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_RECOVERY,
			       flag_recovery);
	ret |= atc260x_pstore_set(ATC260X_PSTORE_TAG_DIS_MCHRG, flag_dis_mchrg);
	if (ret) {
		pr_err("%s() failed to update pstore\n", __func__);
		return -1;
	}

	return _atc2603c_9a_pm_do_reboot();

	return -1;
}

int atc260x_misc_enter_S2(u32 regu_shutdwn_bm)
{
	uint reg_sysctl1, reg_sysctl3;
	int ret;

	pr_info("%s() enter\n", __func__);

	ret = atc260x_prepare_for_s2(&g_atc260x_master_dev, regu_shutdwn_bm);
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
		return ret;
	}

	reg_sysctl1 = sc_atc260x_regtbl_sysctl1;
	reg_sysctl3 = sc_atc260x_regtbl_sysctl3;

	/* set S2, clear S3 */
	ret =
	    atc260x_reg_setbits(reg_sysctl3, (1U << 15) | (1U << 14),
				(1U << 15));
	if (ret) {
		pr_err("%s() %u err\n", __func__, __LINE__);
		return ret;
	}

	mdelay(2);		/* wait for UART print... */

	ret = atc260x_reg_setbits(reg_sysctl1, (1U << 0), 0);
	while (ret == 0)
	;
	/* wait for S2 if success. */
	return ret;
}

int atc260x_misc_probe(struct udevice *dev)
{
	/*printf("atc260x_misc_probe\n"); */
	return 0;
}

static const struct udevice_id atc260x_misc_ids[] = {
	{.compatible = "actions,atc2609a-misc"},
	{.compatible = "actions,atc2603c-misc"},
	{}
};

U_BOOT_DRIVER(atc260x_misc) = {
.id = UCLASS_ROOT, .name = "atc260x_misc", .of_match =
	    atc260x_misc_ids, .probe = atc260x_misc_probe,};
