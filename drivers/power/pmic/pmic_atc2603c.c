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
#include <power/atc260x/atc2603c.h>
DECLARE_GLOBAL_DATA_PTR;
static const struct pmic_child_info pmic_children_regulator_info[] = {
	{.prefix = "dcdc", .driver = ATC2603C_DCDC_DRIVER},
	{.prefix = "ldo", .driver = ATC2603C_LDO_DRIVER},
	{},
};

static const char *autoset_regulators[] = {
	"dcdc1",
	"dcdc2",
	"dcdc3",
	"ldo1",
	"ldo2",
	"ldo3",
	"ldo5",
	"ldo7",
	"ldo8",
	NULL,
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

	printf("probe owl pmic!!!!!!!!\n");
	atc260x_early_init(dev);
	for (device_find_first_child(dev, &child), child_count = 0; child;
	     device_find_next_child(&child)) {
		printf("%s\n", child->name);
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
	{.compatible = "actions, atc2603c"},
	{}
};

U_BOOT_DRIVER(pmic_atc2603c) = {
		.name = "atc2603c pmic",
		.id = UCLASS_PMIC,
		.of_match = atc2603c_ids,
		.ops = &atc2603c_ops,
		.probe = atc2603c_probe,
		.bind = atc2603c_bind,
};

int owl_pmic_init(void)
{
	struct udevice **devp = NULL;
	struct udevice *bus = NULL;
	int pmic_slave_addr;
	const void *blob = gd->fdt_blob;
	int node;
	const char *pmic_name = "atc2603c_pmic";
	int ret;
	struct udevice **rdev = NULL;
	const char *rdev_name = "dcdc1";
	char buf[2];
	ret = pmic_get(pmic_name, devp);
	if (ret) {
		printf("%s not exsit!\n", pmic_name);
		return -ENODEV;
	}
	node = (*devp)->of_offset;
	bus = dev_get_parent(*devp);
	if (ret) {
		printf("parent dev %s not exsit!\n", bus->name);
		return -ENODEV;
	}

	pmic_slave_addr = fdtdec_get_int(blob, node, "reg", 0);
	if (pmic_slave_addr == 0) {
		printf("pmic_slave_addr 0x%x not exsit!\n", pmic_slave_addr);
		return -ENXIO;
	}

	ret = dm_i2c_probe(bus, pmic_slave_addr, 0, devp);
	if (ret) {
		printf("%s probe %s fail!\n", bus->name, (*devp)->name);
		return -ENODEV;
	}

	if (regulator_list_autoset(autoset_regulators, NULL, true))
		error("Unable to init all mmc regulators");

	return 0;
}
