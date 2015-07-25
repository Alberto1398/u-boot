/*******************************************************************************
 *                              Nand Flash
 *                            Module: Storage
 *                 Copyright(c) 2003-2009 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       kyle.yu    2011-08-03           0.1             first verstion
*******************************************************************************/
/*!
 * \file     event_log.h
 * \brief    declare of event log operations
 * \author   kyle
 * \version 1.0
 * \date  2011/08/03
*******************************************************************************/

/********************************************************************************
* Event_log_item_t is basic event log type. All other event are member of its -log,
* such as fw_event_log_t or dbg_event_log_t.
*
* event_log_item_t offer basic read and write operations. The sub-type event log can
* use those directly.
*
*  |------------------|                 |------------------|
*  | event_log_item_t |       |----> 0. |  fw_event_log_t  |
*  --------------------       |	        --------------------
*  |      -type       |       |         |    -fw_event_t   |
*  |------------------|       |         |------------------|
*  |      -sequence   |       |         |    -reserved     |
*  |------------------|       |         |------------------|
*  |      -stamp      |       |
*  |------------------|       |       								   |-->2. info_event_log_t
*  |      -log        |-----> |                                        |
*  |------------------|       |----> -. (union)string_event_log_t----->|
*                             |                                        |
*                             |                                        |-->3. dbg_event_log_t
*                             |
*                             |----> 1. critical_event_log_t
********************************************************************************/
#ifndef __NAND_EVENT_LOG_H__
#define __NAND_EVENT_LOG_H__

//#include "flash_driver_config_default.h"
/*----------------------------------------------------------------------*/
/*   Event Type Constant	                                   			*/
/*----------------------------------------------------------------------*/
#define EVENT_TYPE_FIRMWARE			(0x00)  /*firmware event*/
#define EVENT_TYPE_CRITICAL			(0x01)	/*critical event*/
#define EVENT_TYPE_INFO				(0x02)	/*log info*/
#define EVENT_TYPE_DBG				(0x03)	/*debug info*/
#define EVENT_TYPE_INVALID			(0xFF)	/*invalid type.*/
#define EVENT_LOG_FLUSH				(0xAA)	/*Force flush event log page buffer*/

/*----------------------------------------------------------------------*/
/*   Event Size Constant	                                   			*/
/*----------------------------------------------------------------------*/
#define EVENT_ITEM_SIZE					(4096) /*<= SuperPage, change this if needed!*/
#define EVENT_ITEM_DISCRIP_INFO_SIZE	(40)	/*count the _event_log_item_t*/
#define EVENT_LOGINFO_SIZE				(EVENT_ITEM_SIZE - EVENT_ITEM_DISCRIP_INFO_SIZE)


/*----------------------------------------------------------------------*/
/*   FIRMWARE Event Type Constant	                                   	*/
/*----------------------------------------------------------------------*/
#define FW_SUB_SHIFT				(16)
#define FW_SUB_MASK					(0xFF)
#define FW_SUB_LFIHEAD				(0)	/*lfi head*/
#define FW_SUB_LFIDIR				(1)	/*lfi dir*/
#define FW_SUB_LFI					(2)	/*lfi*/
#define FW_SUB_APP					(3)	/*app*/

#define EVENT_FW_SHIFT				(0)
#define EVENT_FW_MASK				(0xFF)
#define EVENT_FW_CORRUPTED			(0x0)	/*firmware wreck*/
#define EVENT_FW_FIXED				(0x1)	/*firmware fixed*/

/*For user use outside*/
#define EVENT_FW_LFIHEAD_CORRUPTED	(EVENT_FW_CORRUPTED | (FW_SUB_LFIHEAD << FW_SUB_SHIFT))
#define EVENT_FW_LFIHEAD_FIXED		(EVENT_FW_FIXED | (FW_SUB_LFIHEAD<< FW_SUB_SHIFT))
#define EVENT_FW_LFIDIR_CORRUPTED	(EVENT_FW_CORRUPTED | (FW_SUB_LFIDIR << FW_SUB_SHIFT))
#define EVENT_FW_LFIDIR_FIXED		(EVENT_FW_FIXED | (FW_SUB_LFIDIR<< FW_SUB_SHIFT))
#define EVENT_FW_LFI_CORRUPTED		(EVENT_FW_CORRUPTED | (FW_SUB_LFI << FW_SUB_SHIFT))
#define EVENT_FW_LFI_FIXED			(EVENT_FW_FIXED | (FW_SUB_LFI<< FW_SUB_SHIFT))
#define EVENT_FW_APP_CORRUPTED		(EVENT_FW_CORRUPTED | (FW_SUB_APP << FW_SUB_SHIFT))
#define EVENT_FW_APP_FIXED			(EVENT_FW_FIXED | (FW_SUB_APP<< FW_SUB_SHIFT))
#define EVENT_FW_LOG_FLUSH			(0xFFFFAAAA)	/*confirm write down the event log to flash*/

#define CALC_FW_SUB_IDX(x)			((x >> FW_SUB_SHIFT) & FW_SUB_MASK)
#define CALC_FW_EVENT(x)			(x & EVENT_FW_MASK)

/*----------------------------------------------------------------------*/
/*   Event Property Constant	                                   		*/
/*----------------------------------------------------------------------*/
#define EVENT_PROPERTY_ORDINAL				(0x1 << 1)
#define EVENT_PROPERTY_IRREPEALABLE			(0x1 << 2)

/*----------------------------------------------------------------------*/
/*   Event Type Constant	                                   			*/
/*----------------------------------------------------------------------*/
#define MAX_EVENT_TYPE_COUNT			(4)
#define FIRMWARE_COPYS_MAX				(4)
#define FIRMWARE_COPYS_USED				(3)
#define MAX_FW_SUB_EVENT_COUNT			(4)

/*event last used page*/
#define EVENT_BLK_LAST_USED_PAGE		(event_log_glb_info.last_page)
/*total event count*/
#define EVENT_TOTAL_CNT					(EVENT_BLK_LAST_USED_PAGE + 1)
/*event physic block number*/
#define EVENT_BLK_NUMBER				(event_log_glb_info.phyblk)
/*event page bit map*/
#define EVENT_PAGE_BITMAP				(0x1)
/*appointed type event count*/
#define EVENT_ITEM_CNT(x)				(event_log_glb_info.item_cnt[x])
/*Get composed idx of appointed event type*/
#define EVENT_COMPOSED_IDX(type, idx)	((unsigned short)(((type & 0xFF) << 8) | (idx)))

/*----------------------------------------------------------------------*/
/*   store global info, use individual buffer (a sector length)			*/
/*----------------------------------------------------------------------*/
typedef struct _event_log_glb_info_t
{
	//struct DataBlkTblType blk;	//physic block.
	unsigned short phyblk;
	unsigned short last_page;	//last used page.
	unsigned int flag;			//dirty flag.
	unsigned int item_cnt[MAX_EVENT_TYPE_COUNT];//idx use the type defined.
	unsigned short *idx_table;	//store each log index.
} event_log_glb_info_t;


/*----------------------------------------------------------------------*/
/*   Event structure that stored in flash, SIZE!						*/
/*----------------------------------------------------------------------*/
typedef struct _event_log_item_t
{
	unsigned int type;		//4 bytes, must be set by user!!
	unsigned int sequence;	//4 bytes, no in global, but under the type. usr do not need set this
	unsigned int stamp[8]; 	//32 bytes record time, this may used to identify the most new sequence, usr do not need set this
	unsigned char log[EVENT_LOGINFO_SIZE];	//event content. user should set this
} event_log_item_t;



/*
 *fw idx: there are 3 copy of sys in usdk213F, reserved 1 for later
 *sub idx: 0.lfihead, 1.lfidir, 2.lfi, 3.app
 *event idx: corrupt or fixed!
 */
typedef struct _fw_event_t
{
	/*dim 0 - fw idx; dim 1- sub idx; dim 2 - event idx*/
	unsigned int count[FIRMWARE_COPYS_MAX][MAX_FW_SUB_EVENT_COUNT][2];		//4*4*2*4 = 128 bytes!
} fw_event_t;

#define FW_EVENT_LOGINFO_SIZE		(sizeof(fw_event_t))
#define FW_EVENT_RESERVED_SIZE		(EVENT_LOGINFO_SIZE - FW_EVENT_LOGINFO_SIZE)
/*-----------------------------------------------------------------------*/
/*Firmware Event structure, stored in flash as a part of event_log_item_t*/
/*-----------------------------------------------------------------------*/
typedef struct _fw_event_log_t	/*472 bytes max, so it can be stored in flash*/
{
	fw_event_t fw_event;				/*128 bytes*/
	unsigned char reserved[FW_EVENT_RESERVED_SIZE];		//472-128 = 344 bytes
} fw_event_log_t;

/*----------------------------------------------------------------------*/
/*   string Event structure 							                */
/*----------------------------------------------------------------------*/
#define STRING_CONTAINER_SIZE		(EVENT_LOGINFO_SIZE)
#define STRING_LOG_FORCE_FLUSH		(1)	/*direct write log info down!*/
#define DEFAULT_CURSOR_OFFSET		(0)
typedef struct _string_event_log_t
{
	char buffer[STRING_CONTAINER_SIZE];
} string_event_log_t;

/*!
 * global infomation to identify event log.
 */
extern event_log_glb_info_t event_log_glb_info;

/******************************************************************************/
/*!
* \par  Description:
*     get event (EVENT_TYPE_FW_CORRUPTED/EVENT_TYPE_FW_FIXED) value of given firmware .
* \param[in]    event_type: EVENT_TYPE_FW_CORRUPTED or EVENT_TYPE_FW_FIXED
*				fw_index: sequence number of sys
* \param[out]   value: event value
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int _get_fw_event(unsigned int event_type, unsigned int fw_index, unsigned int * value);	/*use this in nand driver*/
//extern unsigned int get_fw_event(unsigned int event_type, unsigned int fw_index, unsigned int * value);		/*use this under os*/

/******************************************************************************/
/*!
* \par  Description:
*     set event (EVENT_TYPE_FW_CORRUPTED/EVENT_TYPE_FW_FIXED) value of given firmware .
* \param[in]    event_type: EVENT_TYPE_FW_CORRUPTED or EVENT_TYPE_FW_FIXED
*				fw_index: sequence number of sys
* \param[out]   value: event value
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int _set_fw_event(unsigned int event_type, unsigned int fw_index, unsigned int value);	/*use this in nand driver*/
//extern unsigned int set_fw_event(unsigned int event_type, unsigned int fw_index, unsigned int value);	/*use this under os*/

/******************************************************************************/
/*!
* \par  Description:
*     initialize log event envirments, including buffer and statistics .
* \param[in]    caller: 's' (system)\ 'b' (boot) \ 'f' (fwsc)
* \param[out]   value: event value
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int log_event_init(unsigned char caller);

/******************************************************************************/
/*!
* \par  Description:
*     clear log event envirments, writing back cache and free buffers.
* \param[in]    none
* \param[out]   value: event value
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
void log_event_exit(void);

/******************************************************************************/
/*!
* \par  Description:
*     set debug event infomation.
* \param[in]    log: debug information to log
*				length: information length, in char.
*				opt: write down to flash at once or not.
* \param[out]   none.
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int _set_dbg_event(char * log, unsigned int length, unsigned int opt);
extern unsigned char *logStringBuf; /*string log temp buffer.*/

#define LOG_DBG(fmt...) 	do { \
									if (logStringBuf != NULL) \
									{ \
										strcpy(logStringBuf, "[DGB]"); \
										sprintf(logStringBuf + 5, fmt); \
										_set_dbg_event(logStringBuf, strlen(logStringBuf)+1, 0); \
									} \
									else \
									{ \
										PRINT(fmt); \
									} \
								} while(0)

/******************************************************************************/
/*!
* \par  Description:
*     set info event infomation.
* \param[in]    log: info information to log
*				length: information length, in char.
*				opt: write down to flash at once or not.
* \param[out]   none.
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int _set_info_event(char * log, unsigned int length, unsigned int opt);

#define LOG_INFO(fmt...) 	do { \
									if (logStringBuf != NULL) \
									{ \
										strcpy(logStringBuf, "[INFO]"); \
										sprintf(logStringBuf + 6, fmt); \
										_set_info_event(logStringBuf, strlen(logStringBuf)+1, 0); \
									} \
									else \
									{ \
										PRINT(fmt); \
									} \
								}while(0)
/******************************************************************************/
/*!
* \par  Description:
*     set critical(error) event infomation.
* \param[in]    log: critical information to log
*				length: information length, in char.
*				opt: write down to flash at once or not.
* \param[out]   none.
* \return       0: 		success
*				not 0:	failed
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern unsigned int _set_critical_event(char * log, unsigned int length, unsigned int opt);
#define LOG_CRITICAL(fmt...)	 	do { \
											if (logStringBuf != NULL) \
											{ \
												strcpy(logStringBuf, "[CRITICAL]"); \
												sprintf(logStringBuf + 10, fmt); \
												_set_critical_event(logStringBuf, strlen(logStringBuf)+1, STRING_LOG_FORCE_FLUSH);\
											} \
											else \
											{ \
												PRINT(fmt); \
											} \
										} while(0)

/******************************************************************************/
/*!
* \par  Description:
*     dump out string event.
* \param[in]    none
* \param[out]   none.
* \return       none.
* \ingroup      nand
* \par          exmaple code
* \code
*               list example code in here
* \endcode
*******************************************************************************/
extern void dump_string_event(void);
#endif /*__NAND_EVENT_LOG_H__*/
/*blank line*/

