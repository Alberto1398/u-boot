#ifndef __AFINFO_H__
#define __AFINFO_H__

#define DDR_CFG_OFFSET_IN_AFI	0x30

#if !(defined(_ASSEMBLER_) || defined(__ASSEMBLY__))

/* ddr_param_t, 16bytes */
typedef struct
{
	unsigned short ddr_clk;
	unsigned short ddr_cap;
	unsigned char ddr_bits_width;
	unsigned char rank;
	unsigned char ddr_type;
	unsigned char dual_chan;
	unsigned char rdodt;
	unsigned char wrodt;
	unsigned char zpzq;
	unsigned char Reserve0[4];
} __attribute__ ((packed)) ddr_param_t;

#define SUPPORT_DDR_CDC_SCAN
#define AFINFO_CDC_SCAN_MAGIC       (0xcdc)

/* ddr cdc scan parameter */
typedef struct
{
	unsigned short magic;           /* magic code for verification */
	unsigned short adfu_timeout;    /* adfu product watchdog timeout, (s) */
	unsigned short mbrc_timeout;    /* android boot watchdog timeout, (s) */
} cdc_param_t;

typedef struct
{
    unsigned char Major;
    unsigned char Minor;
} version_t;

#define CPU_PWM_VOLT_TABLE_LEN 16
struct cpu_pwm_volt_table {
	unsigned int pwm_val;
	unsigned int voltage_mv;
};

typedef struct
{
	/*! magic:'A''F''I'*/
	char magic[4];
	/*! sector为单位 = 2 */
	unsigned char blength;
	/*! descriptor type, afinfo type = 6 */
	unsigned char btype;    
	
	unsigned char reserve2;
	unsigned char reserve3;
	unsigned char boot_dev;
	
	unsigned char flash_read_check;
	version_t version;
	
    unsigned char Reserve0[4];

	/*! Offset = 0x10 */
	unsigned char berase_flash_flag;	// erase flash flag
	unsigned char probatch_restart_flag;	// flag of restart after probatch
	cdc_param_t   cdc_param;
    unsigned char Reserve1[4];

	unsigned char key_mode; // 0 = single key , 1 =much key 
	/*card burn flag bit0=1 is cardburn, bit1=0 is nand burn, bit1=1 emmc burn(uboot auto detect)*/
	unsigned char burn_flag; 
	unsigned char burn_uart;  /*cfg card burn debug log to uart index, 0-5*/
	
	unsigned char loglevel;  // loglevel

	/*! offset = 0x20, byte */
	unsigned long uart_baudrate;       //38400(0x9600)
	unsigned char uart_index;        //0: uart0; 1: uart1; 2: uart2. >7 diable
	unsigned char jtag_mode;        //0: jtag disable; 1: ksoutin-jtag-out; 2: sd0-jtag-out
	unsigned char jtag_wait;
	unsigned char s2ddr_check;      //check ddr checksum when resume, 0: not check ; 1: check
	unsigned char Reserve2[4];
	
	/*! offset = 0x2c，byte为单位 */
	unsigned long clk_spead;		// setting spread spectrum for EMI
	
	/*! offset = 0x30，byte为单位 */
	ddr_param_t		ddr_param;
	/*! offset = 0x3f，byte为单位 */
	unsigned char is_product;

    /*! offset = 0x40，byte为单位 */
	unsigned long corepll;
	unsigned long devpll;
	unsigned long buspll;
	unsigned long display;

	/*! offset = 0x50, byte为单位*/
	unsigned char bus_id;    /*0x50 PMU挂接的BUS号. */
	unsigned char pmu_id;	 /*0x51 PMU类型, see OWLXX_PMU_ID_ATC2603A ... */
	unsigned char pmu_dcdc_en_bm;  /*0x52 DCDC boot enable bitmap, 1-set-enable, 0-not-change. */
	unsigned char pmu_dcdc_cfg[7]; /*0x53 DCDCx voltage config. */
	unsigned short pmu_ldo_en_bm;   /*0x5a LDO boot enable bitmap, 1-set-enable, 0-not-change. */
	unsigned char Reserve4_not_used[4];	/*0x5c*/
	unsigned long battery_low_cap;	/*0x60*/
	unsigned long gauge_bus_id; /*0x64 gauge bus id*/
	unsigned long gauge_id;		/*0x68*/
	unsigned long reserve_pwm;	/*0x6c*/
	unsigned long power_style;	/*0x70 0: DCIN+USB  1: USB  2:USB wtith GPIO control*/
	unsigned long charger_mode;	/*0x74 0: no charger boot mode  1: have charger mode*/
	unsigned char bl_off_current_usb_pc;	/*0x78*/
	unsigned char bl_off_current_usb_adp;	/*0x79*/
	unsigned char bl_off_current_wall_adp;	/*0x7a*/
	unsigned char ext_charger;				/*0x7b*/
	unsigned char extchg_gpio;				/*0x7c*/
	unsigned char extchg_gpio_flag;			/*0x7d*/
	unsigned char support_adaptor_type;		/*0x7e*/
	unsigned char wallctl_gpio;				/*0x7f*/
	unsigned char wallctl_gpio_flag;		/*0x80*/
	unsigned char otg_gpio;					/*0x81*/
	unsigned char pmu_ldo_cfg[12]; /*0x82 LDOx voltage config. offset=0x82 */
	unsigned char Reserve4[0x90-0x50-50-12]; /*0x8e 2! */

	/* offset = 0x90, ddr test */
	unsigned long mtest_start;				/*0x90*/
	unsigned long mtest_end;				/*0x94*/
	unsigned long mtest_pattern;			/*0x98*/
	unsigned long mtest_limit;				/*0x9c*/
    
	unsigned char Reserve7[0xf0-0x90-0x10-0x10-0x10];	/*0xa0*/
	
	/* offset = 0xd0*/
	unsigned long bus1pll;					/*0xd0*/
	unsigned long pll_reserve[3];			/*0xd4*/

	/* offset = 0xe0*/
	unsigned long pwm1_dcdc_config;			/*0xe0*/
	unsigned long pwm2_dcdc_config;			/*0xe4*/
	unsigned long pwm_reserve[2];			/*0xe8*/
	
	/* offset = 0xf0, sn */
	unsigned char sn[0x10];					/*0xf0*/

    /*! offset = 0x100，byte为单位 */
    unsigned char lsb_tbl[128]; /*0x100 readretry lsb table */
    unsigned char partition_info[160]; /* 分区信息 */  //off 0x180
    unsigned char flash_id_table[64]; /* flash id  */  //off 0x220
    unsigned char Reserved5[80];  /*  */ //off 0x260
    unsigned int  DataBlkNumInBoot;  /* DataBlkNumPerZone*/  //off 0x2b0
    unsigned int  ceconfig; /*nand ce gpio 配置*/            //off 0x2b4
    unsigned short nandPadDrv; /*nand pad drv 配置*/         //off 0x2b8
    unsigned char nandMaxClk; /*nand ce gpio 配置*/          //off 0x2ba
    unsigned int  ceconfig_ex;  /*nand ce gpio externt use for 8 ce!*/     //off 0x2bb
    unsigned char interleave_sw; //external interleave swither.            //off 0x2bf

	/* offset = 0x2c0, store cpu voltage test table*/
	unsigned int cpu_pwm_volt_tb_len;
	struct cpu_pwm_volt_table cpu_pwm_volt_tb[CPU_PWM_VOLT_TABLE_LEN];

	unsigned char Reserve6[0x400-0x2c0-4-CPU_PWM_VOLT_TABLE_LEN*8 - 4];
	int dvfs;
} __attribute__ ((packed)) afinfo_t;

/*! afinfo length，字节为单位 */
#define AFINFO_LENGTH   sizeof (afinfo_t)       //0x400, 字节为单位

/*! afinfo 的标志位 */
#define AFINFO_FLAG     0x0602


#ifdef SUPPORT_DDR_CDC_SCAN
static inline int is_cdc_mode(afinfo_t *p_afinfo)
{
    if (p_afinfo == 0)
        return 0;

    if (p_afinfo->cdc_param.magic == AFINFO_CDC_SCAN_MAGIC)
        return 1;

    return 0;
}

#else
static inline int is_cdc_mode(afinfo_t *p_afinfo)
{
    return 0;
}
#endif



/*******************/
#include <common.h>

struct gamma_info {
	unsigned int gamma_table[CONFIG_SYS_GAMMA_SIZE / 4];
	int is_valid;
};

struct cpu_pwm_volt_info {
	struct cpu_pwm_volt_table cpu_pwm_volt_tb[CPU_PWM_VOLT_TABLE_LEN];
	int cpu_pwm_volt_tb_len;
};

struct kernel_reserve_info {
	struct gamma_info gamma;
	struct cpu_pwm_volt_info cpu_pwm_volt;
} __attribute__ ((packed));

extern struct kernel_reserve_info *kinfo;

extern void kinfo_init(void);



#endif /* _ASSEMBLER_ */

#endif  //__AFINFO_H__

