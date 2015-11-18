#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <common.h>
#include <dm.h>
#include <i2c.h>
#if defined CONFIG_DM_REGULATOR_ATC2609A
#include <power/atc260x/actions_reg_atc2609a.h>
#elif defined CONFIG_DM_REGULATOR_ATC2603C
#include <power/atc260x/actions_reg_atc2603c.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define WAKEUP_SRC_RESET            (1 << 1)
#define WAKEUP_SRC_HDSW             (1 << 2)
#define WAKEUP_SRC_ALARM            (1 << 3)
#define WAKEUP_SRC_ONOFF_SHORT      (1 << 7)
#define WAKEUP_SRC_ONOFF_LONG       (1 << 8)
#define WAKEUP_SRC_WALL_IN          (1 << 9)
#define WAKEUP_SRC_VBUS_IN          (1 << 10)

/* register read/write ------------------------------------------------------ */

int atc260x_m_reg_read(struct atc260x_dev *atc260x, uint reg)
{
	int ret;
	ret = (atc260x->reg_read) (atc260x, reg);
	if (ret < 0) {
		printf(" read reg 0x%x err, ret=%d\n", reg, ret);
	}
	return ret;
}

int atc260x_m_reg_write(struct atc260x_dev *atc260x, uint reg, u16 val)
{
	int ret;
#if defined CONFIG_DM_REGULATOR_ATC2609A
	/* TESTCODE! */
	if (reg == ATC2609A_PMU_SYS_CTL0 && (val & 0x1) == 0) {
		printf("GOT YOU, PMU_SYS_CTL0[0] set to 0!\n"
		       "  builtin_return_address %p\n",
		       __builtin_return_address(1));

		while (1) ;
	}
#endif
	ret = (atc260x->reg_write) (atc260x, reg, val);
	if (ret < 0) {
		printf("write reg 0x%x err, ret=%d\n", reg, ret);
	}
	return ret;
}

int atc260x_m_reg_setbits(struct atc260x_dev *atc260x, uint reg, u16 mask,
			  u16 val)
{
	int ret;
	u16 new_val, old_val;
	ret = atc260x_m_reg_read(atc260x, reg);
	if (ret < 0)
		return ret;
	new_val = old_val = ret;
	new_val &= ~mask;
	new_val |= (val & mask);
	ret = 0;
	if (new_val != old_val) {
		ret = atc260x_m_reg_write(atc260x, reg, new_val);
	}
	return ret;
}

/* 仅访问主设备的寄存器的接口 */
int atc260x_reg_read(uint reg)
{
	return atc260x_m_reg_read(&g_atc260x_master_dev, reg);
}

int atc260x_reg_write(uint reg, u16 val)
{
	return atc260x_m_reg_write(&g_atc260x_master_dev, reg, val);
}

int atc260x_reg_setbits(uint reg, u16 mask, u16 val)
{
	return atc260x_m_reg_setbits(&g_atc260x_master_dev, reg, mask, val);
}

static int _atc260x_get_version(struct atc260x_dev *atc260x)
{
#if defined CONFIG_DM_REGULATOR_ATC2609A
	static const u16 sc_ver_reg_addr_tbl = ATC2609A_CHIP_VER;
#elif defined CONFIG_DM_REGULATOR_ATC2603C
	static const u16 sc_ver_reg_addr_tbl = ATC2603C_CHIP_VER;
#endif
	int ret;
	ret = atc260x_m_reg_read(atc260x, sc_ver_reg_addr_tbl);
	if (ret < 0) {
		printf("failed to get IC ver\n");
		return ret;
	}
	switch (ret & 0xf) {
	case 0:
		return ATC260X_ICVER_A;
	case 1:
		return ATC260X_ICVER_B;
	case 3:
		return ATC260X_ICVER_C;
	case 7:
		return ATC260X_ICVER_D;
	case 15:
		return ATC260X_ICVER_E;
	}
	return -1;
}
#ifdef CONFIG_DM_REGULATOR_ATC2603C
/* gl5307 ------------------------------------------------------------------- */

static int _atc2603c_probe(struct atc260x_dev *atc260x)
{
	int ret = atc260x_m_reg_read(atc260x, ATC2603C_PMU_OC_INT_EN);
	if (ret < 0) {
		return -1;
	}
	if (ret != 0x1bc0) {
		return -1;
	}
	return 0;
}

static int _atc2603c_init(struct atc260x_dev *atc260x,
	struct atc260x_pmu_initdata *initdata)
{
	uint dat, val;
	/*uint wakeup_flag, rtc_h, rtc_ms; */
	/*unsigned long cpu_resume_fn_addr; */

	/* update bandgap from BDG_VOL */
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_BDG_CTL, (1 << 11), (1 << 11));

	/*close charger*/
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_CHARGER_CTL0, (1<<15), 0);

	/* goto S4 when battery is lower than 3.1v */
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_SYS_CTL1, 0x7 << 2, 0x5 << 2);

	/* dcdc1 VDD=1.0V , pwm mode*/
	val = 0x804f;
	val |= initdata->dcdc_cfg[1] << 7;
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC1_CTL0, val);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC1_CTL1, 0x6cae);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC1_CTL2, 0x334b);

	/*dcdc2 1.7v, auto mode*/
	val = 0x80af;
	val |= initdata->dcdc_cfg[2] << 8;
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC2_CTL0, val);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC2_CTL1, 0xecae);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC2_CTL2, 0x334b);

	/* dcdc3 3.1v, PWM */
	val = 0x80cf;
	val |= initdata->dcdc_cfg[3] << 9;
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC3_CTL0, 0x8acf);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC3_CTL1, 0x254c);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_DC3_CTL2, 0x338a);

	/* LDO 1 */
	if (initdata->ldo_en_bm & (1 << 1)) {
		atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO1_CTL, initdata->ldo_cfg[1]<<13);
	}

	/* LDO 2 & 3  (avcc & avdd1v8) fixed usage */
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO2_CTL, 5<<13);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO3_CTL, 3<<13);

	/* LDO 5 */
	if (initdata->ldo_en_bm & (1 << 5)) {
		atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO5_CTL, (initdata->ldo_cfg[5]<<13) | 1);
	}

	/* ldo6 avdd 1.2v  fixed usage */
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO6_CTL, 0xa000);

	/* LDO 7 */
	if (initdata->ldo_en_bm & (1 << 7)) {
		atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO7_CTL, (initdata->ldo_cfg[7]<<13) | 1);
	}

	/* LDO 8 */
	if (initdata->ldo_en_bm & (1 << 8)) {
		atc260x_m_reg_write(atc260x, ATC2603C_PMU_LDO8_CTL, (initdata->ldo_cfg[8]<<12) | 1);
	}

	/* SWITCH 1, sd power */
	if (initdata->ldo_en_bm & (1 << 0)) {
		dat = atc260x_m_reg_read(atc260x, ATC2603C_PMU_SWITCH_CTL);
		dat &= ~0x803a;  /* LDO mode */
		dat |= ((3 & initdata->ldo_cfg[0]) << 3) | (0 << 15);
		atc260x_m_reg_write(atc260x, ATC2603C_PMU_SWITCH_CTL, dat);
	}

#if 0 /* TESTCODE */
	/*-------------------------------------------------- */
	/* S2 resume here. */
	/*-------------------------------------------------- */

	s2_resume = 0;
	cpu_resume_fn = NULL;
	alarm_wakeup = 0;
	/* check suspend to S2 flag */
	dat = atc260x_m_reg_read(ATC2603C_PMU_SYS_CTL3);
	if (dat & PMU_SYS_CTL3_FW_FLAG_S2) {

		cpu_resume_fn_addr = (atc260x_m_reg_read(ATC2603C_PMU_SYS_CTL8) & 0xffff) |
			(atc260x_m_reg_read(ATC2603C_PMU_FW_USE0 & 0xffff) << 16);
		rtc_h = atc260x_m_reg_read(ATC2603C_RTC_H);
		rtc_ms = atc260x_m_reg_read(ATC2603C_RTC_MS);
		cpu_resume_fn = (typeof(cpu_resume_fn))cpu_resume_fn_addr;

		wakeup_flag = atc260x_m_reg_read(ATC2603C_PMU_SYS_CTL1);
		printf("%02d:%02d:%02d cpu_resume = %x wakeup flag = %x\n",
			rtc_h,
			(rtc_ms & (0x3f << 6)) >> 6,
			(rtc_ms & 0x3f),
			(u32)cpu_resume_fn,
			wakeup_flag);
		/*find the wakeup source */
		if ((wakeup_flag & (1 << 8)) && ((wakeup_flag & 0xf000) == 0))
			alarm_wakeup = 1;
		if ((alarm_wakeup == 0) || (((u32)cpu_resume_fn & 0x80000000) == 0)) {
			/* clear the S2 flag anyway */
			val = dat & (~PMU_SYS_CTL3_FW_FLAG_S2);
			atc260x_m_reg_write(ATC2603C_PMU_SYS_CTL3, val);
		} else if ((u32)cpu_resume_fn & 0x80000000) {
			/* clear the highest bit of cpu_resume_fn */
			val = atc260x_m_reg_read(ATC2603C_PMU_FW_USE0);
			val &= ~(1U << 15);
			atc260x_m_reg_write(ATC2603C_PMU_FW_USE0, val);
		}
		s2_resume = 1;

		/* check if wakeup from S2 */
		if (dat & 0x8000) {
			/*wakeup from S2, return directly*/
			return 1;
		}
	}
#endif

	/*setup wakup source. */
	dat = atc260x_m_reg_read(atc260x, ATC2603C_PMU_SYS_CTL0);
	dat &= (~(0x7ff<<5));
	dat |= ((WAKEUP_SRC_RESET | WAKEUP_SRC_HDSW |
				 WAKEUP_SRC_ONOFF_LONG |
				 WAKEUP_SRC_WALL_IN |
				 WAKEUP_SRC_VBUS_IN)<<5);
	atc260x_m_reg_write(atc260x, ATC2603C_PMU_SYS_CTL0, dat);

	/*for BUG00187912, enable adc data */
	if (atc260x->ic_ver == ATC260X_ICVER_A) {
		/* for revA */
		atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_AUXADC_CTL1, (1<<3), 0);
	}

	return 0;
}

static int _atc2603c_prepare_for_s2(struct atc260x_dev *atc260x, u32 regu_shutdwn_bm)
{
	/* close all LDO opened by _init */
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_LDO5_CTL, (1U<<0), 0);
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_LDO7_CTL, (1U<<0), 0);
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_LDO8_CTL, (1U<<0), 0);
	atc260x_m_reg_setbits(atc260x, ATC2603C_PMU_SWITCH_CTL, (1U<<15), (1U<<15));
	return 0;
}

/* gl5303 ------------------------------------------------------------------- */
#endif
#ifdef CONFIG_DM_REGULATOR_ATC2609A
static int _atc2609a_probe(struct atc260x_dev *atc260x)
{
	int ret = atc260x_m_reg_read(atc260x, ATC2609A_PMU_OC_INT_EN);
	if (ret < 0) {
		return -1;
	}
	if (ret != 0x0ff8) {
		return -1;
	}
	return 0;
}

static int _atc2609a_init(struct atc260x_dev *atc260x,
			  struct atc260x_pmu_initdata *initdata)
{

	int dat;
	/*uint wakeup_flag, rtc_h, rtc_ms; */
	/*unsigned long cpu_resume_fn_addr; */
	/* update bandgap from BDG_VOL */
	atc260x_m_reg_setbits(atc260x, ATC2609A_PMU_BDG_CTL, (1 << 14),
			      (1 << 14));

	/*close charger */
	atc260x_m_reg_setbits(atc260x, ATC2609A_PMU_SWCHG_CTL0, (1 << 15), 0);

	/* goto S4 when battery is lower than 3.1v */
	atc260x_m_reg_setbits(atc260x, ATC2609A_PMU_SYS_CTL1, 0x7 << 2,
			      0x5 << 2);

	/* DCDC init */
	/* dcdc0 : VDD-GPU
	 * dcdc1 : VDD-CPU
	 * dcdc2 : VDDR
	 * dcdc3 : VCC
	 * dcdc4 : VDD_CORE */

	if (atc260x->ic_ver <= ATC260X_ICVER_D) {
		/* for rev D */
		/* dcdc0  */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC0_CTL1, 0x6c0c);
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC0_CTL6, 0x3285);
		/*atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC0_CTL0,
		   (initdata->dcdc_cfg[0] << 8)); */

		/* dcdc1 */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC1_CTL1, 0x6c0c);
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC1_CTL6, 0x3285);
		/*atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC1_CTL0,
		   (initdata->dcdc_cfg[1] << 8)); */

		/* dcdc2 */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC2_CTL1, 0x6c0c);
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC2_CTL6, 0x3285);
		/*atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC2_CTL0,
		   (initdata->dcdc_cfg[2] << 8)); */

		/* dcdc3 */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC3_CTL1, 0x2c0e);
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC3_CTL6, 0xac85);
		/*atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC3_CTL0,
		   (initdata->dcdc_cfg[3] << 8)); */

		/* dcdc4 */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC4_CTL1, 0x6c0c);
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC4_CTL6, 0x3285);
		/*atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC4_CTL0,
		   (initdata->dcdc_cfg[4] << 8)); */

		/* all dcdc on. lower OSC, */
		atc260x_m_reg_write(atc260x, ATC2609A_PMU_DC_OSC, 0Xabfb);
	} else {
		/* TODO */
		printf("TODO 5303 rev E\n");
		while (1) ;
	}

	/* for suspend/resume debug */
	dev_info(atc260x,
		 "PMU_SYS_CTL0 0x%x, CTL1 0x%x, CTL2 0x%x, CTL3 0x%x\n",
		 atc260x_m_reg_read(atc260x, ATC2609A_PMU_SYS_CTL0),
		 atc260x_m_reg_read(atc260x, ATC2609A_PMU_SYS_CTL1),
		 atc260x_m_reg_read(atc260x, ATC2609A_PMU_SYS_CTL2),
		 atc260x_m_reg_read(atc260x, ATC2609A_PMU_SYS_CTL3));

	/* setup wakup source (reset/hdsw/onoff_l/wall/usb) */
	dat = atc260x_m_reg_read(atc260x, ATC2609A_PMU_SYS_CTL0);
	dat &= (~(0x7ff << 5));
	dat |= (1U << 15) | (1U << 14) | (1U << 13) | (1U << 7) | (1U << 6);
	atc260x_m_reg_write(atc260x, ATC2609A_PMU_SYS_CTL0, dat);
	/*disable high-tempurature interrupt,
	   enable high-tempurature shutdwon,
	   raise shutdown tempurature to 120 */
	atc260x_reg_setbits(ATC2609A_PMU_OT_CTL,
			    (1 << 12) | (1 << 11) | (3 << 9),
			    (1 << 11) | (3 << 9));
	return 0;
}

static int _atc2609a_prepare_for_s2(struct atc260x_dev *atc260x,
				    u32 regu_shutdwn_bm)
{
	uint i, reg_bm, reg_bv = 0;

	for (i = 0; i < 16; i++)
		reg_bv |= ((regu_shutdwn_bm >> i) & 1U) << (15 - i);
	reg_bm = 0xfe7c;

	/* set PMU_S2_PWR */
	return atc260x_m_reg_setbits(atc260x, ATC2609A_PMU_S2_PWR, reg_bm,
				     reg_bv);
}

/* common ------------------------------------------------------------------- */
#endif
struct atc260x_dev g_atc260x_master_dev;

uint atc260x_get_ic_ver(struct atc260x_dev *atc260x)
{
	if (atc260x == NULL)
		atc260x = &g_atc260x_master_dev;
	return atc260x->ic_ver;
}

static int atc260x_early_init_inner(struct atc260x_dev *atc260x,
				    struct atc260x_pmu_initdata *initdata,
				    uint interface_only)
{
	int (*probe_func) (struct atc260x_dev *);
	int (*init_func) (struct atc260x_dev *, struct atc260x_pmu_initdata *);
	int (*if_init_func) (struct atc260x_dev *);
	int ret;
	atc260x->_obj_type_id = ATC260x_PARENT_OBJ_TYPE_ID;
	#if defined CONFIG_DM_REGULATOR_ATC2609A
	atc260x->bus_addr = 0x65;
	if_init_func = atc260x_i2c_if_init;
	probe_func = _atc2609a_probe;
	init_func = _atc2609a_init;
	#elif defined CONFIG_DM_REGULATOR_ATC2603C
	atc260x->bus_addr = 0x65;
	if_init_func = atc260x_i2c_if_init;
	probe_func = _atc2603c_probe;
	init_func = _atc2603c_init;
	#else
	printf("Can't find PMU\n");
	while(1) ;
	#endif
	
	/* init interface */
	ret = if_init_func(atc260x);
	if (ret) {
		printf("dev if_init err, ic_type=%u bus_num=%u ret=%d\n",
		       atc260x->ic_type, atc260x->bus_num, ret);
		return ret;
	}
	/* probe */
	ret = probe_func(atc260x);
	if (ret) {
		printf("dev probe err, ic_type=%u bus_num=%u\n",
		       atc260x->ic_type, atc260x->bus_num);
		return ret;
	}

	/* get chip version */
	ret = _atc260x_get_version(atc260x);
	if (ret < 0) {
		printf("unknown dev ver, ic_type=%u\n", atc260x->ic_type);
		return ret;
	}
	atc260x->ic_ver = ret;
	dev_info(atc260x, "found PMIC type %u ver %u\n",
		 atc260x->ic_type, atc260x->ic_ver);

	/* base init */
	if (interface_only == 0) {
		ret = init_func(atc260x, initdata);
		if (ret) {
			printf("functional init err\n");
			return ret;
		}
	}

	return 0;
}

int atc260x_early_init(struct udevice *udev)
{
	struct atc260x_pmu_initdata initdata;
	struct atc260x_dev *atc260x;
	struct udevice *parent = dev_get_parent(udev);
	g_atc260x_master_dev.udev = udev;
	atc260x = &g_atc260x_master_dev;
	atc260x->dev_id = 0;
	#if defined CONFIG_DM_REGULATOR_ATC2609A
	atc260x->ic_type = ATC260X_ICTYPE_2609A;	/* TESTCODE! */
	#elif defined CONFIG_DM_REGULATOR_ATC2603C
	atc260x->ic_type = ATC260X_ICTYPE_2603C;
	#endif
	atc260x->bus_num = parent->seq;	/* I2C-which */
	return atc260x_early_init_inner(atc260x, &initdata, 0);
}

int atc260x_prepare_for_s2(struct atc260x_dev *atc260x, u32 regu_shutdwn_bm)
{
	int ret = -1;
	#if defined CONFIG_DM_REGULATOR_ATC2609A
		ret = _atc2609a_prepare_for_s2(atc260x, regu_shutdwn_bm);
	#elif defined CONFIG_DM_REGULATOR_ATC2603C
		ret = _atc2603c_prepare_for_s2(atc260x, regu_shutdwn_bm);
	#else
		printf("Can't find PMU\n");
		while(1) ;
	#endif
	return ret;
}
