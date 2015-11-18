/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <dwc3-uboot.h>
#include <dwc3-owl-uboot.h>
#include <owl-usb-phy-uboot.h>
#include "board.h"

extern int read_mi_item(char *name, void *buf, unsigned int count);
#if  defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
extern int owl_mmc_init(int dev_index);

int board_mmc_init(bd_t *bis)
{
#ifdef SLOT0
	owl_mmc_init(SLOT0);
#endif
#ifdef SLOT2
	owl_mmc_init(SLOT2);
#endif
	return 0;
}
#endif

/*check serialno vaild*/
static int serialno_check(char *sn, int num)
{
	int i;

	for (i = 0; i < num; i++) {
		if ((sn[i] >= '0' && sn[i] <= '9')
		    || (sn[i] >= 'A' && sn[i] <= 'Z')
		    || (sn[i] >= 'a' && sn[i] <= 'z'))
			continue;
		if (sn[i] == 0)
			break;
		if (sn[i] == 10) {
			sn[i] == 0;
			break;
		}
		printf("no=%d invaild:%d\n", i, sn[i]);
		return 0;
	}
	if (i >= 6)
		return 1;

	printf("sn=%s err\n", sn);
	return 0;
}

int serialno_read(char *buf)
{
	char sn[20];
	int ret = 0;

	memset(sn, 0, sizeof(sn));
#if (defined(CONFIG_OWL_NAND) ||  defined(CONFIG_OWL_MMC))
	ret = read_mi_item("SN", sn, sizeof(sn));
	if (ret < 0) {
		printf("read bf-sn faild,errorcode:%d,\n", ret);
		return -1;
	} else {
		printf("bf-sn:%d, %s\n", ret, sn);
	}

	if (ret >= 6 && serialno_check(sn, ret)) {
		sn[ret] = 0;
		printf("use bf serialno\n");
		strcpy(buf, sn);
		return 0;
	}
#endif
	printf("use deafult serialno\n");
	strcpy(buf, "abcdef0123456789");

	return 0;

}

int boot_append_serialno(void *fdt)
{
	char sn[64];
	char buf[32];
	int node = 0, ret = 0;
	const char *bootargs;
	char new_prop[256];

	ret = serialno_read(buf);
	if (ret < 0)
		return ret;

	sprintf(sn, "androidboot.serialno=%s", buf);

	/* find "/chosen" node. */
	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		printf("fdt_path_offset failed %d\n", node);
		return -ENOENT;
	}

	bootargs = fdt_getprop(fdt, node, "bootargs", NULL);
	if (!bootargs) {
		printf("%s: Warning: No bootargs in fdt %s\n", __func__,
		       bootargs);
		return -ENOENT;
	}

	/* set serialNo  */
	sprintf(new_prop, "%s %s", bootargs, sn);
	ret = fdt_setprop(fdt, node, "bootargs",
			  new_prop, strlen(new_prop) + 1);
	if (ret < 0) {
		printf("could not set bootargs %s\n", new_prop);
		return ret;
	}

	printf("bootargs %s\n", new_prop);

	return 0;
}

int boot_append_bootargs_add(void *fdt)
{
	int node = 0, ret = 0;
	const char *bootargs;
	char *bootargs_add;
	char new_prop[CONFIG_SYS_BARGSIZE];

	bootargs_add = getenv("bootargs.add");
	if (bootargs_add == NULL)
		return 0;

	/* find "/chosen" node. */
	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		printf("fdt_path_offset failed %d\n", node);
		return -ENOENT;
	}

	bootargs = fdt_getprop(fdt, node, "bootargs", NULL);
	if (!bootargs) {
		printf("%s: Warning: No bootargs in fdt %s\n", __func__,
		       bootargs);
		return -ENOENT;
	}

	snprintf(new_prop, CONFIG_SYS_BARGSIZE, "%s %s",
		 bootargs, bootargs_add);

	ret = fdt_setprop(fdt, node, "bootargs",
			  new_prop, strlen(new_prop) + 1);
	if (ret < 0) {
		printf("could not set bootargs %s\n", new_prop);
		return ret;
	}

	printf("%s, bootargs %s\n", __func__, new_prop);

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int fdt_fixup_bootargs(void *fdt)
{
	int node, ret;
	const char *bootargs;
	char new_prop[CONFIG_SYS_BARGSIZE];
	int power_status;

	/* find "/chosen" node. */
	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0)
		return -ENOENT;

	bootargs = fdt_getprop(fdt, node, "bootargs", NULL);
	if (!bootargs) {
		printf("%s: Warning: No bootargs in fdt\n", __func__);
		return -ENOENT;
	}
#ifdef CONFIG_ANDROID_RECOVERY
	if (owl_get_recovery_mode())
		/* enter recovery */
		return 0;
#endif

	power_status = owl_get_power_status();
	if ((power_status == POWER_EXCEPTION) ||
	    ((power_status != POWER_NORMAL_CHARGER) &&
	     (power_status != POWER_CRITICAL_LOW_CHARGER)))
		/* no need to update bootargs */
		return 0;

	/* set androidboot to charger */
	printf("%s: set androidboot.mode: charger\n", __func__);
	snprintf(new_prop, CONFIG_SYS_BARGSIZE, "%s %s ",
		 bootargs, "androidboot.mode=charger");

	ret = fdt_setprop(fdt, node, "bootargs",
			  new_prop, strlen(new_prop) + 1);
	if (ret < 0) {
		printf("%s: could not set bootargs %s\n", __func__, new_prop);
		return ret;
	}

	printf("new bootargs %s\n", new_prop);

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	fdt_fixup_bootargs(blob);
	boot_append_serialno(blob);

	boot_append_bootargs_add(blob);
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
extern int owl_mac_initialize(bd_t *bis);
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_OWL_MAC
	rc = owl_mac_initialize(bis);
	if (rc)
		printf("Error %d registering ETHERNET\n", rc);
#endif
#ifdef CONFIG_USB_DWC3
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif
	return rc;
}
#endif

#ifdef CONFIG_USB_DWC3

#define USB3_REGISTER_BASE 0xe0400000

static struct dwc3_device usb_otg_ss1 = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = (void __iomem *)USB3_REGISTER_BASE,
	.tx_fifo_resize = false,
	.index = 0,
};

int board_usb_init(int index, enum usb_init_type init)
{
	switch (index) {
	case 0:
		if (init == USB_INIT_DEVICE)
			usb_otg_ss1.dr_mode = USB_DR_MODE_PERIPHERAL;
		else
			usb_otg_ss1.dr_mode = USB_DR_MODE_HOST;

		dwc3_owl_uboot_init();
		dwc3_uboot_init(&usb_otg_ss1);

		if (init == USB_INIT_DEVICE)
			owl_usb2phy_param_uboot_setup(1);
		else
			owl_usb2phy_param_uboot_setup(0);
		owl_usb3phy_uboot_init();

		break;

	default:
		printf("Invalid Controller Index\n");
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	switch (index) {
	case 0:
		dwc3_uboot_exit(index);
		dwc3_owl_uboot_exit();
		break;
	default:
		printf("Invalid Controller Index\n");
	}

	return 0;
}

int usb_gadget_handle_interrupts(int index)
{
	dwc3_uboot_handle_interrupt(index);

	return 0;
}
#endif

int owl_board_late_init(void)
{
	int ret = 0;

	owl_check_enter_shell();

#ifdef CONFIG_CHECK_KEY
	ret = owl_check_key();
	if (ret)
		return ret;
#endif

#ifdef CONFIG_CHECK_POWER
	ret = owl_check_power();
	if (ret)
		return ret;
#endif

#ifdef CONFIG_POWER_BATTERY
	owl_battery_reset();
#endif

	return ret;
}
