/*
 * act_boot_pic.h
 *
 *  Created on: Aug 12, 2014
 *      Author: clamshell
 */

#ifndef __ACT_BOOT_PIC_H__
#define __ACT_BOOT_PIC_H__

#include <common.h>
#include <asm/arch/actions_arch_common.h>

typedef struct
{
    int checksum;                            /*除去checksum字段外的所有数据（图片数据及文件头）的校验和*/
    int width;                               /*图片每行的像素个数*/
    int height;                              /*图片的行数*/
    int bytes_per_pix;                       /*每个像素占用的BYTE数*/
    int length;                              /*文件总长度，包括文件头及data数据*/
    int x;                                   /*图片在屏上的起始x坐标*/
    int y;                                   /*图片在屏上的起始y坐标*/
    int reserved[1];                         /*保留位*/
} boot_pic_info_t;                           /*结构体总大小为32bytes*/

#endif /* __ACT_BOOT_PIC_H__ */
