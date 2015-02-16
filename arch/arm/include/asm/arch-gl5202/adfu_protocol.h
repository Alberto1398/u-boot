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

#ifndef  _ADFU_PROTOCOL_H_
#define  _ADFU_PROTOCOL_H_

#pragma pack(1) //force sizeof() to byte alligned

//adfu:

/***************adfu command********************/
#ifndef FOR_OLD_PROTOCOL_OF_ADFU 
#define ACCESS_INTERNAL_RAM     0x13
#define ACCESS_UDISK_SPACE      0x60
#else
#define ACCESS_INTERNAL_RAM    0x05
#define ACCESS_UDISK_SPACE     0x08
#endif

#define EXTENDED_CMD            0xcd
#define SWITCH_FW              	0x20
//#define SWITCH_FW              0x10
#define GET_ACTDRM_INFO        0xca
#define SET_ACTDRM_INFO        0xc9
#define HANDLE_ACTDRM_INFO     0xb0

#define ACCESS_UDISK_TYPE_0		0x0
#define ACCESS_UDISK_TYPE_1		0x1

//sub cmd of handle actdrm INFO
#define SUB_DETACH_DEVICE   	0x00
#define SUB_POLL_READY      	0x01
#define SUB_GET_STATUS      	0x02
#define SUB_CALL_ENTRY      	0x03
#define SUB_WRITE_MBRC      	0x06
#define SUB_WRITE_BREC      	0x07
#define SUB_WRITE_LFI       	0x08
#define SUB_WRITE_USER_CODE 	0x0a

#define SUB_WRITE_LFI_TYPE0       	0x0
#define SUB_WRITE_LFI_TYPE1       	0x1

//sub cmd of default pdc
#define DEF_SUB_DETACH_DEVICE       0xB586
#define DEF_SUB_POLL_READY          0x6AAE
#define DEF_SUB_GET_STATUS          0xC480
#define DEF_SUB_CALL_ENTRY          0x4658
#define DEF_Set_Device_ID_CMD       0xC3AB
#define DEF_Get_Device_ID_CMD       0x9DCC
#define DEF_SUB_WRITE_MBRC          0x6F38
#define DEF_SUB_WRITE_BREC          0x5BE7
#define DEF_SUB_WRITE_LFI           0x6A1B
#define DEF_Set_Device_Clock_CMD    0xC27C
#define DEF_SUB_WRITE_USER_CODE     0x1D57

#define DEF_SUB_COUNTS			    11

//actdrm_info type
#define TYPE_DEVICE_CERTIFICATION	0
#define TYPE_DEVICE_ID				1
#define TYPE_PDCDEC_DSP				2
#define TYPE_DECRYPT_PDC			3
#define TYPE_ERASE_PDC				4
#define TYPE_LFI_HEAD				5

/***************cbw: 31 bytes********************/
typedef struct _CBW
{
	unsigned long	dCBWSignature;
	unsigned long   dCBWTag;
	unsigned long   dCBWDataTransferLength;
	unsigned char	bmCBWFlags;
	unsigned char	bReserved[2];
	unsigned char	bOperationCode;
	unsigned char	cCBWCB[15];
} CBW;

typedef union _UNION_CBW
{
	unsigned long all[8];
	CBW		 sCbw;
} UNION_CBW;


#define CBW_LENGTH  sizeof(CBW)
#define CBW_SIGNATURE   0x43425355

/***************csw: 13 bytes********************/
typedef struct _CSW{
	unsigned long    dCSWSignature;
	unsigned long    dCSWTag;
	unsigned char    bReserved[4];
	unsigned char    bCSWStatus;
} CSW;

typedef union _UNION_CSW
{
	unsigned long all[4];
	CSW		 sCsw;
} UNION_CSW;

#define CSW_LENGTH  sizeof(CSW)
#define CSW_SIGNATURE  0x53425355
#define CSW_STATUS_COMMAND_PASS  0
#define CSW_STATUS_COMMAND_FAIL  1

/***************adfu error********************/
typedef struct _error_warning_config_info
{
	unsigned char	blength;	// Size of this descriptor in bytes
	unsigned char	btype;	// type of this descriptor
	unsigned short	wtotal_length;	// Total length of data returned for this descriptors
	unsigned short	wLanguageID;	//支持的语言类型，由USBIF制定。当前仅支持美式英语
	unsigned char	berror_num;	//number of errors
	unsigned char	bwarning_num;	//number of warnings
	unsigned char	berror_type[5];
	unsigned char	bwarning_type[3];
}error_warning_config_info;
#define	ERROR_WARNING_CONFIG_INFO_LENGTH	sizeof(error_warning_config_info)

typedef struct _error_warning_data_info
{
	unsigned char blength;	// Size of this descriptor in bytes
	unsigned char btype;	// type of this descriptor
	unsigned char bsub_type;	//type of error or warning
	unsigned char data_string[29];		//error or warning的描述字串
}error_warning_data_info;
#define	ERROR_WARNING_DATA_INFO_LENGTH	sizeof(error_warning_data_info)

typedef struct _error_warning_info
{
	error_warning_config_info	s_error_warning_config_info;
	error_warning_data_info		s_error_warning_data_info[8];
}error_warning_info;
#define	ERROR_WARNING_INFO_LENGTH	sizeof(error_warning_info)

/***************descriptor type********************/
#define ERROR_WARNING_CONFIG_INFO_TYPE    0x2
#define ERROR_INFO_TYPE		0x3
#define WARNING_INFO_TYPE   0x4
#define FWINFO_TYPE   0x5
#define AFINFO_TYPE   0x6

/***************error type********************/
#define MBRC_ERROR_TYPE    			0x1
#define BREC_ERROR_TYPE				0x2
#define NANDFLASH_SCAN_ERROR_TYPE   	0x3
#define NANDFLASH_TABLE_ERROR_TYPE   0x4
#define MCU_INTERNAL_ERROR_TYPE   	0x5
#define MCU_PIN_ERROR_TYPE   		0x6
#define SDRAM_PIN_ERROR_TYPE   		0x7
#define SDRAM_CAPACITY_SMALL_ERROR_TYPE   0x8
#define NANDFLASH_TYPE_ERROR_TYPE   	0x9

/***************warning type********************/
#define SDRAM_BAD_CELL_WARNING_TYPE			0x1

/***************language ID********************/
#define CHINESE_TAIWAN_ID			0x0404
#define CHINESE_PRC_ID				0x0804
#define ENGLISH_UNITED_STATES_ID	0x0409

#pragma pack()

#endif  //_ADFU_PROTOCOL_H_
