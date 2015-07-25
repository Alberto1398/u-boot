#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int owlxx_speaker_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_OWLXX_GPIO)
	int devnode, active_level;
	struct owlxx_fdt_gpio_state gpio;

	devnode = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_ACTIONS_AUDIO_SPEAKER);
	if (devnode < 0) {
		debug("%s: Cannot find device tree node\n", __func__);
		return -1;
	}

	if(!owlxx_fdtdec_decode_gpio(gd->fdt_blob, devnode, "speaker_gpios", &gpio)) {
		//printf("speaker : pin(%d), level(%d)\n ", gpio.gpio, !gpio.flags);
		active_level = (gpio.flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		owlxx_gpio_generic_direction_output(gpio.chip, gpio.gpio, !active_level);
	}
#endif
	return 0;
}
