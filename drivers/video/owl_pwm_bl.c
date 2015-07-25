/*
 * PWM BACKLIGHT driver for Board based on OWL.
 *
 */
#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>

#include <common.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pwm.h>
#include <asm/arch/pwm_backlight.h>

DECLARE_GLOBAL_DATA_PTR;

struct owlxx_pwm_bl {
	struct pwm_device pwm;
	int total_steps;
	int min_brightness;
	int max_brightness;
	int dft_brightness;

	int brightness;

	struct owlxx_fdt_gpio_state en_gpio;

	int node;
} pwm_bl;

static int pwm_activate_gpio(struct owlxx_fdt_gpio_state *gpio)
{
	int active_level;

	active_level = (gpio->flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
	owlxx_gpio_generic_direction_output(gpio->chip, gpio->gpio, active_level);
	return 0;
}

static int pwm_deactivate_gpio(struct owlxx_fdt_gpio_state *gpio)
{
	int active_level;

	active_level = (gpio->flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
	owlxx_gpio_generic_direction_output(gpio->chip, gpio->gpio, !active_level);
	return 0;
}


int owlxx_pwm_backlight_update_status(struct pwm_backlight_data *pd)
{
	int brightness = pd->brightness;
	int total_steps = pwm_bl.total_steps;

	struct pwm_device *pwm;

	pwm = &pwm_bl.pwm;
	if (pd->power == 0)
		brightness = 0;

	if (brightness > 0)
		brightness += pwm_bl.min_brightness;

	if (brightness > total_steps)
		return -1;

	if (brightness == 0) {
		pwm_deactivate_gpio(&pwm_bl.en_gpio);
		pwm_config(
			pwm->hwpwm, pwm->period, pwm->period, !pwm->polarity);
		mdelay(10);
		pwm_disable(pwm->hwpwm);
	} else {
		pwm_config(
			pwm->hwpwm, brightness * pwm->period / total_steps,
			pwm->period, pwm->polarity);
		pwm_enable(pwm->hwpwm);
		mdelay(10);
		pwm_activate_gpio(&pwm_bl.en_gpio);
	}

	return 0;
}

int owlxx_pwm_backlight_init(struct pwm_backlight_data *pd)
{
	int devnode;

	devnode = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_ACTIONS_OWLXX_PWM_BACKLIGHT);
	if (devnode < 0) {
		debug("%s: Cannot find device tree node\n", __func__);
		return -1;
	}

	pwm_bl.node = devnode;

	if (fdtdec_pwm_get(gd->fdt_blob, devnode, "pwms", &pwm_bl.pwm))
		return -1;

	pwm_bl.total_steps =
		fdtdec_get_int(gd->fdt_blob, devnode, "total_steps", -1);
	pwm_bl.min_brightness =
		fdtdec_get_int(gd->fdt_blob, devnode, "min_brightness", -1);
	pwm_bl.max_brightness =
		fdtdec_get_int(gd->fdt_blob, devnode, "max_brightness", -1);
	pwm_bl.dft_brightness =
		fdtdec_get_int(gd->fdt_blob, devnode, "dft_brightness", -1);

	owlxx_fdtdec_decode_gpio(
		gd->fdt_blob, devnode, "backlight_en_gpios", &pwm_bl.en_gpio);

	pd->power = 0;
	pd->max_brightness = pwm_bl.max_brightness - pwm_bl.min_brightness;
	pd->brightness = pwm_bl.dft_brightness - pwm_bl.min_brightness;

	return 0;
}
