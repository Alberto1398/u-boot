/*
 * Copyright (C) 2015 Actions Semi Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_S700_EVB_H__
#define __CONFIG_S700_EVB_H__

#include "s700_common.h"

#define CONFIG_IDENT_STRING	 "S700 EVB"

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_SETTINGS_COMMON \
					"devif=mmc\0" \
					"bootdisk=1\0"

#define CONFIG_BOOTCOMMAND		"run emmcboot;"
#define CONFIG_BOOTDELAY		0	/* autoboot after 1 seconds */

#define CONFIG_PWM_OWL

#define CONFIG_BOOTDEV_AUTO

/* video support */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_OWL
#define CONFIG_VIDEO_OWL_DE_S700
#define CONFIG_VIDEO_OWL_DE_S700_OTT
/* #define CONFIG_VIDEO_OWL_DSI */
#define CONFIG_VIDEO_OWL_HDMI
#define CONFIG_VIDEO_OWL_CVBS

#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO

/* splash image */
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE		(10 * 1024 * 1024)
#define CONFIG_SYS_VIDEO_LOGO_NAME              "boot_logo.bmp.gz"
#define CONFIG_SYS_VIDEO_LOGO_NAME_QC_SHOW_CHIPID_OK                   "qc_chipid_ok.bmp.gz"
#define CONFIG_SYS_VIDEO_LOGO_NAME_QC_SHOW_CHIPID_ERROR             "qc_chipid_err.bmp.gz"
#define CONFIG_SYS_BATTERY_LOW_NAME             "battery_low.bmp.gz"
#define CONFIG_SYS_CHARGER_LOGO_NAME            "charger_logo.bmp.gz"
#define CONFIG_SYS_RECOVERY_LOGO_NAME           "recovery_logo.bmp.gz"
#define CONFIG_SYS_CHARGER_FRAME_NAME           "charger_frame.bmp.gz"
#define CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OWL_MMC
#define SLOT0			0
#define SLOT2			2

#define CONFIG_OWL_NAND

/* support enter android recovery */
#define CONFIG_ANDROID_RECOVERY
#define ANDROID_RECOVERY_PART                "2"
#define ANDROID_CACHE_PART                   "6"
#define ANDROID_DATA_PART                    "5"

/* DWC3 */
#define CONFIG_USB_SOFTVBUS
#define CONFIG_USB_DWC3_OWL_S700
#define CONFIG_USB_DWC3_PHY_OWL
#define CONFIG_USB_DWC3_OWL
#define CONFIG_USB_DWC3
#define CONFIG_USB_DWC3_GADGET

/* USB gadget */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2

/* Downloader */
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_G_DNL_VENDOR_NUM		0x18d1
#define CONFIG_G_DNL_PRODUCT_NUM	0x0c02
#define CONFIG_G_DNL_MANUFACTURER	"Actions-Semi"

/* ADFU */
#define CONFIG_BOOT_ADFU
#define SUPPORT_UDISK_UPGRADE
#define SUPPORT_SET_SERIAL_NUMBER
#define CONFIG_G_DNL_ADFU_VENDOR_NUM	0x10d6
#define CONFIG_G_DNL_ADFU_PRODUCT_NUM	0x10d6

/* Fastboot */
#define CONFIG_FASTBOOT_FLASH_MMC_DEV 0
#define CONFIG_CMD_FASTBOOT
#define CONFIG_USB_FASTBOOT_BUF_ADDR    0x20000000
#define CONFIG_USB_FASTBOOT_BUF_SIZE    0x40000000

/* UMS */
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_G_DNL_UMS_VENDOR_NUM	0x10D6
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	0x0C02

/* USB_ETHER */
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USBNET_DEV_ADDR		"de:ad:be:ef:00:01"
#define CONFIG_USBNET_HOST_ADDR	"de:ad:be:ef:00:02"
#define CONFIG_LIB_RAND
#define CONFIG_NET_RANDOM_ETHADDR

#define CONFIG_CMD_LINK_LOCAL
#define CONFIG_CMD_PING

/*  XHCI */
#define CONFIG_USB_XHCI_S700
#define CONFIG_USB_XHCI
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2
#define CONFIG_USB_XHCI_OWL

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE

/* support ir key */
//#define CONFIG_ATC260X_IRKEY

#define CONFIG_ATC260X_LED
/*irkey detect uhost ,if the uhost had update.zip , enter recovery update*/
#define IR_KEY_UDISK_DETECT         KEY_BACK
#define IR_KEY_ENTER_RECOVERY     KEY_SELECT
//#define CONFIG_IR_CONTROL_RECOVERY

#define CONFIG_SMC
#define CONFIG_USE_SHOW_CHIPID_BOOT_LOGO

/* check key & power in board later init stage */
#define CONFIG_BOARD_LATE_INIT

/* support adckey */
#define CONFIG_ATC260X_ADCKEY
#define CONFIG_CHECK_KEY

#define CONFIG_DM_PMIC_ATC260X_MFD_MISC

#endif /* __CONFIG_S700_EVB_H__ */
