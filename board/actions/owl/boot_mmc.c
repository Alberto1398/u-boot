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

#include <common.h>
#include <asm/io.h>
#include <spl.h>
#include <flash/osboot.h>
#include <asm/arch/clocks.h>
#include <asm/arch/gmp.h>
#include <flash/address.h>

typedef void (*mbrc_mmc_power_func_t)(void);
typedef int (*mbrc_mmc_read_func_t)(int lba, int sectors, char *buf);

int mbrc_mmc_read(int lba, int sectors, char *buf)
{
	int ret;
	mbrc_mmc_read_func_t mbrc_mmc_read_func =
		(mbrc_mmc_read_func_t) SRAM_DRV_ADDR;

	asm volatile("push {r4-r9}\n");
	asm volatile("mov r9, %0\n" : : "r"(Brom_SDMMCRead));
	ret = mbrc_mmc_read_func(lba, sectors, buf);
	asm volatile("pop {r4-r9}\n");
	return ret;
}

void mbrc_mmc_power(void)
{
	mbrc_mmc_power_func_t brom_mmc_poweroff =
		(mbrc_mmc_power_func_t) SRAM_DRV_ADDR;
	asm volatile("push {r4-r9}\n");
	asm volatile("mov r9, %0\n" : : "r"(Brom_SDMMCPowerOff));
	brom_mmc_poweroff();
	asm volatile("pop {r4-r9}\n");
}

unsigned int checksum_le32(char *buf, unsigned int size)
{
	unsigned int value = 0, *p = (unsigned int *) buf;

	size /= 4;
	while (size--)
		value += *p++;

	return value;
}

static unsigned short wchecksum(char *buf, int start, int length)
{
	unsigned int sum = 0;
	unsigned short *tmp_buf = (unsigned short *)buf;
	int tmp_start = start / sizeof(unsigned short);
	int tmp_length = length / sizeof(unsigned short);
	int i;

	for(i = tmp_start + tmp_length -1; i >= tmp_start; i--)
		sum += *(tmp_buf + i);

	sum = sum & 0xffff;

	return sum;
}



int uboot_mmc_read(int start, char *buf)
{
	u32 i, sectors, quot, remd, ret = 0;
	u16 check, val;
	char tmp[16];

	u32 uboot_len;

	u32 *mbrc = (u32 *)MBRC_START_IN_RAM;
	uboot_len = readl(&mbrc[11]);

	printf("uboot_len: %u, start=%u, b=0x%x\n", uboot_len, start, SRAM_DRV_ADDR);

	const struct image_header *header =
		(struct image_header *) CONFIG_SYS_UBOOT_BASE;

#define MMC_SECTOR_SIZE (0x1 << 7)

	ret = mbrc_mmc_read(start, MMC_SECTOR_SIZE, buf);
	if (ret)
		return 0xF1;

	/* check the image header, return if fail */
	if(image_get_magic(header) != IH_MAGIC)
		return 1;

	spl_parse_image_header(header);

#define MMC_ONE_SECTOR_PAD ((0x1 << 9) - 1)
#define OWLXX_CHECKSUM_PAD (12)


	sectors = uboot_len >> 9;

	quot = sectors >> 7;
	remd = sectors % MMC_SECTOR_SIZE;
	debug("spl_image.size(%d) sectors(%d), quot(%d), remd(%d)\n",
		spl_image.size, sectors, quot, remd);

	if (!remd) {
		remd  = MMC_SECTOR_SIZE;
		quot -= 1;
	}

	ret = mbrc_mmc_read(start + (quot << 7), remd,
		buf + (quot << 16));

	debug("mmc read rest: ret(%d) quot(%d) remd(%d) start(0x%x), buf(%p)\n",
		ret, quot, remd, start + (quot << 7),
		buf + (quot << 16));

	if (ret)
		return 0xF2;

	for (i = 1; i < quot; i++) {
		ret = mbrc_mmc_read(start + (i << 7), MMC_SECTOR_SIZE,
			buf + (i << 16));
		debug("mmc read two: ret(%d) start(0x%x), buf(%p)\n",
			ret, start + (i << 7), buf + (i << 16));

		if (ret)
			return 0xF3;
	}

	/* skip Actions Boot code check */
	sprintf(tmp, "%c%c", 0xaa, 0x55);

	if (memcmp(buf + (sectors << 9) - 6, tmp, 2)) {
		printf("mmc: start(%d) check flag fail\n", start);
		return 2;
	}

	val = checksum_le32(CONFIG_SYS_UBOOT_BASE, uboot_len - 4) + 0x1234;
	check = readl(CONFIG_SYS_UBOOT_BASE + uboot_len - 4);
	if (check != val) {
		printf("mmc: check value got(0x%x), expect(0x%x)\n", val, check);
		return 2;
	}

	return 0;
}

void spl_mmc_set_rate(unsigned long rate)
{
	unsigned long regv, div, div128;
	unsigned long parent_rate;

	if ((readl(CMU_DEVPLL) & (1 << 12)) && (readl(CMU_DEVPLL) & (1 << 8))) {
		parent_rate = readl(CMU_DEVPLL) & 0x7f;
		parent_rate *= 6000000;
		printf("MMC: source clk CMU_DEVPLL:%luHz\n", parent_rate);

	} else {
		printf("MMC: parent clock not used, CMU_DEVPLL:0x%x\n",
			readl(CMU_DEVPLL));
		return -1;
	}

	rate *= 2;

	if (rate >= parent_rate) {
		div = 0;
	} else {
		div = parent_rate / rate;
		if (div >= 128) {
			div128 = div;
			div = div / 128;

			if (div128 % 128)
				div++;

			div--;
			div |= 0x100;
		} else {
			if (parent_rate % rate)
				div++;

			div--;
		}

		printf("CMU_DEVPLL=0x%x\n", readl(CMU_DEVPLL));

		if ( afinfo->boot_dev == OWLXX_BOOTDEV_SD2 ) {
			regv  = readl(CMU_SD2CLK);
			regv &= 0xfffffce0;
			regv |= div;
			writel(regv, CMU_SD2CLK);
			printf("CMU_SD2CLK=0x%x\n", readl(CMU_SD2CLK));
		}else {
			regv  = readl(CMU_SD1CLK);
			regv &= 0xfffffce0;
			regv |= div;
			//writel(regv, CMU_SD0CLK);
			writel(regv, CMU_SD1CLK);
			//printf("CMU_SD0CLK=0x%x\n", readl(CMU_SD0CLK));
			printf("CMU_SD0CLK=0x%x\n", readl(CMU_SD1CLK));
		}

	}
}

void spl_rawmmc_load_image(void)
{
#define MMC_MBRC_SECTORS (2)
#define MMC_STAGE1_SECTORS	MBRC_SECTOR_SIZE /* 2 + 50*/
	u32 i, ret;
	u32 uboot_len;
	u32 *mbrc = (u32 *)MBRC_START_IN_RAM;
	uboot_len = readl(&mbrc[11]);

	printf("uboot_len-4: %u\n", uboot_len);

	spl_mmc_set_rate(25000000);

	/* put setctor 1, 9 as last one, to deprecated them */
	u32 inx[] = { 1, 9, 17, 25 };

	for (i = 0; i < sizeof(inx)/sizeof(u32); i++) {
		ret = uboot_mmc_read(inx[i] + MMC_STAGE1_SECTORS,
			(char *) CONFIG_SYS_UBOOT_BASE);
		printf("mmc boot: read sectores(%d-%d) error(0x%x)\n", i, inx[i], ret);

		if (!ret || ret > 0xF0)
			break;
	}


	if (ret) {
		printf("mmc: tried with all sectores fail, or read error\n");
		owlxx_enter_adfu();
	}
	
}
