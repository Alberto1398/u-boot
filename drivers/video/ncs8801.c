#include <asm/arch/clocks.h>
#include <asm/arch/regs.h>
#include <asm/arch/owlxx_lcd.h>
#include <asm/io.h>
#include <asm/arch/pwm_backlight.h>
#include <asm/gpio.h>

#include <common.h>
#include <malloc.h>
#include <i2c.h>

#include <video_fb.h>
#include <owlxx_dss.h>
#include <linux/list.h>
#include <linux/fb.h>

#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

struct ncs8801_data {
	int i2c_bus;
	int i2c_dev_addr;

	struct owlxx_fdt_gpio_state reset_gpio;
	struct owlxx_fdt_gpio_state power_gpio;

};

static struct ncs8801_data ncs8801;

static int activate_gpio(struct owlxx_fdt_gpio_state *gpio)
{
	int active_level;

	active_level = (gpio->flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
	owlxx_gpio_generic_direction_output(gpio->chip, gpio->gpio, active_level);
	return 0;
}

static int deactivate_gpio(struct owlxx_fdt_gpio_state *gpio)
{
	int active_level;

	active_level = (gpio->flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
	owlxx_gpio_generic_direction_output(gpio->chip, gpio->gpio, !active_level);
	return 0;
}

int ncs8801_enable(struct lcdi_convertion *lcdic)
{
	u8 read_tmp;
	u8 read_reg;

	I2C_SET_BUS(ncs8801.i2c_bus);

	activate_gpio(&ncs8801.reset_gpio);
	activate_gpio(&ncs8801.power_gpio);

	udelay(60);
	deactivate_gpio(&ncs8801.reset_gpio);
	udelay(60);

	debug("%s, xyl revise 0113\n");

	i2c_reg_write(ncs8801.i2c_dev_addr, 0x5, 0xf);
	i2c_reg_write(ncs8801.i2c_dev_addr, 0xd, 0xf);

	debug("%s, xyl revise 0113 1111\n");

/*
	read_reg = 0x0;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);
*/


	i2c_reg_write(ncs8801.i2c_dev_addr, 0x4b, 0x4);
	i2c_reg_write(ncs8801.i2c_dev_addr, 0xc, 0x0);
	i2c_reg_write(ncs8801.i2c_dev_addr, 0x0, 0xc);
	i2c_reg_write(ncs8801.i2c_dev_addr, 0x3c, 0x7);

/*
	read_reg = 0x4e;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);

	read_reg = 0x4f;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);

	read_reg = 0x4b;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);

	read_reg = 0xc;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);

	read_reg = 0x0;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);


	read_reg = 0x2d;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);

	read_reg = 0x2c;
	read_tmp = i2c_reg_read(ncs8801.i2c_dev_addr, read_reg);
	debug("%s, read ncs8801 reg = 0x%x, val = 0x%x\n",
			__func__, read_reg, read_tmp);
*/

	return 0;
}

int ncs8801_disable(struct lcdi_convertion *lcdic)
{
	activate_gpio(&ncs8801.reset_gpio);
	deactivate_gpio(&ncs8801.power_gpio);

	return 0;
}

struct lcdi_convertion_ops ncs8801_ops = {
	.enable = ncs8801_enable,
	.disable = ncs8801_disable,
};

int fdtdec_get_ncs8801_data(struct ncs8801_data *data)
{
	enum fdt_compat_id compat;
	int node;
	int parent;

	/* Get the node from FDT for codec */
	node = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_NCS8801_LVDS2EDP);
	if (node <= 0) {
		debug("ncs8801: No node for ncs8801 in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	parent = fdt_parent_offset(gd->fdt_blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	compat = fdtdec_lookup(gd->fdt_blob, parent);
	switch (compat) {
	case COMPAT_ACTIONS_OWLXX_I2C:
		data->i2c_bus = i2c_get_bus_num_fdt(parent);
		debug("i2c bus = %d\n", data->i2c_bus);
		if (data->i2c_bus < 0)
			return -1;

		data->i2c_dev_addr = fdtdec_get_int(gd->fdt_blob, node,
							"reg", 0);
		debug("i2c dev addr = %d\n", data->i2c_dev_addr);
		if (data->i2c_dev_addr == 0)
			return -1;

		break;
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}

	owlxx_fdtdec_decode_gpio(
		gd->fdt_blob, node, "reset-gpio", &data->reset_gpio);

	owlxx_fdtdec_decode_gpio(
		gd->fdt_blob, node, "power-gpio", &data->power_gpio);
	
	return 0;
}

int ncs8801_init(void)
{
#ifdef CONFIG_OF_CONTROL
	if (fdtdec_get_ncs8801_data(&ncs8801)) {
		printf("NCS8801: fdt No ncs8801 par\n");
		return -1;
	}
#endif
	lcdi_convertion_register(LCDIC_TYPE_LVDS2EDP,
				&ncs8801_ops);
	return 0;
}

