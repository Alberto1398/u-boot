#ifndef _ATC260X_POWER_LED_H
#define _ATC260X_POWER_LED_H

struct mfp_cfg {
	unsigned mfp_io;
	unsigned io_start_bit;
};
struct mux {
	unsigned mux;
	unsigned start_bit;
	unsigned bit_val;
};
struct gpio_data {
	unsigned data_reg;
	unsigned index;
	unsigned active_low;
};
struct atc260x_gpio_cfg {
	struct mfp_cfg *mfp;
	struct gpio_data *gpio;
	struct mux *mux;
	unsigned default_state;
};

void gpio_led_init(void);

#endif	/* _ATC260X_POWER_LED_H */
