#include <power/atc260x/atc260x_core.h>
#include <i2c.h>

static int _atc260x_i2c_reg_read(struct atc260x_dev *dev, uint reg)
{
	int ret;
	u8 dat[2];
	ret = dm_i2c_read(dev->udev, reg, dat, 2);
	if (ret) {
		printf("i2c read err, reg=0x%x ret=%d\n", reg, ret);
		return ret;
	}
	ret = (dat[0] << 8) | dat[1];
	dev_dbg(dev, "dm:read reg 0x%x value 0x%x\n", reg, ret);
	return ret;
}

static int _atc260x_i2c_reg_write(struct atc260x_dev *dev, uint reg, u16 val)
{
	int ret;
	u8 dat[2];

	dev_dbg(dev, "write reg 0x%x value 0x%x\n", reg, val);
	dat[0] = (val >> 8) & 0xff;
	dat[1] = val & 0xff;
	ret = dm_i2c_write(dev->udev, reg, dat, 2);
	if (ret) {
		printf("i2c write failed, reg=0x%x ret=%d\n", reg, ret);
		return ret;
	}

	return 0;
}

int atc260x_i2c_if_init(struct atc260x_dev *dev)
{
	dev_dbg(dev, "i2c_if_init enter\n");
	dev->reg_read = _atc260x_i2c_reg_read;
	dev->reg_write = _atc260x_i2c_reg_write;

	return 0;
}
