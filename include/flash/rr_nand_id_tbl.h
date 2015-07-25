/********************************************************************************
*                              USDK130
*                            ucOS + MIPS,Nand Module
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
*       mengzh     2007-10-17 9:22     1.0             build this file
********************************************************************************/
/*!
* \file     nand_smode_map_tbl.c
* \brief    The flash map talbe for secure rw mode
* \author   foxhsu
* \par      This module defines the flash id tables
* \par      EXTERNALIZED FUNCTIONS:
*               这里描述调用到外面的模块
*
*      Copyright(c) 2001-2007 Actions Semiconductor, All Rights Reserved.
*
* \version 1.0
* \date  2011-3-7
*******************************************************************************/
#ifndef __RR_NAND_ID_TBL_H__
#define __RR_NAND_ID_TBL_H__

//#include "nand_flash_driver_type.h"
#include "types.h"

//#define SUPPORT_SHOW_FLASH_NAME

#define LSB_PAGE_MAP_TBL_SIZE          (128+7)  //7 byte for aligned
#define MAX_READ_RETRY_TBL_SIZE            (160+1) //1 byte for aligned
#define MAX_READ_RETRY_REG_ADDR_CNT         (16)
#define MAX_LSB_ENHANCE_REG_ADDR_CNT         (16)



#define OPT_NEED_PRE_CONDITION         (1<<0)    //READ RETRY前需预置环境
#define OPT_NEED_TERMINATE_CONDITION   (1<<1)    // 需特殊命令退出 READ RETRY MODE
#define OPT_NEED_SEQUENCE_REGS_GET      (1<<2)    //寄存器需连续读
#define OPT_SUPPORT_LSB_ENHANCE         (1<<3)    //是否支持lsb enhance
#define OPT_NEED_SEQUENCE_REGS_SET      (1<<4)    //寄存器需连续写
#define OPT_NEED_SET_REG_END_CMD       (1<<6)     //SET REG 后需加特殊命令结束
#define OPT_NEED_GET_REG_END_CMD       (1<<7)      //GET REG 后需加特殊命令结束
#define OPT_RR_NEED_ADD_CMD            (1<<9)                   //RR 需附加读取命令
#define OPT_RR_ADDR_SAMPLE_BY_ALE   (1<<10)     //REG ADDR 需作为地址信号传输
#define OPT_TLC_NAND                (1<<11)     //Distinguish is tlc/mlc flash
#define OPT_RRT_IN_OPT				(1 << 12)	//read retry info is stored in nand opt.
#define OPT_DUMMY_READ				(1 << 13)	//dummy read to terminate enhance mode.

struct SpecicalRrCmdType{
    UINT8      PreConditionCmd[4];                    //the command for read retry environment prepare
    UINT8      TerminateRrCmd[2];                  //the command for read retry environment terminate
    UINT8      SetRegCmd[5];                    //the command for   setting  reg
    UINT8      GetRegCmd[5];                 //the command for   getting  reg
    UINT8      RrAddedCmd[2];                  // toshiba read retry 需于00~30 命令前加0x260x5d
	/*add for hynix 20 nm mlc*/
	UINT8		GetOTPAddr0[2];			/*hynix 2x nm mlc get otp param command*/
	UINT8		GetOTPData0[2];			/*hynix 2x nm mlc get otp param command*/
	UINT8		GetOTPCmd1[3];			/*hynix 2x nm mlc get otp param command*/
	UINT8		GetOTPAddr1[5];			/*hynix 2x nm mlc get otp param command*/
	UINT8		GetOTPCmd2[1];			/*hynix 2x nm mlc get otp param command, END*/
	UINT8       Reserved [1];         //Reserved bytes for Expansion and 32 bytes align
};


/*each use 512 bytes*/
typedef struct rr_flash_attr_info_s{
    UINT8   NandChipId[8];
    UINT32  OperationOpt ;                                 //Behavior Properties
    UINT8   MaxReadRetryTimes;                          //最大读取次数
    UINT8   BytesPerRegAddr;                            //寄存器地址字节长度
    UINT8   ReadRetryRegsCnt;                          //read retry reg addr count
    UINT8   ReadRetryTbl[MAX_READ_RETRY_TBL_SIZE];           //read retry table
    UINT16   ReadRetryRegAddr[MAX_READ_RETRY_REG_ADDR_CNT];  //read retry reg addr assigned
    UINT32   ReadRetryValFixedBitmap;                  // Fixed value bitmap for rr reg set 1: FIXED 0:NOT FIXED
    UINT8   LsbEnhanceTbl[MAX_LSB_ENHANCE_REG_ADDR_CNT];           //lsb enhance  value table
    UINT8   LsbEnhanceRegsAddr[MAX_LSB_ENHANCE_REG_ADDR_CNT];    //regs used to lsb enhance
    UINT32   LsbEnhanceValFixedBitmap;                  // Fixed value bitmap for lsb enhance reg set
    UINT8   LsbEnhanceRegsCnt;                            //the cnt of regs involved in lsb enhance
    UINT8   SmodeMap[LSB_PAGE_MAP_TBL_SIZE];                // lsb mode page mapping table
    struct  SpecicalRrCmdType    RrSpecialCmd;           // special cmd used to tlc support
#ifdef  SUPPORT_SHOW_FLASH_NAME

    UINT8   mark[96];
#endif /*SUPPORT_SHOW_FLASH_NAME*/
} __attribute__((packed))  rr_flash_attr_info_t;

struct FlashRrInfoTblHeader
{
    UINT16 WLength;                               /* File length (Bytes) */
    UINT8  HeaderLengthInByte;                          /* File header length (sectors), now fixed at 1 */
    UINT8  HeaderType;                                 /* File header type，now fixed at 9 */
    UINT16 IdLength;                               /* flash id item length, now fixed at 512 bytes */
    UINT8  Version;                               /* id define verion */
    UINT8  Rsv1[0x10-7];                           /* pad to 16 bytes */
    UINT8  MergeBitMap[0x40];                     /* merge的bitmap, every bit for a bytes in item,
                                                 * '0' means reserve the old value
                                                 * '1' means set to the new value
                                                 */
    UINT8  Rsv2[0x200-0x10-0x40];
};


#endif /*__NAND_TLC_TBL_H__*/
