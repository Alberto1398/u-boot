#include <asm/io.h>
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_adckey.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/input/input.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <power/atc260x/actions_reg_atc2609a.h>

#define OWL_PMU_ID_CNT 1
#define OWL_PMU_ID	0

DECLARE_GLOBAL_DATA_PTR;

static int adc_channel;

struct adckey {
	unsigned short adc_min;
	unsigned short adc_max;
	unsigned int key_val;
};
static struct adckey adckey[3];

static void pmu_adc_init(void)
{
	static const u16 pmu_adc_ctl[OWL_PMU_ID_CNT] = {
		/*ATC2603A_PMU_AUXADC_CTL0,*/
		ATC2609A_PMU_AUXADC_CTL0,
		/*ATC2603C_PMU_AUXADC_CTL0,*/
	};
	int adc_en_bit = 5;

	if (adc_channel < 3) {
		/* bit15=aux0, bit14=aux1, bit13= aux2 */
		adc_en_bit = 15 - adc_channel;
	} else {
		/* bit5 = REMCON */
		adc_en_bit = 5;
	}
	atc260x_set_bits(pmu_adc_ctl[OWL_PMU_ID], (1 << adc_en_bit),
			 (1 << adc_en_bit));
}

static int pmu_adc_read(void)
{
	static const u16 pmu_adc_reg[OWL_PMU_ID_CNT][4] = {
		/*{ATC2603A_PMU_AUXADC0, ATC2603A_PMU_AUXADC1, ATC2603A_PMU_AUXADC2, ATC2603A_PMU_REMCONADC},*/
		{ATC2609A_PMU_AUXADC0, ATC2609A_PMU_AUXADC1, ATC2609A_PMU_AUXADC2, ATC2609A_PMU_REMCONADC},
		/*{ATC2603C_PMU_AUXADC0, ATC2603C_PMU_AUXADC1, ATC2603C_PMU_AUXADC2, ATC2603C_PMU_REMCONADC},*/
	};

	return atc260x_reg_read(pmu_adc_reg[OWL_PMU_ID][adc_channel]);
}

static int pmu_adckey_read(void)
{
	int adc_val;

	pmu_adc_init();
	adc_val = pmu_adc_read();
	udelay(1);
	adc_val = pmu_adc_read();

	return adc_val;
}

static int adckey_index(void)
{
	int i;
	unsigned short adc_val;
	adc_val = pmu_adckey_read();
	printf("adc_val=%d\n", adc_val);
	for (i = 0; i < sizeof(adckey)/sizeof(struct adckey); i++) {
		if (adc_val >= adckey[i].adc_min &&
		    adc_val <= adckey[i].adc_max)
			return i;
	}
	return -1;
}

static int adckey_init(void)
{
	int	node, ret;
	int keymapsize, i;
	u32 keyval[10], left_adc_val[10], right_adc_val[10];
	const char *str_channel;
	const char *name_channel[4] = {"AUX0", "AUX1", "AUX2", "REMCON"};
	const char *adc_compat[3] = {"actions,atc2603c-adckeypad",
				      "actions,atc2603a-adckeypad",
				      "actions,atc2609a-adckeypad"};

	for (i = 0; i < 3; i++) {
		node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
						     adc_compat[i]);
		if (node >= 0)
			break;
	}
	if (i == 3) {
		printf("cannot locate keyboard node\n");
		return node;
	}

	keymapsize = fdtdec_get_int(gd->fdt_blob,
		       node, "keymapsize", 0);
	if (keymapsize <= 0) {
		printf("adckey: keymapsize err\n");
		return keymapsize;
	}

	str_channel = fdt_getprop(gd->fdt_blob, node, "adc_channel_name", NULL);
	if (str_channel != NULL) {
		for (i = 0; i < 4; i++) {
			if (0 == strcmp(name_channel[i], str_channel)) {
				adc_channel = i;
				break;
			}
		}
		printf("adckey: %s, channel=%d\n", str_channel, adc_channel);
	}

	keymapsize = (keymapsize < 10) ? keymapsize : 10;

	ret = fdtdec_get_int_array(gd->fdt_blob,
			node, "key_val", keyval, keymapsize);
	ret += fdtdec_get_int_array(gd->fdt_blob,
			node, "left_adc_val", left_adc_val, keymapsize);
	ret += fdtdec_get_int_array(gd->fdt_blob,
			node, "right_adc_val", right_adc_val, keymapsize);
	if (ret != 0) {
		printf("adckey: key or adc val err\n\n");
		return -1;
	}

	printf("adckey: keynum=%d\n", keymapsize);
	for (i = 0; i < keymapsize; i++) {
		adckey[i].adc_min =  left_adc_val[i];
		adckey[i].adc_max =  right_adc_val[i];
		adckey[i].key_val =  keyval[i];
	}
	return 0;
}


int atc260x_adckey_scan(void)
{
	int ret;
	int index;

	adckey_init();
	index = adckey_index();
	if (-1 == index)
		return -1;
	ret = adckey[adckey_index()].key_val;

	return ret;
}
