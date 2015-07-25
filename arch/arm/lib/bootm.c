/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *  - Added prep subcommand support
 *  - Reorganized source - modeled after powerpc version
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/bootm.h>
#include <linux/compiler.h>
#include <asm/arch/pmu.h>
#include <mmc.h>

#include <power/boot_power.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
static struct tag *params;
#endif

static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	ulong sp;

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CONFIG_SYS_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 4K to be safe */
	sp -= 4096;
	lmb_reserve(lmb, sp,
		    gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size - sp);
}

#ifdef CONFIG_OF_LIBFDT
static int fixup_memory_node(void *blob)
{
	bd_t	*bd = gd->bd;
	int bank;
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = bd->bi_dram[bank].start;
		size[bank] = bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
}
#endif

static void announce_and_cleanup(void)
{
	printf("\nStarting kernel ...\n\n");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");
#ifdef CONFIG_BOOTSTAGE_FDT
	bootstage_fdt_add_report();
#endif
#ifdef CONFIG_BOOTSTAGE_REPORT
	bootstage_report();
#endif

#ifdef CONFIG_USB_DEVICE
	udc_disconnect();
#endif
	cleanup_before_linux();
}

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *)bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}
#endif

#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags(bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}
}
#endif

#ifdef CONFIG_CMDLINE_TAG
static void setup_commandline_tag(bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}
#endif

#ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag(bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size (tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next (params);
}
#endif

#ifdef CONFIG_SERIAL_TAG
void setup_serial_tag(struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;
	void get_board_serial(struct tag_serialnr *serialnr);

	get_board_serial(&serialnr);
	params->hdr.tag = ATAG_SERIAL;
	params->hdr.size = tag_size (tag_serialnr);
	params->u.serialnr.low = serialnr.low;
	params->u.serialnr.high= serialnr.high;
	params = tag_next (params);
	*tmp = params;
}
#endif

#ifdef CONFIG_REVISION_TAG
void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;
	u32 get_board_rev(void);

	rev = get_board_rev();
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size (tag_revision);
	params->u.revision.rev = rev;
	params = tag_next (params);
}
#endif

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
static void setup_end_tag(bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}
#endif

#ifdef CONFIG_OF_LIBFDT
static int create_fdt(bootm_headers_t *images)
{
	ulong of_size = images->ft_len;
	char **of_flat_tree = &images->ft_addr;
	ulong *initrd_start = &images->initrd_start;
	ulong *initrd_end = &images->initrd_end;
	struct lmb *lmb = &images->lmb;
	ulong rd_len;
	int ret;

	debug("using: FDT\n");

	boot_fdt_add_mem_rsv_regions(lmb, *of_flat_tree);

	rd_len = images->rd_end - images->rd_start;
	ret = boot_ramdisk_high(lmb, images->rd_start, rd_len,
			initrd_start, initrd_end);
	if (ret) {
		printf("WARNING: "
			"boot_ramdisk_high rd_start=%d, rd_len=%d, *initrd_start=0x%x, *initrd_end=0x%x\n",
			images->rd_start, rd_len, *initrd_start, *initrd_end);
		return ret;
	}

	ret = boot_relocate_fdt(lmb, of_flat_tree, &of_size);
	if (ret) {
		printf("WARNING: "
			"boot_relocate_fdt failed\n");
		return ret;
	}

	fdt_chosen(*of_flat_tree, 1);
	fixup_memory_node(*of_flat_tree);
	fdt_fixup_ethernet(*of_flat_tree);
	fdt_initrd(*of_flat_tree, *initrd_start, *initrd_end, 1);
#ifdef CONFIG_OF_BOARD_SETUP
	ft_board_setup(*of_flat_tree, gd->bd);
#endif

	return 0;
}
#endif
extern int ReadStorage(int type, char * buf, int size);

/*check serialno vaild*/
static int serialno_check(char *sn, int num)
{
	int i;
	for ( i = 0; i < num ; i++ ) {
		if (   ( sn[i] >= '0' && sn[i] <='9') 
			|| (sn[i] >= 'A' && sn[i] <= 'Z')
			|| (sn[i] >= 'a' && sn[i] <= 'z') )
			continue;
		if ( sn[i] == 0 )
			break;
		if ( sn[i] == 10 ) { //'\r'
			sn[i] == 0; //
			break;
		}
		printf("no=%d invaild:%d\n" , i, sn[i]);
		return 0; // invaild		
	}
	if ( i >= 6 )
		return 1; // ok

	printf("sn=%s err\n", sn);
	return 0;
}
void serialno_read(char *buf)
{
	char sn[37];
	int ret = 0;
	sn[0] = sn[20] = 0;
	#if defined(CONFIG_OWLXX_NAND)
	ret = ReadStorage(0, &sn, 20);	
	printf("bf-sn:%d, %s\n", ret, sn);
	#elif defined(CONFIG_GENERIC_MMC)
	ret = owlxx_mmc_get_miscinfo(0, &sn, 20);
	printf("MMC-sn:%d, %s\n", ret, sn);
	#endif
	if ( ret >= 6 && serialno_check(sn, ret) ) {
		sn[ret] = 0;
		printf("use bf serialno\n");
		strcpy(buf, sn);
		return;
	}
	
	if ( serialno_check(afinfo->sn, 16) ) {
		memcpy(buf, afinfo->sn, 16);
		buf[16] = 0;
		buf[0] = 'A';
		buf[1] = '0';
		printf("use afi serialno\n");
		return ;
	}
	printf("use deafult serialno\n");
	strcpy(buf, "abcdef0123456789");	
	
}


void boot_append_serialno(void)
{
	char buf[64];
	char sn[40];
	serialno_read(sn);	
	sprintf(buf, "androidboot.serialno=%s", sn);
	boot_append_remove_args(buf, NULL);
}
static void boot_enable_upgrade_flag(void)
{
	char buf[64] ;

	strcpy(buf, "owlxx_upgrade");
	boot_append_remove_args(buf, NULL);	
	print_bootargs();		
}

static void boot_enable_adfuserver(void)
{
	char buf[64] ;

	strcpy(buf, "upgrade_process");
	boot_append_remove_args(buf, NULL);	
	print_bootargs();	
	strcpy(buf, "adfuserver_process");
	boot_append_remove_args(buf, NULL);	
	print_bootargs();		
}

static void boot_enable_uart(int index)
{
	const void *blob = gd->fdt_blob;
	const char *cell;
	int node;
	char buf[20];
	if ( index < 0 || index > 6 )
		return;
	sprintf(buf, "serial%d", index);
	node = fdt_path_offset(blob, buf);
	if ( node < 0 ) {
		printf("dts:get %s fail\n", buf);
		return;
	}
	cell = fdt_getprop(blob, node, "status", NULL);
	if ( cell == NULL || memcmp(cell, "okay", 4) ) {
		if ( cell != NULL )		
			printf("%s status is %s, set to okay\n", buf, cell);
		else
			printf("%s status not exist, set to okay\n", buf);
		
		if (fdt_setprop(blob, node, "status", (const void*)"okay", 5) )
			printf("set fail\n");
	} 
	
}

void print_bootargs(void)
{
	const void *blob = gd->fdt_blob;
	const char *cell;
	int node;

	node = fdt_path_offset(blob, "/chosen");
	cell = fdt_getprop(blob, node, "bootargs", NULL);
	if (cell)
		printf("bootcmd=%s\n", cell);
	else
		printf("not bootargs\n");
}

void boot_append_bootdev(void)
{
	char buf[64] ;
	unsigned char index; 	

	index = ((afinfo->boot_dev == OWLXX_BOOTDEV_SD0)?afinfo->burn_uart: afinfo->uart_index);
	sprintf(buf, "console=ttyS%d", index);// add console by afi.cfg
	boot_append_remove_args(buf, NULL);
	boot_enable_uart(index);
	/*add bootdev and loglevel*/
	if ( afinfo->boot_dev == OWLXX_BOOTDEV_NAND) {
		sprintf(buf, "androidboot.bootdev=nand loglevel=%d", afinfo->loglevel);
	}else {
		sprintf(buf, "androidboot.bootdev=sd loglevel=%d", afinfo->loglevel);
	}
	boot_append_remove_args(buf, NULL);	
		
}

void boot_append_s2ddr_check(void)
{
	char buf[64] ;
	
	if (afinfo->s2ddr_check) {
		strcpy(buf, "s2ddr_check");
    	boot_append_remove_args(buf, NULL);	    			
	}
}

int check_fdtnode_status(const char *path_node)
{
	const void *blob = gd->fdt_blob;
	const char *cell;
	int node;

	node = fdt_path_offset(blob, path_node);
	if ( node < 0 ) {
		printf("find node =%s fail, %d\n", path_node, node);
		return 0;
	}
	
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell) {
		printf("%s: status=%s\n", path_node, cell);
		if ( !strcmp(cell, "okay") )
			return 1;
		else
			return 0;
	}
	else {
		printf("%s: status not exist\n",path_node);
		return 0;
	}
}
void boot_append_recovery_mode(void)
{
	char buf[64];
	//"/spi@b0204000/atc260x/atc260x-adckeypad"
	if (afinfo->key_mode != 0)  {
		strcpy(buf, "androidboot.mode=recovery1"); 
	}else {
		strcpy(buf, "androidboot.mode=recovery2"); 
	}
	boot_append_remove_args(buf, NULL);/*cxj*/
}

#ifndef CONFIG_SPL_OWLXX_UPGRADE
/*author:chenxijian @2014-09-17*/
void boot_check_charger_mode(void)
{
	char buf[64];
	if ( owlxx_in_recovery() ) {
		boot_append_recovery_mode();
	} else if(gd->flags & GD_FLG_CHARGER) {
		strcpy(buf, "androidboot.mode=charger");
		printf("=======androidboot.mode:charger=========\n");
		boot_append_remove_args(buf, NULL);
	}	
}

void boot_setup_or_not(void)
{
	if((gd->flags & 0x80000) == GD_FLG_LOWPOWER)
	{
		printf("power low, shut down the power now!\n");
		mdelay(3000);
		atc260x_shutoff();	
	}
	return;
}


void boot_dual_logo_cmd(void)
{
	int val;
	char buf[64];
	val = dual_logo_cfgval_get();
	if ( val >= 0 ) {
		sprintf(buf, "androidboot.supportduallogo=%d", val);
		boot_append_remove_args(buf, NULL);
	}	
	printf("dual_logo=%d\n", val);
}
#endif


__weak void setup_board_tags(struct tag **in_params) {}

/* Subcommand: PREP */
static void boot_prep_linux(bootm_headers_t *images)
{
	static const u16 sc_pmu_regtbl_sysctl2[OWLXX_PMU_ID_CNT] = {
		[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_SYS_CTL2,
		[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_SYS_CTL2,
		[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_SYS_CTL2,
	};
	u16 reg_sysctl2;
	char *cmd_add = getenv("bootargs.add");
	char *cmd_remove = getenv("bootargs.remove");
	int ret;

	reg_sysctl2 = sc_pmu_regtbl_sysctl2[OWLXX_PMU_ID];
	ret = atc260x_reg_read(reg_sysctl2);
	printf("SYS_CTL2=0x%x\n", ret);
	atc260x_set_bits(reg_sysctl2, (1<<9) , (1<<9)); /* reset_EN */

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = getenv("bootargs");
#endif
#ifdef CONFIG_OF_LIBFDT
	if (images->ft_len)
		fdt_get_checksum(1);
#endif

	if (OWLXX_PRODUCT_STATE)
		boot_append_remove_args("owlxx_product=1", NULL);
    boot_append_s2ddr_check();
	boot_append_bootdev();
	boot_append_serialno();	
#ifdef CONFIG_SPL_OWLXX_UPGRADE
	boot_enable_adfuserver();
	boot_enable_upgrade_flag();
#endif
	boot_append_remove_args(cmd_add, cmd_remove);
	//boot_append_recovery_mode();
#ifndef CONFIG_SPL_OWLXX_UPGRADE		
	boot_check_charger_mode();/* by cxj */
	boot_setup_or_not();/*by cxj @2014-10-07:when low power,shut down!*/

	boot_dual_logo_cmd();
#endif
	print_bootargs();

#ifdef CONFIG_OF_LIBFDT
	if (images->ft_len) {
		char buf[32];

		sprintf(buf, "afinfo=0x%lx,0x%x", (unsigned long)afinfo, sizeof(afinfo_t));
		boot_append_remove_args(buf, NULL);

		debug("using: FDT\n");
		if (create_fdt(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
	} else
#endif
	{
#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
		debug("using: ATAGS\n");
		setup_start_tag(gd->bd);
#ifdef CONFIG_SERIAL_TAG
		setup_serial_tag(&params);
#endif
#ifdef CONFIG_CMDLINE_TAG
		setup_commandline_tag(gd->bd, commandline);
#endif
#ifdef CONFIG_REVISION_TAG
		setup_revision_tag(&params);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
		setup_memory_tags(gd->bd);
#endif
#ifdef CONFIG_INITRD_TAG
		if (images->rd_start && images->rd_end)
			setup_initrd_tag(gd->bd, images->rd_start,
			images->rd_end);
#endif
		setup_board_tags(&params);
		setup_end_tag(gd->bd);
#else /* all tags */
		printf("FDT and ATAGS support not compiled in - hanging\n");
		hang();
#endif /* all tags */
	}
}

/* Subcommand: GO */
static void boot_jump_linux(bootm_headers_t *images)
{
	unsigned long machid = gd->bd->bi_arch_number;
	char *s;
	void (*kernel_entry)(int zero, int arch, uint params);
	unsigned long r2;

	kernel_entry = (void (*)(int, int, uint))images->ep;

	s = getenv("machid");
	if (s) {
		strict_strtoul(s, 16, &machid);
		printf("Using machid 0x%lx from environment\n", machid);
	}

	debug("## Transferring control to Linux (at address %08lx)" \
		"...\n", (ulong) kernel_entry);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);
	announce_and_cleanup();

#ifdef CONFIG_OF_LIBFDT
	if (images->ft_len)
		r2 = (unsigned long)images->ft_addr;
	else
#endif
		r2 = gd->bd->bi_boot_params;
	print_btime("bootk");
	kernel_entry(0, machid, r2);
}

/* Main Entry point for arm bootm implementation
 *
 * Modeled after the powerpc implementation
 * DIFFERENCE: Instead of calling prep and go at the end
 * they are called if subcommand is equal 0.
 */
int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	/* No need for those on ARM */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & BOOTM_STATE_OS_GO) {
		boot_jump_linux(images);
		return 0;
	}

	boot_prep_linux(images);
#ifndef CONFIG_SPL_OWLXX_UPGRADE
	// detect_quick_plug();
#endif
	boot_jump_linux(images);
	return 0;
}

#ifdef CONFIG_CMD_BOOTZ

struct zimage_header {
	uint32_t	code[9];
	uint32_t	zi_magic;
	uint32_t	zi_start;
	uint32_t	zi_end;
};

#define	LINUX_ARM_ZIMAGE_MAGIC	0x016f2818

int bootz_setup(void *image, void **start, void **end)
{
	struct zimage_header *zi = (struct zimage_header *)image;

	if (zi->zi_magic != LINUX_ARM_ZIMAGE_MAGIC) {
		puts("Bad Linux ARM zImage magic!\n");
		return 1;
	}

	*start = (void *)zi->zi_start;
	*end = (void *)zi->zi_end;

	debug("Kernel image @ 0x%08x [ 0x%08x - 0x%08x ]\n",
		(uint32_t)image, (uint32_t)*start, (uint32_t)*end);

	return 0;
}
#endif	/* CONFIG_CMD_BOOTZ */
