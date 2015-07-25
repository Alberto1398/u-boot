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

ENTRY(secondary_startup)
	msr cpsr_c, #0xd3 @set svc mode

	mrc p15, 0, r1, c0, c0, 5 @ Multiprocessor ID register
	and r1, r1, #15

	sub r1, r1, #1
	adr r0, stack_pointer
	ldr sp, [r0, r1, lsl #2]

	bl arm_init_before_mmu

	mov r0, #0
	movw r2, #0xffff
	movt r2, #0xffff
	ldr r1, pg_dir

	mcr   p15, 0, r2, c3, c0, 0  @ Set the access control to all-supervisor
	mcr   p15, 0, r1, c2, c0, 0  @ set TTB 0
	mcr   p15, 0, r0, c7, c5, 4  @ CP15ISB

	mrc p15, 0, r2, c1, c0, 0 @ get CR
	orr r2, r2, #0x5
	orr r2, r2, #0x1000

	mcr p15, 0, r2, c1, c0, 0  @ set CR

	mcr p15, 0, r0, c7, c5, 0  @ ISB

	mcr p15, 0, r0, c8, c7, 0  @ Invalidate entire unified TLB

	mcr p15, 0, r0, c8, c6, 0  @ Invalidate entire data TLB
	mcr p15, 0, r0, c8, c5, 0  @ Invalidate entire instruction TLB

	/* Full system DSB - make sure that the invalidation is complete */
	mcr p15, 0, r0, c7, c10, 4
	/* Full system ISB - make sure the instruction stream sees it */
	mcr p15, 0, r0, c7, c5, 0

	bl decompress_fn

	.globl stack_pointer
	.globl pg_dir
stack_pointer:
	.long 1
	.long 1
	.long 1
	.long 1
pg_dir:
	.long 1
ENDPROC(secondary_startup)
