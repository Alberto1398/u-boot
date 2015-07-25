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

#ifndef ATM7059TC_UPGRADE_SPL_H
#define ATM7059TC_UPGRADE_SPL_H

/* Please DO NOT enable DEBUG when upload code, ONLY for local debug */
#undef DEBUG

#include <asm/mach-types.h>

#define CONFIG_BOOT_SPL

#define CONFIG_MACH_TYPE 6666
#define CONFIG_OWL		/* OWL family */
#define CONFIG_ATM7059TC
#define CONFIG_ATC2603A

#include <asm/arch/cpu.h>

#define CONFIG_SYS_TEXT_BASE		0x02000040
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SYS_TEXT_BASE - 0x40)

#define CONFIG_SYS_MONITOR_LEN		(640 * 1024) /* 640K, Uboot.bin size */
#define CONFIG_USE_IRQ

#define CONFIG_STACKSIZE_IRQ	(4 * 1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(1 * 1024)	/* FIQ stack */

#define CONFIG_PANIC_HANG

#define CONFIG_BOARD_EARLY_INIT_F	1
#define CONFIG_BOARD_LATE_INIT 1
#define CONFIG_MISC_INIT_R		1

#define CONFIG_LZO

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_DEFAULT_DEVICE_TREE	owl-lt705a
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE
#define CONFIG_OF_LIBFDT		1
#define CONFIG_FIT                      1
#define CONFIG_CMD_IMI                  1
#define CONFIG_SYS_BOOTM_LEN            (64 << 20) /* Increase max gzip size */

#define OWLXX_BOOTDEV_NAND (0x00)
#define OWLXX_BOOTDEV_SD0 (0x20)
#define OWLXX_BOOTDEV_SD1 (0x21)
#define OWLXX_BOOTDEV_SD2 (0x22)
#define OWLXX_BOOTDEV_SD02NAND (0x30) 
/*
 * Physical Memory Map
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define PHYS_SDRAM_1 CONFIG_SYS_SDRAM_BASE

/* CS1 may or may not be populated */
#define CONFIG_NR_DRAM_BANKS	1

#define CONFIG_SYS_MEMTEST_START	0x00400000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00C00000	/* 4-12 MB in DRAM */

#define CONFIG_FAT_WRITE
/*
 * Command line configuration.
 */
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ELF
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_RUN
#define CONFIG_CMD_MALLOC
#define CONFIG_CMD_FAT		/* FAT support */
#define CONFIG_CMD_DMA		/* DAM support */
#define CONFIG_CMD_ADFUDEC
#define CONFIG_CMD_SAVEENV

#define CONFIG_OWLXX_UPGRADE		1
#define CONFIG_OWLXX_SERIAL		1
#define CONFIG_OWLXX_SERIAL_INDEX	0 /* active UART console controller */

#define CONFIG_SYS_NO_FLASH		1
/* ENV related config options */
#define CONFIG_ENV_IS_IN_FAT		1
#define FAT_ENV_FILE			"boot.env"
/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			(0x4 << 10)
 /* environment starts here  */
#define CONFIG_ENV_OFFSET		0x20000

#define CONFIG_SYS_PROMPT		"lt703a # "

#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)

#define CONFIG_SYS_MAXARGS		16

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_I2C_SPEED		400000 /* I2C Speed */

#define CONFIG_OWLXX_SPI

#define	CONFIG_KERNEL_FDTADDR	0x04000000
/* boot command related */
#define CONFIG_BOOTDELAY		0

#define CONFIG_NFSBOOTCOMMAND				\
	"env set ipaddr $ipaddr ; "			\
	"env set serverip $serverip ; "			\
	"tftpboot 0x8000 $bootfile ; "			\
	"go 0x8000"

/**TO BE SUPPORTED ENV SETTING************/
/*
	"splashimage=4000000\0"                   \
*/

#define CONFIG_COMMON_ENV_SETTINGS			\
	"stdin=serial\0"				\
	"stdout=serial\0"				\
	"stderr=serial\0"				\
	"splashpos=m,m\0"				\
	"verify=yes\0"					\
	"loadaddr=0x7fc0\0"				\
	"ramdiskaddr=0x1ffffc0\0"			\
	"ft2test_addr=0x00300000\0"                     \
	"fdt_high=0xffffffff\0"				\
	"initrd_high=0xffffffff\0"			\
	"mmcpart=1\0"					\
	"emmcpart=1\0"					\
	"nandpart=1\0"					\
	"fdtaddr2=0x04000000\0"                     \
	"loaduimage=fatload ${devtype} ${devpart} ${loadaddr} uImage\0"	\
	"loadramdisk=fatload ${devtype} ${devpart} ${ramdiskaddr} boot.img\0" \
	"loadfdt2=fatload ${devtype} ${devpart} ${fdtaddr2} u-boot.dtb\0" \
	"loadft2test=fatload ${devtype} ${devpart} ${ft2test_addr} ft2_test.bin\0"	\
	"loadtestdata=fatload mmc ${mmcpart} 0x0 img_dat.dat\0"	\
	"mmcargs=setenv devtype mmc; setenv devpart ${mmcpart}\0"	\
	"emmcargs=setenv devtype mmc; setenv devpart ${emmcpart}\0"	\
	"nandargs=setenv devtype nand; setenv devpart ${nandpart}\0"	\
	"mboot=run loaduimage; run loadramdisk; run loadfdt2;"	\
		"bootm ${loadaddr} ${ramdiskaddr} ${fdtaddr2}\0"	\
	"mmcboot=echo boot from mmc card ...; "		\
		"run mmcargs; run mboot\0"				\
	"emmcboot=echo boot from emmc card ...; "		\
		"run emmcargs; run mboot\0"				\
	"nandboot=echo boot from nand card ...; "	\
		"run nandargs; run mboot\0"		\
	"bootmenu_1=1) Boot from NAND=setenv stdout serial; run nandboot\0"	\
	"bootmenu_2=2) Boot from MMC/SD=run mmcboot\0"	\
	"bootmenu_3=3) Enter ADFU Mode=setenv stdout serial; romadfu\0"	\
	"bootmenu_delay=30\0" 

#define CONFIG_MMC_BOOTCOMMAND				\
	"run mmcboot; "

#define CONFIG_EMMC_BOOTCOMMAND				\
	"run emmcboot; "

#define CONFIG_NAND_BOOTCOMMAND				\
	"run nandboot; "

#define CONFIG_FT2TEST_BOOTCOMMAND				\
	"run ft2boot; "

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER	/* use "hush" command parser */
#define CONFIG_SYS_CBSIZE		512
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_LOAD_BASE		0xB4063000
#define CONFIG_SPL_AFINFO_SIZE		0x900		/* 2304B, afinfo */
#define CONFIG_SPL_TEXT_BASE		0xB4063900	/* after afinfo */
#define CONFIG_SPL_MAX_SIZE		    0x9000

/* Defines for SP */
#define	CONFIG_SYS_INIT_SP_ADDR		0xB4070800
#define CONFIG_SPL_STACK		    CONFIG_SYS_INIT_SP_ADDR	/* 9.5KB, +8KB */
#define CONFIG_SPL_IRQ_STACK		(CONFIG_SYS_INIT_SP_ADDR - 0x2600)	/* 8KB */


#define CONFIG_SPL_LDSCRIPT "$(CPUDIR)/owl/u-boot-spl.lds"

#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_USE_IRQ
#if defined(CONFIG_SPL_USE_IRQ) && !defined(CONFIG_USE_IRQ)
#define CONFIG_USE_IRQ
#endif

#define CONFIG_KEYBOARD
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

/* SPL - temp for compile */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1 /* Boot partition */
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.bin"

/* I2C */
#define CONFIG_OWLXX_I2C
#define CONFIG_CMD_OWLXX_I2C

/*power*/
#define CONFIG_SPL_OWLXX_POWER
#define CONFIG_OWLXX_POWER
#define CONFIG_BOOT_POWER
#define CONFIG_GAUGE_EG2801
#define CONFIG_GAUGE_BQ27441
#define CONFIG_ATC2603A_POWER
#undef  CONFIG_ATC2603B_POWER

#define CONFIG_I2C_MULTI_BUS		1

#define RECOVERY_MMC_DEV "2"
#define RECOVERY_NAND_DEV "2"
#define RECOVERY_EMMC_DEV "2"

#define FAT_ENV_DEVICE			1
#define FAT_ENV_PART			0

#define FAT_MISC_DEV		1
#define FAT_RECOVERY_DEV	2


/*gamma support*/
#define CONFIG_SYS_GAMMA_NAME		"new_gamma_data"
#define CONFIG_SYS_GAMMA_SIZE			(256 * 3)


#endif

