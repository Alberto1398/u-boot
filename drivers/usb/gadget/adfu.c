/*
 * adfu.c -- Actions Device Firmware Update USB function
 *
 * Copyright 2016 Actions Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <errno.h>
#include <common.h>
#include <g_dnl.h>
#include <usb.h>
#include <fdtdec.h>
#include <adfu.h>
#include <asm/io.h>
#include <usb_mass_storage.h>
#include <power/atc260x/owl_atc260x.h>

#ifdef CONFIG_USB_DWC3_OWL_S700
#define USB3_ECS	(0xe024c090)
#define USB3_LINESTATE	(0x03<<3)
#define USB3_LINESTATE_SHIFT	3
#define USB3_VBUS_P0	(1<<5)
#define USB3_BVALID_P0	(1<<10)
#define USB3_VBUS USB3_VBUS_P0
#define USB3_DPPUEN	(1<<14)
#define USB3_DMPUEN	(1<<15)
#define USB3_DPPDDIS	(1<<12)
#define USB3_DMPDDIS	(1<<13)
#endif

#ifdef CONFIG_USB_DWC3_OWL_S900
#define USB3_ECS	(0xe0228090)
#define USB3_LINESTATE	(0x03<<8)
#define USB3_LINESTATE_SHIFT	8
#define USB3_VBUS_VALID	(1<<10)
#define USB3_IDPIN_STATE	(1<<12)
#define USB3_VBUS USB3_VBUS_VALID
#define USB3_DPPUEN	(1<<2)
#define USB3_DMPUEN	(1<<3)
#define USB3_DPPDDIS	(1<<0)
#define USB3_DMPDDIS	(1<<1)
#endif

/* PMU usb vbus dectect voltage threshold */
#define ATC260X_VBUS_VOL_THRESHOLD	(3900)

DECLARE_GLOBAL_DATA_PTR;
int connect_to_pc = 0;
int adfu_restart_complete = 0;

static void usb_set_dpdm_500k_15k(int pu_500k_enable, int pd_15k_disable);
static int usb_get_linestates(void);
static int usb3_dpdm_state(void);
static int usb_host_mode_detect(void);
static int usb3_vbus_state(int vbus_type);
static bool usb_is_host_mode(void);
static int usb_device_type_detect(void);

/**
 * Detect usb3 vbus state by IC or PMU.
 * IC detect: directly read from register (normally this way).
 * PMU detect: need to use PMU interface to get vbus state.
 * PMU is independent of IC, different ICs maybe use same PMU.
 */
static int usb3_vbus_state(int vbus_type)
{
	int state;
	u32 reg;
	int ret;
	int i;
	int vbusv = 0;

	if (vbus_type == VBUS_IC) {
		reg = readl(USB3_ECS);
		reg &= (USB3_VBUS);
		state = (reg == 0x0) ? VBUS_LOW : VBUS_HIGH;
	} else if (vbus_type == VBUS_PMU) {
		/* copy from charger driver */
		for (i = 0; i < 5; i++) {
			ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_VBUSV, &vbusv);
			if (ret) {
				/* native driver quit if err, we give it 5 times */
				printf("adfu get vbusv auxadc err!\n");
			}
			printf("vbusv: %d\n", vbusv);
			if (vbusv > ATC260X_VBUS_VOL_THRESHOLD) {
				printf("USB ONLINE\n");
				break;
			}
			mdelay(2);
		}

		state = (vbusv < ATC260X_VBUS_VOL_THRESHOLD) ? VBUS_LOW : VBUS_HIGH;
	} else {
		state = 0;
	}

	return state;
}

static void usb_set_dpdm_500k_15k(int pu_500k_enable, int pd_15k_disable)
{
	u32 reg;

	reg = readl(USB3_ECS) & (~(USB3_DPPUEN | USB3_DMPUEN |
		USB3_DPPDDIS | USB3_DMPDDIS));

	if (pu_500k_enable != 0)
		reg |= (USB3_DPPUEN | USB3_DMPUEN);
	if (pd_15k_disable != 0)
		reg |= (USB3_DPPDDIS | USB3_DMPDDIS);
	writel(reg, USB3_ECS);
	mdelay(1);
}

static int usb_get_linestates(void)
{
	u32 val;

	val = (readl(USB3_ECS) & USB3_LINESTATE) >> USB3_LINESTATE_SHIFT;

	return val;
}

/**
 * only 5209 can usb this way to detect pc/adapter for now.
 * 5206/5211 can not: IC bug!
 */
static int usb3_dpdm_state(void)
{
	int state;

	usb_set_dpdm_500k_15k(1, 1);
	state = usb_get_linestates();
	usb_set_dpdm_500k_15k(0, 0);

	return state;
}

static int usb_host_mode_detect(void)
{
	u32 reg;
	int ret;
	int delay_times = 0;

	/*
	 * For now S700 USB3 PORT VBUS is connected to WALL directly.
	 * Only ebox serials are designed this way, others not.
	 * Some device will go wrong if USB is in device mode.
	 * At boot time, uboot adfu will work immediately and USB is
	 * in device mode.
	 * So we try to detect USB device first, if there is device,
	 * boot continue, or do adfu.
	 */
	if (board_usb_init(0, USB_INIT_HOST)) {
		error("Couldn't init USB controller in host mode\n");
		return -1;
	}

	while (1) {
		reg = readl((void __iomem *)USB3_ECS);
		if (reg & USB3_LINESTATE) {
			printf("usb3_ecs: 0x%x, delay_times: %d\n", reg,
					delay_times);
			ret = CONNECT_UDISK;
			break;
		}
		if (delay_times >= 5) {
			printf("no usb device attached!\n");
			ret = CONNECT_NO_DEVICE;
			break;
		}
		mdelay(10);
		delay_times++;
	}

	board_usb_cleanup(0, USB_INIT_HOST);
	return ret;
}

static bool usb_is_host_mode(void)
{
	int ret;

	ret = usb_host_mode_detect();
	if (ret == CONNECT_UDISK)
		return true;
	else
		return false;
}

static int usb_device_type_detect(void)
{
	int node;
	unsigned int detect_times;
	unsigned int detect_interval;
	unsigned int count;
	unsigned int vbus_type;
	int ret = 0;

	/**
	 * use the new dts node name because it is irrelavant to ic type.
	 */
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
			"actions,usb-adfu");

	if (node < 0) {
		printf("cannot find actions adfu config\n");
		return -1;
	}

	detect_times = fdtdec_get_int(gd->fdt_blob, node,
			"detect_times", 100);
	detect_interval = fdtdec_get_int(gd->fdt_blob, node,
			"detect_interval", 5);

	vbus_type = fdtdec_get_int(gd->fdt_blob, node,
				"vbus_type", 0);

	printf("detec times: %d, interval: %d, vbus_type: %d\n",
		detect_times, detect_interval, vbus_type);

	/**
	 * avoid using IC vbus detection, use PMU detection instead,
	 * reasons are as follows.
	 * 1. IC vbus may not connect to port vbus.
	 * 2. IC vbus depends on phy(need to enable phy ldo & clk first).
	 */
	if (vbus_type != 0) {
		ret = usb3_vbus_state(vbus_type);
		if (ret == VBUS_LOW) {
			printf("vbus is low!\n");
			return -1;
		}
	}

	if (board_usb_init(0, USB_INIT_DEVICE)) {
		error("Couldn't init USB controller.");
		return -1;
	}

#ifdef CONFIG_USB_DWC3_OWL_S900
	ret = usb3_dpdm_state();
	if (ret != 0) {
		printf("dpdm: connect to usb adapter\n");
		return CONNECT_TO_ADAPTER;
	}
#endif

	ret = g_dnl_register("usb_dnl_adfu");
	if (ret) {
		error("g_dnl_register failed");
		board_usb_cleanup(0, USB_INIT_DEVICE);
		return -1;
	}

	for (count = 0; count < detect_times; count++) {
		usb_gadget_handle_interrupts(0);

		if (!connect_to_pc)
			mdelay(detect_interval);
		else
			break;
	}

	if (connect_to_pc)
		ret = CONNECT_TO_PC;
	else
		ret = CONNECT_TO_ADAPTER;

	return ret;
}

#ifdef CONFIG_USB_CONNECT_DETECT
int owl_usb_get_connect_type(void)
{
	int ret;

	ret = usb_device_type_detect();
	if (ret < 0) {
		printf("usb disconnect!\n");
		return ret;
	} else {
		printf("connect type: %d\n", ret);
	}

	g_dnl_unregister();
	board_usb_cleanup(0, USB_INIT_DEVICE);
	return ret;
}
#else
int owl_usb_get_connect_type(void)
{
	return 0;
}
#endif

int do_adfu(void)
{
	int ret;

#ifdef CONFIG_USB_DWC3_OWL_S700
	if (usb_is_host_mode())
		return -1;
#endif

	ret = usb_device_type_detect();
	if (ret < 0) {
		printf("someting goes wrong!\n");
		return CMD_RET_FAILURE;
	} else if (ret == CONNECT_TO_ADAPTER) {
		printf("connect to usb adapter!\n");
		goto exit;
	} else
		printf("connectint to pc...\n");

	while (1) {
		usb_gadget_handle_interrupts(0);

		ret = fsg_main_thread(NULL);
		if (ret) {
			/* Check I/O error */
			if (ret == -EIO)
				printf("\rCheck USB cable connection\n");

			/* Check CTRL+C */
			if (ret == -EPIPE)
				printf("\rCTRL+C - Operation aborted\n");

			goto exit;
		}
	}
exit:
	g_dnl_unregister();
	board_usb_cleanup(0, USB_INIT_DEVICE);
	return 0;

}
