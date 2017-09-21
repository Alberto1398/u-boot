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
#include <asm/arch/sys_proto.h>
#include "board.h"

extern int read_mi_item(char *name, void *buf, unsigned int count);
extern int write_mi_item(char *name, void *buf, unsigned int count);
#if  defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
extern int owl_mmc_init(int dev_index);
extern int owl_mmc_raid0_init(int dev_index_0, int dev_index_1);

int board_mmc_init(bd_t *bis)
{
    int id = 0, dev0_id = -1, dev1_id = -1;

#ifdef SLOT0
	owl_mmc_init(SLOT0);
    id++;
#endif
#ifdef SLOT2
	#ifdef CONFIG_BOOTDEV_AUTO
	if (owl_get_bootdev() == BOOTDEV_EMMC) {
		owl_mmc_init(SLOT2);
		dev0_id = id;
		id++;
	}
	#else
	owl_mmc_init(SLOT2);

	dev0_id = id;
	id++;
	#endif
#endif
#if defined(CONFIG_S900)
#ifdef SLOT3
	owl_mmc_init(SLOT3);
	dev1_id = id;
	id++;
#endif
#ifdef CONFIG_OWL_EMMC_RAID0
	owl_mmc_raid0_init(dev0_id, dev1_id);
#endif
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
			sn[i] = 0;
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
	char new_prop[CONFIG_SYS_BARGSIZE];

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


static void boot_dvfslevel_to_add(void)
{
#ifdef CONFIG_S700
	char *boot_add;
	char new_args[CONFIG_SYS_BARGSIZE];
	const char *dvfs = "androidboot.dvfslevel=0x700x";

	boot_add = getenv("bootargs.add");
	if (boot_add != NULL) {
		snprintf(new_args, CONFIG_SYS_BARGSIZE, "%s %s",
			 boot_add, dvfs);
		setenv("bootargs.add", new_args);
	} else {
		setenv("bootargs.add", dvfs);
	}
#endif
}

static void boot_boardinfo_to_add(void)
{
#ifdef CONFIG_S700
#if (defined(CONFIG_OWL_NAND) ||  defined(CONFIG_OWL_MMC))
	char binfo[17];
	int ret;
	char *boot_add;
	char new_args[CONFIG_SYS_BARGSIZE];
	memset(binfo, 0, sizeof(binfo));
	ret = read_mi_item("VENDOR", binfo, 16);
	if (ret < 0) {
		printf("read VENDOR faild,errorcode:%d,\n", ret);
		return ;
	}
	boot_add = getenv("bootargs.add");
	if (boot_add != NULL) {
		snprintf(new_args, CONFIG_SYS_BARGSIZE,
			"%s androidboot.boardinfo=%s",
			 boot_add, binfo);
		setenv("bootargs.add", new_args);
	} else {
		snprintf(new_args, CONFIG_SYS_BARGSIZE,
			"androidboot.boardinfo=%s", binfo);
		setenv("bootargs.add", new_args);
	}
#endif
#endif
}
static void boot_env_to_add(void)
{
	char *boot_env, *bootargs_env;
	char new_env[CONFIG_SYS_BARGSIZE];

	boot_env = getenv("bootargs_env");
	if (boot_env == NULL)
		return;

	bootargs_env = getenv("bootargs.add");
	if (bootargs_env != NULL) {
		snprintf(new_env, CONFIG_SYS_BARGSIZE, "%s %s",
			 boot_env, bootargs_env);
		setenv("bootargs.add", new_env);
	} else {
		setenv("bootargs.add", boot_env);
	}
}

int boot_append_bootargs_add(void *fdt)
{
	int node = 0, ret = 0;
	const char *bootargs;
	char *bootargs_add;
	char new_prop[CONFIG_SYS_BARGSIZE];

	boot_env_to_add();
	boot_dvfslevel_to_add();
	boot_boardinfo_to_add();
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

int boot_modify_eth_mac_dts(void *fdt)
{
#ifdef CONFIG_S700
	int node, ret;
	u8 mac[6];
	unsigned int mac_rand;
#if (defined(CONFIG_OWL_NAND) ||  defined(CONFIG_OWL_MMC))
	memset(mac, 0, sizeof(mac));
	ret = read_mi_item("ETHMAC", mac, sizeof(mac));
	if (ret < 0 || ret > 128) {
		srand(get_ticks());
		mac_rand = rand();
		mac[0] = 0x00;
		mac[1] = 0x18;
		mac[2] = 0xfe;
		mac[3] = mac_rand & 0xff;
		mac[4] = (mac_rand >> 8) & 0xff;
		mac[5] = (mac_rand >> 16) & 0xff;
		printf("w: mac_rand = %x\n", mac_rand);
		ret = write_mi_item("ETHMAC", mac, sizeof(mac));
		if (ret < 0)
			printf("write mac fail %d\n", ret);
	}
#else
	return 0;
#endif
	node = fdt_node_offset_by_compatible(fdt, 0, "actions,s700-ethernet");
	if (node < 0) {
		printf("s700-ethernet: no match in DTS\n");
		return -1;
	}

	printf("mac=%x %x %x %x %x %x\n", mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);
	ret = fdt_setprop(fdt, node,  "local-mac-address", mac, 6);
	if (ret < 0) {
		printf("fdt_setprop mac fail, %d\n", ret);
		return -1;
	}
#endif
	return 0;
}
int ft_board_setup(void *blob, bd_t *bd)
{
	boot_modify_eth_mac_dts(blob);
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

int owl_board_first_init(void)
{
#ifdef CONFIG_BOOTDEV_AUTO
	owl_bootdev_init();
#endif
	owl_clk_init();
}