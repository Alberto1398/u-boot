/*
* Actions ATC260X PMIC charger driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc2609a_charger.h>
#include <power/atc260x_charger.h>

DECLARE_GLOBAL_DATA_PTR;

#define ATC260X_VBUS_VOL_THRESHOLD	(3900)
#define ATC260X_WALL_VOL_THRESHOLD	(3400)

enum  SUPPORT_ADAPTER {
	SUPPORT_DCIN_ONLY = 0x1,
	SUPPORT_USB_ONLY = 0x2,
	SUPPORT_DCIN_USB = 0x3
};

struct atc260x_charger_data {
	int rsense;
	int support_adaptor_type;
};

struct atc260x_charger_info {
	struct atc260x_charger_data data;
	int charger_online;
};

struct atc260x_charger_info info;

static int atc260x_charger_cfg_init(struct atc260x_charger_info *info,
	const void *blob, const char *compatible)
{
	int node;
	int error = 0;

	node = fdt_node_offset_by_compatible(blob, 0, compatible);
	if (node < 0) {
		printf("%s no match in dts\n", __func__);
		return -1;
	}

	info->data.rsense = fdtdec_get_int(blob, node,
		"rsense", 0);
	error |= (info->data.rsense == -1);
	info->data.support_adaptor_type = fdtdec_get_int(blob, node,
		"support_adaptor_type", 2);
	error |= (info->data.support_adaptor_type == -1);
	if (error) {
		printf("%s get dts value failed\n", __func__);
		return -1;
	}

	debug("%s rsense(%d)\n",
		__func__, info->data.rsense);
	debug("%s support_adaptor_type(%d)\n",
		__func__, info->data.support_adaptor_type);

	return 0;
}

static int atc260x_charger_check_wall_online(struct atc260x_charger_info *info)
{
	int wallv;
	int ret;
	int i;

	for (i = 0; i < 5; i++) {
		ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_WALLV, &wallv);
		if (ret) {
			printf("[%s] get wallv auxadc err!\n", __func__);
			return ret;
		}

		if (wallv > ATC260X_WALL_VOL_THRESHOLD) {
			printf("WALL ONLINE\n");
			return ADAPTER_TYPE_WALL_PLUGIN;
		}
		mdelay(2);
		debug("%s the %d time\n", __func__, i);
	}

	printf("WALL OFFLINE(%dmv)\n", wallv);

	return ADAPTER_TYPE_NO_PLUGIN;

}

static int atc260x_charger_check_usb_online(struct atc260x_charger_info *info)
{
	struct atc260x_charger_data *data = &info->data;
	int vbusv;
	int ret;
	int i;

	if (data->support_adaptor_type == SUPPORT_DCIN_ONLY) {
		atc260x_misc_enable_wakeup_detect(ATC260X_WAKEUP_SRC_VBUS_IN, 0);
		return ADAPTER_TYPE_NO_PLUGIN;
	} else {
		atc260x_misc_enable_wakeup_detect(ATC260X_WAKEUP_SRC_VBUS_IN, 1);
	}

	for (i = 0; i < 5; i++) {
		ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_VBUSV, &vbusv);
		if (ret) {
			printf("[%s] get vbusv auxadc err!\n", __func__);
			return ret;
		}

		if (vbusv > ATC260X_VBUS_VOL_THRESHOLD) {
			printf("USB ONLINE\n");
			return ADAPTER_TYPE_USB_PLUGIN;
		}
		mdelay(2);
		debug("%s the %d time\n", __func__, i);
	}

	printf("USB OFFLINE(%dmv)\n", vbusv);

	return ADAPTER_TYPE_NO_PLUGIN;
}

int atc260x_charger_check_online(void)
{
	int mode = ADAPTER_TYPE_NO_PLUGIN;

	mode |= atc260x_charger_check_wall_online(&info);
	mode |= atc260x_charger_check_usb_online(&info);

	return mode;
}

int atc260x_charger_release_guard(void)
{
#ifdef CONFIG_ATC2609A_CHARGER
	return atc2609a_charger_release_guard();
#endif
	return -1;
}

int atc260x_charger_init(void)
{
	int ret;

#ifdef CONFIG_ATC2609A_CHARGER
	ret = atc260x_charger_cfg_init(&info, gd->fdt_blob,
		"actions,atc2609a-charger");
	if (ret)
		return ret;

	atc2609a_charger_init();
	atc2609a_charger_set_rsense(info.data.rsense);
#endif

	return 0;
}
