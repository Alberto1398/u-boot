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

#ifndef __CONFIG_ATM7021_COMMON_H__
#define __CONFIG_ATM7021_COMMON_H__

#include <asm/mach-types.h>
#ifdef MACH_TYPE_GL520X_LION
#error "MACH_TYPE_GL520X_LION has been accepted by upstream, please remove this."
#else
#define MACH_TYPE_GL520X_LION 6666
#endif

/* machine define */
#define CONFIG_MACH_TYPE                MACH_TYPE_GL520X_LION
#define CONFIG_ACTS                     /* in a ACTIONS core */
#define CONFIG_ACTS_GL520X
#define CONFIG_ACTS_GL5207            /* defined in board config file */



/* link address & size */
#define CONFIG_SYS_TEXT_BASE		    0x02000000
#define CONFIG_SYS_MONITOR_LEN		    (640 * 1024) /* 640K, Uboot.bin size */
#define CONFIG_SYS_INIT_SP_ADDR         0xb406f800
#define CONFIG_SYS_SDRAM_BASE           0x00000000

/* low-level */
#define CONFIG_SYS_HZ                   1000        /* MUST BE 1000 */
//#define CONFIG_SYS_ICACHE_OFF
//#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_L2_PL310                         /* PL310 is the former name of L2C-310 */
#define CONFIG_SYS_PL310_BASE           0xb0022000
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_SYS_CACHELINE_SIZE       32          /* L1 cache : 32bytes, L2 cache 32bytes */
#define CONFIG_NR_DRAM_BANKS	        1
#define CONFIG_STACKSIZE_IRQ	        (256 * 1024)/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	        (1 * 1024)	/* FIQ stack */

/* code interface */
//#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT
//#define CONFIG_BOARD_POSTCLK_INIT
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_USE_IRQ
#define CONFIG_DISPLAY_CPUINFO
#ifdef CONFIG_SPL_BUILD
#define CONFIG_ARM_NO_VECTORS
#else
#define CONFIG_USE_ARCH_MEMCPY          /* to save space in SPL */
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_SKIP_LOWLEVEL_INIT       /* SPL have done that, no need to do it again. */
#endif
//#define CONFIG_ACTS_FOR_BOOT          /* defined in board config file */
//#define CONFIG_ACTS_FOR_UPGRADE       /* defined in board config file */

/* core */
#define CONFIG_SYS_MALLOC_LEN           (64 * 1024 * 1024)
#define CONFIG_SHA1
/* bootm */
#define CONFIG_SYS_BOOTM_LEN            (16U << 20)     /* 16MiB */
#define CONFIG_SYS_LOAD_ADDR            0x00100000

/* console */
#define CONFIG_ACTS_GL520X_SERIAL
#define CONFIG_BAUDRATE                 115200
//#define CONFIG_PRE_CONSOLE_BUFFER
//#define CONFIG_PRE_CON_BUF_SZ         /* power of 2 */
//#define CONFIG_PRE_CON_BUF_ADDR
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_CBSIZE				0
#define CONFIG_SYS_PBSIZE               0 /* we use our tiny_print, so no need */
#define CONFIG_ALTERNATE_VSPRINTF
#define CONFIG_ALTERNATE_CONSOLE
#else
#define CONFIG_SYS_VSNPRINTF
#define CONFIG_SYS_CBSIZE				512
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + 16)
#endif

/* shell */
//#define CONFIG_SYS_HUSH_PARSER
//#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING          /* for command-line history */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS              32

/* debug */
#define CONFIG_PANIC_HANG

/* auto-boot */
#define CONFIG_BOOTDELAY                1     /* in seconds */
//#define CONFIG_BOOTCOMMAND              ""    /* defined in board config file */
//#define CONFIG_BOOTARGS                 ""
//#define CONFIG_BOOT_RETRY_TIME          300
//#define CONFIG_AUTOBOOT_KEYED
//#define CONFIG_AUTOBOOT_PROMPT          "about to autoboot...\n"
//#define CONFIG_AUTOBOOT_DELAY_STR       "console\n"
//#define CONFIG_AUTOBOOT_STOP_STR        "debug\n"
//#define CONFIG_PREBOOT                  ""

/* frame buffer & LOGO */
//#define CONFIG_VIDEO
//#define CONFIG_ACTS_VIDEO_GL520X
//#define CONFIG_SPLASH_SCREEN
//#define CONFIG_SPLASHIMAGE_GUARD
//#define CONFIG_SPLASH_SCREEN_ALIGN
//#define CONFIG_VIDEO_BMP_GZIP
//#define CONFIG_VIDEO_BMP_RLE8
#include <configs/atm702x_display_cfg.h> /* for CONFIG_ACTS_DISP_LCD_TYPE_LVDS ... */

/* SPI */
#define CONFIG_ACTS_GL520X_SPI

/* ADFU driver */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_ACTS_ADFU_SUPPORT
#define CONFIG_ACTS_ADFUS_BOOT_KRNL
#else
#define CONFIG_ACTS_SPL_ADFU_SUPPORT
#endif


/* network */


/* Environment variables */
#define CONFIG_ENV_IS_NOWHERE
//#define CONFIG_ENV_IS_IN_NVRAM
//#define CONFIG_ENV_ADDR                 (0xffff0000)
#define CONFIG_ENV_SIZE                 (16*1024)
#define CONFIG_ACT_COMMON_ENV_SETTINGS       \
        ""
//#define CONFIG_EXTRA_ENV_SETTINGS     ""  /* defined in board config files */

/* storage */
#define CONFIG_SYS_NO_FLASH             /* we DO NOT use the MTF FLASH layer, don't touch that! */
//#define CONFIG_ACTS_STORAGE_NAND      /* defined in board config files */
//#define CONFIG_ACTS_STORAGE_EMMC      /* defined in board config files */
//#define CONFIG_MMC                    /* defined in board config files */
//#define CONFIG_GENERIC_MMC            /* defined in board config files */
//#define CONFIG_ACTS_GL520X_MMC        /* defined in board config files */

/* filesystem */
#define CONFIG_DOS_PARTITION
#define CONFIG_ACTS_AFI_PARTITION
#define CONFIG_FS_FAT
#define CONFIG_FS_EXT4
//#define CONFIG_EXT4_WRITE

/* device tree */
#undef CONFIG_OF_LIBFDT              /* not FDT support */
//#define CONFIG_OF_SEPARATE

/* kernel ATAG */
#define CONFIG_ACTS_ATAG
#define CONFIG_INITRD_TAG

/* SPL */
#define CONFIG_SPL
//#define CONFIG_SPL_LDSCRIPT             "$(BOARDDIR)/u-boot-spl.lds"
#define CONFIG_SPL_MAX_SIZE             (24*1024)
#define CONFIG_SPL_MAX_FOOTPRINT        (26*1024)
#define CONFIG_SPL_TEXT_BASE            0xb4068800
#define CONFIG_SPL_STACK                0xb406f800  /* stack space: 0xb406f000~0xb406ffff */
#define CONFIG_ACTS_SPL_IRQ_STACK       0xb406ffe0  /* SPL adfuserver use IRQ */
#define CONFIG_ACTS_SPL_MISC_STACK      0xb406fff8
#define CONFIG_SPL_INIT_MINIMAL
#define CONFIG_SPL_SKIP_RELOCATE
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
//#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBGENERIC_SUPPORT
//#define CONFIG_SPL_BOARD_INIT


/* Monitor Functions */
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_ENV_CALLBACK
#define CONFIG_CMD_ENV_FLAGS
#define CONFIG_CMD_ENV_EXISTS
//#define CONFIG_CMD_EXPORTENV
//#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMORY
//#define CONFIG_CMD_MEMTEST
#define CONFIG_CMD_MISC
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART
#define CONFIG_CMD_READ
//#define CONFIG_CMD_REGINFO
//#define CONFIG_CMD_RUN
//#define CONFIG_CMD_EXT2
//#define CONFIG_CMD_EXT4
//#define CONFIG_CMD_GPIO
//#define CONFIG_ACTS_CMD_ADFUSERVER        /* defined in board config file */
//#define CONFIG_ACTS_CMD_ACTBOOT           /* defined in board config file */
#if defined(CONFIG_MMC)
#define CONFIG_CMD_MMC
#endif

#endif /* __CONFIG_ATM7021_COMMON_H__ */
