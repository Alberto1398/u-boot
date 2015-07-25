/********************************************************************************
*                              NAND FLASH DRIVER MODULE
*                            Module: global.h
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved. 
*
* History:         
*      <author>    <time>           <version >             <desc>
* nand flash group    2007-10-16 9:33     1.0             build this file 
********************************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//#define SYMBOLTEST // һ���Բ���


// ÿ��symbol��������bit��
#define SYMBOLBITS		(9)
// �ɾ��Ĵ�����
#define ERRCAP			(4)

#define RSCODELEN 		((1 << SYMBOLBITS) - 1)
// ����У���λ��
#define NPAR            (2*ERRCAP)

#define NMSG            (RSCODELEN - NPAR)

#define ROM_EXPLEN		(RSCODELEN + 1)
#define ROM_LOGLEN		(RSCODELEN + 1)
#define ROM_INVLEN		(RSCODELEN + 1)
// ���������Ķ���ʽ����
// ��������Ӳ��ʵ��ʱ����ʽ�˷����ĸ���
// ��RS���У�һ������Ϊ���Ծ�����������4����4t
#define MAXDEG			(NPAR)
// ��������ʽ��˵�������
#define DMAXDEG			(2*MAXDEG)

typedef short BIT9;
typedef short BIT4;
typedef short BIT3;
typedef short BIT2;
typedef short BIT1;
//-------------------------------------------------------------------------------------
#endif
