#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_core.h>
#include <common.h>
#include <dm.h>
#include <power/atc260x/actions_reg_atc2609a.h>

#ifndef KEY_VOLUMEUP
#define KEY_VOLUMEUP (115)
#endif

#ifndef KEY_VOLUMEDOWN
#define KEY_VOLUMEDOWN (114)
#endif

/* atc2609a ----------------------------------------------------------------- */

static int _atc2609a_auxadc_get_raw(struct atc260x_dev *atc260x, uint channel)
{
	static const u16 sc_reg_tbl[] = {
		[ATC260X_AUXADC_BATV] = ATC2609A_PMU_BATVADC,
		[ATC260X_AUXADC_VBUSV] = ATC2609A_PMU_VBUSVADC,
		[ATC260X_AUXADC_WALLV] = ATC2609A_PMU_WALLVADC,
		[ATC260X_AUXADC_SYSPWRV] = ATC2609A_PMU_SYSPWRADC,
		[ATC260X_AUXADC_REMCON] = ATC2609A_PMU_REMCONADC,
		[ATC260X_AUXADC_AUX0] = ATC2609A_PMU_AUXADC0,
		[ATC260X_AUXADC_AUX1] = ATC2609A_PMU_AUXADC1,
		[ATC260X_AUXADC_AUX2] = ATC2609A_PMU_AUXADC2,
		[ATC260X_AUXADC_AUX3] = ATC2609A_PMU_AUXADC3,
	};
	int ret;

	ret = atc260x_m_reg_read(atc260x, sc_reg_tbl[channel]);
	if (ret < 0) {
		dev_err(atc260x, "auxadc io err, ret=%d\n", ret);
	}
	return ret & ((1U << 12) - 1U);
}

static int _atc2609a_auxadc_get_translated(struct atc260x_dev *atc260x,
					   uint channel, s32 *p_tr_value)
{
	ulong raw_value;
	int ret;

	ret = _atc2609a_auxadc_get_raw(atc260x, channel);
	if (ret < 0)
		return ret;
	raw_value = ret;

	switch (channel) {
	case ATC260X_AUXADC_BATV:
		*p_tr_value = (raw_value * 375) >> 8;	/* raw * 0.732421875mv * 2 */
		break;
	case ATC260X_AUXADC_VBUSV:
	case ATC260X_AUXADC_WALLV:
	case ATC260X_AUXADC_SYSPWRV:
		*p_tr_value = (raw_value * 1875) >> 10;	/* raw * 0.732421875mv * 2.5 */
		break;
	case ATC260X_AUXADC_REMCON:
		*p_tr_value = raw_value / 4U;	/* raw/4 , ( n/1024 ) */
		break;
	case ATC260X_AUXADC_AUX0:
	case ATC260X_AUXADC_AUX1:
	case ATC260X_AUXADC_AUX2:
	case ATC260X_AUXADC_AUX3:
		*p_tr_value = (raw_value * 375) >> 9;	/* raw * 0.732421875mv */
		break;
	}

	return 0;
}

/* common ------------------------------------------------------------------- */

int atc260x_m_auxadc_get_raw(struct atc260x_dev *atc260x, uint channel)
{
	int ret = -1;

	if (channel >= ATC260X_AUXADC_CNT) {
		dev_err(atc260x, "unknown auxadc channel: %u\n", channel);
		return -1;
	}

	ret = _atc2609a_auxadc_get_raw(atc260x, channel);

	return ret;
}

int atc260x_m_auxadc_get_translated(struct atc260x_dev *atc260x, uint channel,
				    s32 *p_tr_value)
{
	int ret = -1;

	if (channel >= ATC260X_AUXADC_CNT) {
		dev_err(atc260x, "unknown auxadc channel: %u\n", channel);
		return -1;
	}

	ret = _atc2609a_auxadc_get_translated(atc260x, channel, p_tr_value);

	return ret;
}

int atc260x_auxadc_init(struct atc260x_dev *atc260x)
{
	/* no need to do anything currently. */
	return 0;
}

/* check the status of adckey. If a key is pressed, return the key code */
int atc260x_adckey_check(void)
{
	int val;
	/* get the raw value of adc */
	val = atc260x_auxadc_get_raw(5);

	if (0 <= val && val <= 100) {
		dev_info(atc260x, "val=%d,KEY_VOLUMEUP pressed\n", val);
		/* return the key code of KEY_VOLUMEUP */
		return KEY_VOLUMEUP;
	}

	if (3300 <= val && val <= 3700) {
		dev_info(atc260x, "val=%d,KEY_VOLUMEDOWN pressed\n", val);
		/* return the key code of KEY_VOLUMEDOWN */
		return KEY_VOLUMEDOWN;
	}
	return 0;
}

int atc260x_auxadc_probe(struct udevice *dev)
{
	/*printf("atc260x_auxadc_probe\n");*/
	return 0;
}

static const struct udevice_id atc260x_auxadc_ids[] = {
	{.compatible = "actions,atc2609a-auxadc"},
	{.compatible = "actions,atc2603c-auxadc"},
	{}
};

U_BOOT_DRIVER(atc260x_auxadc) = {
		.id = UCLASS_ROOT,
		.name = "atc260x_auxadc",
		.of_match = atc260x_auxadc_ids,
		.probe = atc260x_auxadc_probe
};
