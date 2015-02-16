/********************************************************************************
*                              usdk130
*                            Module: adfu
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved. 
*
* History:         
*      <author>    <time>           <version >             <desc>
*       huanghe     2008-06-20      1.0                    build this file 
********************************************************************************/
/*!
* \file			adfusever.h
* \brief		adfu引用外部的头文件汇总
* \author		黄河
* \par Copyright(c) 2001-2007 Actions Semiconductor, All Rights Reserved.
*
* \version		1.0
* \date			2008/6/20
*******************************************************************************/

#ifndef __ADFUSEVER_7021_H__
#define __ADFUSEVER_7021_H__


#include <common.h>
#include <android_image.h>
#include <asm/arch/adfu_protocol.h>
#include <asm/arch/adfu_brom_version.h>
#include <asm/arch/adfuserver_asm.h>

#if defined(CONFIG_ACTS_GL5207)
#include <asm/arch/usb/usb2/otg.h>
#include <asm/arch/usb/usb2/usb.h>
#include <asm/arch/usb/usb2/usb200.h>
#include <asm/arch/usb/usb2/usb2_io.h>
#include <asm/arch/usb/usb2/adfu_intr.h>
#include <asm/arch/usb/usb2/ep0.h>
#elif defined(CONFIG_ACTS_GL5202)
#include <asm/arch/usb/usb3/usb3_io.h>
#endif


/*! adfu的ic版本 */
#define	IC_VER	0

/*!
 *   adfu的调试打印宏，宏在__debug_sdk__.h中控制，1为使能，0取消 \n
 */
#define	 __DEBUG_ADFU__		1
#if __DEBUG_ADFU__
#define INFO(stuff...)		printf("adfu: <INFO>" stuff)
#else
#define INFO(stuff...)		do{}while(0)
#endif

/* kernel image & atag_data */
typedef struct
{
    /* android boot image header */
    struct andr_img_hdr *p_boot_img_hdr;

    /* MFP & GPIO XML string */
    void *p_xml_config;     /* config.xml */
    void *p_xml_mfps;       /* pinctrl.xml */
    void *p_xml_gpios;      /* gpiocfg.xml */
} adfus_dl_img_rec_t;

/*!
 *   adfudec的全局变量 \n
 */
typedef struct _s_adfudec_info
{
	/*! 系统的时钟记录，s为单位 */
    unsigned int	d_time_tick;
	
	/*! 系统运行出错的标志 */
    int			d_upgrade_error;
	
	/*! 系统运行结束的标志 */
    int			d_upgrade_over;
	
	/*! usb out0端点的最大包长度 */
    //unsigned int	d_out0_max_packet;
	
	/*! usb in0端点的最大包长度 */
    //unsigned int	d_in0_max_packet;
	
	/*! usb out1端点的最大包长度 */
    //unsigned int	d_out1_max_packet;
	
	/*! usb in2端点的最大包长度 */
    //unsigned int	d_in2_max_packet;
	
	/*! usb的disonnect位 */
    unsigned int	d_usb_discon;
	
	/*! usb的out1中断标志位 */
    //unsigned int	d_out1_int_flag;
	
	/*! 设备收到switch_fw命令的标志位 */
    unsigned int	d_switch_fw_flag;

	/*! 设备收到call_entry命令的标志位 */
    //unsigned int	d_call_entry_flag;
	
	/*! 设备从switch_fw切换过来后，先要enable_out1fifo_flag，out1端点才能继续接收数据 */
    //unsigned int	d_first_enable_out1fifo_flag;
	
	/*! sdram初始化后获得的，反映当前sdram的0xa的最高地址，如8MB==>0xa0800000 */
    unsigned int	d_largest_sdram_addr;
	
	/*! sdram检测后获得的，反映当前IC可以访问sdram的0xa的最高地址，如8MB，对应2135为0xa0200000 */
    unsigned int	d_real_largest_sdram_addr;
	
	/*! sdram检测出错的标志位 */
    unsigned int	d_check_error;
	
	/*! sdram检测出坏点的标志位 */
    unsigned int	d_check_warning;
	
	/*! call entry函数返回的数据长度 */
    unsigned int	d_my_ret_length;
	
	/*! 设备收到access_mem命令的次数 */
    unsigned int	d_access_mem_times;
	
	/*! 升级所需buffer的地址数组 */
    unsigned int *	pd_upgrade_buffer_addr;
    
    /*! sdram_pin_scan()的扫描结果 */
    int				d_sdram_pin_scan;

	/**/
	unsigned int  d_transfer_stage;
	
	unsigned int d_first_enable_out1fifo_flag;
	
} s_adfudec_info;

#define CBW_STAGE                       0x00
#define DATA_OUT_STAGE                  0x01
#define DATA_IN_STAGE                   0x02
#define CSW_STAGE                       0x03

/*********************for adfuserver******************************************/

/*********** 定义需要从堆中申请的buffer的长度 ***********/
/*! pd证书的长度 */
#define		PDC_LENGTH		0x400

/*! chipid的长度 */
#define		CHIPID_LENGTH		0x200

/*! call entry函数返回数据的最大长度 */
#define		RET_V0_LENGTH		0x200

/*! lfi head的长度 */
#define		LFI_HEAD_LENGTH				0x200

/*! 解密后的pdcert的长度 */
#define		DECRYPTED_PDC_LENGTH			0x400

/*! 暂态buffer */
#define		TEMP_LENGTH					0x200

/*! 解密lfi时用到的in buffer长度 */
#define		IN_BUFFER_LENGTH				0x8000

/*! 解密lfi时用到的out buffer长度 */
#define		OUT_BUFFER_LENGTH			0x4000

/*! flash读校验需要的最大buffer长度 */
#define		NAND_READ_CHECK_BUFFER_LENGTH	0x8000

/*! fwsc提供的nandflash驱动相对于fwsc链接地址的偏移量 */
#define		FWSC_DRIVER_ENTRY_ADDR_OFFSET	(0x600)

/*! 从fwsc入口地址算出nandflash驱动的入口地址的函数宏 */
#define		ConvertToNandDrv(addr)	(((unsigned int)(addr)) + FWSC_DRIVER_ENTRY_ADDR_OFFSET)

/*! lfi一次解密最大的数据量，sector为单位 */
#define		LFI_BUFFER_SECTOR_LENGTH	16

/*! lfi一次解密最大的数据量，byte为单位 */
#define		LFI_BUFFER_LENGTH			(LFI_BUFFER_SECTOR_LENGTH << 9)	

/*********** define error ***********/
/*! usb线强制拔出 */
#define	USB_PLUG_OUT_ERROR		1

/*! cbw长度不对 */
#define	USB_CBW_LENGTH_ERROR	2

/*! cbw的sign不对 */
#define	USB_CBW_SIGN_ERROR		3

/*! cbw未定义 */
#define	USB_UNKNOWN_CMD_ERROR	4

/*! in2端点传输错误 */
#define	USB_IN2_DATA_TRANSFER_ERROR		5

/*! out1端点传输错误 */
#define	USB_OUT1_DATA_TRANSFER_ERROR	6

/*! get actdrm命令的type错误 */
#define	USB_GET_ACTDRM_TYPE_ERROR	7

/*! set actdrm命令的type错误 */
#define	USB_SET_ACTDRM_TYPE_ERROR	8

/*! 0xb0命令的子命令未定义 */
#define	USB_DEF_SUB_CMD_ERROR		9

/*! poll ready命令的数据长度错误 */
#define	USB_SUB_POLL_READY_ERROR	10

/*! get status命令的数据长度错误 */
#define	USB_SUB_GET_STATUS_ERROR	11

/*! dsp初始化错误 */
#define	DSP_INIT_ERROR				12

/*! dsp升频错误 */
#define	DSP_RAISE_FREQUENCY_ERROR	13

/*! dsp输出chipid错误 */
#define	DSP_GET_CHIPID_ERROR		14

/*! dsp等待pd证书错误 */
#define	DSP_WAIT_PDCERTI_ERROR		15

/*! dsp解密pdcert错误 */
#define	DSP_DECRYPT_PDCERTI_ERROR	16

/*! dsp等待lfi错误 */
#define	DSP_WAIT_LFI_ERROR			17

/*! dsp解密头错误 */
#define	DSP_DECRYPT_HEAD_ERROR		18

/*! dsp解密lfi错误 */
#define	DSP_DECRYPT_LFI_ERROR		19

/*! fwinfo.bin中的mtp配置值错误 */
#define	FWINFO_MTP_ERROR			20

/*! write lfi命令的type错误 */
#define	SUB_WRITE_LFI_TYPE_ERROR	21

/*! access udisk命令的type错误 */
#define	ACCESS_UDISK_TYPE_ERROR		22

/*! sdram容量检测错误 */
#define	SDRAM_CHECK_CAP_ERROR		23

/*! sdram pin检测错误 */
#define	SDRAM_CHECK_CONTACT_ERROR	24

/*! sdram的坏buf太多 */
#define	SDRAM_CHECK_MORE_BAD_BUF_ERROR	25

/*! sdram的好buf太多，导致平均buf太小 */
#define	SDRAM_CHECK_MORE_GOOD_BUF_ERROR	26

/*! sdram的总容量不够 */
#define	SDRAM_CHECK_CAP_FOR_SYS_ERROR	27

/*! sdram在区分IC的时候碰到坏点 */
#define	SDRAM_CHECK_SOC_ERROR		28

/*! nandflash驱动接口返回错误 */
#define	NAND_DRV_RET_ERROR			29

/*! nandflash读回比较出错 */
#define	NAND_READ_CHECK_ERROR		30

/*! NO AFINFO */
#define	NO_AFINFO_ERROR		31

/*! sdram初始化出错 */
#define	SDRAM_INIR_ERROR		    (32)

/* define warning */
/*! sdram检测出坏点 */
#define	SDRAM_BAD_CELL_WARNING		1

#define USB_ERROR              33

/*********** define error_warning ***********/
/*! adfudec报错的type */
#define	WARNING_TYPE		1

/*! adfudec报警的type */
#define	ERROR_TYPE			2

/*********** backup sectors ***********/
/*! adfu升级时，pdcert区需要备份的数据长度，sector为单位 */
#define	LICENSE_BACKUP_SECTOR_LENGTH	58

/*! adfu升级时，pdcert区需要备份的数据长度，byte为单位 */
#define	LICENSE_BACKUP_LENGTH			(LICENSE_BACKUP_SECTOR_LENGTH << 9)

/*! adfu升级时，appinfo区需要备份的数据长度，sector为单位 */
#define	APPINFO_BACKUP_SECTOR_LENGTH	2

/*! adfu升级时，appinfo区需要备份的数据长度，byte为单位 */
#define	APPINFO_BACKUP_LENGTH			(APPINFO_BACKUP_SECTOR_LENGTH << 9)

/*! dev_id在pdcert区的偏移量，sector为单位 */
#define	DEVID_BACKUP_SECTOR_OFFSET		(LICENSE_BACKUP_SECTOR_LENGTH - 2)

/*********** define data transfer mode ***********/
/*! usb数据传输采用mips通道 */
#define USB_TRANS_MIPS_MODE     1

/*! usb数据传输采用dma通道 */
#define USB_TRANS_DMA_MODE      2

/*********** define sdram cap check mode ***********/
/*! 宽松模式，只是比较sdram容量大小是否满足系统的总体所需 */
#define	LOOSE_MODE		1

/*! 紧缩模式，比较sdram的可用buffer分布是否满足系统的平均所需 */
#define	CONTRACT_MODE		2

/*********** define delay para, for 60MHz of cclk ***********/
/*! 调试使用延时 */
#define DEBUG_DELAY				0x70000000

/*! dsp运行超时 */
#define WAIT_DSP_STAGE_DELAY	0x2000000

/*! out1端点busy超时 */
#define WAIT_USB_BUSY_DELAY		0x4000000

/*! 等待usb suspend超时 */
#define WAIT_USB_SUSPEND_DELAY	0x9000000

/*! 大约1.5s延时 */
#define DELAY_1s				0x800000

/*! 低频1s延时 */
#define LOSC_DELAY_1s			0x1000

/*! out1的busy操作延时 */
#define OUT1_FIFO_DELAY			6

/*********** define write mode ***********/
/*! write lfi函数的type， lfi0 */
#define	WRITE_LFI_TYPE0_MODE		1

/*! write lfi函数的type， user code */
#define	WRITE_USER_CODE_MODE		2

/*! write物理区的type，mbrc */
#define	WRITE_MBRC_MODE				3

/*! write物理区的type，brec */
#define	WRITE_BREC_MODE				4

/*! write lfi函数的type， lfi1 */
#define	WRITE_LFI_TYPE1_MODE		5

/******************************************************************************/
/*!                    
* \par  Description:
*		nandflash驱动，提供初始化、物理/逻辑、读/写、擦除等功能
* \param[in]	FunctionNum，访问模式
* \par	
*				#define   PHY_BLOCK_ERASE		0 \n
*				#define   PHY_SECTOR_READ       1 \n
*				#define   PHY_SECTOR_WRITE      2 \n
*				#define   LOG_INIT           	3 \n
*				#define   LOG_EXIT           	4 \n
*				#define   LOG_SECTOR_READ       5 \n
*				#define   LOG_SECTOR_WRITE      6 \n
*				#define   LOG_UPDATE         	7 \n
*				#define   LOG_SCAN_MODULE_MAIN  8 \n
*				#define   NAND_ERASE_ALL		9		
* \param[in]	BlockNum，物理区有效，指定mbrc、brec份数偏移量
* \param[in]    SectorNum，物理层有效，指定sector偏移量
* \param[in]    lba，逻辑层有效，指定sector偏移量
* \param[in]    srambuffer，缓冲区地址，接受0x8或0xa开头的地址，不接受其它地址
* \param[in]    length，读写的数据长度，扇区为单位。目前逻辑层有效，物理层只允许1个扇区操作
* \return       the result，0成功，其它fail。但LOG_EXIT操作时，1成功，其它fail
* \ingroup      fwsc
* \par          exmaple code
* \code 
*               fwsc_drv_entry(LOG_SECTOR_READ, 0, 0, 0, &lfi_head, 1);
* \endcode
*******************************************************************************/
typedef int (* fwsc_drv_entry) (unsigned char FunctionNum, unsigned short BlockNum, unsigned short SectorNum, unsigned int lba, unsigned char *srambuffer, unsigned int length);

/************* define physical LAYER*************/
/*!  */
#define   PHY_BLOCK_ERASE		0

/*!  */
#define   PHY_SECTOR_READ       1

/*!  */
#define   PHY_SECTOR_WRITE      2

/************* define logical LAYER *************/
/*!  */
#define   LOG_INIT           	3

/*!  */
#define   LOG_EXIT           	4

/*!  */
#define   LOG_SECTOR_READ       5

/*!  */
#define   LOG_SECTOR_WRITE      6

/*!  */
#define   LOG_UPDATE         	7

/*!  */
#define   LOG_SCAN_MODULE_MAIN  8

/*!  */
#define   NAND_ERASE_ALL		9


//static inline void local_irq_enable(void)
//{
//	asm volatile(
//		"	cpsie i			@ arch_local_irq_enable"
//		:
//		:
//		: "memory", "cc");
//}
//
//static inline void local_irq_disable(void)
//{
//	asm volatile(
//		"	cpsid i			@ arch_local_irq_disable"
//		:
//		:
//		: "memory", "cc");
//}



/******************************************************************************/
/*!                    
* \par  Description:
*		switch_fw命令的跳转函数
* \param		void
* \return       void   
* \ingroup      adfudec
* \par          exmaple code
* \code 
*               switch_func_entry();
* \endcode
*******************************************************************************/
typedef void (*switch_func_entry) (void *p_gd, void *p_afinfo);


/*! 从0x8地址转换为0xa地址的函数宏 */
#define ConvertToA(addr)	(((unsigned int)(addr)) | 0x20000000)

extern UNION_CBW	my_cbw; 
extern UNION_CSW	my_csw ;
extern switch_func_entry	my_switch_func_entry;
extern s_adfudec_info	my_s_adfudec_info;

#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
/* kernel download records */
extern adfus_dl_img_rec_t g_adfus_dl_img_rec;
#endif

#endif //__ADFUSEVER_H__

