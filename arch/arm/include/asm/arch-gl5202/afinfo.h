#ifndef __AFINFO_H__
#define __AFINFO_H__

#define DDR_CFG_OFFSET_IN_AFI           0x30
#define SPREAD_SPECTRUM_FLAG_IN_AFI     0x40

#if !(defined(_ASSEMBLER_) || defined(__ASSEMBLY__))

/* ddr_param_t, 16bytes */
typedef struct
{
    unsigned short ddr_clk;
    unsigned short dram_num;
	unsigned char single_ddr_width;
	unsigned char wr_dqs;
	unsigned char rd_dqs;
	unsigned char ddr_drv0;
	unsigned short dram_cap;  /* MB */

	unsigned char ddr_drv1;
	unsigned char freq_init;
	unsigned char ddr_type;
	unsigned char ddr_vddr;
	unsigned char ddr_misc; //bit0:1:dll on;0:dll off
	unsigned char ddr_auto_scan; /*bit0=1 ddr auto scan ddr para*/
}ddr_param_t;

#define SUPPORT_DDR_CDC_SCAN
#define AFINFO_CDC_SCAN_MAGIC       (0xcdc)

/* ddr cdc scan parameter */
typedef struct
{
    unsigned short magic;           /* magic code for verification */
    unsigned short adfu_timeout;    /* adfu product watchdog timeout, (s) */
    unsigned short mbrc_timeout;    /* android boot watchdog timeout, (s) */
} cdc_param_t;


/* Don't change these macro define */
#define BOARD_OPT_MAGIC             (0x706f)   /* 'op' */
#define BOARD_OPT_MAX_CNT           (6)

/* board_opt_t, size: 0x60 bytes */
typedef struct
{
    /* offs: 0x0 */
    unsigned short magic;           /* magic code for verification */
    unsigned char cur_opt;
    unsigned char opt_cnt;

    /* offs: 0x4, 2 bytes for a board opt */
    unsigned short opt_flag[BOARD_OPT_MAX_CNT];

    /* offs: 0x10 */
    ddr_param_t ddr_param[BOARD_OPT_MAX_CNT - 1];

    /* offs: 0x60 */
} board_opt_t;

#define GPIO_TYPE_NULL              (0x00)
#define GPIO_TYPE_OUTPUT_LOW        (0x10)
#define GPIO_TYPE_OUTPUT_HIGH       (0x11)
#define GPIO_TYPE_INPUT             (0x20)

/* gpio_cfg_t */
typedef struct
{
    unsigned char gpio;
    unsigned char type;
} gpio_cfg_t;

/* adc_key_t */
typedef struct
{
    unsigned short left;
    unsigned short right;
} adc_key_t;

#define KEY_NULL                    (0x00)
#define KEY_RECOVERY                (0x01)
#define KEY_ADFU                    (0x02)


/* uart_config, size: 0x8 bytes */
struct uart_config
{
    unsigned char enable;
    unsigned char chan;
    unsigned char group;
    unsigned char loglevel;
    unsigned long baudrate;
} ;


typedef struct
{
	/*! magic:'A''F''I'*/
	char magic[4];
    /*! sector为单位 = 2 */
    unsigned char blength;
    /*! descriptor type, afinfo type = 6 */
    unsigned char btype;

    unsigned char Reserve0[2];

    /*! Offset = 0x8 */
    unsigned char boot_dev;
    unsigned char Reserve0_1[7];

    /*! Offset = 0x10 */
    unsigned char berase_flash_flag;	// erase flash flag
    unsigned char probatch_restart_flag;	// flag of restart after probatch
    cdc_param_t   cdc_param;

    /*! Offset = 0x18 */
    unsigned int fixed_dvfslevel;

    /*! Offset = 0x1c */
    /* pcba, for nand rw test */
    unsigned char pcba_test;

    unsigned char Reserve1[3];

    /*! offset = 0x20, byte为单位 */
    struct uart_config   uart_config;
    unsigned char Reserve2[8];
	
    /*! offset = 0x30，byte为单位 */
	ddr_param_t		ddr_param;
	
    /*! offset = 0x40，byte为单位 */
	unsigned char spread_spectrum_flag;
    /*
     * VCC voltage selector:
     *  0: 3.1v; 1: 3.2v; 3: 3.3v;
     *  others: reserved
     */
 	unsigned char vcc_voltage_select;
	unsigned short displaypll;
	unsigned char Reserve6[12];
	
	/*! offset = 0x50, byte为单位*/
	unsigned char battery_ignore;
    unsigned char boot_key_scan_mode; // 0-combo key, 1-single key
    adc_key_t adc_key[9];
    unsigned char Reserve4[10];

    /* 0x80 */
    gpio_cfg_t  gpio_cfg[8];

    /* 0x90 */
    board_opt_t     board_opt;
    
    /* 0xf0 */
    unsigned char sn[0x10];

    /*! offset = 0x100，byte为单位 */
    unsigned char lsb_tbl[128]; /* readretry lsb table */
    unsigned char partition_info[160]; /* 分区信息 */
    unsigned char flash_id_table[64]; /* flash id  */
    
    
    
    unsigned char Reserved5[80];  /*  */
    
    
    
    unsigned int  DataBlkNumInBoot;  /* DataBlkNumPerZone*/
    unsigned int  ceconfig; /*nand ce gpio 配置*/
    unsigned short nandPadDrv; /*nand pad drv 配置*/
    unsigned char nandMaxClk; /*nand ce gpio 配置*/
    unsigned int  ceconfig_ex;  /*nand ce gpio externt use for 8 ce!*/
    unsigned char interleave_sw; //external interleave swither.
    unsigned int  nandReserved;
    unsigned int  atc260x_cfg;
    //unsigned char Reserve5[0x400-0x100-128-160-64-80-4];
    unsigned char Reserve5[0x400-0x100-128-160-64-80-4-4-2-1-4-1-4-4];//313 bytes reseverd
} __attribute__ ((packed)) afinfo_t;

/*! afinfo length，字节为单位 */
#define AFINFO_LENGTH   sizeof (afinfo_t)       //0x400, 字节为单位

/*! afinfo 的标志位 */
#define AFINFO_FLAG     0x0602

static inline int has_board_opt(afinfo_t *p_afinfo)
{
    if (p_afinfo == 0)
        return 0;

    if (p_afinfo->board_opt.magic == BOARD_OPT_MAGIC)
        return 1;

    return 0;
}


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

/* Global AFINFO pointer */
#define ACTS_AFINFO_PTR         ((afinfo_t const *)(gd->arch.p_afinfo))

#endif /* _ASSEMBLER_ */

#endif  //__AFINFO_H__

