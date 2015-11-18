/*
 *  Copyright (C) 2012-2015 Samsung Electronics
 *
 *  Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/atc260x/actions_reg_atc2609a.h>
#include <power/atc260x/atc260x_core.h>
#include <power/atc260x/atc2609a.h>

DECLARE_GLOBAL_DATA_PTR;
static int dcdc_reg_ctl[5] = { ATC2609A_PMU_DC0_CTL0,
	ATC2609A_PMU_DC1_CTL0,
	ATC2609A_PMU_DC2_CTL0,
	ATC2609A_PMU_DC3_CTL0,
	ATC2609A_PMU_DC4_CTL0
};

static int ldo_reg_ctl[9] = { ATC2609A_PMU_LDO0_CTL0,
	ATC2609A_PMU_LDO1_CTL0,
	ATC2609A_PMU_LDO2_CTL0,
	ATC2609A_PMU_LDO3_CTL0,
	ATC2609A_PMU_LDO4_CTL0,
	ATC2609A_PMU_LDO5_CTL0,
	ATC2609A_PMU_LDO6_CTL0,
	ATC2609A_PMU_LDO7_CTL0,
	ATC2609A_PMU_LDO8_CTL0
};

static int atc2609a_ldo_probe(struct udevice *dev)
{
	return 0;
}

static int atc2609a_ldo_get_value(struct udevice *dev)
{
	int ret, num, step, uV;
	char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	switch (num) {
	case 0:
	case 1:
	case 2:
		step = (buf[1] & (0xf << 2)) >> 2;
		uV = step * 100000 + 2300000;
		break;
	case 3:
	case 4:
	case 5:
	case 7:
	case 8:
		if (buf[1] & (1 << 5)) {
			pmu_warn("Driver not support\n");
			return -EPERM;
		} else {
			step = (buf[1] & (0xf << 1)) >> 1;
			uV = step * 100000 + 700000;
		}
		break;
	case 6:
		if (buf[1] & (1 << 5)) {
			pmu_warn("Driver not support\n");
			return -EPERM;
		} else {
			step = (buf[1] & (0xf << 1)) >> 1;
			if (step == 0xF)
				uV = 2200000;
			else
				uV = step * 50000 + 850000;
			break;
		}
	}
	return uV;
}

static int atc2609a_ldo_set_value(struct udevice *dev, int uV)
{
	int ret, num, step;
	char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	switch (num) {
	case 0:
	case 1:
	case 2:
		if (uV > 3400000)	/*max voltage */
			return -ENODEV;
		step = (uV - 2300000) / 100000;
		buf[1] &= ~(0xf << 2);
		buf[1] |= step << 2;
		ret =
		    pmic_write(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		break;
	case 3:
	case 4:
	case 5:
	case 7:
	case 8:
		if (buf[1] & (1 << 5)) {
			pmu_warn("Driver not support\n");
			return -EPERM;
		} else {
			if (uV > 2200000) {
				pmu_warn("Device not support\n");
				return -ENODEV;
			} else {
				step = (uV - 700000) / 100000;
				buf[1] &= ~(0xf << 1);
				buf[1] |= step << 1;
				ret =
				    pmic_write(dev->parent, ldo_reg_ctl[num],
					       buf, sizeof(buf));
				if (ret)
					return ret;
				break;
			}
		}
	case 6:
		if (buf[1] & (1 << 5)) {
			pmu_warn("Driver not support\n");
			return -EPERM;
		} else {
			if (uV > 2200000) {
				pmu_warn("Device not support\n");
				return -ENODEV;
			} else {
				step = (uV - 850000) / 50000;
				buf[1] &= ~(0xf << 1);
				buf[1] |= step << 1;
				if (uV > 1250000)
					step = 0xE;
				if (uV == 2200000)
					step = 0xF;
				ret =
				    pmic_write(dev->parent, ldo_reg_ctl[num],
					       buf, sizeof(buf));
				if (ret)
					return ret;
				break;
			}
		}
	}
}

static bool atc2609a_ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret, num;
	char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	enable = ((buf[1] << 7) > 0) ? true : false;
	return enable;
}

static int atc2609a_ldo_set_enable(struct udevice *dev, bool enable)
{
	int ret, num, step;
	char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	if (enable)
		buf[1] |= 0x1;
	else
		buf[1] &= ~0x1;
	ret = pmic_write(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	pmu_dbg("ldo%d %s\n", num, (enable == true) ? "enable" : "disable");
	return ret;
}

static int atc2609a_dcdc_probe(struct udevice *dev)
{
	return 0;
}

static int atc2609a_dcdc_get_value(struct udevice *dev)
{
	int ret, num, step, uV;
	char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, dcdc_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	switch (num) {
	case 0:
	case 1:
	case 2:
		uV = buf[0] * 6250 + 600000;
		break;
	case 3:
		uV = (buf[0] < 128) ? (600000 + buf[0] * 6250) :
		    (1400000 + (buf[0] - 128) * 25000);
		break;
	}

	return uV;

}

static int atc2609a_dcdc_set_value(struct udevice *dev, int uV)
{
	int ret, num, step;
	char buf[2];
	num = dev->driver_data;

	switch (num) {
	case 0:
	case 1:
	case 2:
	case 4:
		if (uV > 2193750)	/*max voltage */
			return -ENODEV;
		step = (uV - 600000) / 6250;
		ret =
		    pmic_read(dev->parent, dcdc_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		buf[0] = step;
		ret =
		    pmic_write(dev->parent, dcdc_reg_ctl[num], buf,
			       sizeof(buf));
		if (ret)
			return ret;
		pmu_dbg("Set dcdc%d %d\n", num, 600000 + step * 6250);
		break;
	case 3:
		if (uV > 4000000)	/*max voltage */
			return -ENODEV;
		if ((uV > 600000) && (uV < 1393750))
			step = (uV - 600000) / 6250;
		else
			step = (uV - 1400000) / 25000 + 128;
		ret =
		    pmic_read(dev->parent, dcdc_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		buf[0] = step;
		ret =
		    pmic_write(dev->parent, dcdc_reg_ctl[num], buf,
			       sizeof(buf));
		if (ret)
			return ret;
		pmu_dbg("Set dcdc%d %d\n", num,
		       (step <
			128) ? (600000 + step * 6250) : (1400000 + (step -
								    128) *
							 25000));
		break;
	}
	return 0;
}

static bool atc2609a_dcdc_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;
	int num;
	num = dev->driver_data;
	char buf[2];
	ret = pmic_read(dev->parent, ATC2609A_PMU_DC_OSC, buf, sizeof(buf));
	ret = (buf[0] << 8) | buf[1];
	if (ret)
		return ret;
	enable = (ret & (1 << (num + 4))) > 0 ? true : false;
	return enable;
}

static int atc2609a_dcdc_set_enable(struct udevice *dev, bool enable)
{
	/*bool enable = false; */
	int ret;
	int num;
	num = dev->driver_data;
	char buf[2];
	ret = pmic_read(dev->parent, ATC2609A_PMU_DC_OSC, buf, sizeof(buf));
	if (ret)
		return ret;
	ret = (buf[0] << 8) | buf[1];
	if (enable) {
		if (ret & (1 << (num + 4))) {
			pmu_dbg("%s already enable\n", dev->name);
			return 0;
		} else {
			ret |= num << 4;
			buf[0] = ret & 0xf0;
			buf[1] = ret & 0xf;
			ret =
			    pmic_write(dev->parent, ATC2609A_PMU_DC_OSC, buf,
				       sizeof(buf));
			return ret;
		}
	} else {
		if (!(ret & (1 << (num + 4)))) {
			pmu_dbg("%s already disable\n", dev->name);
			return 0;
		} else {
			ret &= ~(num << 4);
			buf[0] = ret & 0xf0;
			buf[1] = ret & 0xf;
			ret =
			    pmic_write(dev->parent, ATC2609A_PMU_DC_OSC, buf,
				       sizeof(buf));
			return ret;
		}
	}
}

static const struct dm_regulator_ops atc2609a_ldo_ops = {
	.get_value = atc2609a_ldo_get_value,
	.set_value = atc2609a_ldo_set_value,
	.get_enable = atc2609a_ldo_get_enable,
	.set_enable = atc2609a_ldo_set_enable,
};

U_BOOT_DRIVER(atc2609a_ldo) = {
		.name = ATC2609A_LDO_DRIVER,
		.id = UCLASS_REGULATOR,
		.ops = &atc2609a_ldo_ops,
		.probe = atc2609a_ldo_probe,
};

static const struct dm_regulator_ops atc2609a_dcdc_ops = {
	.get_value = atc2609a_dcdc_get_value,
	.set_value = atc2609a_dcdc_set_value,
	.get_enable = atc2609a_dcdc_get_enable,
	.set_enable = atc2609a_dcdc_set_enable,
};

U_BOOT_DRIVER(atc2609a_dcdc) = {
		.name = ATC2609A_DCDC_DRIVER,
		.id = UCLASS_REGULATOR,
		.ops = &atc2609a_dcdc_ops,
		.probe = atc2609a_dcdc_probe,
};
