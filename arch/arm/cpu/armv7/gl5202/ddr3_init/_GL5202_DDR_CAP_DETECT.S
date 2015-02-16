/*
=================================================================================     
*                        GL5202_DDR_CAP_DETECT
* FileName: GL5202_DDR_CAP_DETECT.S
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2011-7-5
* Description:  GL5202_DDR_CAP_DETECT
* History :
*   v1.0    create this file    2011-7-5
=================================================================================  
*/


#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"

	.section ".text._GL5202_DDR_CAP_DETECT", "x"
	.globl _GL5202_DDR_CAP_DETECT
_GL5202_DDR_CAP_DETECT:

    stmfd   sp!,{r2-r12,lr}
/*
=================================================================================
*   config CMU register
=================================================================================
*/     
       
    ldr r4,=DCU_FEA
    ldr r5,[r4]
    ldr r6,=0xffffff0f
    and r5,r5,r6
    ldr r6,=CAP_16Gbit
    orr r5,r5,r6
    str r5,[r4]
               
    ldr r5,=0x00000020
    ldr r6,=0x00
    ldr r4,=0x00
        
data_init:
    str  r6,[r4]
    adds r4,r4,#0x04    
    cmp  r4,r5
    bne data_init

    ldr  r4,=0x40000008//16Gbit
    ldr  r5,=0x77777777
    str  r5,[r4]
    
    ldr  r4,=0x20000008//8Gbit
    ldr  r5,=0x66666666
    str  r5,[r4]
    
    ldr  r4,=0x10000008//4Gbit
    ldr  r5,=0x55555555
    str  r5,[r4]
    
    ldr  r4,=0x08000008//2Gbit
    ldr  r5,=0x44444444
    str  r5,[r4]
    
    ldr  r4,=0x04000008//1Gbit
    ldr  r5,=0x33333333
    str  r5,[r4]
    
    ldr  r4,=0x02000008//512Mbit
    ldr  r5,=0x22222222
    str  r5,[r4]
    
    ldr  r4,=0x01000008//256Mbit
    ldr  r5,=0x11111111
    str  r5,[r4]
    
    ldr  r6,=0x00000000
    ldr  r4,=0x00000008
    ldr  r5,[r4]
    cmp  r5,r6
    beq _CAP_16Gbit

    
    ldr  r6,=0x77777777
    cmp  r5,r6
    beq _CAP_8Gbit
    
    ldr  r6,=0x66666666
    cmp r5,r6
    beq _CAP_4Gbit
    
    ldr  r6,=0x55555555
    cmp r5,r6
    beq _CAP_2Gbit
    
    ldr r6,=0x44444444
    cmp r5,r6
    beq _CAP_1Gbit

    ldr r6,=0x33333333
    cmp r5,r6
    beq _CAP_512Mbit
    
    ldr  r6,=0x22222222
    cmp r5,r6
    bne DDRCAP_SCAN_ERR
    
    ldr  r4,=0x01000008
    ldr  r5,[r4]
    ldr  r6,=0x11111111
    cmp r5,r6
    beq _CAP_256Mbit
    

_CAP_16Gbit:
    ldr  r1,=0x80000000
    b   DDRCAP_SCAN_OK
       
_CAP_8Gbit:
    ldr	r4,=DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_8Gbit
	  orr  r5,r5,r6
	  str	r5,[r4]
	  
	  
	  ldr  r1,=0x40000000
    b   DDRCAP_SCAN_OK

_CAP_4Gbit:
    ldr	r4,=DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_4Gbit
	  orr  r5,r5,r6
	  str	r5, [r4]
	  
	  
	  ldr  r1,=0x20000000
    b   DDRCAP_SCAN_OK
    

_CAP_2Gbit:
    ldr	r4, =DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_2Gbit
	  orr  r5,r5,r6
	  str	r5, [r4]
	  
	  
	  ldr  r1,=0x10000000
    b   DDRCAP_SCAN_OK


_CAP_1Gbit: 
    ldr	r4, =DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_1Gbit
	  orr  r5,r5,r6
	  str	r5, [r4]
	  
   
    ldr  r1,=0x08000000
    b   DDRCAP_SCAN_OK

    
_CAP_512Mbit: 
    ldr	r4, =DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_512Mbit
	  orr  r5,r5,r6
	  str	r5, [r4]
	  
   
    ldr  r1,=0x04000000
    b   DDRCAP_SCAN_OK
    
    
_CAP_256Mbit: 
    ldr	r4,=DCU_FEA
	  ldr	r5,[r4]
	  
	  ldr  r6,=0xffffff0f
	  and r5,r5,r6
	  ldr  r6,=CAP_256Mbit
	  orr  r5,r5,r6
	  str	r5, [r4]
	  
   
    ldr  r1,=0x02000000
    b   DDRCAP_SCAN_OK
    
    
    
DDRCAP_SCAN_OK:    
    ldr r0,=0x00
    b TEST_END

    
DDRCAP_SCAN_ERR:
    ldr r0,=0xffffffff
    b TEST_END
    
TEST_END:
    ldmfd   sp!,{r2-r12,lr}  
    bx      lr  
