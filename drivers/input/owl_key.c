/*
 *  (C) Copyright 2011
 *  Actions Semi .Inc <www.actions-semi.com>
 *
 *
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <input.h>
#include <stdio_dev.h>
#include <asm/io.h>
#include <asm/arch/clocks.h>
#include <power/owlxx_power.h>
#include <asm/arch/pmu.h>
#include <malloc.h>
#include <linux/input.h>

#include "owlxx_key.h"

DECLARE_GLOBAL_DATA_PTR;
struct adkey *adkey;
enum {
	KBC_DEBOUNCE_COUNT	= 2,
	KBC_REPEAT_RATE_MS	= 5,
	KBC_REPEAT_DELAY_MS	= 500,
	KBC_CLOCK_KHZ		= 32,	/* Keyboard uses a 32KHz clock */
};

static struct keyb {
	struct input_config input;	/* The input layer */

	unsigned char inited;		/* 1 if keyboard has been inited */
	unsigned char first_scan;	/* 1 if this is our first key scan */
	unsigned char created;		/* 1 if driver has been created */

	/*
	 * After init we must wait a short time before polling the keyboard.
	 * This gives the tegra keyboard controller time to react after reset
	 * and lets us grab keys pressed during reset.
	 */
	unsigned int init_dly_ms;	/* Delay before we can read keyboard */
	unsigned int start_time_ms;	/* Time that we inited (in ms) */
	unsigned int last_poll_ms;	/* Time we should last polled */
	unsigned int next_repeat_ms;	/* Next time we repeat a key */
} config;

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
int kbd_tstc(void)
{
	/* Just get input to do this for us */
	return input_tstc(&config.input);
}

/**
 * Read a key
 *
 * TODO: U-Boot wants 0 for no key, but Ctrl-@ is a valid key...
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
int kbd_getc(void)
{
	/* Just get input to do this for us */
	return input_getc(&config.input);
}

static unsigned int key_convert(unsigned int adc_val)
{
	unsigned int i;
	unsigned int key_val = 0;

	for (i = 0; i < adkey->keymapsize; i++) {

		if ((adc_val >= *(adkey->left_adc_val + i))
			&& (adc_val <= *(adkey->right_adc_val + i))) {
			key_val = *(adkey->key_values + i);
			break;

		}
	}

	return key_val;
}

static void owlxx_kbc_open(void)
{
	unsigned int scan_period;
	scan_period = KBC_REPEAT_RATE_MS / 2;
	config.init_dly_ms = scan_period * 2 + 2;

	config.start_time_ms = get_timer(0);
	config.last_poll_ms = config.next_repeat_ms = get_timer(0);
	config.first_scan = 1;
	config.input.fifo_in = 0;
	config.input.fifo_out = 0;

}

static int _pmu_get_remcon_adc_val(void)
{
	int val = -1;
	switch(OWLXX_PMU_ID) {
	case OWLXX_PMU_ID_ATC2603A:
		val = atc260x_reg_read(ATC2603A_PMU_REMCONADC);
		break;
	case OWLXX_PMU_ID_ATC2603B:
		val = atc260x_reg_read(ATC2609A_PMU_REMCONADC) / 4U;
		break;
	case OWLXX_PMU_ID_ATC2603C:
		val = atc260x_reg_read(ATC2603C_PMU_REMCONADC);
		break;
	}
	return val;
}

int owlxx_kbc_check(struct input_config *input)
{
	static const u16 sc_pmu_regtbl_sysctl2[OWLXX_PMU_ID_CNT] = {
		[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_SYS_CTL2,
		[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_SYS_CTL2,
		[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_SYS_CTL2,
	};
	u16 reg_sysctl2;
	int adc_val = 0;
	int temp = 0;
	int changed = 0;
	int filter_index = 0;
	int onoff_val = 0;

	if (!(&config)->first_scan &&
			get_timer((&config)->last_poll_ms) < KBC_REPEAT_RATE_MS)
		return 0;

	reg_sysctl2 = sc_pmu_regtbl_sysctl2[OWLXX_PMU_ID];

	temp = adkey->filter_dep;
	/*if this is the first scan,it may be a long press,
	so we just do a simple filt*/
	if ((&config)->first_scan) {
		onoff_val = atc260x_reg_read(reg_sysctl2);
		if (onoff_val & (1 << 14) || onoff_val & (1 << 13)) {
			atc260x_set_bits(reg_sysctl2, (3<<13), onoff_val);
			input_queue_ascii(&config.input, KEY_ONOFF);
			return 1;
		}
		while (temp > 0) {
			adc_val = _pmu_get_remcon_adc_val();
			adkey->key_val = key_convert(adc_val);

			if (adkey->key_val == adkey->old_key_val
				&& adkey->key_val != 0) {
				filter_index++;
				*(adkey->adc_buffer + filter_index - 1) =
					adc_val;
				mdelay(20);
			}
			temp--;
			adkey->old_key_val = adkey->key_val;
		}

		if (filter_index >= adkey->filter_dep - 1) {
			/*index:the value of pressed key*/
			if (adkey->key_val != 0)
				input_queue_ascii(&config.input, adkey->old_key_val);
		}
		(&config)->last_poll_ms = get_timer(0);
		(&config)->first_scan = 0;
		return 1;
	}
	temp = adkey->filter_dep;
	/*if not the first scan, we should report only when key state changed*/
	adc_val = _pmu_get_remcon_adc_val();
	adkey->key_val = key_convert(adc_val);
	changed = adkey->old_key_val ^ adkey->key_val;

	if (changed && adkey->key_val != 0) {
		while (temp > 0) {
			adc_val = _pmu_get_remcon_adc_val();
			adkey->key_val = key_convert(adc_val);
			if (adkey->key_val == adkey->old_key_val) {
				filter_index++;
				*(adkey->adc_buffer + filter_index) =
					adc_val;
				mdelay(50);
			}
			temp--;
			adkey->old_key_val = adkey->key_val;
		}
		temp = adkey->filter_dep;
		if (filter_index >= adkey->filter_dep - 1) {
			adkey->key_val =
				key_convert(*(adkey->adc_buffer + filter_index - 1));
			adkey->old_key_val = adkey->key_val;
			input_queue_ascii(&config.input, adkey->key_val);
		}
		adkey->old_key_val = adkey->key_val;
		return 1;
	}
	adkey->old_key_val = adkey->key_val;
	onoff_val = atc260x_reg_read(reg_sysctl2);
	if (onoff_val & (1 << 14) || onoff_val & (1 << 13)) {
		atc260x_set_bits(reg_sysctl2, (3<<13), onoff_val);
		input_queue_ascii(&config.input, KEY_ONOFF);
		return 1;
	}
	return 0;
}

static int init_owlxx_keyboard(void)
{
	/* check if already created */
	if (config.created)
		return 0;
	int	node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
					  COMPAT_ACTIONS_OWLXX_ADCKEY);
	if (node < 0) {
		printf("cannot locate keyboard node\n");
		return node;
	}

	adkey->keymapsize = fdtdec_get_int(gd->fdt_blob,
		       node, "keymapsize", 9);
	adkey->filter_dep = fdtdec_get_int(gd->fdt_blob,
		       node, "filter_dep", 5);

	adkey->variance = fdtdec_get_int(gd->fdt_blob,
		       node, "variance", 20);
	fdtdec_get_int_array(gd->fdt_blob,
			node, "key_val",
			adkey->key_values,
			adkey->keymapsize);

	fdtdec_get_int_array(gd->fdt_blob,
			node, "left_adc_val",
			adkey->left_adc_val,
			adkey->keymapsize);
	fdtdec_get_int_array(gd->fdt_blob,
			node, "right_adc_val",
			adkey->right_adc_val,
			adkey->keymapsize);

	input_set_delays(&config.input,
		KBC_REPEAT_DELAY_MS, KBC_REPEAT_RATE_MS);


	adkey->old_key_val = 0;
	owlxx_kbc_open();
	config.created = 1;

	return 0;


}

int drv_keyboard_init(void)
{
	struct stdio_dev dev;
	adkey->adc_buffer = (unsigned int *)malloc(40);
	adkey->left_adc_val = (unsigned int *)malloc(40);
	adkey->right_adc_val = (unsigned int *)malloc(40);
	adkey->key_values = (unsigned int *)malloc(40);
	setenv("stdin" , "serial,owl-kbc");
	/*char *stdinname = getenv("stdin");*/

	int error;
	if (input_init(&config.input, 0)) {
		printf("Cannot set up input\n");
		return -1;
	}
	config.input.read_keys = owlxx_kbc_check;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "owl-kbc");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = kbd_getc;
	dev.tstc = kbd_tstc;
	dev.start = init_owlxx_keyboard;

	/* Register the device. init_owlxx_keyboard() will be called soon */
	error = input_stdio_register(&dev);

	if (error)
		return error;
	return 0;
}
