/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ATM7039C_NANDBOOT_H
#define __ATM7039C_NANDBOOT_H

#include <configs/atm7039c_boot_spl.h>

#define CONFIG_SPL_RAWNAND_SUPPORT
#define CONFIG_OWLXX_NAND
#define CONFIG_BOOTCOMMAND		CONFIG_NAND_BOOTCOMMAND
#define CONFIG_SYS_VSNPRINTF
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"devif=nand\0"							\
	CONFIG_COMMON_ENV_SETTINGS				\
	"bootmenu_0=0) OTA Upgrade=setenv stdout serial; setenv nandpart " 		\
		RECOVERY_NAND_DEV "; recovery_show; "	CONFIG_BOOTCOMMAND "\0"


#define CONFIG_LOOPW
#define CONFIG_CMD_MEMTEST
#define CONFIG_MX_CYCLIC
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_KARGS

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MMC		/* MMC support */
#undef CONFIG_CMD_NET

#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#define CONFIG_CMD_TFTPPUT
#define CONFIG_CMD_RECOVERY
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_RECOVERY

#ifndef CONFIG_IPADDR
#define CONFIG_IPADDR			192.168.90.246
#endif

#ifndef	CONFIG_SERVERIP
#define CONFIG_SERVERIP			192.168.90.103
#endif

#ifndef CONFIG_BOOTFILE
#define CONFIG_BOOTFILE		"Image"
#endif

/* Net conifgs */
#undef CONFIG_ACTE100              /* Actions ethernet 10/100M */
#define CONFIG_ACTE100_IOBASE	0xB0310000
#define CONFIG_SYS_RX_ETH_BUFFER 16
#undef CONFIG_SMII_INTERFACE
#undef CONFIG_RMII_INTERFACE
#define CONFIG_ACTE100_RX_BUF CONFIG_SYS_RX_ETH_BUFFER
#define CONFIG_ACTE100_TX_BUF 4

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_OWLXX_MMC		1
#define CONFIG_DOS_PARTITION		1

/* video support */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_OWL
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE		(10 * 1024 * 1024)
#define CONFIG_SYS_VIDEO_LOGO_NAME		"boot_logo.bmp.gz"
#define CONFIG_SYS_BATTERY_LOW_NAME		"battery_low.bmp.gz"
#define CONFIG_SYS_CHARGER_LOGO_NAME		"charger_logo.bmp.gz"
#define CONFIG_SYS_RECOVERY_LOGO_NAME		"recovery_logo.bmp.gz"
#define CONFIG_SYS_CHARGER_FRAME_NAME		"charger_frame.bmp.gz"
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_CONSOLE_MUX

/* low level display device */
#define CONFIG_OWLXX_DISPLAY_LCD

/* low level dsi display device */
#define CONFIG_OWLXX_DISPLAY_DSI

/* ncs8801 lvds to edp IC*/
#define CONFIG_NCS8801

/*pwm support*/
#define CONFIG_OWLXX_PWM

/*pwm backlight support*/
#define CONFIG_OWLXX_PWM_BACKLIGHT

/*pinctrl support */
#define CONFIG_OWLXX_PINCTRL

/*gpio support*/
#define CONFIG_OWLXX_GPIO_GENERIC
#define CONFIG_OWLXX_GPIO

#define CONFIG_ANDROID_RECOVERY		1
#define CONFIG_RECOVERYFILE_SIZE	 1024
#define CONFIG_ANDROID_RECOVERY_CMD_FILE  "recovery/command"

/*recovery support*/
#define EXT4_CACHE_DEVICE         6
#define EXT4_CACHE_PART            0


#undef CONFIG_OWLXX_QUICKBOOT

#define CONFIG_MENU
#define CONFIG_MENU_SHOW
#define CONFIG_MENU_BOOTDELAY		120
#define CONFIG_CMD_BOOTMENU
#define CONFIG_CFB_CONSOLE_ANSI

#define CONFIG_CMD_CHARGELOGO

/*======usb dwc3 gadge support=====*/
#define	CONFIG_USB_GADGET
#define	CONFIG_USB_GADGET_ACTIONS
#define	CONFIG_USBDOWNLOAD_GADGET
#define	CONFIG_DFU_FUNCTION
#define	CONFIG_CMD_FASTBOOT
#define	CONFIG_USB_FASTBOOT_BUF_SIZE (1024*1024*700)
/*#define	CONFIG_USB_FASTBOOT_BUF_ADDR CONFIG_SYS_SDRAM_BASE*/
#define	CONFIG_USB_FASTBOOT_BUF_ADDR 0x100

#define	CONFIG_CMD_USB_MASS_STORAGE



#define CONFIG_USB_GADGET_VBUS_DRAW 400
#define CONFIG_SYS_CACHELINE_SIZE	64
#define CONFIG_USBD_HS
#define CONFIG_G_DNL_VENDOR_NUM 0x18d1
#define CONFIG_G_DNL_PRODUCT_NUM 0x0c02
#define CONFIG_G_DNL_MANUFACTURER "Actions-Semi"
#define CONFIG_USB_GADGET_DUALSPEED


#endif
