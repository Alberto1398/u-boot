#include <config.h>
#include <version.h>
#include <asm/system.h>
#include <linux/linkage.h>


    // unsigned int read_irq_ack(void)
    // Returns the value of the Interrupt Acknowledge Register
    .section ".text._read_irq_ack", "x"
LENTRY(_read_irq_ack)
    MRC     p15, 4, r0, c15, c0, 0  // Read periph base address
    LDR     r0, [r0, #0x010C]       // Read the Interrupt Acknowledge Register  (ICCIAR)
    BX      lr                      // 注意:此ICCIAR寄存器cpu读一次后便会回到默认值0x3ff
ENDPROC(_read_irq_ack)

    // void write_end_of_irq(unsigned int ID)
    // Writes ID to the End Of Interrupt register
    .section ".text._write_end_of_irq", "x"
LENTRY(_write_end_of_irq)
    // Get base address of private perpherial space
    MOV     r1, r0                  // Back up passed in ID value
    MRC     p15, 4, r0, c15, c0, 0  // Read periph base address
    STR     r1, [r0, #0x0110]       // Write ID to the End of Interrupt register (ICCEOIR)
    BX      lr
ENDPROC(_write_end_of_irq)


    .section ".text._act_null_handler", "x"
LENTRY(_act_null_handler)
    b       .
    bx      lr
ENDPROC(_act_null_handler)


    .section ".text._act_irq_handler", "x"
LENTRY(_act_irq_handler)
    // now in IRQ mode
    // sub procedure will save {r4~r8, r9, r10, r11}
    // KEEP 8-bytes aligned!
    sub     lr, lr, #4
    srsdb   sp!, #0x12  /* lr spsr */
    push    {r0-r3, r4, r12}

    /* r9 holds the GD, do not modify it here! */

    bl      _read_irq_ack
    mov     r4, r0
    blx     act_do_irq_inner
    mov     r0, r4
    bl      _write_end_of_irq

    pop     {r0-r3, r4, r12}
    rfeia   sp!
ENDPROC(_act_irq_handler)


    .section ".text.act_init_exception_stacks", "x"
ENTRY(act_init_exception_stacks)
    // init exception stack
    ldr     r0, =CONFIG_ACTS_SPL_IRQ_STACK
    ldr     r1, =CONFIG_ACTS_SPL_MISC_STACK
    cpsid   if, #0x11   // FIQ
    mov     sp, r1
    cps     #0x12       // IRQ
    mov     sp, r0
    cps     #0x17       // ABT
    mov     sp, r1
    cps     #0x1b       // UND
    mov     sp, r1
    cps     #0x1f       // SYS
    mov     sp, r1
    cps     #0x13       // back to SVC

    bx      lr
ENDPROC(act_init_exception_stacks)




//==============================================================================

    .section ".text.vectors", "x"
    .globl _start
_start:
    ldr	pc, =reset
    ldr pc, =_act_null_handler      // _undefined_instruction
    ldr pc, =_act_null_handler      // _software_interrupt
    ldr pc, =_act_null_handler      // _prefetch_abort
    ldr pc, =_act_null_handler      // _data_abort
    ldr pc, =_act_null_handler      // _not_used
    ldr pc, =_act_irq_handler       // _irq
    ldr pc, =_act_null_handler      // _fiq

    /* now offset == 32. */

/* special FW parameter table goes here ... */
#ifdef CONFIG_SPL_BUILD
_act_spl_fw_param_tbl:
    .word   _start          /* bin exec address (used by upgrade_script & upgrade_app) */
    .word   g_afinfo_space  /* afinfo address (in bytes, used by upgrade_script & upgrade_app) */
    .word   2048            /* size of afinfo space (in bytes, used by upgrade_script & upgrade_app) */
#endif

