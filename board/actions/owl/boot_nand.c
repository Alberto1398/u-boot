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
#include "afinfo.h"
#include <flash/osboot.h>
#include <flash/address.h>

#define ONE_REAS_SIZE (0x1 << 10)
#define SHRAM_RAM_STORE_ADDR (0xB4068000)
#define __NAND_DEBUG_READ_DATA

typedef enum {
    slcMode_sharePage = 1,
} Enum_slcModeType;

struct nand_boot_op_param {
	u32 nand_addr1;
	u32 nand_addr2;
	u32 ram_addr;
} __attribute__ ((packed));

typedef int (*boot_func_t)(u32 a, u32 b);

#define nand_boot_read_tlc(a)   ((boot_func_t)SRAM_DRV_ADDR)(a, Brom_NANFLBSectorRead_TTlc)
#define nand_boot_read(a) 	((boot_func_t)SRAM_DRV_ADDR)(a, Brom_NANFLBSectorRead)
#define nand_boot_disable(a)	((boot_func_t)SRAM_DRV_ADDR)(a, Brom_NANFDisable)
#define nand_boot_exit(a)	((boot_func_t)SRAM_DRV_ADDR)(a, Brom_NANFExit)

unsigned char ecc_config;
unsigned char ud_cnt;
unsigned int sectors_per_page;
unsigned short size_per_page;
unsigned char slc_mode;
unsigned char tlc_enhanced_cmd;
unsigned int pageAddrPerBlock;
unsigned int pagePerBlock;

static u32 wchecksum(u32 sum, char * buf, int start, int length)
{
    u16 * tmp_buf = (u16 *)buf;
    int tmp_start = start / sizeof(u16);
    int tmp_length = length / sizeof(u16);
    int i;

    for (i = tmp_start + tmp_length -1; i >= tmp_start; i--)
        sum + = *(tmp_buf + i);

    return sum;
}

static int init_nand_global_var(void)
{
	pageAddrPerBlock = *(volatile unsigned short *)(MBRC_START_IN_RAM + 8);
	pagePerBlock = *(volatile unsigned short *)(MBRC_START_IN_RAM + 10);
	sectors_per_page = *(volatile unsigned int *)(MBRC_START_IN_RAM + 0xc);
	ecc_config = *(volatile unsigned char *)(MBRC_START_IN_RAM + 0x18);
	ud_cnt = *(volatile unsigned char *)(MBRC_START_IN_RAM + 0x19);
	slc_mode = *(volatile unsigned char *)(MBRC_START_IN_RAM + 0x30);
	tlc_enhanced_cmd = *(volatile unsigned char *)(MBRC_START_IN_RAM + 0x31);

    printf("pageAddrPerBlock %d, pagePerBlock %d, sectors_per_page %d, ecc_config 0x%x\n",
            pageAddrPerBlock, pagePerBlock, sectors_per_page, ecc_config);
    printf("ud_cnt %d, slc_mode %d, tlc_enhanced_cmd 0x%x\n", 
            ud_cnt, slc_mode, tlc_enhanced_cmd);

    return 0;
}

static void calc_rw_addr(struct nand_boot_op_param *param, unsigned int lba)
{
    u8 *nand_slb_table = &afinfo->lsb_tbl[0];
    unsigned int sec_num_in_page=0;
    unsigned int page_num_in_logic=0, page_num_in_blk, page_num_in_global;
    unsigned int block_num;
	//unsigned int tmp;

    if(sectors_per_page != 0) { //qac
        sec_num_in_page = lba % sectors_per_page;
    }

    param->nand_addr1 = (sec_num_in_page << 9);
    param->nand_addr2 = 0;

    if(sectors_per_page != 0) { //qac
        page_num_in_logic = lba / sectors_per_page;
    }
    page_num_in_blk = page_num_in_logic % pagePerBlock;

    /* get mapped page if nand is MLC or TLC */
    if (slc_mode == slcMode_sharePage) {
        page_num_in_blk = nand_slb_table[page_num_in_blk];
    }

    block_num = page_num_in_logic / pagePerBlock;

    page_num_in_global = block_num * pageAddrPerBlock + page_num_in_blk;

    if ((page_num_in_global & 0xFFFF) != 0) { /* 5 byte nand physic address */
        param->nand_addr2 = (page_num_in_global >> 16) & 0xFF;
    }

    param->nand_addr1 |= (page_num_in_global << 16);
}

static int __nand_sector1024_read(u32 sector, void *buf)
{
	struct nand_boot_op_param op_param;

	calc_rw_addr(&op_param, sector);
	op_param.ram_addr = (u32)buf;
    
	if (tlc_enhanced_cmd == 0xa2) {
        	return nand_boot_read_tlc((u32)(&op_param));
	}

	return nand_boot_read((u32)(&op_param));
}

int spl_raw_nand_check_header(uint32_t addr)
{
	uint32_t header_magic;
	const struct image_header *header = (struct image_header *) addr;

	header_magic = image_get_magic(header);

	if (header_magic != IH_MAGIC) {
		printf("addr: 0x%x, mbrc header ERROR. this: %u, expected: %u\n",
				addr, header_magic, IH_MAGIC);
		return -1;
	}

	spl_parse_image_header(header);

	return 0;
}

int spl_raw_nand_check_sum(u32 addr, u16 chksum)
{
	u8 mbrc_flag0, mbrc_flag1;
	u16 checksum1;

	/* checksum if read data sucess */
	mbrc_flag0 = readb(addr + ONE_REAS_SIZE - 4);
	mbrc_flag1 = readb(addr + ONE_REAS_SIZE - 3);

	if (mbrc_flag0 != 0xaa || mbrc_flag1 != 0x55) {
		printf("mbrc flag not 0xaa55, addr: 0x%x, mbrc_flag0: 0x%x, mbrc_flag1: 0x%x\n",
				addr, mbrc_flag0, mbrc_flag1);
		return -1;
	}

	checksum1 = readw(addr + ONE_REAS_SIZE - 2);

	if (checksum1 != chksum) {
		printf("mbrc checksum fail, addr(0x%x), expect(0x%x), got(0x%x)\n",
			addr, checksum1, chksum);
		return -1;
	}

	return 0;
}

#ifdef __NAND_DEBUG_DUMP_DATA
int spl_raw_nand_dump_data(u32 addr, u32 size)
{
	u32 i;

	printf("err: read uboot fail, dump img, addr(0x%x), size(0x%x)\n",
			addr, size);

	for (i = 0; i < size; i + = 16) {
		printf("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n",
				i, readl(addr + i), readl(addr + i + 4),
				readl(addr + i + 8), readl(addr + i + 0xc));
	}
}
#endif

#ifdef __NAND_DEBUG_CMP_DATA
int spl_raw_nand_cmp_data(u32 index)
{
	static u32 cnt = 0;
	u32 i, s, d, vs, vd;
	s = CONFIG_SYS_UBOOT_BASE + (index << 9);
	d = SHRAM_RAM_STORE_ADDR;

	for (i = 0; i < ONE_REAS_SIZE; i + = 4) {
		vs = readl(s);
		vd = readl(d);
		if (vs != vd && cnt +  + < 100)
			printf("data un-match: src(0x%08x -> 0x%08x), dst(0x%08x -> 0x%08x)\n",
					s, vs, d, vd);
		writel(vd, s);
		vs = readl(s);
		if (vs != vd) {
			printf("fail un-match: src(0x%08x -> 0x%08x), dst(0x%08x -> 0x%08x)\n",
					s, vs, d, vd);
			return -1;
		}
		s + = 4;
		d + = 4;
	}

	return 0;
}
#endif

static int nand_sector_read(u32 sector, u32 size, void *buf)
{
	u32 i, n_secotor, n_size;
	void *tmp = buf;
	u16 checksum = 0;

	n_secotor = size >> 9;

	if (sector & 0x1)
		goto err;

	if (n_secotor & 0x1)
		goto err;

	if ((u32) buf & 0x3)
		goto err;

	for (i = 0; i < n_secotor; i + = 2) {
		if (__nand_sector1024_read(sector, tmp)) {
			printf("nand boot read err, secotr: %u\n", sector);
			return -1;
		}

#ifdef __NAND_DEBUG_CMP_DATA
		if ((u32) buf == SHRAM_RAM_STORE_ADDR && spl_raw_nand_cmp_data(i) < 0)
			goto err;
#endif

		if (i == 0 && spl_raw_nand_check_header((u32)tmp) < 0)
			goto err;

		if (i < (n_secotor - 2))
			n_size = ONE_REAS_SIZE;
		else
			n_size = ONE_REAS_SIZE - 2;

		checksum = wchecksum(checksum, (char *)tmp, 0, n_size);

		if (i >= (n_secotor - 2) && spl_raw_nand_check_sum((u32) tmp, checksum & 0xFFFF) < 0)
			goto err;

		sector + = 2;

		if ((u32) buf == CONFIG_SYS_UBOOT_BASE)
			tmp + = 1 << 10;
	}

	return 0;

err:
	return -1;
}

#define NAND_ANALOG_CTL		0xb02100a8
#define NAND_TIMING		0xb02100a4

void spl_raw_nand_load_internal(uint32_t addr)
{
	typedef void __noreturn (*image_run_t)(void);
	u32 i;
	int ret = -1;
	u32 *mbrc = (u32 *)MBRC_START_IN_RAM;
	u32 cur_sector;
	u32 mbrc_size;
	u32 cfg;

	debug("%s %d\n", __func__, __LINE__);

	init_nand_global_var();

	mbrc_size = readl(&mbrc[11]);
	printf("mbrc_size: %u\n", mbrc_size);

	cfg = readl(0xb0210008);
	cfg &= ~0x7;
	cfg |= (ecc_config & 0x7);
	writel(cfg, 0xb0210008);

#define NAND_TIMING_DCBAC_CYCLE_4	(0x1 << 16)
#define NAND_TIMING_TRPRE_CYCLE_2	(0x1 << 14)
#define NAND_TIMING_TWH_CYCLE_1		(0x0 << 12)
#define NAND_TIMING_DCAD_CYCLE_32	(0x4 << 9)
#define NAND_TIMING_DCBD_CYCLE_32	(0x3 << 6)
#define NAND_TIMING_DCADD_CYCLE_1	(0x1 << 3)
#define NAND_TIMING_DCCA_CYCLE_1	(0x1 << 0)

	cfg = 0x10 << 27 | 0x11 << 22 | NAND_TIMING_DCBAC_CYCLE_4
			| NAND_TIMING_TRPRE_CYCLE_2 | NAND_TIMING_TWH_CYCLE_1
			| NAND_TIMING_DCAD_CYCLE_32 | NAND_TIMING_DCBD_CYCLE_32
			| NAND_TIMING_DCADD_CYCLE_1 | NAND_TIMING_DCCA_CYCLE_1;

	writel(cfg, NAND_TIMING);
	writel((readl(NAND_ANALOG_CTL) & ~(0xffff)) | 0x4444, NAND_ANALOG_CTL);

	debug("NAND_ANALOG_CTL 0x%08x, NAND_CONFIG 0x%08x\n", 
			readl(0xb02100a8), readl(0xb0210008));

	cur_sector = MBRC_SECTOR_SIZE;

	for (i = 0; i < 4; i +  + ) {
		ret = nand_sector_read(cur_sector, mbrc_size, addr);
#ifdef __NAND_DEBUG_READ_DATA
		/* read data into share ram to test */
		if (ret) {
			ret = nand_sector_read(cur_sector, mbrc_size, SHRAM_RAM_STORE_ADDR);
			printf("read image into shareram, start sector(%d), ret(%d)\n",
			cur_sector, ret);
		}
#endif
		cur_sector + = pagePerBlock*sectors_per_page;

        	if (!ret)
			break;
	}

exit:
	if (ret) {
		printf("fail to read uboot normal image, enter adfu mode\n");
		owlxx_enter_adfu();
	}
}

void spl_raw_nand_load_image(void)
{
	spl_raw_nand_load_internal(CONFIG_SYS_UBOOT_BASE);
}
