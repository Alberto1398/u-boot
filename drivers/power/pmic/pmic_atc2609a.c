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
#include <power/atc260x/actions_reg_atc2609a.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <power/atc260x/atc2609a.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#define MAX_REGULATOR_COUNT 20
#define MAX_NAME_LENGTH 10
DECLARE_GLOBAL_DATA_PTR;
static const struct pmic_child_info pmic_children_regulator_info[] = {
	{.prefix = "dcdc", .driver = ATC2609A_DCDC_DRIVER},
	{.prefix = "ldo", .driver = ATC2609A_LDO_DRIVER},
	{},
};

static char *autoset_regulators[MAX_REGULATOR_COUNT];

static const unsigned short check_reg[] = {
	ATC2609A_PMU_DC0_CTL0,
	ATC2609A_PMU_DC0_CTL1,
	ATC2609A_PMU_DC0_CTL6,
	ATC2609A_PMU_DC1_CTL0,
	ATC2609A_PMU_DC1_CTL1,
	ATC2609A_PMU_DC1_CTL6,
	ATC2609A_PMU_DC2_CTL0,
	ATC2609A_PMU_DC2_CTL1,
	ATC2609A_PMU_DC2_CTL6,
	ATC2609A_PMU_DC3_CTL0,
	ATC2609A_PMU_DC3_CTL1,
	ATC2609A_PMU_DC3_CTL6,
	ATC2609A_PMU_DC4_CTL0,
	ATC2609A_PMU_DC4_CTL1,
	ATC2609A_PMU_DC4_CTL6,
	ATC2609A_PMU_BASE,
};


static int atc2609a_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		error("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}
	return 0;
}

static int atc2609a_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		error("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}
	return 0;
}

static int atc2609a_bind(struct udevice *parent)
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

static int atc2609a_probe(struct udevice *dev)
{
	struct udevice *child;
	int ret;
	int child_count;
	pmu_dbg("probe owl pmic!!!!!!!!\n");
	atc260x_early_init(dev);
	pmu_dbg("atc2609 probe end\n");
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

static struct dm_pmic_ops atc2609a_ops = {
	.read = atc2609a_read,
	.write = atc2609a_write,
};

static const struct udevice_id atc2609a_ids[] = {
	{.compatible = "actions,atc2609a"},
	{}
};

U_BOOT_DRIVER(pmic_atc2609a) = {
		.name = "atc2609a pmic",
		.id = UCLASS_PMIC,
		.of_match = atc2609a_ids,
		.ops = &atc2609a_ops,
		.probe = atc2609a_probe,
		.bind = atc2609a_bind,
};

int owl_pmic_init(void)
{
	struct udevice **devp = NULL;
	struct udevice *dev = NULL;
	struct udevice *bus = NULL;
	int pmic_slave_addr;
	const void *blob = gd->fdt_blob;
	int node;
	const char *pmic_name = "atc2609a_pmic";
	int ret;
	char buf[2];
	unsigned long time = 0;
	struct dm_regulator_uclass_platdata *uc_pdata;
	ret = pmic_get(pmic_name, devp);
	if (ret) {
		pmu_dbg("%s not exsit!\n", pmic_name);
		return -ENODEV;
	}
	node = (*devp)->of_offset;
	bus = dev_get_parent(*devp);
	if (ret) {
		pmu_dbg("parent dev %s not exsit!\n", bus->name);
		return -ENODEV;
	}

	pmic_slave_addr = fdtdec_get_int(blob, node, "reg", 0);
	if (pmic_slave_addr == 0) {
		pmu_dbg("pmic_slave_addr 0x%x not exsit!\n", pmic_slave_addr);
		return -ENXIO;
	}

	ret = dm_i2c_probe(bus, pmic_slave_addr, 0, devp);
	if (ret) {
		pmu_dbg("%s probe %s fail!\n", bus->name, (*devp)->name);
		return -ENODEV;
	}

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret)
			continue;

		uc_pdata = dev_get_uclass_platdata(dev);
		if (!uc_pdata)
			continue;

		autoset_regulators[time] = malloc(MAX_NAME_LENGTH * sizeof(char));
		strcpy(autoset_regulators[time], uc_pdata->name);
		time++;

	}
	autoset_regulators[time] = NULL;
	if (regulator_list_autoset((const char **)autoset_regulators, NULL, false))
		error("Unable to init all mmc regulators");

	ret = 0;

	while (check_reg[ret] != ATC2609A_PMU_BASE) {
		pmic_read(*devp, check_reg[ret], (uint8_t *)buf, 2);
		pmu_dbg("0x%x:0x%x\n", check_reg[ret], (buf[0] << 8)| buf[1]);
		ret++;
	}

	return 0;
}
