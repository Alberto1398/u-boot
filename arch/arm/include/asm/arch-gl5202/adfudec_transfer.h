/********************************************************************************
*                              usdk130
*                            Module: adfudec
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved. 
*
* History:         
*      <author>    <time>           <version >             <desc>
*       huanghe     2008-06-20      1.0                    build this file 
********************************************************************************/
/*!
* \file			pub_adfudec_transfer.h
* \brief		adfudec_transfer.c提供的外部变量和接口函数
* \author		黄河
* \par Copyright(c) 2001-2007 Actions Semiconductor, All Rights Reserved.
*
* \version		1.0
* \date			2008/6/20
*******************************************************************************/

#ifndef  _PUB_ADFUDEC_TRANSFER_H_
#define  _PUB_ADFUDEC_TRANSFER_H_

/******************************************************************************/
/*!                    
* \par  Description:
*		使能out1的fifo，从而可以接续接受pc发出的数据
* \param		void      
* \return       void  
* \ingroup      adfudec
* \par          exmaple code
* \code 
*               enalbe_out1_fifo();
* \endcode
*******************************************************************************/
void enalbe_out1_fifo (void);

/******************************************************************************/
/*!                    
* \par  Description:
*		UsbOtgEp1OutDeal为代码入口，然后解析命令并执行相应操作\n
*		从adfudec.c的adfuserver中调用
* \param		void      
* \return       0 
* \ingroup      adfudec
* \par          exmaple code
* \code 
*              UsbOtgEp1OutDeall();
* \endcode
*******************************************************************************/
int UsbOtgEp1OutDeal (void);

#endif  //_PUB_ADFUDEC_TRANSFER_H_
