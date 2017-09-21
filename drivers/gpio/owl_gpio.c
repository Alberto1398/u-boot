/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/regs.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/gpio.h>

DECLARE_GLOBAL_DATA_PTR;


#undef OWL_GPIO_DBG
#ifdef OWL_GPIO_DBG
#define gpio_dbg(fmt, args...)	printf(fmt, ##args)
#else
#define gpio_dbg(fmt, args...)
#endif


#define OWL_GPIOS_PER_BANK 32
#define GPIO_BIT(x)		(1 << (x & 0x1f))

struct owl_gpio {
	u32 outen;
	u32 inen;
	u32 dat;
};


struct owl_gpio_privdata {
	struct owl_gpio *regs;
	const char *bank_name;	/* GPIOA,B,C */
	unsigned int gpio_base;
};

/*
 * Generic_GPIO primitives.
 */
static int owl_gpio_request(struct udevice *dev, unsigned offset,
			const char *label)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);

#ifdef CONFIG_S900
	/* GPIOD0 ~ GPIOD19 are analog pads by default.
	 * so we need to change the pad mode to digital function
	 */
	if (dev->seq == 3) {
		if (offset < 10)
			/* LVDS_OXX pad */
			writel(readl(MFP_CTL1) | (1 << 22), MFP_CTL1);
		else if (offset < 20)
			/* LVDS_EXX pad */
			writel(readl(MFP_CTL1) | (1 << 21), MFP_CTL1);
	}
#endif
	return 0;
}

/* set GPIO pin 'gpio' as an input */
static int owl_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);

	clrsetbits_le32(&priv->regs->inen, GPIO_BIT(offset), GPIO_BIT(offset));
	clrsetbits_le32(&priv->regs->outen, GPIO_BIT(offset), 0);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int owl_gpio_direction_output(struct udevice *dev, unsigned offset,
					int value)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);

	clrsetbits_le32(&priv->regs->outen, GPIO_BIT(offset), GPIO_BIT(offset));
	clrsetbits_le32(&priv->regs->inen, GPIO_BIT(offset), 0);
	clrsetbits_le32(&priv->regs->dat, GPIO_BIT(offset),
			value ? GPIO_BIT(offset) : 0);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int owl_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);
	u32 val;

	val = readl(&priv->regs->dat);

	return GPIO_BIT(offset) & val ? 1 : 0;
}

/* write GPIO OUT value to pin 'gpio' */
static int owl_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);

	clrsetbits_le32(&priv->regs->dat, GPIO_BIT(offset),
			value ? GPIO_BIT(offset) : 0);

	return 0;
}


static int owl_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct owl_gpio_privdata *priv = dev_get_priv(dev);
	unsigned int val;

	val = readl(&priv->regs->outen);
	if (val & GPIO_BIT(offset))
		return GPIOF_OUTPUT;

	val = readl(&priv->regs->inen);
	if (val & GPIO_BIT(offset))
		return GPIOF_INPUT;

	return GPIOF_FUNC;
}

static int owl_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
				struct fdtdec_phandle_args *args)
{
	desc->offset = args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;
	gpio_dbg("xlate,gpio: offset=%d,\n", desc->offset);

	return 0;
}

static const struct dm_gpio_ops gpio_owl_ops = {
	.request		= owl_gpio_request,
	.direction_input	= owl_gpio_direction_input,
	.direction_output	= owl_gpio_direction_output,
	.get_value		= owl_gpio_get_value,
	.set_value		= owl_gpio_set_value,
	.get_function		= owl_gpio_get_function,
	.xlate			= owl_gpio_xlate,
};


/**
 * Returns the name of a GPIO port
 *
 * GPIOs are named A, B, C,
 * @return allocated string containing the name
 */
static char *gpio_bank_name(int bank)
{
	char *name;

	name = malloc(6);
	if (name) {
		strcpy(name, "GPIO");
		name[4] = 'A' + bank;
		name[5] = '\0';
	}

	return name;
}

static int gpio_owl_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct owl_gpio_privdata *priv = dev_get_priv(dev);

	priv->regs = (struct owl_gpio *)fdtdec_get_addr(gd->fdt_blob,
						   dev->of_offset, "reg");
	priv->bank_name = gpio_bank_name(dev->seq);
	uc_priv->gpio_count = OWL_GPIOS_PER_BANK;
	uc_priv->bank_name = priv->bank_name;
	uc_priv->gpio_base = dev->seq * OWL_GPIOS_PER_BANK;

	return 0;
}


static const struct udevice_id owl_gpio_ids[] = {
	{ .compatible = "actions,s900-gpio" },
	{ .compatible = "actions,s700-gpio" },
	{ },
};


U_BOOT_DRIVER(gpio_owl) = {
	.name	= "gpio_owl",
	.id	= UCLASS_GPIO,
	.of_match = owl_gpio_ids,
	.probe = gpio_owl_probe,
	.priv_auto_alloc_size = sizeof(struct owl_gpio_privdata),
	.ops	= &gpio_owl_ops,
};
