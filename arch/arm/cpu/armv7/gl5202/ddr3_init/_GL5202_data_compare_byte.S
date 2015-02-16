/*
=================================================================================     
*                        GL5202 data compare
* FileName: GL5202_data_compare.S
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2011-7-5
* Description:  GL5202 data compare function
* History :
*   v1.0    create this file    2011-7-5
=================================================================================  
*/


#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"

	.section ".text._GL5202_data_compare_byte", "x"
	.globl _GL5202_data_compare_byte
_GL5202_data_compare_byte:
/*
=================================================================================
*   
=================================================================================
*/  
    stmfd   sp!,{r1-r12,lr} 
        

    mov  r4,r0
    mov  r5,r1
    mov  r6,r2
    add  r6,r6,r0
    
    
data_compare:    
    ldrb r7,[r4]
    ldrb r8,[r5]
    cmp  r7,r8
    beq  bypass
    nop
    b    GL5202_data_compare_byte_error
    nop
    
bypass:    
    adds r4,r4,#0x01
    adds r5,r5,#0x01
    cmp  r4,r6
    bne  data_compare

        
GL5202_data_compare_byte_ok:    
    mov r0,#0x0
    b GL5202_data_compare_byte_end

    
GL5202_data_compare_byte_error:
    ldr r0,=0xffffffff
    b GL5202_data_compare_byte_end     
	  

GL5202_data_compare_byte_end:    
    ldmfd   sp!,{r1-r12,lr}  
    bx      lr 
