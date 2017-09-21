#include <asm/io.h>
#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <malloc.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_power_leds.h>
//#define DEBUG_GPIO_LED
DECLARE_GLOBAL_DATA_PTR;

struct atc260x_gpio_led {
	int num_leds;
	struct atc260x_gpio_cfg cfg[10];
}gpio_led;

static int get_gpio_led_cfg(void)
{
	int node = -1;
	int subnode = 1;
	int ret = 0;
	char subnode_name[6];
	char index[2] = "1";
	int i = 0;
	int mux_reg;
	printf("[%s]start...\n", __func__);
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "actions,atc2603c-power-led");
	if (node < 0)
		return -1;
	strcpy(subnode_name, "led-1");
	printf("[%s]node:%d\n", __func__, node);

	while (node > 0 && subnode > 0) {
		subnode = fdt_subnode_offset(gd->fdt_blob, node, subnode_name);
		if (subnode > 0) {
			gpio_led.cfg[gpio_led.num_leds].mfp = malloc(sizeof(struct mfp_cfg));
			gpio_led.cfg[gpio_led.num_leds].gpio = malloc(sizeof(struct gpio_data));
			gpio_led.cfg[gpio_led.num_leds].mux = malloc(sizeof(struct mux));

			fdtdec_get_int_array(gd->fdt_blob, subnode,
					"mfp_io", gpio_led.cfg[gpio_led.num_leds].mfp, 2);
			fdtdec_get_int_array(gd->fdt_blob, subnode,
					"gpio", gpio_led.cfg[gpio_led.num_leds].gpio, 3);
			fdtdec_get_int_array(gd->fdt_blob, subnode,
					"mux", gpio_led.cfg[gpio_led.num_leds].mux, 3);

			gpio_led.cfg[gpio_led.num_leds].default_state = fdtdec_get_int(gd->fdt_blob, subnode, "default-state", 0);
			gpio_led.num_leds++;
		}
		subnode_name[4]++;
		if (subnode_name[4] > '9')
			break;
	}
	return gpio_led.num_leds;
}
void pr_debug_info(int led_num)
{
	int i;
	printf("======================\n");
	for (i = 0; i < led_num; i++) {
		printf("mux[%d]:<0x%x %d %d>\n", i + 1, gpio_led.cfg[i].mux->mux, gpio_led.cfg[i].mux->start_bit, gpio_led.cfg[i].mux->bit_val);
		printf("mfp_io[%d]:<0x%x %d>\n", i + 1, gpio_led.cfg[i].mfp->mfp_io,
			gpio_led.cfg[i].mfp->io_start_bit);
		printf("gpio[%d]:<0x%x %d %d>\n", i + 1, gpio_led.cfg[i].gpio->data_reg,
			gpio_led.cfg[i].gpio->index, gpio_led.cfg[i].gpio->active_low);
		printf("default-state[%d]:<%d>\n", i + 1, gpio_led.cfg[i].default_state);
	}
	printf("======================\n");
}
void gpio_led_init(void)
{
	int led_num;
	int i;
	unsigned reg_addr;
	unsigned reg_val;
	printf("[%s]start...\n", __func__);
	led_num = get_gpio_led_cfg();
	printf("led_num:%d\n", led_num);
	if (led_num < 0)
		return;
#ifdef DEBUG_GPIO_LED
	pr_debug_info(led_num);
#endif
	for(i = 0; i < led_num; i++) {
		/*MUX*/
		reg_addr = gpio_led.cfg[i].mux->mux;
		reg_val = atc260x_reg_read(reg_addr) |
		(gpio_led.cfg[i].mux->bit_val <<
		gpio_led.cfg[i].mux->start_bit);
		atc260x_reg_write(reg_addr, reg_val);
		/*input/output set*/
		reg_addr = gpio_led.cfg[i].mfp->mfp_io;
		reg_val = atc260x_reg_read(reg_addr) | 1 << (gpio_led.cfg[i].mfp->io_start_bit +
		gpio_led.cfg[i].gpio->index);
		atc260x_reg_write(reg_addr, reg_val);

		/*data set*/
		reg_addr = gpio_led.cfg[i].gpio->data_reg;
		reg_val = atc260x_reg_read(reg_addr);

		if (gpio_led.cfg[i].gpio->active_low) {
			if (gpio_led.cfg[i].default_state) {
				reg_val &= ~(1 << gpio_led.cfg[i].gpio->index);
				atc260x_reg_write(reg_addr, reg_val);
			} else {
				reg_val |= 1 << gpio_led.cfg[i].gpio->index;
				atc260x_reg_write(reg_addr, reg_val);
			}
		} else {
			reg_val |= gpio_led.cfg[i].default_state << gpio_led.cfg[i].gpio->index;
			atc260x_reg_write(reg_addr, reg_val);
		}
	}
}
