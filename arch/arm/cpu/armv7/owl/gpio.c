#include <common.h>
#include <asm/gpio.h>

int __gpio_request(unsigned gpio, const char *label)
{
	return 0;
}
int gpio_request(unsigned gpio, const char *label)
			__attribute__((weak, alias("__gpio_request")));

int __gpio_free(unsigned gpio)
{
	return 0;
}
int gpio_free(unsigned gpio)
			__attribute__((weak, alias("__gpio_free")));

int __gpio_direction_input(unsigned gpio)
{
	return 0;
}
int gpio_direction_input(unsigned gpio)
			__attribute__((weak, alias("__gpio_direction_input")));

int __gpio_direction_output(unsigned gpio, int value)
{
	return 0;
}
int gpio_direction_output(unsigned gpio, int value)
			__attribute__((weak, alias("__gpio_direction_output")));

int __gpio_get_value(unsigned gpio)
{
	return 0;
}
int gpio_get_value(unsigned gpio)
			__attribute__((weak, alias("__gpio_get_value")));

int __gpio_set_value(unsigned gpio, int value)
{
	return 0;
}
int gpio_set_value(unsigned gpio, int value)
			__attribute__((weak, alias("__gpio_set_value")));


