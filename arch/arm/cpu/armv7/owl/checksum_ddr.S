/*
 * Copyright 2013 Actions Semi Inc.
 * Author: Actions Semi, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <config.h>
#include <version.h>
#include <asm/system.h>
#include <linux/linkage.h>
#include <asm/arch/osboot.h>

ENTRY(restore_code_for_ddr_train)
    push {r1, r2, r3, r4, r5, lr}
    
	ldr	r5, =0x00002900     //0~16K reserved, 8K for ddr test, we use position 10k to save checksum
	ldr r2, =0x0000a000
	ldr r3, =0x0000a040
    
	mov r4, #0
restore_loop:
	ldmia r5!, {r0-r1}
	stmia r2!, {r0-r1}
	cmp r2, r3
	blo	restore_loop

    pop	{r1, r2, r3, r4, r5, pc}
ENDPROC(restore_code_for_ddr_train)

ENTRY(checksum_ddr)
    push {r1, r2, r3, r4, r5, lr}
    
	ldr	r5, =0x00002800     //0~16K reserved, 8K for ddr test, we use position 10k to save checksum
	ldr r2, [r5]
	ldr r3, [r5, #4]
    ldr r1, =0x3fffffff
    and r2, r2, r1
    and r3, r3, r1
    
	mov r4, #0
add_loop:
	ldmia r2!, {r0-r1}
	add r4, r4, r0
	add r4, r4, r1
	cmp r2, r3
	blo	add_loop

	ldr r1, [r5, #8]
	cmp r1, r4
	moveq r0, #0
	movne r0, #1
	
    pop	{r1, r2, r3, r4, r5, pc}
ENDPROC(checksum_ddr)
