/*
 * gl520x_mmch.c
 *
 *  Created on: Aug 1, 2014
 *      Author: clamshell
 */

/*
 * notes:
 * 1. forget the card detect pin, don't used it, we use cmd-poll instead; */

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>

#include <asm/arch/actions_arch_common.h>
#include <asm/arch/dvfs_level.h>

#include "gl520x_mmch.h"


/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;

static act_mmc_hc_t s_mmc_hc_pool[3];

//==============================================================================
/* note: 因为有多个线程/ISR会修改state寄存器, 而且state寄存器中混有多种不同写策略的bits
 * 所以就有了下面的专用修改函数. */
static void _mmc_hc_state_reg_clear_pending(act_mmc_hc_t *p_hc, uint32_t pnd_bm)
{
    uint32_t reg_value;

    pnd_bm &= BM_SD_STATE_SDIOB_P|BM_SD_STATE_SDIOA_P|BM_SD_STATE_TEI;

    /* use the R-M-W sequence, since the STATE register have normal RW bits!
     * also, because those pending bits are write-1-cleared, we should
     * clear pending bits read from the STATE register first. */
    reg_value = act_readl(MMCHC_STATE(p_hc));
    reg_value &= ~(BM_SD_STATE_SDIOB_P|BM_SD_STATE_SDIOA_P|BM_SD_STATE_TEI);
    reg_value |= pnd_bm;
    act_writel(reg_value, MMCHC_STATE(p_hc));
    act_readl(MMCHC_STATE(p_hc));
}


//==============================================================================

#if CONFIG_MMCHC_DEBUG_PRINT
static void _mmc_hc_dump_registers(act_mmc_hc_t *p_hc)
{
    DMSG_DBG("hc%u dump registers:\n", p_hc->id);
    // MFP
    DMSG_DBG("  MFP_CTL2=0x%x MFP_CTL3=0x%x PAD_DRV1=0x%x PAD_DRV2=0x%x\n",
            act_readl(MFP_CTL2), act_readl(MFP_CTL3),
            act_readl(PAD_DRV1), act_readl(PAD_DRV2));
    DMSG_DBG("  PAD_PULLCTL1=0x%x PAD_PULLCTL2=0x%x PAD_CTL=0x%x\n",
            act_readl(PAD_PULLCTL1), act_readl(PAD_PULLCTL2), act_readl(PAD_CTL));
    // CLOCK
    DMSG_DBG("  CMU_DEVCLKEN0=0x%x CMU_DEVRST0=0x%x CMU_SD%uCLK=0x%x\n",
            act_readl(CMU_DEVCLKEN0), act_readl(CMU_DEVRST0),
            p_hc->id, act_readl(p_hc->hw_clk_reg));
    // module
    DMSG_DBG("  MMCHC_EN=0x%x MMCHC_CTL=0x%x MMCHC_STATE=0x%x\n",
            act_readl(MMCHC_EN(p_hc)), act_readl(MMCHC_CTL(p_hc)),
            act_readl(MMCHC_STATE(p_hc)));
    DMSG_DBG("  MMCHC_CMD=0x%x MMCHC_ARG=0x%x\n",
            act_readl(MMCHC_CMD(p_hc)), act_readl(MMCHC_ARG(p_hc)));
    DMSG_DBG("  MMCHC_RSPBUF0=0x%x 1=0x%x 2=0x%x 3=0x%x 4=0x%x\n",
            act_readl(MMCHC_RSPBUF0(p_hc)), act_readl(MMCHC_RSPBUF1(p_hc)),
            act_readl(MMCHC_RSPBUF2(p_hc)), act_readl(MMCHC_RSPBUF3(p_hc)),
            act_readl(MMCHC_RSPBUF4(p_hc)));
    DMSG_DBG("  MMCHC_BLK_SIZE=0x%x MMCHC_BLK_NUM=0x%x\n",
            act_readl(MMCHC_BLK_SIZE(p_hc)), act_readl(MMCHC_BLK_NUM(p_hc)));
    DMSG_DBG("  MMCHC_DMA_ADDR=0x%x MMCHC_DMA_CTL=0x%x\n",
            act_readl(MMCHC_DMA_ADDR(p_hc)), act_readl(MMCHC_DMA_CTL(p_hc)));
}
static void _mmc_hc_dump_request(act_mmc_hc_t *p_hc,
        struct mmc_cmd *p_cmd, struct mmc_data *p_data, int err_code)
{
    DMSG_DBG("hc%u dump request: cmd_opcode=%u cmd_arg=0x%x resp_type=0x%x\n",
            p_hc->id, p_cmd->cmdidx, p_cmd->cmdarg, p_cmd->resp_type);
    if(p_data)
    {
        DMSG_DBG("  data_dir=%c data_blksz=%u data_blks=%u data_buf=0x%x\n",
                ((p_data->flags & MMC_DATA_READ)?'r':'w'),
                p_data->blocksize, p_data->blocks, (uint32_t)(p_data->src));
    }
    DMSG_DBG("  result=%d respond=0x%08x,0x%08x,0x%08x,0x%08x\n",
            err_code, p_cmd->response[0], p_cmd->response[1],
            p_cmd->response[2], p_cmd->response[3]);
    if(p_data)
    {
        uint dump_size = p_data->blocksize*p_data->blocks;
        if(dump_size > 512)
        {
            dump_size = 512;
        }
        print_buffer(0, p_data->src, 1, dump_size, 0);
    }
}
#else
#define _mmc_hc_dump_registers(ARGS...) do{}while(0)
#define _mmc_hc_dump_request(ARGS...) do{}while(0)
#endif

static int _mmc_hc_sned_256_dummy_clks(act_mmc_hc_t *p_hc)
{
    uint32_t ctl_reg_val, state_reg_val;

    // send clocks
    ctl_reg_val = act_readl(MMCHC_CTL(p_hc));
    DASSERT((ctl_reg_val & BM_SD_CTL_TS) == 0);
    ctl_reg_val = (ctl_reg_val & ~(BM_SD_CTL_TCN|BM_SD_CTL_TOUTEN|
            BM_SD_CTL_LBE|BM_SD_CTL_C7EN|BM_SD_CTL_TM)) | (8U << BP_SD_CTL_TM);
    act_writel(ctl_reg_val, MMCHC_CTL(p_hc));
    act_readl(MMCHC_CTL(p_hc));
    act_writel(ctl_reg_val | BM_SD_CTL_TS, MMCHC_CTL(p_hc));

    // wait transfer end
    while((act_readl(MMCHC_CTL(p_hc)) & BM_SD_CTL_TS) != 0);
    _mmc_hc_state_reg_clear_pending(p_hc, BM_SD_STATE_TEI);

    state_reg_val = act_readl(MMCHC_STATE(p_hc));
    if(state_reg_val & (BM_SD_CTL_TOUTEN|BM_SD_STATE_CLNR|
            BM_SD_STATE_WC16ER|BM_SD_STATE_RC16ER|BM_SD_STATE_CRC7ER))
    {
        DMSG_ERR("hc%u send 256 extra clock err, state_reg=0x%x\n",
                p_hc->id, state_reg_val);
        return -1;
    }
    return 0;
}

static void _mmc_hc_switch_data_bus(act_mmc_hc_t *p_hc, uint new_bus)
{
    uint32_t reg_val;
    reg_val = act_readl(MMCHC_EN(p_hc));
    if(((reg_val >> BP_SD_EN_BSEL) & 1U) != new_bus)
    {
        act_writel((reg_val&~BM_SD_EN_BSEL) | ((new_bus&1U)<<BP_SD_EN_BSEL), MMCHC_EN(p_hc));
        act_readl(MMCHC_EN(p_hc));
    }
}

static void _mmc_hc_handle_cpu_mode_data_transfer(act_mmc_hc_t *p_hc)
{
    uint32_t data_size, state_reg;
    uint32_t *p_buf4;
    uint8_t *p_buf1;

    p_buf4 = (uint32_t*)(p_hc->dma_buf_vaddr);
    data_size = p_hc->dma_buf_size;

    if(p_hc->dma_dir == 0) // read?
    {
        if(IS_ALIGNED((uint32_t)p_buf4, 4U))
        {
            while(data_size >= 4U)
            {
                while(1)
                {
                    state_reg = act_readl(MMCHC_STATE(p_hc));
                    if((state_reg & BM_SD_STATE_MEMRDY) != 0) /* check FIFO first */
                    {
                        break;
                    }
                    if(unlikely((state_reg & BM_SD_STATE_TEI) != 0))
                    {
                        goto label_end;
                    }
                }
                *p_buf4++ = act_readl(MMCHC_DAT(p_hc));
                data_size -= 4U;
            }
        }
        if(data_size != 0)
        {
            uint32_t reg_data = 0, cnt = 0;
            p_buf1 = (uint8_t*)p_buf4;
            do
            {
                if(unlikely(cnt == 0))
                {
                    while(1)
                    {
                        state_reg = act_readl(MMCHC_STATE(p_hc));
                        if((state_reg & BM_SD_STATE_MEMRDY) != 0) /* check FIFO first */
                        {
                            break;
                        }
                        if(unlikely((state_reg & BM_SD_STATE_TEI) != 0))
                        {
                            goto label_end;
                        }
                    }
                    reg_data = act_readl(MMCHC_DAT(p_hc));
                    cnt = 4;
                }
                *p_buf1++ = reg_data;
                reg_data = reg_data >> 8;
                cnt--;
            } while(--data_size != 0);
        }
    }
    else
    {
        uint32_t tmp;
        if(IS_ALIGNED((uint32_t)p_buf4, 4U))
        {
            while(data_size >= 4U)
            {
                do
                {
                    state_reg = act_readl(MMCHC_STATE(p_hc));
                    if(unlikely((state_reg & BM_SD_STATE_TEI) != 0))
                    {
                        goto label_end;
                    }
                } while((state_reg & BM_SD_STATE_MEMRDY) == 0);
                tmp = *p_buf4++;
                act_writel(tmp, MMCHC_DAT(p_hc));
                data_size -= 4U;
            }
        }
        if(data_size != 0)
        {
            uint32_t reg_data = 0, cnt = 0;
            p_buf1 = (uint8_t*)p_buf4;
            do
            {
                tmp = *p_buf1++;
                reg_data |= tmp << cnt;
                cnt += 8;
                if(unlikely(cnt == 32))
                {
                    do
                    {
                        state_reg = act_readl(MMCHC_STATE(p_hc));
                        if(unlikely((state_reg & BM_SD_STATE_TEI) != 0))
                        {
                            goto label_end;
                        }
                    } while((state_reg & BM_SD_STATE_MEMRDY) == 0);
                    act_writel(reg_data, MMCHC_DAT(p_hc));
                    cnt = 0;
                    reg_data = 0;
                }
            } while(--data_size != 0);
            if(cnt != 0)
            {
                do
                {
                    state_reg = act_readl(MMCHC_STATE(p_hc));
                    if(unlikely((state_reg & BM_SD_STATE_TEI) != 0))
                    {
                        goto label_end;
                    }
                } while((state_reg & BM_SD_STATE_MEMRDY) == 0);
                act_writel(reg_data, MMCHC_DAT(p_hc));
            }
        }
    }

    label_end:
    act_readl(MMCHC_STATE(p_hc)); // drain BUS write buffer.
    return;
}

static int _mmc_hc_cmd_end_handler(act_mmc_hc_t *p_hc,
        struct mmc_cmd *p_cmd, struct mmc_data *p_data)
{
    uint32_t state_reg_val, ctrl_reg_val, state_reg_err_msk;
    int cmd_err;

    cmd_err = 0;
    ctrl_reg_val = act_readl(MMCHC_CTL(p_hc));
    DASSERT((ctrl_reg_val & BM_SD_CTL_TS) == 0); // be sure not in transfer state
    state_reg_val = act_readl(MMCHC_STATE(p_hc));

    state_reg_err_msk = BM_SD_STATE_TOUTE | BM_SD_STATE_BAEP |
            BM_SD_STATE_WC16ER | BM_SD_STATE_RC16ER | BM_SD_STATE_CLNR |
            BM_SD_STATE_CRC7ER;
    if(unlikely((state_reg_val & state_reg_err_msk) != 0))
    {
        // the check sequence is important
        if(unlikely(state_reg_val & BM_SD_STATE_TOUTE))
        {
            // timeout
            DMSG_ERR("hc%u cmd timeout, cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                    p_hc->id, p_cmd->cmdidx, state_reg_val, ctrl_reg_val);
            cmd_err = TIMEOUT;
        }
        if(unlikely(state_reg_val & BM_SD_STATE_BAEP))
        {
            // boot mode error.
            DASSERT(0);
        }
        if(unlikely(state_reg_val & (BM_SD_STATE_WC16ER|BM_SD_STATE_RC16ER)))
        {
            // CRC16 error
            if(state_reg_val & BM_SD_STATE_WC16ER)
            {
                DMSG_ERR("hc%u data write CRC16 err, cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                        p_hc->id, p_cmd->cmdidx, state_reg_val, ctrl_reg_val);
            }
            if(state_reg_val & BM_SD_STATE_RC16ER)
            {
                DMSG_ERR("hc%u data read CRC16 err, cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                        p_hc->id, p_cmd->cmdidx, state_reg_val, ctrl_reg_val);
            }
            cmd_err = COMM_ERR;
        }
        if(p_cmd->resp_type & MMC_RSP_PRESENT)
        {
            if(unlikely(state_reg_val & BM_SD_STATE_CLNR))
            {
                // no response
                DMSG_ERR("hc%u cmd missing response, cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                        p_hc->id, p_cmd->cmdidx, state_reg_val, ctrl_reg_val);
                cmd_err = COMM_ERR;
            }
            if(unlikely(state_reg_val & BM_SD_STATE_CRC7ER))
            {
                // CRC7 error
                DMSG_ERR("hc%u cmd CRC7 err, cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                        p_hc->id, p_cmd->cmdidx, state_reg_val, ctrl_reg_val);
                cmd_err = COMM_ERR;
            }
        }
    }
    else
    {
        DASSERT((state_reg_val & BM_SD_STATE_CLC) != 0);
    }

    // fill the response
    // the upplayer expect a big-endian order, and only the payload is need.
    if(likely(p_cmd->resp_type & MMC_RSP_PRESENT))
    {
        uint32_t resp_opcode_st;

        if(p_cmd->resp_type & MMC_RSP_136)
        {
            // long response
            p_cmd->response[0] = act_readl(MMCHC_RSPBUF3(p_hc));
            p_cmd->response[1] = act_readl(MMCHC_RSPBUF2(p_hc));
            p_cmd->response[2] = act_readl(MMCHC_RSPBUF1(p_hc));
            p_cmd->response[3] = act_readl(MMCHC_RSPBUF0(p_hc));
            resp_opcode_st = act_readl(MMCHC_RSPBUF4(p_hc)) & 0xffU;
        }
        else
        {
            // short response
            uint32_t reg_l = act_readl(MMCHC_RSPBUF0(p_hc));
            uint32_t reg_h = act_readl(MMCHC_RSPBUF1(p_hc));
            p_cmd->response[0] = (reg_h << 24) | (reg_l >> 8);
            p_cmd->response[1] = 0;
            p_cmd->response[2] = 0;
            p_cmd->response[3] = 0;
            resp_opcode_st = (reg_h >> 8) & 0xffU;
        }
        /* 5009的SD控制器收respond是全收的, 包括start和end bit都会收进来,
         * 而mmc的上层只需要其中的有效载荷部分, 所以这里要作截取.
         * 翻阅其它的host实现得知, 对于短respond, 32bit载荷放在resp[0],
         * 对于长respond, bit127:96放在resp[0], bit95:64放在resp[1], 类推. */

        // check the op_code in respond.
        if(likely(cmd_err==0) && likely(p_cmd->resp_type & MMC_RSP_OPCODE))
        {
            if(unlikely(resp_opcode_st != p_cmd->cmdidx))
            {
                DMSG_ERR("hc%u wrong resp_cmd, "
                        "cmd=%u resp_cmd=%u state_reg=0x%x ctrl_reg=0x%x\n",
                        p_hc->id, p_cmd->cmdidx, resp_opcode_st,
                        state_reg_val, ctrl_reg_val);
                cmd_err = COMM_ERR;
            }
        }
    }

    if(p_data != NULL)
    {
        if(likely(p_hc->dma_mode != 0))
        {
            // clear DMA pending & start flag
            act_writel(act_readl(MMCHC_DMA_CTL(p_hc)) & ~BM_SD_DMA_CTL_SDDS,
                    MMCHC_DMA_CTL(p_hc));
            act_readl(MMCHC_DMA_CTL(p_hc));

            // invalidate cache
            if(p_data->flags & MMC_DATA_READ)
            {
                invalidate_dcache_range(
                        p_hc->dma_buf_vaddr,
                        p_hc->dma_buf_vaddr + p_hc->dma_buf_size);
            }
        }
    }

    if(unlikely(cmd_err != 0))
    {
        _mmc_hc_sned_256_dummy_clks(p_hc);
    }

    _mmc_hc_dump_request(p_hc, p_cmd, p_data, cmd_err);
    return cmd_err;
}

static int _mmc_hc_send_cmd(struct mmc *mmc, struct mmc_cmd *p_cmd, struct mmc_data *p_data)
{
    act_mmc_hc_t *p_hc;
    uint transfer_mode, need_crc7, timeout, dma_mode;
    uint32_t ctrl_reg_val;
    int iret;

    p_hc = mmc->priv;
    DBG_CHK_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);
    DASSERT(p_cmd != NULL);

    DMSG_DBG("hc%u call send_cmd, cmd_opcode=%u\n", p_hc->id, p_cmd->cmdidx);

    timeout = 0;
    need_crc7 = 0;
    transfer_mode = 0;
    switch(p_cmd->resp_type)
    {
    case MMC_RSP_NONE: /* no response */
        transfer_mode = 0;
        break;
    case MMC_RSP_R1: /* 48-bit response */
        if(p_data)
        {
            transfer_mode = (p_data->flags & MMC_DATA_READ) ? 4 : 5;
            timeout = p_hc->cmd_hw_timeout; // ~2s
        }
        else
        {
            transfer_mode = 1;
        }
        need_crc7 = 1;
        break;
    case MMC_RSP_R1b: /* 48-bit response, with busy */
        transfer_mode = 3;
        need_crc7 = 1;
        if(likely(p_hc->mod_out_clk_freq != 0))
        {
            /* note: ios.clock 和 cmd_timeout_ms 都可能是0. */
            //hw_to_ticks = (ms / 1000U) * (out_clk_hz / (256U*4096U));
            //hw_to_ticks = (ms * out_clk_hz) / (256U*4096U*1000U);
            uint32_t timeout_ms = 4000U;
            uint32_t tmp = (p_hc->mod_out_clk_freq + 4096U -1U) / 4096U;
            timeout = (timeout_ms * tmp + 256U*1000U -1U) / (256U*1000U);
            if(timeout > 128U)
            {
                DMSG_WARN("hc%u hw_timeout exceed limit, need=%u limit=128\n",
                        p_hc->id, timeout);
                timeout = 128; // set to max valid timeout: 128 ticks (~2.6s@50MHz)
            }
        }
        break;
    case MMC_RSP_R2: /* 136-bit response, without CRC7 */
        /* the CRC-7 in this respond packet is just for the internal data,
         * not for the whole packet, so do not enable CRC-7 checking here. */
        transfer_mode = 2;
        break;
    case MMC_RSP_R3: /* 48-bit response, without CRC7 */
        transfer_mode = 1;
        break;
    //case MMC_RSP_R4: /* SDIO CMD5 response, same as R3 */
    //case MMC_RSP_R5: /* SDIO CMD52 CMD53 response, same as R1 */
    //case MMC_RSP_R6: /* same as R1 */
    //case MMC_RSP_R7: /* same as R1 */
    default:
        DASSERT(0);
        hang();
    }

    // also clear SCC here.
    ctrl_reg_val = ((p_hc->r_delay & 0xfU) << BP_SD_CTL_RDELAY) |
            ((p_hc->w_delay & 0xfU) << BP_SD_CTL_WDELAY) |
            BM_SD_CTL_LBE | (transfer_mode<<BP_SD_CTL_TM); // do not set BM_SD_CTL_TS
    if(timeout != 0)
    {
        if(timeout >= 128U)
        {
            timeout = 0; // 0 means 128 ticks.
        }
        ctrl_reg_val |= BM_SD_CTL_TOUTEN | (timeout<<BP_SD_CTL_TOUTCNT);
    }
    if(unlikely(need_crc7 == 0))
    {
        // C7EN is the function of checking CRC-7 in a respond.
        ctrl_reg_val |= BM_SD_CTL_C7EN; // set 1 disable.
    }
    act_writel(ctrl_reg_val, MMCHC_CTL(p_hc));
    // wait TS==0, also drain the bus write-buffer
    while((act_readl(MMCHC_CTL(p_hc)) & BM_SD_CTL_TS) != 0);

    // set command and argument
    act_writel(p_cmd->cmdarg, MMCHC_ARG(p_hc)); // argument go first.
    act_writel(p_cmd->cmdidx & 0x3fU, MMCHC_CMD(p_hc));

    // if the cmd has data transfer, config the DMA
    dma_mode = 0;
    if(p_data != NULL)
    {
        uint32_t blk_size, blk_cnt, total_dma_size;
        uint32_t ram_vaddr;

        blk_size = p_data->blocksize;
        DASSERT(blk_size <= MMCHC_MAX_BLK_SIZE); /* 注册驱动时应该向上层设置这些限制. */
        blk_cnt = p_data->blocks;
        DASSERT(blk_cnt <= MMCHC_MAX_BLK_PER_TRAN);
        DASSERT(!((blk_cnt>1U) && !IS_ALIGNED(blk_size, 4U))); /* blksize limit */
        total_dma_size = blk_size * blk_cnt;
        DASSERT(total_dma_size <= MMCHC_MAX_DMA_SIZE_PER_TRAN);

        // set block size & block count
        act_writel(blk_size, MMCHC_BLK_SIZE(p_hc));
        act_writel(blk_cnt, MMCHC_BLK_NUM(p_hc));

        ram_vaddr = (p_data->flags & MMC_DATA_READ) ?
                (uint32_t)(p_data->dest) : (uint32_t)(p_data->src);

        if(likely(IS_ALIGNED(ram_vaddr, ARCH_DMA_MINALIGN) &&
                IS_ALIGNED(total_dma_size, ARCH_DMA_MINALIGN)))
        {
            _mmc_hc_switch_data_bus(p_hc, 1); // switch to SPECIAL BUS

            // config special DMA
            act_writel(ram_vaddr, MMCHC_DMA_ADDR(p_hc));
            act_writel(BM_SD_DMA_CTL_SDDFIP | (total_dma_size << BP_SD_DMA_CTL_SDDNUM),
                    MMCHC_DMA_CTL(p_hc));
            act_readl(MMCHC_DMA_CTL(p_hc));

            // flush cache
            flush_dcache_range(ram_vaddr, ram_vaddr+total_dma_size);

            dma_mode = 1;
        }
        else
        {
            _mmc_hc_switch_data_bus(p_hc, 0); // switch to AHB

            if(unlikely(total_dma_size > 256))
            {
                DMSG_WARN("hc%u data not aligned, use CPU transfer instead (slow)\n",
                        p_hc->id);
            }
            DMSG_DBG("hc%u CPU transfer mode activated (slow)\n", p_hc->id);
        }
        p_hc->dma_buf_vaddr = ram_vaddr;
        p_hc->dma_buf_size  = total_dma_size;
        p_hc->dma_dir = (p_data->flags & MMC_DATA_READ) ? 0 : 1;
        p_hc->dma_mode = dma_mode;
    }

    // start command
    {
        DASSERT((act_readl(MMCHC_STATE(p_hc)) & BM_SD_STATE_TEI) == 0);

        // start transfer
        act_writel(ctrl_reg_val | BM_SD_CTL_TS, MMCHC_CTL(p_hc));
        act_readl(MMCHC_CTL(p_hc));
        // start DMA & wait
        if(p_data)
        {
            if(likely(dma_mode))
            {
                act_writel(act_readl(MMCHC_DMA_CTL(p_hc)) | BM_SD_DMA_CTL_SDDS,
                        MMCHC_DMA_CTL(p_hc));
                while((act_readl(MMCHC_DMA_CTL(p_hc)) & BM_SD_DMA_CTL_SDDFIP) == 0);
            }
            else
            {
                _mmc_hc_handle_cpu_mode_data_transfer(p_hc);
            }
        }

        // wait transfer end
        while((act_readl(MMCHC_CTL(p_hc)) & BM_SD_CTL_TS) != 0);
        _mmc_hc_state_reg_clear_pending(p_hc, BM_SD_STATE_TEI);

        // call end handler
        iret = _mmc_hc_cmd_end_handler(p_hc, p_cmd, p_data);
    }

    return iret;
}

static void _mmc_hc_cfg_outclk(act_mmc_hc_t *p_hc, uint32_t rq_out_freq)
{
    uint div, div128, comp_div;
    uint32_t reg_val, real_out_freq_h, hw_timeout_ticks;

    if(rq_out_freq != 0)
    {
        div = (p_hc->mod_src_clk_freq -1U) / (rq_out_freq * 2U) +1U;
        if(div <= 32)
        {
            div128 = 0;
        }
        else
        {
            div128 = 1;
            div = (p_hc->mod_src_clk_freq -1U) / (rq_out_freq * 2U * 128U) +1U;
            if(div > 32)
            {
                div = 32;
            }
        }
    }
    else
    {
        div128 = 1;
        div = 32;
    }
    if(div > 0)
    {
        div--;
    }
    reg_val = act_readl(p_hc->hw_clk_reg);
    reg_val &= ~((0x1fU << 0) | (1U << 8));
    reg_val |= (div << 0) | (div128 << 8);
    act_writel(reg_val, p_hc->hw_clk_reg);
    act_readl(p_hc->hw_clk_reg);

    comp_div = 2U * (div + 1) * ((div128 == 0) ? 1 : 128U);
    real_out_freq_h = (p_hc->mod_src_clk_freq +comp_div -1U) / comp_div; // in Hz
    p_hc->mod_out_clk_freq = real_out_freq_h;

    // clock delay
    if(real_out_freq_h < 10000000)
    {
        p_hc->r_delay = 0x1;
        p_hc->w_delay = 0xf;
    }
    else if(real_out_freq_h<=40000000)
    {
        p_hc->r_delay = 0x1;
        p_hc->w_delay = 0xf;
    }
    else if(real_out_freq_h<=60000000)
    {
        p_hc->r_delay = 0x1;
        p_hc->w_delay = 0xb;
    }
    else if(real_out_freq_h<=80000000)
    {
        p_hc->r_delay = 0x2;
        p_hc->w_delay = 0x9;
    }
    else if(real_out_freq_h<=130000000)
    {
        p_hc->r_delay = 0x3;
        p_hc->w_delay = 0x7;
    }
    else
    {
        DMSG_ERR("hc%u: no valid clk delay @ %uHz\n", p_hc->id, real_out_freq_h);
        DASSERT(0);
    }

    // re-calculate command HW timeout value (in case timeout==1000ms)
    //hw_to_ticks = (1000U / 1000U) * (real_out_clk_hz / (256U*4096U));
    //hw_to_ticks = (1000U * real_out_clk_hz) / (256U*4096U*1000U);
    /* 变换时注意防止溢出 */
    hw_timeout_ticks = (1U * real_out_freq_h + (256U*4096U -1U)) / (256U*4096U);
    if(hw_timeout_ticks > 128U)
    {
        hw_timeout_ticks = 128U;
    }
    p_hc->cmd_hw_timeout = hw_timeout_ticks;

    DMSG_DBG("hc%u clk_cfg, src_clk=%u rq_clk=%u real_clk=%u c_div=%u "
            "hw_to=%u r_dly=%u w_dly=%u\n",
            p_hc->id, p_hc->mod_src_clk_freq, rq_out_freq, real_out_freq_h,
            comp_div, hw_timeout_ticks, p_hc->r_delay, p_hc->w_delay);
}

static void _mmc_hc_set_ios(struct mmc *mmc)
{
    act_mmc_hc_t *p_hc;
    uint32_t reg_val, reg_old_val;

    p_hc = mmc->priv;
    DBG_CHK_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);

    DMSG_DBG("hc%u call set_ios, clock=%u bus_width=%u\n",
            p_hc->id, mmc->clock, mmc->bus_width);

    /* currently, only mmc->clock & mmc->bus_width changes. */

    // clock
    if(mmc->clock != p_hc->rq_out_clk_freq)
    {
        p_hc->rq_out_clk_freq = mmc->clock;
        _mmc_hc_cfg_outclk(p_hc, mmc->clock);

        if(p_hc->first_scc != 0)
        {
            p_hc->first_scc = 0; // disable

            // enable continue clock output, to meet the initial 74clk requirement.
            act_writel(act_readl(MMCHC_CTL(p_hc)) | BM_SD_CTL_SCC, MMCHC_CTL(p_hc));
            act_readl(MMCHC_CTL(p_hc));
            // the SCC will be clear when first command comes.
            DMSG_DBG("hc%u ios: sending_init_clock enabled\n", p_hc->id);
            mdelay(2);
        }
    }

    // bus_width
    reg_val = reg_old_val = act_readl(MMCHC_EN(p_hc));
    switch(mmc->bus_width)
    {
    case 1:
        reg_val = (reg_val & ~(BM_SD_EN_DATAWID)) | (0 << BP_SD_EN_DATAWID);
        break;
    case 4:
        reg_val = (reg_val & ~(BM_SD_EN_DATAWID)) | (1 << BP_SD_EN_DATAWID);
        break;
    default:
        DMSG_ERR("hc%u ios: unknown bus width %u\n", p_hc->id, mmc->bus_width);
    }
    if(reg_val != reg_old_val)
    {
        clrbits_le32(MMCHC_EN(p_hc), BM_SD_EN_EN);
        act_readl(MMCHC_EN(p_hc));
        act_writel(reg_val & ~BM_SD_EN_EN, MMCHC_EN(p_hc));
        act_readl(MMCHC_EN(p_hc));
        act_writel(reg_val, MMCHC_EN(p_hc));
        act_readl(MMCHC_EN(p_hc));
    }
}

static int _mmc_hc_getcd(struct mmc *mmc)
{
#if CONFIG_MMCHC_DEBUG_PRINT
    act_mmc_hc_t *p_hc = mmc->priv;
    DBG_CHK_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);
    DMSG_DBG("hc%u call getcd\n", p_hc->id);
#endif
    return 1; // always in
}

static void _mmc_hc_clk_init(act_mmc_hc_t *p_hc)
{
                                          //  1 2 3 4 5 6 7 8
    static const uint8_t s_nic_div_tbl[] = {0,0,1,2,2,3,3,3,3};
    uint dev_clk_freq, nic_div, nic_div_log2;

    dev_clk_freq = act_cmu_get_dev_clk();
    nic_div = (dev_clk_freq -1U) / 120000000U + 1U;
    if(nic_div > 8)
    {
        nic_div = 8;
    }
    nic_div_log2 = s_nic_div_tbl[nic_div];
    act_writel(0 | (nic_div_log2<<16) | (1U<<8) | 0x1fU, p_hc->hw_clk_reg);
    p_hc->mod_out_clk_freq = 0;
    p_hc->rq_out_clk_freq  = 0;
    p_hc->mod_src_clk_freq = dev_clk_freq;

    DMSG_DBG("hc%u clk_init, mod_src_clk=%u nic_div=%u nic_div_reg=%u\n",
            p_hc->id, dev_clk_freq, nic_div, nic_div_log2);
}

static void _mmc_hc_mfp_mod_reg(act_mmc_hc_t *p_hc,
        uint32_t reg_addr, uint32_t reg_mask, uint32_t new_val)
{
    uint32_t i, reg_val;

    i = p_hc->mfp_reg_cnt;
    DASSERT(i < ARRAY_SIZE(p_hc->mfp_reg_addr));
    p_hc->mfp_reg_addr[i] = reg_addr;
    p_hc->mfp_reg_mask[i] = reg_mask;

    reg_val = act_readl(reg_addr);
    p_hc->mfp_reg_save[i] = reg_val;
    act_writel((reg_val & ~reg_mask) | (new_val & reg_mask), reg_addr);

    p_hc->mfp_reg_cnt = i + 1;
}

static void _mmc_hc_mfp_init(act_mmc_hc_t *p_hc)
{
    uint32_t reg_val;

    //pad enable
    reg_val = act_readl(PAD_CTL);
    if((reg_val & (1U << 1)) == 0)
    {
        act_writel(reg_val | (1U<<1), PAD_CTL);
    }

    p_hc->mfp_reg_cnt = 0;

    switch(p_hc->id)
    {
    case 0:
        /*global sd0 mfp enable*/
        _mmc_hc_mfp_mod_reg(p_hc, MFP_CTL2, 0x000ff9e0, 0);
        /* ASOC must set these bits,to choise the sd_bus mode*/
        _mmc_hc_mfp_mod_reg(p_hc, PAD_PULLCTL1, 0x0003e000, 0x0003e000);
        /* level 2 */
        _mmc_hc_mfp_mod_reg(p_hc, PAD_DRV1, 0x00cf0000, 0x00450000);
        break;

    case 1:
        /*global sd1 mfp enable*/
        _mmc_hc_mfp_mod_reg(p_hc, MFP_CTL2, 0x00000618, 0x00000600);
        _mmc_hc_mfp_mod_reg(p_hc, PAD_PULLCTL1, 0x00000878, 0x00000878);
        //level2
        _mmc_hc_mfp_mod_reg(p_hc, PAD_DRV1, 0x0030f000, 0x00105000);
        break;

    case 2:
        if(ASOC_GET_IC(gd->arch.dvfs_level)==0x7021 ||
                ASOC_GET_IC(gd->arch.dvfs_level)==0x7023)
        {
            /*global sd2 mfp enable*/
            _mmc_hc_mfp_mod_reg(p_hc, MFP_CTL3, 0x01800000, 0x00800000);
            _mmc_hc_mfp_mod_reg(p_hc, PAD_PULLCTL2, 0x0000f800, 0x0000f800);
            _mmc_hc_mfp_mod_reg(p_hc, PAD_DRV2, 0x00000300, 0x00000100);
        }
        else
        {
            /*global sd2 mfp enable*/
            _mmc_hc_mfp_mod_reg(p_hc, MFP_CTL3, 0xf000000, 0xf000000);
            _mmc_hc_mfp_mod_reg(p_hc, PAD_PULLCTL1, 0x0e000000, 0x0e000000);
            _mmc_hc_mfp_mod_reg(p_hc, PAD_PULLCTL2, 0x00000078, 0x00000048);
        }
        break;
    }

    DMSG_DBG("hc%u mfp_cfg, MFP_CTL2=0x%x MFP_CTL3=0x%x "
            "PAD_PULLCTL1=0x%x PAD_PULLCTL2=0x%x PAD_DRV1=0x%x PAD_DRV2=0x%x\n",
            p_hc->id, act_readl(MFP_CTL2), act_readl(MFP_CTL3),
            act_readl(PAD_PULLCTL1), act_readl(PAD_PULLCTL2),
            act_readl(PAD_DRV1), act_readl(PAD_DRV2));
}
static void _mmc_hc_mfp_uninit(act_mmc_hc_t *p_hc)
{
    uint32_t i, reg_addr, reg_val, reg_mask;

    // mainly for re-enable UART5 via SD0 pins
    for(i=0; i<p_hc->mfp_reg_cnt; i++)
    {
        reg_addr = p_hc->mfp_reg_addr[i];
        reg_val = p_hc->mfp_reg_save[i];
        reg_mask = p_hc->mfp_reg_mask[i];
        act_writel((act_readl(reg_addr) & ~reg_mask) | (reg_val & reg_mask), reg_addr);
    }

    DMSG_DBG("hc%u mfp_restore, MFP_CTL2=0x%x MFP_CTL3=0x%x "
            "PAD_PULLCTL1=0x%x PAD_PULLCTL2=0x%x PAD_DRV1=0x%x PAD_DRV2=0x%x\n",
            p_hc->id, act_readl(MFP_CTL2), act_readl(MFP_CTL3),
            act_readl(PAD_PULLCTL1), act_readl(PAD_PULLCTL2),
            act_readl(PAD_DRV1), act_readl(PAD_DRV2));
}

static int _mmc_hc_init_hw_mod(act_mmc_hc_t *p_hc)
{
    uint32_t reg_en_val;

    // clock init
    _mmc_hc_clk_init(p_hc);

    // enable hw module
    act_cmu_module_clk_ctrl(p_hc->hwmod_clk_idx, 1);
    act_cmu_reset_modules(p_hc->hwmod_rst_idx);

    // mfp & external pull-up
    _mmc_hc_mfp_init(p_hc);

    // basic controller config
    reg_en_val = (p_hc->randomize_en << BP_SD_EN_RANE) |
            (p_hc->randomize_seed << BP_SD_EN_RAN_SEED) |
            (p_hc->alt_clk_pin << BP_SD_EN_CLK_S) |
            (p_hc->alt_data1_pin << BP_SD_EN_DAT1_S) |
            BM_SD_EN_BSEL |
            (0 << BP_SD_EN_DATAWID);
    act_writel(reg_en_val, MMCHC_EN(p_hc));
    act_readl(MMCHC_EN(p_hc));
    reg_en_val |= BM_SD_EN_EN;
    act_writel(reg_en_val, MMCHC_EN(p_hc));
    act_readl(MMCHC_EN(p_hc));

    _mmc_hc_dump_registers(p_hc);
    return 0;
}

static void _mmc_hc_exit_hw_mod(act_mmc_hc_t *p_hc)
{
    // enable hw module
    act_cmu_reset_modules(p_hc->hwmod_rst_idx);
    act_cmu_module_clk_ctrl(p_hc->hwmod_clk_idx, 0);

    // mfp & external pull-up
    _mmc_hc_mfp_uninit(p_hc);
}

// this function will be call many times
static int _mmc_hc_init(struct mmc *mmc)
{
    act_mmc_hc_t *p_hc;

    p_hc = mmc->priv;
    DBG_CHK_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);

    DMSG_DBG("hc%u call init\n", p_hc->id);

    // init hw module if need
    if(p_hc->hw_has_init == 0)
    {
        p_hc->hw_has_init = 1;
        _mmc_hc_init_hw_mod(p_hc);
    }

    // reset state
    p_hc->first_scc = 1;

    return 0;
}

static void _mmc_hc_exit(struct mmc *mmc)
{
    act_mmc_hc_t *p_hc;

    p_hc = mmc->priv;
    DBG_CHK_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);

    DMSG_DBG("hc%u call exit\n", p_hc->id);

    if(p_hc->hw_has_init != 0)
    {
        p_hc->hw_has_init = 0;
        _mmc_hc_exit_hw_mod(p_hc);
    }
}

static const struct mmc_ops s_act_mmc_ops = {
    .send_cmd   = _mmc_hc_send_cmd,
    .set_ios    = _mmc_hc_set_ios,
    .init       = _mmc_hc_init,
    .exit       = _mmc_hc_exit,
    .getcd      = _mmc_hc_getcd,
};

static struct mmc_config s_act_mmc_cfg = {
    .name       = "gl520x MMC host",
    .ops        = &s_act_mmc_ops,
    .host_caps  = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT | MMC_MODE_HC,
    .voltages   = MMC_VDD_32_33 | MMC_VDD_33_34,
    .b_max      = MMCHC_MAX_BLK_PER_TRAN, /* Maximum blocks that can go in each command. */
};

static int _mmc_hc_init_obj(act_mmc_hc_t *p_hc)
{
    struct mmc *mmc = mmc_create(&s_act_mmc_cfg, p_hc);
    if (mmc == NULL)
    {
        DMSG_ERR("no mem\n");
        return -1;
    }
    p_hc->mmc = mmc;
    DBG_SET_OBJ_TYPE_ID(p_hc, MMCHC_OBJ_TYPEID);
    return 0;
}

static void _mmc_hc_destroy_obj(act_mmc_hc_t *p_hc)
{
    if(p_hc->mmc == NULL)
    {
        // not init.
        return;
    }

    // stop host
    _mmc_hc_exit(p_hc->mmc);

    // free mmc obj
    mmc_destroy(p_hc->mmc);
    memset(p_hc, 0, sizeof(act_mmc_hc_t));
}

int act_mmchc_init(bd_t *bis)
{
    act_mmc_hc_t *p_hc;
    int ret;

    s_act_mmc_cfg.f_max = 50000000;
    s_act_mmc_cfg.f_min = 1;

    memset(s_mmc_hc_pool, 0, sizeof(s_mmc_hc_pool));

    // SD #0
    p_hc = &s_mmc_hc_pool[0];
    ret = _mmc_hc_init_obj(p_hc);
    if(ret != 0)
    {
        goto label_err;
    }
    p_hc->iobase = SD0_EN;
    p_hc->hw_clk_reg = CMU_SD0CLK;
    p_hc->hwmod_clk_idx = 0+5;  // 5 6 7
    p_hc->hwmod_rst_idx = 0+4;  // 4 5 9
    p_hc->id = 0;

    // SD #1
    p_hc = &s_mmc_hc_pool[1];
    ret = _mmc_hc_init_obj(p_hc);
    if(ret != 0)
    {
        goto label_err;
    }
    p_hc->iobase = SD1_EN;
    p_hc->hw_clk_reg = CMU_SD1CLK;
    p_hc->hwmod_clk_idx = 0+6;  // 5 6 7
    p_hc->hwmod_rst_idx = 0+5;  // 4 5 9
    p_hc->id = 1;

    // SD #2
    p_hc = &s_mmc_hc_pool[2];
    ret = _mmc_hc_init_obj(p_hc);
    if(ret != 0)
    {
        goto label_err;
    }
    p_hc->iobase = SD2_EN;
    p_hc->hw_clk_reg = CMU_SD2CLK;
    p_hc->hwmod_clk_idx = 0+7;  // 5 6 7
    p_hc->hwmod_rst_idx = 0+9;  // 4 5 9
    p_hc->id = 2;

    return 0;

    label_err:
    act_mmchc_exit();
    return -1;
}

void act_mmchc_exit(void)
{
    uint i;
    for(i=0; i<ARRAY_SIZE(s_mmc_hc_pool); i++)
    {
        _mmc_hc_destroy_obj(&s_mmc_hc_pool[i]);
    }
}
