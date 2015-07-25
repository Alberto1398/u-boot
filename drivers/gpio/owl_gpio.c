#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>

#include <common.h>

#include <asm/arch/gpio.h>
#include <asm/arch/clocks.h>
#include <asm/arch/regs.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_REG_BASE               (GPIO_MFP_PWM_BASE)

#define GPIO_BANK(gpio)             ((gpio) / 32)
#define GPIO_IN_BANK(gpio)          ((gpio) % 32)
#define GPIO_BIT(gpio)              (1 << GPIO_IN_BANK(gpio))

#define GPIO_REG_OUTEN(gpio)	(GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x0)
#define GPIO_REG_INEN(gpio)	(GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x4)
#define GPIO_REG_DAT(gpio)	(GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x8)

int owlxx_gpio_get_value(unsigned gpio)
{
	return readl(GPIO_REG_DAT(gpio)) & GPIO_BIT(gpio);
}

int owlxx_gpio_set_value(unsigned gpio, int value)
{
	if (value)
		setbits_le32(GPIO_REG_DAT(gpio), GPIO_BIT(gpio));
	else
		clrbits_le32(GPIO_REG_DAT(gpio), GPIO_BIT(gpio));

	return 0;
}

int owlxx_gpio_direction_input(unsigned gpio)
{
	clrbits_le32(GPIO_REG_OUTEN(gpio), GPIO_BIT(gpio));
	setbits_le32(GPIO_REG_INEN(gpio), GPIO_BIT(gpio));
	return 0;
}

int owlxx_gpio_direction_output(unsigned gpio, int value)
{
	clrbits_le32(GPIO_REG_INEN(gpio), GPIO_BIT(gpio));
	setbits_le32(GPIO_REG_OUTEN(gpio), GPIO_BIT(gpio));

	owlxx_gpio_set_value(gpio, value);
	return 0;
}

struct gpiochip_ops owlxx_gpiochip_ops = {
	.direction_input = owlxx_gpio_direction_input,
	.direction_output = owlxx_gpio_direction_output,
	.get_value = owlxx_gpio_get_value,
	.set_value = owlxx_gpio_set_value,
};

int owlxx_gpio_init(void)
{
	int dev_node = 0;
	int ret;

	dev_node = fdtdec_next_compatible(gd->fdt_blob,
			0, COMPAT_ACTIONS_OWLXX_GPIO);
	if (dev_node <= 0) {
		debug("Can't get owl gpio device node\n");
		return -1;
	}

/***enable gpio module*************/
	setbits_le32(CMU_DEVCLKEN0, DEVCLKEN_GPIO);
/*****************************/

	ret = gpiochip_add(OWLXX_GPIOID_MASTER_IC,
			dev_node, &owlxx_gpiochip_ops);
	if (ret)
		return -1;

	return 0;
}

