/*
 * gl520x_mmch.h
 *
 *  Created on: Aug 1, 2014
 *      Author: clamshell
 */

#ifndef __GL520X_MMCH_H__
#define __GL520X_MMCH_H__

#include <common.h>
#include <part.h>
#include <mmc.h>
#include <asm/arch/actions_arch_common.h>

// config macro
//==============================================================================
#if defined(CONFIG_SUPPORT_EMMC_BOOT)
#error "CONFIG_SUPPORT_EMMC_BOOT is not recommend, please do not defined it."
#endif
#if defined(CONFIG_SUPPORT_EMMC_RPMB)
#error "CONFIG_SUPPORT_EMMC_RPMB is not recommend, please do not defined it."
#endif


// REG define
//==============================================================================

#define BP_SD_EN_RANE               31
#define BP_SD_EN_RAN_SEED           24
#define BP_SD_EN_DAT1_S             9
#define BP_SD_EN_CLK_S              8
#define BP_SD_EN_EN                 7
#define BP_SD_EN_BSEL               6
#define BP_SD_EN_SDIOEN             3
#define BP_SD_EN_DATAWID            0
#define BM_SD_EN_RANE               (1U    << BP_SD_EN_RANE     )
#define BM_SD_EN_RAN_SEED           (0x3fU << BP_SD_EN_RAN_SEED )
#define BM_SD_EN_DAT1_S             (1U    << BP_SD_EN_DAT1_S   )
#define BM_SD_EN_CLK_S              (1U    << BP_SD_EN_CLK_S    )
#define BM_SD_EN_EN                 (1U    << BP_SD_EN_EN       )
#define BM_SD_EN_BSEL               (1U    << BP_SD_EN_BSEL     )
#define BM_SD_EN_SDIOEN             (1U    << BP_SD_EN_SDIOEN   )
#define BM_SD_EN_DATAWID            (3U    << BP_SD_EN_DATAWID  )

#define BP_SD_CTL_TOUTEN            31
#define BP_SD_CTL_TOUTCNT           24
#define BP_SD_CTL_RDELAY            20
#define BP_SD_CTL_WDELAY            16
#define BP_SD_CTL_CMDLEN            13
#define BP_SD_CTL_SCC               12
#define BP_SD_CTL_TCN               8
#define BP_SD_CTL_TS                7
#define BP_SD_CTL_LBE               6
#define BP_SD_CTL_C7EN              5
#define BP_SD_CTL_TM                0
#define BM_SD_CTL_TOUTEN            (1U    << BP_SD_CTL_TOUTEN  )
#define BM_SD_CTL_TOUTCNT           (0x7fU << BP_SD_CTL_TOUTCNT )
#define BM_SD_CTL_RDELAY            (0xfU  << BP_SD_CTL_RDELAY  )
#define BM_SD_CTL_WDELAY            (0xfU  << BP_SD_CTL_WDELAY  )
#define BM_SD_CTL_CMDLEN            (1U    << BP_SD_CTL_CMDLEN  )
#define BM_SD_CTL_SCC               (1U    << BP_SD_CTL_SCC     )
#define BM_SD_CTL_TCN               (0xfU  << BP_SD_CTL_TCN     )
#define BM_SD_CTL_TS                (1U    << BP_SD_CTL_TS      )
#define BM_SD_CTL_LBE               (1U    << BP_SD_CTL_LBE     )
#define BM_SD_CTL_C7EN              (1U    << BP_SD_CTL_C7EN    )
#define BM_SD_CTL_TM                (0xfU  << BP_SD_CTL_TM      )

#define BP_SD_STATE_D1B_S           18
#define BP_SD_STATE_SDIOB_P         17
#define BP_SD_STATE_SDIOB_EN        16
#define BP_SD_STATE_TOUTE           15
#define BP_SD_STATE_BAEP            14
#define BP_SD_STATE_MEMRDY          12
#define BP_SD_STATE_CMDS            11
#define BP_SD_STATE_D1A_S           10
#define BP_SD_STATE_SDIOA_P         9
#define BP_SD_STATE_SDIOA_EN        8
#define BP_SD_STATE_DAT0S           7
#define BP_SD_STATE_TEIE            6
#define BP_SD_STATE_TEI             5
#define BP_SD_STATE_CLNR            4
#define BP_SD_STATE_CLC             3
#define BP_SD_STATE_WC16ER          2
#define BP_SD_STATE_RC16ER          1
#define BP_SD_STATE_CRC7ER          0
#define BM_SD_STATE_D1B_S           (1U << BP_SD_STATE_D1B_S    )
#define BM_SD_STATE_SDIOB_P         (1U << BP_SD_STATE_SDIOB_P  )
#define BM_SD_STATE_SDIOB_EN        (1U << BP_SD_STATE_SDIOB_EN )
#define BM_SD_STATE_TOUTE           (1U << BP_SD_STATE_TOUTE    )
#define BM_SD_STATE_BAEP            (1U << BP_SD_STATE_BAEP     )
#define BM_SD_STATE_MEMRDY          (1U << BP_SD_STATE_MEMRDY   )
#define BM_SD_STATE_CMDS            (1U << BP_SD_STATE_CMDS     )
#define BM_SD_STATE_D1A_S           (1U << BP_SD_STATE_D1A_S    )
#define BM_SD_STATE_SDIOA_P         (1U << BP_SD_STATE_SDIOA_P  )
#define BM_SD_STATE_SDIOA_EN        (1U << BP_SD_STATE_SDIOA_EN )
#define BM_SD_STATE_DAT0S           (1U << BP_SD_STATE_DAT0S    )
#define BM_SD_STATE_TEIE            (1U << BP_SD_STATE_TEIE     )
#define BM_SD_STATE_TEI             (1U << BP_SD_STATE_TEI      )
#define BM_SD_STATE_CLNR            (1U << BP_SD_STATE_CLNR     )
#define BM_SD_STATE_CLC             (1U << BP_SD_STATE_CLC      )
#define BM_SD_STATE_WC16ER          (1U << BP_SD_STATE_WC16ER   )
#define BM_SD_STATE_RC16ER          (1U << BP_SD_STATE_RC16ER   )
#define BM_SD_STATE_CRC7ER          (1U << BP_SD_STATE_CRC7ER   )

#define BP_SD_DMA_CTL_SDDS          31
#define BP_SD_DMA_CTL_SDDFIP        29
#define BP_SD_DMA_CTL_SDDFIE        28
#define BP_SD_DMA_CTL_SDDNUM        0
#define BM_SD_DMA_CTL_SDDS          (1U        << BP_SD_DMA_CTL_SDDS   )
#define BM_SD_DMA_CTL_SDDFIP        (1U        << BP_SD_DMA_CTL_SDDFIP )
#define BM_SD_DMA_CTL_SDDFIE        (1U        << BP_SD_DMA_CTL_SDDFIE )
#define BM_SD_DMA_CTL_SDDNUM        (0xffffffU << BP_SD_DMA_CTL_SDDNUM )

/* MMC HC registers */
#define MMCHC_EN(hc)        ((hc)->iobase + 0x00U)
#define MMCHC_CTL(hc)       ((hc)->iobase + 0x04U)
#define MMCHC_STATE(hc)     ((hc)->iobase + 0x08U)
#define MMCHC_CMD(hc)       ((hc)->iobase + 0x0CU)
#define MMCHC_ARG(hc)       ((hc)->iobase + 0x10U)
#define MMCHC_RSPBUF0(hc)   ((hc)->iobase + 0x14U)
#define MMCHC_RSPBUF1(hc)   ((hc)->iobase + 0x18U)
#define MMCHC_RSPBUF2(hc)   ((hc)->iobase + 0x1CU)
#define MMCHC_RSPBUF3(hc)   ((hc)->iobase + 0x20U)
#define MMCHC_RSPBUF4(hc)   ((hc)->iobase + 0x24U)
#define MMCHC_DAT(hc)       ((hc)->iobase + 0x28U)
#define MMCHC_BLK_SIZE(hc)  ((hc)->iobase + 0x2CU)
#define MMCHC_BLK_NUM(hc)   ((hc)->iobase + 0x30U)
#define MMCHC_DMA_ADDR(hc)  ((hc)->iobase + 0x34U)
#define MMCHC_DMA_CTL(hc)   ((hc)->iobase + 0x38U)


#define MMCHC_MAX_DMA_SIZE_PER_TRAN     ((1U<<24)-1U)
#define MMCHC_MAX_BLK_PER_TRAN          ((1U<<14)-1U)
#define MMCHC_MAX_BLK_SIZE              ((1U<<10)-1U)
/* note: MMCHC_MAX_DMA_SIZE_PER_TRAN > (MMCHC_MAX_BLK_PER_TRAN * MMCHC_MAX_BLK_SIZE) */


// DEBUG
//==============================================================================

#define CONFIG_MMCHC_DEBUG          1
#define CONFIG_MMCHC_DEBUG_PRINT    0

/* for debug */
#define DMSG_ERR(FMT, ARGS...)  printf("[MMCH]" FMT, ## ARGS)
#define DMSG_WARN(FMT, ARGS...) printf("[MMCH]" FMT, ## ARGS)
#define DMSG_INFO(FMT, ARGS...) printf("[MMCH]" FMT, ## ARGS)
#if CONFIG_MMCHC_DEBUG_PRINT
#define DMSG_DBG(FMT, ARGS...)  printf("[MMCH]" FMT, ## ARGS)
#else
#define DMSG_DBG(FMT, ARGS...)  do{}while(0)
#endif
#if CONFIG_MMCHC_DEBUG
#define DASSERT(EXP)                    BUG_ON(!(EXP))
#define DBG_DEFINE_OBJ_TYPE_ID          uint32_t __obj_type_id
#define DBG_SET_OBJ_TYPE_ID(P_OBJ,VAL)  ((P_OBJ)->__obj_type_id = (VAL))
#define DBG_CHK_OBJ_TYPE_ID(P_OBJ,VAL)  DASSERT(((P_OBJ)!=NULL)&&((P_OBJ)->__obj_type_id==(VAL)))
#else
#define DASSERT(EXP) do{}while(0)
#define DBG_DEFINE_OBJ_TYPE_ID         uint32_t __obj_type_id[0]
#define DBG_SET_OBJ_TYPE_ID(P_OBJ,VAL) do{}while(0)
#define DBG_CHK_OBJ_TYPE_ID(P_OBJ,VAL) do{}while(0)
#endif

#define MMCHC_OBJ_TYPEID    0x693c986aU


#ifndef IS_ALIGNED
#define IS_ALIGNED(PTR, ALIGNMENT)  (((ulong)(PTR) & ((ALIGNMENT) -1U)) == 0)
#endif

//
//==============================================================================


typedef struct
{
    struct mmc *mmc;

    // reg
    uint32_t    iobase;
    uint32_t    hw_clk_reg;
    uint8_t     hwmod_clk_idx;
    uint8_t     hwmod_rst_idx;
    uint8_t     id;

    // DMA
    uint32_t    dma_buf_vaddr;
    uint32_t    dma_buf_size;
    uint8_t     dma_dir;
    uint8_t     dma_mode;

    // clock
    uint32_t    mod_src_clk_freq;
    uint32_t    mod_out_clk_freq;   /* current output freq */
    uint32_t    rq_out_clk_freq;    /* last requested output freq */
    uint8_t     r_delay;            /* current value */
    uint8_t     w_delay;
    uint8_t     cmd_hw_timeout;
    uint8_t     first_scc;

    // MFP
    uint32_t    mfp_reg_addr[4];
    uint32_t    mfp_reg_save[4];
    uint32_t    mfp_reg_mask[4];
    uint8_t     mfp_reg_cnt;

    // state
    uint8_t     randomize_en;
    uint8_t     randomize_seed;
    uint8_t     alt_clk_pin;
    uint8_t     alt_data1_pin;
    uint8_t     hw_has_init;

    DBG_DEFINE_OBJ_TYPE_ID;
} act_mmc_hc_t;


#endif /* __GL520X_MMCH_H__ */
