/*
=================================================================================     
*                        GL5202_BDMA0_TRANSFER
* FileName: GL5202_BDMA0_TRANSFER.S
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2011-12-29
* Description:  GL5202_BDMA0_TRANSFER
* History :
*   v1.0    create this file    2011-12-29
=================================================================================  
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"


	.section ".text._GL5202_BDMA0_TRANSFER", "x"
	.globl _GL5202_BDMA0_TRANSFER

_GL5202_BDMA0_TRANSFER:

    stmfd   sp!,{r4-r12,lr}   

/*
==============================================================
        DMA transfer
==============================================================
*/  
//    ldr r4,=SHARESRAM_CTL //HARESRAM_CTL
//    ldr r5,=0x0 //
//    str r5,[r4]	
		
BDMA0_TRANSFER:        
    
    ldr  r4,=BDMA0_SRC    
    str  r0,[r4]
    
    ldr  r4,=BDMA0_DST
    str  r1,[r4]
    
    ldr  r4,=BDMA0_CNT
    str  r2,[r4]
    
    ldr  r4,=BDMA0_MODE
    str  r3,[r4]
    
    ldr r4,=BDMA0_CACHE 
    ldr r5,=0x60006 
    str r5,[r4]
    
    ldr  r4,=BDMA0_CMD
    ldr  r5,=0x1
    
    str  r5,[r4]
    
BDMA0_complete_check:
    ldr  r6,[r4]
    cmp  r6,r5
    beq BDMA0_complete_check
             
/*
==============================================================
        DMA transfer end
==============================================================
*/      
   
TEST_END:
    mov r0,#0x01
    
    ldmfd   sp!,{r4-r12,lr}  
    bx      lr
    
    


    

