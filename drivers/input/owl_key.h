/*#define REMCON_ADC_EN			(1 << 5)
#define KEY_FILTER_DEPTH		5
#define KEY_VAL_MASK			0x0
#define KEY_UP			1
#define KEY_HOME			2
#define KEY_MENU			3
#define KEY_VOLUMEUP			4
#define KEY_VOLUMEDOWN			5
#define KEY_RESERVED			0
*/


#if 1
static const unsigned int left_adc[9] = {
0x00, 0x32, 0x97, 0xfb,  0x15f, 0x1c3, 0x24e, 0x2b3, 0x317};
static const unsigned int right_adc[9] = {
	0x00, 0x96, 0xfa, 0x15e, 0x1c2, 0x226, 0x2b2, 0x316, 0x400};
static const unsigned int key_val[9] =  {KEY_HOME, KEY_MENU, KEY_VOLUMEUP,
KEY_VOLUMEDOWN, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
KEY_RESERVED,  KEY_UP};
#else
char *key[] = {
	"adckey.size",
	"adckey.left",
	"adckey.right",
	"adckey.values",
	"adckey.filter_dep",
	"adckey.variance",
	"adckey.period",
};
#endif

/*struct adc_key {
	unsigned int min_adc_val;
	unsigned int max_adc_val;
	unsigned int keyval;
};*/

struct adkey {
	unsigned int *adc_buffer;

	unsigned int *left_adc_val;
	unsigned int *right_adc_val;
	unsigned int *key_values;

	unsigned int filter_dep;
	unsigned int variance;

	unsigned int adc_val;
	unsigned int adc_val_sum;
	unsigned int keymapsize;
	unsigned int old_key_val;
	unsigned int key_val;
	unsigned int filter_index;

	unsigned int pressed;
};
