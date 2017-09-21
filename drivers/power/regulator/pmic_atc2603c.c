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
#include <power/atc260x/actions_reg_atc2603c.h>
#include <power/atc260x/atc260x_core.h>
#include <power/atc260x/atc2603c.h>

#define ATC2603C_PMU_DC0_CTL0 0x00
#define ATC2603C_PMU_LDO0_CTL 0x00
DECLARE_GLOBAL_DATA_PTR;
static int dcdc_reg_ctl[4] = { ATC2603C_PMU_DC0_CTL0,
	ATC2603C_PMU_DC1_CTL0,
	ATC2603C_PMU_DC2_CTL0,
	ATC2603C_PMU_DC3_CTL0
};

static int ldo_reg_ctl[12] = { ATC2603C_PMU_LDO0_CTL,
	ATC2603C_PMU_LDO1_CTL,
	ATC2603C_PMU_LDO2_CTL,
	ATC2603C_PMU_LDO3_CTL,
	ATC2603C_PMU_LDO4_CTL,
	ATC2603C_PMU_LDO5_CTL,
	ATC2603C_PMU_LDO6_CTL,
	ATC2603C_PMU_LDO7_CTL,
	ATC2603C_PMU_LDO8_CTL,
	ATC2603C_PMU_LDO9_CTL,
	ATC2603C_PMU_LDO10_CTL,
	ATC2603C_PMU_LDO11_CTL
};

static int atc2603c_ldo_probe(struct udevice *dev)
{
	return 0;
}

static int atc2603c_ldo_get_value(struct udevice *dev)
{
	int ret, num, step, uV;
	uV = 0;
	unsigned char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	switch (num) {
	case 1:
	case 2:
	case 5:
	case 11:
		step = buf[0] >> 5;
		printf("ldo%d %d uV", num, 100000 * step + 2600000);
		break;
	case 3:
	case 7:
		step = buf[0] >> 5;
		printf("ldo%d %d uV", num, 100000 * step + 1500000);
		break;
	case 6:
		step = buf[0] >> 3;
		printf("ldo%d %d uV", num, 25000 * step + 700000);
		break;
	case 8:
		step = buf[0] >> 4;
		printf("ldo%d %d uV", num, 100000 * step + 2300000);
		break;
	}
	return uV;
}

static int atc2603c_ldo_set_value(struct udevice *dev, int uV)
{
	int ret, num, step;
	unsigned char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	switch (num) {
	case 1:
	case 2:
	case 5:
	case 11:
		if ((uV < 2600000) || (uV > 3300000)) {
			printf("ldo%d Not support!\n", num);
			return -ENODEV;
		} else {
			step = (uV - 2600000) / 100000;
			buf[0] &= ~(0x7 << 5);
			buf[0] |= step << 5;
			ret =
			    pmic_write(dev->parent, ldo_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set ldo%d %d uV", num, 100000 * step + 2600000);
		}
		break;
	case 3:
	case 7:
		if ((uV < 1500000) || (uV > 2000000)) {
			printf("ldo%d Not support!\n", num);
			return -ENODEV;
		} else {
			step = (uV - 1500000) / 100000;
			buf[0] &= ~(0x7 << 5);
			buf[0] |= step << 5;
			ret =
			    pmic_write(dev->parent, ldo_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set ldo%d %d uV", num, 100000 * step + 2000000);
		}
		break;
	case 6:
		if ((uV < 700000) || (uV > 1400000)) {
			printf("ldo%d Not support!\n", num);
			return -ENODEV;
		} else {
			step = (uV - 700000) / 25000;
			buf[0] &= ~(0x1f << 3);
			buf[0] |= step << 3;
			ret =
			    pmic_write(dev->parent, ldo_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set ldo%d %d uV", num, 25000 * step + 700000);
		}
		break;
	case 8:
		if ((uV < 2300000) || (uV > 3300000)) {
			printf("ldo%d Not support!\n", num);
			return -ENODEV;
		} else {
			step = (uV - 2300000) / 100000;
			buf[0] &= ~(0xf << 4);
			buf[0] |= step << 4;
			ret =
			    pmic_write(dev->parent, ldo_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set ldo%d %d uV", num, 100000 * step + 2300000);
		}
		break;
	}
	return 0;
}

static bool atc2603c_ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret, num;
	unsigned char buf[2];
	num = dev->driver_data;
	if ((num == 5) || (num == 7) || (num == 8)) {
		ret =
		    pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		enable = ((buf[1] & 0x1) > 0) ? true : false;
	} else {
		enable = true;
	}
	return enable;
}

static int atc2603c_ldo_set_enable(struct udevice *dev, bool enable)
{
	int ret, num;
	ret = 0;
	unsigned char buf[2];
	num = dev->driver_data;
	if ((num == 5) || (num == 7) || (num == 8)) {
		ret =
		    pmic_read(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		if (enable)
			buf[1] |= 0x1;
		else
			buf[1] &= ~0x1;
		ret =
		    pmic_write(dev->parent, ldo_reg_ctl[num], buf, sizeof(buf));
		if (ret)
			return ret;
		printf("ldo%d %s\n", num,
		       (enable == true) ? "enable" : "disable");
	} else {
		if (enable) {
			printf("ldo%d has enable\n", num);
		} else {
			printf("Not support disable ldo%d\n", num);
			return -EPERM;
		}
	}
	return ret;
}

static int atc2603c_dcdc_probe(struct udevice *dev)
{
	return 0;
}

static int atc2603c_dcdc_get_value(struct udevice *dev)
{
	int ret, num, step, uV, ic_ver;
	uV = 0;
	unsigned char buf[2];
	num = dev->driver_data;
	ret = pmic_read(dev->parent, dcdc_reg_ctl[num], buf, sizeof(buf));
	if (ret)
		return ret;
	ret = buf[0] << 8 | buf[1];
	switch (num) {
	case 1:
		step = (ret & (0x1f << 7)) >> 7;
		uV = step * 25000 + 700000;
		break;
	case 2:
		ic_ver =
		    pmic_read(dev->parent, ATC2603C_CHIP_VER, buf, sizeof(buf));
		if (ic_ver > 0) {
			step = (ret & (0x1f << 8)) >> 8;
			uV = step * 50000 + 1000000;
		} else if (ic_ver == 0) {
			step = (ret & (0xf << 8)) >> 8;
			uV = step * 50000 + 1300000;
		} else {
			printf("ERROR: ic_ver = %d\n", ic_ver);
			return ic_ver;
		}
		break;
	case 3:
		step = (ret & (0x7 << 9)) >> 9;
		uV = step * 100000 + 2600000;
		break;
	}
	printf("Get dcdc%d %d uV\n", num, uV);
	return uV;

}

static int atc2603c_dcdc_set_value(struct udevice *dev, int uV)
{
	int ret, num, step, ic_ver;
	unsigned char buf[2];
	num = dev->driver_data;
	switch (num) {
	case 1:
		if ((uV > 700000) && (uV < 1400000)) {
			ret =
			    pmic_read(dev->parent, dcdc_reg_ctl[num], buf,
				      sizeof(buf));
			if (ret)
				return ret;
			ret = buf[0] << 8 | buf[1];
			step = (uV - 700000) / 25000;
			ret &= ~(0x1F << 7);
			ret |= step << 7;
			buf[0] = ret >> 8;
			buf[1] = ret & 0xff;
			ret =
			    pmic_write(dev->parent, dcdc_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set dcdc%d %d uV\n", num,
			       (700000 + step * 25000));
			return ret;
		} else {
			printf("Set dcdc%d beyond select range\n", num);
			return -ENODEV;
		}
		break;
	case 2:
		ic_ver =
		    pmic_read(dev->parent, ATC2603C_CHIP_VER, buf, sizeof(buf));
		if (ic_ver > 0) {
			if ((uV > 1000000) || (uV < 1850000)) {
				ret =
				    pmic_read(dev->parent, dcdc_reg_ctl[num],
					      buf, sizeof(buf));
				if (ret)
					return ret;
				step = (uV - 1000000) / 50000;
				buf[0] &= ~(0x1f << 8);
				buf[0] |= step << 8;
				ret =
				    pmic_write(dev->parent, dcdc_reg_ctl[num],
					       buf, sizeof(buf));
				if (ret)
					return ret;
				printf("Set dcdc%d %d uV\n", num,
				       (1000000 + step * 50000));
				return ret;
			} else {
				printf("Set dcdc%d beyond select range\n", num);
				return -ENODEV;
			}
		} else if (ic_ver == 0) {
			if ((uV > 1300000) || (uV < 2150000)) {
				ret =
				    pmic_read(dev->parent, dcdc_reg_ctl[num],
					      buf, sizeof(buf));
				if (ret)
					return ret;
				step = (uV - 1300000) / 50000;
				buf[0] &= ~(0xf << 8);
				buf[0] |= step << 8;
				ret =
				    pmic_write(dev->parent, dcdc_reg_ctl[num],
					       buf, sizeof(buf));
				if (ret)
					return ret;
				printf("Set dcdc%d %d uV\n", num,
				       (1300000 + step * 50000));
				return ret;
			} else {
				printf("Set dcdc%d beyond select range\n", num);
				return -ENODEV;
			}
		} else {
			printf("ERROR: ic_ver = %d\n", ic_ver);
			return ic_ver;
		}
		break;
	case 3:
		if ((uV > 2600000) || (uV < 3300000)) {
			ret =
			    pmic_read(dev->parent, dcdc_reg_ctl[num], buf,
				      sizeof(buf));
			if (ret)
				return ret;
			step = (uV - 2600000) / 100000;
			buf[0] &= ~(0x7 << 9);
			buf[0] |= step << 9;
			ret =
			    pmic_write(dev->parent, dcdc_reg_ctl[num], buf,
				       sizeof(buf));
			if (ret)
				return ret;
			printf("Set dcdc%d %d uV\n", num,
			       (2600000 + step * 100000));
			return ret;
		} else {
			printf("Set dcdc%d beyond select range\n", num);
			return -ENODEV;
		}
		break;
	}
	return 0;
}

static bool atc2603c_dcdc_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;
	int num;
	num = dev->driver_data;
	unsigned char buf[2];
	switch (num) {
	case 1:
	case 3:
		enable = true;
		break;
	case 2:
		ret = pmic_read(dev->parent, dcdc_reg_ctl[2], buf, sizeof(buf));
		if (ret)
			return ret;
		enable = (buf[0] >> 7) > 0 ? true : false;
		break;
	}
	return enable;
}

static int atc2603c_dcdc_set_enable(struct udevice *dev, bool enable)
{
	/*bool enable = false; */
	int ret = 0;
	int num;
	num = dev->driver_data;
	unsigned char buf[2];

	switch (num) {
	case 1:
	case 3:
		if (!enable)
			printf("dc%d can not be set disable", num);
		break;
	case 2:
		ret = pmic_read(dev->parent, dcdc_reg_ctl[2], buf, sizeof(buf));
		if (ret)
			return ret;
		if (!enable) {
			buf[0] &= ~(0x1 << 7);
		} else {
			buf[0] |= 0x1 << 7;
		}
		ret =
		    pmic_write(dev->parent, dcdc_reg_ctl[num], buf,
			       sizeof(buf));
		if (ret)
			return ret;
		break;

	}
	return ret;
}

static const struct dm_regulator_ops atc2603c_ldo_ops = {
	.get_value = atc2603c_ldo_get_value,
	.set_value = atc2603c_ldo_set_value,
	.get_enable = atc2603c_ldo_get_enable,
	.set_enable = atc2603c_ldo_set_enable,
};

U_BOOT_DRIVER(atc2603c_ldo) = {
.name = ATC2603C_LDO_DRIVER, .id = UCLASS_REGULATOR, .ops =
	    &atc2603c_ldo_ops, .probe = atc2603c_ldo_probe,};

static const struct dm_regulator_ops atc2603c_dcdc_ops = {
	.get_value = atc2603c_dcdc_get_value,
	.set_value = atc2603c_dcdc_set_value,
	.get_enable = atc2603c_dcdc_get_enable,
	.set_enable = atc2603c_dcdc_set_enable,
};

U_BOOT_DRIVER(atc2603c_dcdc) = {
.name = ATC2603C_DCDC_DRIVER, .id = UCLASS_REGULATOR, .ops =
	    &atc2603c_dcdc_ops, .probe = atc2603c_dcdc_probe,};
