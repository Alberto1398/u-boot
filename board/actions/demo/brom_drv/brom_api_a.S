#include <config.h>
#include <version.h>
#include <asm/system.h>
#include <linux/linkage.h>
#include <asm/arch/osboot.h>

/* 注意保存r9, r10! */

/* void act_brom_nand_lb_read_unit(act_brom_nand_read_param_t *p_param) */
    .section .text.bromapi1
ENTRY(act_brom_nand_lb_read_unit)
    push    {r4,r9,r10,lr}
    ldr     r1, =Brom_NANFLBSectorRead
    ldr     r4, =NAND_STAR_ADDR
    blx     r4
    pop     {r4,r9,r10,pc}
ENDPROC(act_brom_nand_lb_read_unit)

/* void act_brom_nand_lb_ttlc_read_unit(act_brom_nand_read_param_t *p_param) */
    .section .text.bromapi2
ENTRY(act_brom_nand_lb_ttlc_read_unit)
    push    {r4,r9,r10,lr}
    ldr     r1, =Brom_NANFLBSectorRead_TTlc
    ldr     r4, =NAND_STAR_ADDR
    blx     r4
    pop     {r4,r9,r10,pc}
ENDPROC(act_brom_nand_lb_ttlc_read_unit)

/* void act_brom_nand_exit(void) */
    .section .text.bromapi3
ENTRY(act_brom_nand_exit)
    push    {r4,r9,r10,lr}
    ldr     r1, =Brom_NANFDisable
    ldr     r4, =NAND_STAR_ADDR
    blx     r4
    ldr     r1, =Brom_NANFExit
    blx     r4
    pop     {r4,r9,r10,pc}
ENDPROC(act_brom_nand_exit)


/* void act_brom_sdmmc_read_lba(uint32_t lba, uint32_t len, uint32_t buf_addr) */
    .section .text.bromapi4
ENTRY(act_brom_sdmmc_read_lba)
    push    {r4,r9,r10,lr}
    ldr     r9, =Brom_SDMMCRead
    ldr     r4, =SDMMC_STAR_ADDR
    blx     r4
    pop     {r4,r9,r10,pc}
ENDPROC(act_brom_sdmmc_read_lba)

/* void act_brom_sdmmc_exit(void) */
    .section .text.bromapi5
ENTRY(act_brom_sdmmc_exit)
    push    {r4,r9,r10,lr}
    ldr     r9, =Brom_SDMMCPowerOff
    ldr     r4, =SDMMC_STAR_ADDR
    blx     r4
    pop     {r4,r9,r10,pc}
ENDPROC(act_brom_sdmmc_exit)


/* void act_brom_enter_adfu(void) */
    .section .text.bromapi6
ENTRY(act_brom_enter_adfu)
    // disable interrupt
    cpsid   if

    /* Set V=1 in CP15 SCTRL register */
    mrc p15, 0, r0, c1, c0, 0   @ Read CP15 SCTRL Register
    orr r0, r0, #CR_V           @ V = 1
    mcr p15, 0, r0, c1, c0, 0   @ Write CP15 SCTRL Register
    // reset VBAR
    mov     r0, #0
    mcr     p15, 0, r0, c12, c0, 0  @Set VBAR

    isb

    // goto ADFU
    ldr     r0, =ADFULAUNCHER_STAR_ADDR
    blx     r0
    b       .
ENDPROC(act_brom_enter_adfu)


