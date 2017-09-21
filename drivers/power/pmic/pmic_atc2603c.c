/*
 *  Copyright (C) 2014-2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/atc260x/actions_reg_atc2603c.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <asm/io.h>
#include <power/atc260x/atc2603c.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <asm/arch/pwm.h>
#include <libfdt.h>

extern int owl_pwm_enable(struct owl_pwm_device *pwm);

extern int owl_pwm_config(struct owl_pwm_device *pwm, int duty_ns,
		int period_ns, enum pwm_polarity polarity);

/* get pwm device from DTS node */
extern int fdtdec_pwm_get(const void *blob, int node, const char *prop_name,
			struct owl_pwm_device *pwm);

#define MAX_NAME_LENGTH 10
#define MAX_REGULATOR_COUNT 20
typedef struct {
	/* offs: 0x0 */
	unsigned char shift;
	/* offs: 0x1 */
	unsigned char mask;
	/* offs: 0x2 */
	unsigned char val;
	/* offs: 0x3 */
	unsigned char no;
} __attribute__ ((packed)) mfp_t;

typedef struct {
	/* offs: 0x0 */
	unsigned long pwm_val;
	/* offs: 0x4 */
	mfp_t mfp;
} __attribute__ ((packed)) pwm_config_t;
struct owl_pmu_config {
	pwm_config_t pwm_config[3];
	int total_steps;
	int vdd_gpu_step;
	int vdd_core_step;
	struct owl_pwm_device	pwm_core;
	struct owl_pwm_device	pwm_gpu;
};
struct owl_pmu_config pmu_config;
#if 0
void vdd_core_init(struct owl_pmu_config *atc260x_pmu_config)
{
#define OWL_BOOT_MAX_PWM_NUM	3
#define CMU_DEVCLKEN1_PWM0_EN	16
#define GPIO_MFP_PWM_BASE		0xE01B0000
#define MFP_CTL0			(GPIO_MFP_PWM_BASE + 0x0040)
#define PWM_CTL0			(GPIO_MFP_PWM_BASE + 0X50)
#define CMU_BASE			0xE0168000
#define CMU_PWM0CLK			(CMU_BASE + 0x0078)
#define CMU_DEVCLKEN1			(CMU_BASE + 0x00A4)

	unsigned int i, val, offset;

	for (i = 0; i < OWL_BOOT_MAX_PWM_NUM; i++) {

		if (atc260x_pmu_config->pwm_config[i].pwm_val == 0)
			continue;

		pmu_dbg("pwm%d val=%ld\n", i,
		       atc260x_pmu_config->pwm_config[i].pwm_val);

		val = readl((void __iomem *)CMU_DEVCLKEN1);
		val |= 1 << (i + CMU_DEVCLKEN1_PWM0_EN);
		writel(val, (void __iomem *)CMU_DEVCLKEN1);

		offset = i * 4;
		writel(0x1000, offset + (void __iomem *)CMU_PWM0CLK);
		writel(atc260x_pmu_config->pwm_config[i].pwm_val,
		       offset + (void __iomem *)PWM_CTL0);
		val = readl(offset + (void __iomem *)PWM_CTL0);
		pmu_dbg("i = %d, pwm val = %d\n", i, val);
	}

	for (i = 0; i < OWL_BOOT_MAX_PWM_NUM; i++) {
		if (atc260x_pmu_config->pwm_config[i].mfp.mask != 0) {
			unsigned int mask =
			    (unsigned int)atc260x_pmu_config->pwm_config[i].mfp.
			    mask << atc260x_pmu_config->pwm_config[i].mfp.shift;
			unsigned int val =
			    (unsigned int)atc260x_pmu_config->pwm_config[i].mfp.
			    val << atc260x_pmu_config->pwm_config[i].mfp.shift;

			offset = atc260x_pmu_config->pwm_config[i].mfp.no * 4;
			val = (readl(offset + (void __iomem *)MFP_CTL0) & (~mask)) | val;
			writel(val, offset + (void __iomem *)MFP_CTL0);
		}
	}

	pmu_dbg("vdd core init\n");
}
#endif
DECLARE_GLOBAL_DATA_PTR;
static const struct pmic_child_info pmic_children_regulator_info[] = {
	{.prefix = "dcdc", .driver = ATC2603C_DCDC_DRIVER},
	{.prefix = "ldo", .driver = ATC2603C_LDO_DRIVER},
	{},
};

static char *autoset_regulators[MAX_REGULATOR_COUNT];

static const unsigned short check_reg[] = {
	ATC2603C_PMU_DC1_CTL0,
	ATC2603C_PMU_DC1_CTL1,
	ATC2603C_PMU_DC1_CTL2,
	ATC2603C_PMU_DC2_CTL0,
	ATC2603C_PMU_DC2_CTL1,
	ATC2603C_PMU_DC2_CTL2,
	ATC2603C_PMU_DC3_CTL0,
	ATC2603C_PMU_DC3_CTL1,
	ATC2603C_PMU_DC3_CTL2,
	ATC2603C_PMU_DC4_CTL0,
	ATC2603C_PMU_DC4_CTL1,
	ATC2603C_PMU_DC5_CTL0,
	ATC2603C_PMU_DC5_CTL1,
	ATC2603C_PMU_BASE,
};

static int atc2603c_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		error("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}
	return 0;
}

static int atc2603c_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		error("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}
	return 0;
}

static int atc2603c_bind(struct udevice *parent)
{
#if 1
	int regulator_node;
	const void *blob = gd->fdt_blob;
	int children;

	regulator_node = fdt_subnode_offset(blob, parent->of_offset,
					    "voltage-regulators");
	if (regulator_node <= 0) {
		debug("%s: %s voltage-regulators subnode not found!", __func__,
		      parent->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulator_node subnode\n", __func__,
	      parent->name);

	children =
	    pmic_bind_children(parent, regulator_node,
			       pmic_children_regulator_info);
	if (!children) {
		debug("%s: %s - no child found\n", __func__, parent->name);
	}

	/* Always return success for this device */
#endif
	return dm_scan_fdt_node(parent, gd->fdt_blob, parent->of_offset, false);
}

static int atc2603c_probe(struct udevice *dev)
{
	struct udevice *child;
	int ret;
	int child_count;

	pmu_dbg("probe owl pmic!!!!!!!!\n");
	atc260x_early_init(dev);
	pmu_dbg("atc2603c probe end\n");
	for (device_find_first_child(dev, &child), child_count = 0; child;
	     device_find_next_child(&child)) {
		pmu_dbg("%s\n", child->name);
		ret = device_probe(child);
		if (ret)
			return ret;
		child_count++;
	}
	return 0;
}

static struct dm_pmic_ops atc2603c_ops = {
	.read = atc2603c_read,
	.write = atc2603c_write,
};

static const struct udevice_id atc2603c_ids[] = {
	{.compatible = "actions,atc2603c"},
	{}
};

U_BOOT_DRIVER(pmic_atc2603c) = {
.name = "atc2603c pmic", .id = UCLASS_PMIC, .of_match =
	    atc2603c_ids, .ops = &atc2603c_ops, .probe =
	    atc2603c_probe, .bind = atc2603c_bind,};

int owl_pmic_init(void)
{
	struct udevice **devp = NULL;
	struct udevice *dev = NULL;
	struct udevice *bus = NULL;
	int pmic_slave_addr;
	const void *blob = gd->fdt_blob;
	int node;
	const char *pmic_name = "atc2603c_pmic";
	int ret;
	char buf[2];
	unsigned long time = 0;
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct owl_pwm_device *pwm;
	ret = pmic_get(pmic_name, devp);
	if (ret) {
		pmu_warn("%s not exsit!\n", pmic_name);
		return -ENODEV;
	}
	node = (*devp)->of_offset;
	bus = dev_get_parent(*devp);
	if (ret) {
		pmu_warn("parent dev %s not exsit!\n", bus->name);
		return -ENODEV;
	}

	pmic_slave_addr = fdtdec_get_int(blob, node, "reg", 0);
	if (pmic_slave_addr == 0) {
		pmu_warn("pmic_slave_addr 0x%x not exsit!\n", pmic_slave_addr);
		return -ENXIO;
	}

	ret = dm_i2c_probe(bus, pmic_slave_addr, 0, devp);
	if (ret) {
		pmu_warn("%s probe %s fail!\n", bus->name, (*devp)->name);
		return -ENODEV;
	}
	#if 0
	pmu_config.pwm_config[0].pwm_val =
	    fdtdec_get_int(blob, node, "pwm_config_0_pwm_val", 0);
	pmu_config.pwm_config[0].mfp.shift =
	    fdtdec_get_int(blob, node, "pwm_config_0_shift", 0);
	pmu_config.pwm_config[0].mfp.mask =
	    fdtdec_get_int(blob, node, "pwm_config_0_mask", 0);
	pmu_config.pwm_config[0].mfp.val =
	    fdtdec_get_int(blob, node, "pwm_config_0_val", 0);
	pmu_config.pwm_config[0].mfp.no =
	    fdtdec_get_int(blob, node, "pwm_config_0_no", 0);

	pmu_config.pwm_config[1].pwm_val =
	    fdtdec_get_int(blob, node, "pwm_config_1_pwm_val", 0);
	pmu_config.pwm_config[1].mfp.shift =
	    fdtdec_get_int(blob, node, "pwm_config_1_shift", 0);
	pmu_config.pwm_config[1].mfp.mask =
	    fdtdec_get_int(blob, node, "pwm_config_1_mask", 0);
	pmu_config.pwm_config[1].mfp.val =
	    fdtdec_get_int(blob, node, "pwm_config_1_val", 0);
	pmu_config.pwm_config[1].mfp.no =
	    fdtdec_get_int(blob, node, "pwm_config_1_no", 0);

	pmu_config.pwm_config[2].pwm_val =
	    fdtdec_get_int(blob, node, "pwm_config_2_pwm_val", 0);
	pmu_config.pwm_config[2].mfp.shift =
	    fdtdec_get_int(blob, node, "pwm_config_2_shift", 0);
	pmu_config.pwm_config[2].mfp.mask =
	    fdtdec_get_int(blob, node, "pwm_config_2_mask", 0);
	pmu_config.pwm_config[2].mfp.val =
	    fdtdec_get_int(blob, node, "pwm_config_2_val", 0);
	pmu_config.pwm_config[2].mfp.no =
	    fdtdec_get_int(blob, node, "pwm_config_2_no", 0);
	for (ret = 0; ret < 3; ret++) {
		pmu_dbg("pmu_config.pwm_config[%d].pwm_val = %ld\n", ret,
		       pmu_config.pwm_config[ret].pwm_val);
		pmu_dbg("pmu_config.pwm_config[%d].mfp.shift = %d\n", ret,
		       pmu_config.pwm_config[ret].mfp.shift);
		pmu_dbg("pmu_config.pwm_config[%d].mfp.mask = %d\n,", ret,
		       pmu_config.pwm_config[ret].mfp.mask);
		pmu_dbg("pmu_config.pwm_config[%d].mfp.val = %d\n", ret,
		       pmu_config.pwm_config[ret].mfp.val);
		pmu_dbg("pmu_config.pwm_config[%d].mfp.no = %d\n", ret,
		       pmu_config.pwm_config[ret].mfp.no);
	}

	vdd_core_init(&pmu_config);
	#endif
#if 0
	if (regulator_list_autoset(autoset_regulators, NULL, true))
		error("Unable to init all mmc regulators");
#endif

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret)
			continue;

		uc_pdata = dev_get_uclass_platdata(dev);
		if (!uc_pdata)
			continue;

		autoset_regulators[time] =
		    malloc(MAX_NAME_LENGTH * sizeof(char));
		strcpy(autoset_regulators[time], uc_pdata->name);
		time++;

	}
	autoset_regulators[time] = NULL;
	if (regulator_list_autoset
	    ((const char **)autoset_regulators, NULL, false))
		error("Unable to init all mmc regulators");

	ret = 0;

	while (check_reg[ret] != ATC2603C_PMU_BASE) {
		pmic_read(*devp, check_reg[ret], (uint8_t *) buf, 2);
		pmu_dbg("0x%x:0x%x\n", check_reg[ret], (buf[0] << 8) | buf[1]);
		ret++;
	}
#if 1
	pmu_config.total_steps = fdtdec_get_int(blob, node, "total_steps", 0);
	if(pmu_config.total_steps) {

	pmu_config.vdd_gpu_step = fdtdec_get_int(blob, node, "vdd_gpu", 0);
	pmu_config.vdd_core_step = fdtdec_get_int(blob, node, "vdd_core", 0);
	pmu_dbg("pmu_config.total_steps = %d\n", pmu_config.total_steps);
	pmu_dbg("pmu_config.vdd_gpu_step = %d\n", pmu_config.vdd_gpu_step);
	pmu_dbg("pmu_config.vdd_core_step = %d\n", pmu_config.vdd_core_step);
	if (fdtdec_pwm_get(blob, node, "pwms_gpu", &pmu_config.pwm_gpu)) {
		error("no 'pwms_1' subnode\n");
		return -1;
	}
	if (fdtdec_pwm_get(blob, node, "pwms_core", &pmu_config.pwm_core)) {
		error("no 'pwms_2' subnode\n");
		return -1;
	}
	pwm = &pmu_config.pwm_gpu;
	owl_pwm_config(pwm, pmu_config.vdd_gpu_step * pwm->period / pmu_config.total_steps,
			   pwm->period, pwm->polarity);
	owl_pwm_enable(pwm);
	pwm = &pmu_config.pwm_core;
	owl_pwm_config(pwm, pmu_config.vdd_core_step * pwm->period / pmu_config.total_steps,
			   pwm->period, pwm->polarity);
	owl_pwm_enable(pwm);
	}
#endif
	return 0;
}
