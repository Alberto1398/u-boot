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

#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_SPL_H_

#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_RAM		1
#define BOOT_DEVICE_NAND	2
#define BOOT_DEVICE_ONE_NAND	3
#define BOOT_DEVICE_MMC1	4
#define BOOT_DEVICE_MMC2	5
#define BOOT_DEVICE_MMC2_2	6
#define BOOT_DEVICE_RAWMMC	7
#define BOOT_DEVICE_RAWFLASH	8
#define BOOT_DEVICE_UPGRADE	9

ddr_param_t *scan_ddr_prepare(int);
int mem_init(ddr_param_t *, int);
int mbrc_mmc_read(int lba, int sectors, char *buf);
void mbrc_mmc_poweroff(void);
void spl_rawmmc_load_image(void);
void spl_raw_nand_load_image(void);
void brom_adfulaucher(void);
#endif
