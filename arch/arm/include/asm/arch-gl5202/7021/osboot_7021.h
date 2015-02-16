/***************************************************************************************
*                    GL5202 BROM include file-osboot.h
*                (c) Copyright 2012, Actions Co,Ld. 
*                        All Right Reserved 
*
* Description: Memory Address Definition
*
* History     :
*      <author>     <time>       <version >      <description>
*       GJ        2012/02/01       V1.0         create this file 
*       GJ        2012/03/01       V1.1         BROM Address modify
*       GJ        2012/03/08       V1.2         BROM Address modify
*       GJ        2012/06/15       V1.3         ADD SRAM_CPU0_SP_USR and SRAM_CPU0_SP_SVC Address 
******************************************************************************************/
/*测试宏定义*/ 
/* #define brom_debug */

#ifndef __OSBOOT_7021_H__
#define __OSBOOT_7021_H__

/*definiton for Watchdog,GL5202 brom不开启watchdog功能*/
#define Watchdog
#ifdef  Watchdog
#define WatchdogTime        2
#endif

#define SIMULATION                              (0)     //1: brom in simulation; 0:brom in FPGA or TapedOutIC
#define TAPED_OUT_IC                            (1)     //1: taped out ic ; 0: FPGA; use to config NAND_ANALOG_CTL_EN_NAND bit 

#define BROM_ERROR_REPORT

/*  Sharesram驱动地址分配　　*/
#define SRAM_START_ADDR                         0xB4068000   //Sharesram起始地址
//#define SRAM_DRV_ADDR                           0xB4068400   //NANDFLASH/SD/SATA Driver在Sharesram的地址
//#define SRAM_BRECLAUNCHER_ADDR                  0xB406AC00   //BRECLAUNCHER在Sharesram的地址
//#define SRAM_I2CDRV_ADDR                        0xB406B800   //I2C Driver在Sharesram的地址
//#define SRAM_ADFULAUNCHER_ADDR                  0xB406C000   //ADFU Driver在Sharesram的地址
#define _512byteMemAddr                         SRAM_START_ADDR

#define BRECLAUNCHER_STAR_ADDR                  0xFFFF1000
#define SDMMC_STAR_ADDR                         0xFFFF1C00
#define NAND_STAR_ADDR                          0xFFFF2A00
#define I2C_STAR_ADDR                           0xFFFF4200
#define ADFULAUNCHER_STAR_ADDR                  0xFFFF4A00
                                                
/*  堆栈指针  */                                
#define SRAM_CPU0_SP_IRQ                        0xB406fc80   //CPU0堆栈指针(IRQ)
#define SRAM_CPU0_SP_SYS                        0xB406fd00   //CPU0堆栈指针(SYS)
#define SRAM_CPU0_SP_USR                        0xB406fd80   //CPU0堆栈指针(USR)
#define SRAM_CPU0_SP_SVC                        0xB406fe00   //CPU0堆栈指针(SVC)
#define SRAM_CPU0_SP_FIQ                        0xB406fe80   //CPU0堆栈指针(FIQ)
#define SRAM_CPU0_SP_ABT                        0xB406ff00   //CPU0堆栈指针(ABT)
#define SRAM_CPU0_SP_UNDEF                      0xB406fff0   //CPU0堆栈指针(UNDEF)


#define SP_HEAD									0xb406fc00

//the flash controller driver
//#define SRAM_FLASH_LB_SPARE_START_BYTE_ADDR     0xB4067FF8
//#define SRAM_FLASH_DISABLE_ECC_ADDR             0xB4067FFA
//#define BROM_FLASH_LB_SPARE_START_POS   2048

/* SRAM_FlashDisableECC_ADDR value bits define */
#define FLASH_DISABLE_ECC                       (1 << 0)
#define FLASH_ENABLE_RANDOMIZER                 (1 << 1)
#define BROM_FLASH_LB_SPARE_START_POS           8192
#define BROM_CONFIG_T_ECC_RND                   1
#define BROM_CONFIG_T_PAGE_SIZE                 2

/*  BROM中的各驱动的地址偏移和长度 */
#define BROM_POWERON_OFFSET                     0
#define BROM_POWERON_SIZE                       4096  /* 4*1024 */           
#define BROM_BRECLAUNCHER_OFFSET                (BROM_POWERON_OFFSET + BROM_POWERON_SIZE)
#define BROM_BRECLAUNCHER_SIZE                  3072  /* 3*1024 */ 
#define BROM_SDMMCDRV_OFFSET                    (BROM_BRECLAUNCHER_OFFSET + BROM_BRECLAUNCHER_SIZE)
#define BROM_SDMMCDRV_SIZE                      3584  /* 3.5*1024 */
#define BROM_NANFDRV_OFFSET                     (BROM_SDMMCDRV_OFFSET + BROM_SDMMCDRV_SIZE)
#define BROM_NANFDRV_SIZE                       6144  /* 6*1024 */  
#define BROM_E2PROMDRV_OFFSET                   (BROM_NANFDRV_OFFSET + BROM_NANFDRV_SIZE)
#define BROM_E2PROMDRV_SIZE                     2048  /* 2*1024 */
#define BROM_ADFULAUNCHER_OFFSET                (BROM_E2PROMDRV_OFFSET + BROM_E2PROMDRV_SIZE)
#define BROM_ADFULAUNCHER_SIZE                  12800 /* 12.5*1024 */

/*  记录引导出错的原因和节点  */
#define BROM_FAIL_CAUSE_ADDR                    0xB407F400

/*  异常处理跳转到Remap_SRAM地址  */
#define Undefined_Handler_ADDR                  0xFFFF8104
#define SWI_Handler_ADDR                        0xFFFF8108
#define Prefetch_Handler_ADDR                   0xFFFF810C
#define Abort_Handler_ADDR                      0xFFFF8110
#define Reserved_Handler_ADDR                   0xFFFF8114
#define IRQ_Handler_ADDR                        0xFFFF8118
#define FIQ_Handler_ADDR                        0xFFFF811C
/* 存储的调回函数地址 */
#define Undefined_Handler_WRADDR                0xFFFF8154
#define SWI_Handler_WRADDR                      0xFFFF8158
#define Prefetch_Handler_WRADDR                 0xFFFF815c
#define Abort_Handler_WRADDR                    0xFFFF8160
#define Reserved_Handler_WRADDR                 0xFFFF8164
#define IRQ_Handler_WRADDR                      0xFFFF8168
#define FIQ_Handler_WRADDR                      0xFFFF816c

/*   CPU1/2/3 WFE处理的Flag和跳转地址 defined in actions_reg_leopard.h*/


/*  nanfdrv 入口函数*/
#define Brom_NANFInit                          (0x01)
#define Brom_NANFEnable                        (0x02)
#define Brom_NANFDisable                       (0x03)
#define Brom_NANFLBSectorRead                  (0x04)
#define Brom_NANFSBSectorRead                  (0x05)
#define Brom_NANFBrecCheckSum512B              (0x06)
#define Brom_NANFReset                         (0x07)
#define Brom_NANFBrecCheckSum                  (0x08)
#define Brom_NANFExit                          (0x09)
#define Brom_NANFLBSectorRead_LBA              (0x0a)
#define Brom_NANFLBPageRead                    (0x0b)
#define Brom_NANFLBSectorRead_TTlc	           (0x0c)
#define Brom_NANFSelectIFMode		               (0x0d)
#define Brom_NANFGetIFMode			               (0x0e)
#define Brom_NANFSetParam			                 (0x0f)  //2012-4-24 foxhsu
#define Brom_NANFGetParam			                 (0x10)  //2012-4-24 foxhsu

/*  nanfdrv bus width   */
#define NANFBusWidth_8bit               0
#define NANFBusWidth_16bit              1

/*  sdmmc 卡入口函数*/
#define Brom_SDMMCSel                          1
#define Brom_SDMMCPowerOn                      2
#define Brom_SDMMCInit                         3
#define Brom_SDMMCRead                         4
#define Brom_SDMMCPowerOff                     5
#define Brom_isMMCCard                         6
#define Brom_eMMCSwitch                        7
                                               
#ifdef  brom_debug                             
#define Brom_SDMMCWrite                        8
#endif

/*   sata 入口函数*/
//#define Brom_SATAEntry                         1
//#define Brom_SATAInit                          2
//#define Brom_SATARead                          3
//#define Brom_SATAPowerOff                      4
//#define Brom_SATAExit                          5
//                                               
//#ifdef  brom_debug                             
//#define Brom_SATAWrite                         6
//#endif

/*  i2c 入口函数*/
#define Brom_E2PROMInit                        1
#define Brom_E2PROMDetect                      2
#define Brom_E2PROMTryType                     3
#define Brom_E2PROMRead                        4
#define Brom_E2PROMExit                        5   

#ifdef  brom_debug                                  
#define Brom_E2PROMWrite                       6
#endif

#define CP_STATUS_MASK                  0x42
#define TP_STATUS_MASK                  0x21
#define NP_STATUS_MASK                  0x41
#define ER_STATUS_MASK                  0x51
#define RD_STATUS_MASK                  0x41

#define CheckSum_Offset                 0x1234

#endif  /*  __OSBOOT_H__ */
