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
#include <environment.h>
#include <netdev.h>
#include <i2c.h>
#include <linux/ctype.h>
//#include <power/owlxx_power.h>
#include <power/boot_power.h>
#include <mmc.h>
#ifdef CONFIG_OWLXX_PWM
#include <asm/arch/pwm.h>
#endif
#ifdef CONFIG_NCS8801
#include <asm/arch/ncs8801.h>
#endif
#include <linux/input.h>
#include <asm/arch/pmu.h>


DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD)
#include <asm/io.h>
#include <spi.h>
#include <asm/arch/pmu.h>
#include <asm/arch/gmp.h>
#include <asm/arch/clocks.h>
#endif



#if !defined(CONFIG_SPL_BUILD)
const char *get_defif(void)
{
	const char *ifname;
	ifname = getenv("devif");
	if ( ifname == NULL) {
		ifname = "nand";
		printf("env:get devif fail\n");
	}
	return ifname;
}

#define ONOFF_LONG_PRESS		(1 << 13)
#define ONOFF_SHORT_PRESS		(1 << 14)
#define ONOFF_PRESS              (1 << 15)

static int count_onoff_short_press(void)
{
	//return 0;  //TESTCODE
#if 1
    int i, poll_times, on_off_val;


    on_off_val = atc260x_reg_read(ATC2603C_PMU_SYS_CTL2);
    if((on_off_val & ONOFF_PRESS) == 0)
        return 0; 
	
	while ( 1 ) { /*wait key move*/
	    on_off_val = atc260x_reg_read(ATC2603C_PMU_SYS_CTL2);
	    if((on_off_val & ONOFF_PRESS) == 0)
	        break; 
		mdelay(1);
	}
        
    printf("start count onoff times\n");
        
    /* clear On/Off press pending */
    atc260x_set_bits(ATC2603C_PMU_SYS_CTL2, 
        ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS, 
        ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
	
    
    for(poll_times = 0; poll_times < 4; poll_times++)
    {
        for(i = 0; i < 2000; i++)
        {
            on_off_val = atc260x_reg_read(ATC2603C_PMU_SYS_CTL2);
            if ((on_off_val & ONOFF_SHORT_PRESS) != 0)
                break;
            mdelay(1);
        }
        
        if(i == 2000)
            break;
            
        atc260x_set_bits(ATC2603C_PMU_SYS_CTL2, 
            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS, 
            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
    }
    
    printf("Onoff press %d times\n", poll_times);
    return poll_times;
#endif
}

#endif 
#if defined(CONFIG_ATM7059TC) && !defined(CONFIG_SPL_BUILD)
int cpy_adfu_to_ram(void)
{
	int ret;
	block_dev_desc_t *dev_desc;
	char *buf = (char*)0xb4064000;
	const char *ifname;

	ifname = get_defif();
	if ( ifname == NULL ) {
		printf("adfu:Failed to get_defif\n");
		return -1;
	}
	dev_desc = get_dev(ifname, FAT_MISC_DEV);
	if (dev_desc == NULL) {
		printf("adfu:Failed to find %s\n", ifname);
		return -1;
	}
	ret = fat_register_device(dev_desc, 0);
	if (ret) {
		printf("adfu:Failed to register nand:\n");
		return -1;
	}
	ret = file_fat_read("adfu_fill.bin", buf, 13*1024);
	if(ret < 0) {
		printf("adfu:Fail to load adfu_fill.bin\n");
		return -1;
	}
	printf("cpy_adfu_to_ram ok\n");
	return 0;
}
void gl6082_check_adfu(void)
{
	static const u16 sc_pmu_regtbl_uv_status[OWLXX_PMU_ID_CNT] = {
		[OWLXX_PMU_ID_ATC2603A] = ATC2603A_PMU_UV_STATUS,
		[OWLXX_PMU_ID_ATC2603B] = ATC2609A_PMU_UV_STATUS,
		[OWLXX_PMU_ID_ATC2603C] = ATC2603C_PMU_UV_STATUS,
	};
	u16 reg_pmu_uv_status, reg_val;
	unsigned int checksum, org_checksum = 0x55aa55aa;
	unsigned int rdat, wdat, sumdat = 0;
	unsigned int save_cin, save_cout;
	int i;

	//printf("PMU_STATUS=%x\n", atc260x_reg_read(gl5302_PMU_UV_STATUS));

	reg_pmu_uv_status = sc_pmu_regtbl_uv_status[OWLXX_PMU_ID];
	reg_val = atc260x_reg_read(reg_pmu_uv_status);
	if (reg_val & (0x1 << 1)) {
		reg_val &= ~((0x1 << 1) | (1U << 0));
		atc260x_reg_write(reg_pmu_uv_status, reg_val);
		goto end;
	}

	if (afinfo->boot_dev != OWLXX_BOOTDEV_SD0 ) {
		writel(readl(PAD_PULLCTL1) & ~(1 << 17), PAD_PULLCTL1);

		save_cin= readl(GPIO_CINEN);
		save_cout= readl(GPIO_COUTEN);

		/* RX - INPUT */
		writel(readl(GPIO_COUTEN) & ~(1 << 10), GPIO_COUTEN);
		writel(readl(GPIO_CINEN) | (1 << 10), GPIO_CINEN);

		/* TX - OUTPUT */
		writel(readl(GPIO_CINEN) & ~(1 << 11), GPIO_CINEN);
		writel(readl(GPIO_COUTEN) | (1 << 11), GPIO_COUTEN);

		checksum = org_checksum;
		for (i = 0; i < 32; i++) {
			wdat = checksum & 0x1;

			/* write data */
			writel(readl(GPIO_CDAT) & ~(1 << 11), GPIO_CDAT);
			writel(readl(GPIO_CDAT) | (wdat << 11), GPIO_CDAT);
			mdelay(5);

			/* read data */
			rdat = readl(GPIO_CDAT) & (1 << 10);
			sumdat |= (!!rdat << i);

			checksum >>= 1;
		}

		writel(save_cin, GPIO_CINEN);
		writel(save_cout, GPIO_COUTEN);

		if (org_checksum == sumdat) {
			goto end;
		}
	}
	return;
end:
	if ( cpy_adfu_to_ram() )
		return;
	printf("ATM7059TC entry ADFU mode\n");	
	owlxx_enter_adfu();
}
#endif


#ifdef CONFIG_GENERIC_MMC
/******************** mmc/tsd check update or burn********************/
static void dump_buf(u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%02x ", *(buf + i));
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}
static unsigned int dwchecksum(char * buf, int start, int length)
{
    unsigned int sum = 0;
    unsigned int * tmp_buf = (unsigned int *)buf;
    int tmp_start = start / sizeof(unsigned int);
    int tmp_length = length / sizeof(unsigned int);
    int i;

    for(i = tmp_start; i < tmp_length; i++)
    {
        sum = sum + *(tmp_buf + i);
    }
 
    return sum;
}

static int check_burn(block_dev_desc_t *dev_desc )
{
	int ret;
	unsigned int mbrc_sum, sum;
	afinfo_t *pafi;
	
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz*5);
	ret = dev_desc->block_read(0, 1, 1, buffer);
	printf("get 1st: ret%d\n", ret);
	dump_buf( buffer, 16);
	if ( memcmp(buffer+500, "ActBrm", 6 ) )
	{
		printf("mbrc512 is not ActBrm!\n");
		return -1;		
	}
	memcpy(&sum, buffer+508, 4 );
	mbrc_sum = dwchecksum((char*)buffer, 0, 508) + 0x1234;
	if ( mbrc_sum != sum )
	{
		printf("mbrc512 checksum err!\n");
		return -1;	
	}
	/*get sd afi*/
	ret = dev_desc->block_read(0, 3, 5, buffer);
	printf("afi: ret%d\n", ret);
	dump_buf( buffer+0x50, 64);
	if ( memcmp(buffer+0x50, "AFI", 3) )
	{
		printf("read sd afi err!\n");
		return -1;	
	}
	pafi = (afinfo_t*)(buffer+0x50);
	
	if (pafi->boot_dev != OWLXX_BOOTDEV_SD0 ){
		printf("err: bootdev=%d!\n", pafi->boot_dev);
		return -1;			
	}
	memcpy(afinfo, buffer+0x50, sizeof(afinfo_t));	
	set_sd_mmc_type(TSD);
	if( mmc_initialize(NULL) ) {
		printf("burn: sd0 init fail\n");
	}
	printf("goto card burn!\n");

	return 0;
}

static int check_update(block_dev_desc_t *dev_desc )
{
	int ret;
	char buf[11];
	ret = fat_register_device(dev_desc, 0);
	if (ret) {
		printf("Failed to register sdcard to fat\n");
		return -1;
	}

	ret = file_fat_read("update.zip", buf, 10);
	if(ret <= 0) {
		printf("not find update.zip, ret=%d\n", ret);
		return -1;
	}
	printf("is card update, ret=%d\n", ret);
	return 0;
}

static int check_burn_update(void)
{
	block_dev_desc_t *dev_desc;
	
	printf("=check burn=\n");
	set_sd_mmc_type(SD);
	if( mmc_initialize(NULL) ) {
		printf("sd: init fail\n");
		return -1;
	}
	dev_desc = get_dev("mmc", 0);
	if ( dev_desc == NULL) {
		printf("sd: get dev fail\n");
		return -1;
	}

	if (check_burn(dev_desc) == 0 )
		return 1;
	if (check_update(dev_desc) == 0 )
		return 0;

	return -1;
}

static int recovery_mode ; // =1 enter recovery ,2= enter tsd recovery
void mmc_sd_init(void)
{
	int ret;
	unsigned char bk_bootdev;
	
	recovery_mode = 0;
	printf("bootdev=%x\n", afinfo->boot_dev);
	if (afinfo->boot_dev == OWLXX_BOOTDEV_SD0 ) {			
		puts("sd0: boot\n");
		set_sd_mmc_type(TSD);
		if( mmc_initialize(NULL) ) {
			printf("sd0: init fail\n");
			return ;
		}
		
		if ( afinfo->burn_flag & 0x01 ) { // boot from sd0 and tf card is burned card			
			recovery_mode = 1; // enter recovery	 to card burn	
			set_sd_mmc_type(MMC);
			if( mmc_initialize(NULL) ) {
				printf("nand burned\n");
				afinfo->burn_flag &= ~0x02; // nand burn
			} else {
				printf("emmc burned\n");
				afinfo->burn_flag |= 0x02; // emmc burn
			}
			set_sd_mmc_type(TSD);			
		}
	} else { //emmc or nand boot to check sd burn or update
 		bk_bootdev = afinfo->boot_dev;
		afinfo->boot_dev = OWLXX_BOOTDEV_SD0;
		serial_init();
		ret = check_burn_update();
		if ( ret == 1) { /*tf boot */
			if ( afinfo->burn_flag & 0x01 ) { // is burned card	
				if ( bk_bootdev == OWLXX_BOOTDEV_NAND ) {
					printf("nand boot to card burned\n");
					afinfo->burn_flag &= ~0x02; // nand burn
					recovery_mode = 2; // to set bootcmd
				} else {
					printf("emmc boot to card burned\n");
					afinfo->burn_flag |= 0x02; // emmc burn
					recovery_mode = 1;
				}
				
			} else {
				printf("card boot =%d\n", bk_bootdev);
			}
		} else {
			
			afinfo->boot_dev = bk_bootdev;
			serial_init();  /*reinit debug uart*/
			
			if (ret  == 0  ) { 
				printf("card update, enter reovery\n");
				recovery_mode = 1;// enter to recovrey , card update		
			}
 	
			if (afinfo->boot_dev == OWLXX_BOOTDEV_SD2  ) { // use emmc
				set_sd_mmc_type(MMC);
				if ( mmc_initialize(NULL) )
					printf("emmc: init fail\n");
				else
					printf("emmc: init ok\n");
			}
		}

	}
				
	#ifdef CONFIG_ATM7059TC
	gl6082_check_adfu();
	#endif

}

void check_burn_recovery (void)
{
	if (recovery_mode ) {// enter recovery 		
		setup_recovery_env();
		printf("card burn or update, enter reovery=%d\n", recovery_mode);
		if ( recovery_mode > 1 ) { // card burn			
			setenv("bootcmd", CONFIG_MMC_BOOTCOMMAND);
			setenv("devif", "mmc");
		}	
	}
}


/******************** mmc/tsd check update or burn end ********************/
#endif

void reset_to_adfu(void)
{
	atc260x_pstore_set(ATC260X_PSTORE_TAG_REBOOT_ADFU, 1);
	printf("reset to adfu\n");
	reset_cpu(0);
}
// check key to adfu  or recovery
void check_key(void)
{
#if defined(CONFIG_MENU_SHOW)

	int key;
	int onoff_shortpress_times;

	key = adckey_scan(); //
	if ( key != KEY_VOLUMEUP && key != KEY_VOLUMEDOWN ) {		
		#ifndef CONFIG_SPL_OWLXX_UPGRADE	
		onoff_shortpress_times = count_onoff_short_press();
		
		if(onoff_shortpress_times == 2)
			key = KEY_VOLUMEDOWN ;
		else if(onoff_shortpress_times == 3)
			key = KEY_VOLUMEUP ;
		else if (onoff_shortpress_times == 4)
			 gd->flags |= GD_FLG_CMDLINE ;
		#endif
	}
	switch(key) {
	case KEY_VOLUMEUP:
		printf("touch v+ key, enter adfu\n");
		#ifdef CONFIG_ATM7059TC
		cpy_adfu_to_ram();
		#endif
		reset_to_adfu();
		//owlxx_enter_adfu();
		break;
	case KEY_VOLUMEDOWN:		
		printf("touch v- key, enter recovery\n");
		setup_recovery_env();		
		break;
	}
#endif
}




#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif	/* EARLY_INIT */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	debug("set bi_boot_params\n");

	pmu_init();
	kinfo_init();
	vdd_cpu_voltage_store();

#if defined(CONFIG_OWLXX_PINCTRL)
	pinctrl_init_r();
#endif

#if defined(CONFIG_OWLXX_GPIO)
	owlxx_gpio_init();

	owlxx_speaker_init();
#endif

#if defined(CONFIG_OWLXX_POWER)
	atc260x_regulators_init();
#endif

#if defined(CONFIG_OWLXX_I2C)
	board_i2c_init(gd->fdt_blob);
#endif

#if defined(CONFIG_NCS8801)
	ncs8801_init();
#endif

#if defined(CONFIG_OWLXX_PWM)
	if (pwm_init(gd->fdt_blob))
		debug("%s: Failed to init pwm\n", __func__);
#endif

	return 0;
}

#ifdef CONFIG_OWLXX_POWER
int power_init_board(void)
{
	//low_power_dect();
	check_power();
	return 0;
}
#endif


#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_ACTE100
	rc = acte100_register(bis);
#endif
	return rc;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{

	int ret = 0;
	int host_type = 0;
	unsigned int boot_dev = get_sd_mmc_type();
	if(boot_dev< 0 || boot_dev>2){
		printf("%d:err\n",boot_dev);
		return -1;
	}	
	
	if( SD == boot_dev){
		host_type = SDC0_SLOT;
		printf("init sd card\n");	
	}else if(TSD == boot_dev){
		host_type = SDC0_SLOT;
		printf("init tsd card\n");	
	}else if(MMC == boot_dev){
		host_type = SDC2_SLOT;
		printf("init mmc card\n");
	}
	
	if(ret = acts_mmc_init(host_type,0,0)){
		printf("error :%s ,ret :%d\n",__FUNCTION__,ret);
	}

	return ret;
}
#endif

int misc_init_r(void)
{
	return 0;
}

int owlxx_in_recovery(void)
{
	if ( gd->flags & GD_FLG_RECOVERY )
		return 1;
/*
	char *e;

	e = getenv("mmcpart");
	if (!strcmp(e, RECOVERY_MMC_DEV))
		return 1;

	e = getenv("nandpart");
	if (!strcmp(e, RECOVERY_NAND_DEV))
		return 1;
*/
	return 0;
}


static void cmd_string_remove(char *o, char *s)
{
	char *m, *f, *p;
	unsigned int len;
	char str[64];

	p = s;
	while (p) {
		m = strstr(p, " ");
		if (m)
			len = m - p;
		else
			len = strlen(p);

		strncpy(str, p, len);
		str[len] = 0;

		f = strstr(o, str);
		if (f) {
			len = strlen(f);
			memmove(f, f + strlen(str), len - strlen(str));
			*(f + len - strlen(str)) = 0;
		}
		if (!m)
			break;

		while(m && isspace(*m))
			m++;
		p = m;
	}
}

void boot_append_remove_args(char *append, char *remove)
{
	void *blob = gd->fdt_blob;
	const char *cell;
	int node;
	char new_prop[256];

	if (!append && !remove)
		return;

	node = fdt_path_offset(blob, "/chosen");
	cell = fdt_getprop(blob, node, "bootargs", NULL);
	if (append)
		sprintf(new_prop, "%s %s", cell, append);
	else
		sprintf(new_prop, "%s", cell);

	if (remove)
		cmd_string_remove(new_prop, remove);
	debug("cmdline: %s\n", new_prop);

	fdt_setprop(blob, node, "bootargs", new_prop, strlen(new_prop) + 1);
}

int dual_logo_cfgval_get(void)
{
	const void *blob = gd->fdt_blob;	
	int node;
	node = fdt_path_offset(blob, "/dual_logo");
	if ( node  <  0 ) {
		printf("find node=dual_logo fail\n");
		return -1;
	}		
	return fdtdec_get_int(blob, node, "value", 0);
}


