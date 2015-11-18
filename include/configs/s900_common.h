/*
 * Copyright (C) 2015 Actions Semi Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_S900_COMMON_H__
#define __CONFIG_S900_COMMON_H__

/* We use generic board and device manager */
#define CONFIG_SYS_GENERIC_BOARD

/* SDRAM Definitions */
#define CONFIG_SYS_SDRAM_BASE		0x0
#define CONFIG_NR_DRAM_BANKS		1

/* Link Definitions */
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7ff00)

/* Some commands use this as the default load address */
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7ffc0)

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0xe00f1000)
#define GICC_BASE			(0xe00f2000)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(24000000)	/* 24MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * 1024 * 1024 + CONFIG_ENV_SIZE)

#define CONFIG_EXTRA_ENV_SETTINGS_COMMON				\
	"bootpart=1\0"							\
	"kernel_addr_r=0x80000\0"					\
	"ramdisk_addr_r=0x1ffffc0\0"					\
	"fdt_addr_r=0x10000000\0"					\
	"fdt_high=0xffffffffffffffff\0"					\
	"initrd_high=0xffffffffffffffff\0"				\
	"scriptaddr=0x10800000\0" \
	"bootenv=uEnv.txt\0" \
	"bootscr=boot.scr\0" \
	"splashimage=0x18000000\0" \
	"splashpos=m,m\0" \
	"loadkernel=fatload ${devtype} ${devpart} ${kernel_addr_r} Image\0"	\
	"loadramdisk=fatload ${devtype} ${devpart} ${ramdisk_addr_r} ramdisk.img\0" \
	"loadfdt=fatload ${devtype} ${devpart} ${fdt_addr_r} kernel.dtb\0" \
	"loadbootscr=fatload ${devtype} ${devpart} ${scriptaddr} ${bootscr} \0" \
	"loadbootenv=fatload ${devtype} ${devpart} ${scriptaddr} ${bootenv} \0" \
	"mmcargs=setenv devtype mmc; setenv devpart ${bootdisk}:${bootpart}\0" \
	"emmcargs=setenv devtype mmc; setenv devpart ${bootdisk}:${bootpart}\0"	\
	"nandargs=setenv devtype nand; setenv devpart ${bootdisk}:${bootpart}\0" \
	"mboot=run loadkernel; run loadramdisk; run loadfdt;" \
		"booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0"	\
	"setbootenv=" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"env import -t ${scriptaddr} ${filesize};" \
		"fi;" \
		"if test -n \\\"${uenvcmd}\\\"; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loadbootscr; then " \
			"echo Jumping to ${bootscr};" \
			"source ${scriptaddr};" \
		"fi;" \
		"run mboot;\0" \
	"mmcboot=echo boot from mmc card ...; "		\
		"run mmcargs; run setbootenv\0"				\
	"emmcboot=echo boot from emmc card ...; "		\
		"run emmcargs; run setbootenv\0"				\
	"nandboot=echo boot from nand card ...; "	\
		"run nandargs; run setbootenv\0"		\
	"ramboot=booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0"  \
	"owlboot=fastboot 0\0"                      

/* Cache Definitions */
#undef CONFIG_SYS_DCACHE_OFF
#undef CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_CACHELINE_SIZE	64

/* UART Definitions */
#define CONFIG_OWL_SERIAL
#define CONFIG_BAUDRATE			115200

/* I2C Definitions */
#define CONFIG_SYS_I2C_OWL

/* GPIO Definitions */
#define CONFIG_OWL_GPIO

/* DMA Definitions */
#define CONFIG_OWL_DMA

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

#define CONFIG_OF_BOARD_SETUP

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

/* support loading RAW ramdisk via bootz command */
#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_BOOTZ

/*
 * Common filesystems support.  When we have removable storage we
 * enabled a number of useful commands and support.
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUID
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_EXT4

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PROMPT		"owl> "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS		16	/* max command args */


/*support booting*/
#define CONFIG_ANDROID_BOOT_IMAGE
#endif /* __CONFIG_S900_COMMON_H__ */
