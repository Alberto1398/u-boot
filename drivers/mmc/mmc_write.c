/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <part.h>
#include <div64.h>
#include <linux/math64.h>
#include "mmc_private.h"

static ulong mmc_erase_t(struct mmc *mmc, ulong start, lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	ulong end;
	int err, start_cmd, end_cmd;

	if (mmc->high_capacity) {
		end = start + blkcnt - 1;
	} else {
		end = (start + blkcnt - 1) * mmc->write_bl_len;
		start *= mmc->write_bl_len;
	}

	if (IS_SD(mmc)) {
		start_cmd = SD_CMD_ERASE_WR_BLK_START;
		end_cmd = SD_CMD_ERASE_WR_BLK_END;
	} else {
		start_cmd = MMC_CMD_ERASE_GROUP_START;
		end_cmd = MMC_CMD_ERASE_GROUP_END;
	}

	cmd.cmdidx = start_cmd;
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.cmdarg = end;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.cmdarg = SECURE_ERASE;
	cmd.resp_type = MMC_RSP_R1b;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	return 0;

err_out:
	puts("mmc erase failed\n");
	return err;
}

unsigned long mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt)
{
	int err = 0;
	u32 start_rem, blkcnt_rem;
	struct mmc *mmc = find_mmc_device(dev_num);
	lbaint_t blk = 0, blk_r = 0;
	int timeout = 1000;
#ifdef CONFIG_OWL_EMMC_RAID0
    int nextIndex = 0, nextStart = 0, nextBlkcnt = 0, blkcntErase = 0;
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/

	if (!mmc)
		return -1;
    
#ifdef CONFIG_OWL_EMMC_RAID0
	if (mmc->raid0[0] != NULL) {
        int blocksPerPage, index, end, endIndex;
        
        blkcntErase = blkcnt;
        blocksPerPage = 4096 / mmc->write_bl_len;
        
        if((start & blocksPerPage) == 0) {
            index = 0;
            nextIndex = 1;
        }
        else {
            index = 1;
            nextIndex = 0;
        }
        
        end = start + blkcnt - 1;
        if((end & blocksPerPage) == 0)
            endIndex = 0;
        else
            endIndex = 1;
        if(index != endIndex)
            end = (end & (~(blocksPerPage - 1))) - 1;
        
        start = ((start / (blocksPerPage * 2)) * blocksPerPage) + (start & (blocksPerPage - 1));
        end = ((end / (blocksPerPage * 2)) * blocksPerPage) + (end & (blocksPerPage - 1));

        nextStart = start & (~(blocksPerPage - 1));
        if(nextIndex == 0)
            nextStart += blocksPerPage;
        
        nextBlkcnt = blkcnt - (end - start + 1);
        blkcnt = blkcnt - nextBlkcnt;
    }
do_next:
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/

	/*
	 * We want to see if the requested start or total block count are
	 * unaligned.  We discard the whole numbers and only care about the
	 * remainder.
	 */
	err = div_u64_rem(start, mmc->erase_grp_size, &start_rem);
	err = div_u64_rem(blkcnt, mmc->erase_grp_size, &blkcnt_rem);
	if (start_rem || blkcnt_rem)
		printf("\n\nCaution! Your devices Erase group is 0x%x\n"
		       "The erase range would be change to "
		       "0x" LBAF "~0x" LBAF "\n\n",
		       mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
		       ((start + blkcnt + mmc->erase_grp_size)
		       & ~(mmc->erase_grp_size - 1)) - 1);

	while (blk < blkcnt) {
		blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
			mmc->erase_grp_size : (blkcnt - blk);
		err = mmc_erase_t(mmc, start + blk, blk_r);
		if (err)
			break;

		blk += blk_r;

		/* Waiting for the ready status */
		if (mmc_send_status(mmc, timeout))
			return 0;
	}

#ifdef CONFIG_OWL_EMMC_RAID0
	if (mmc->raid0[0] != NULL) {
        if(nextBlkcnt > 0) {
            start = nextStart;
            blkcnt = nextBlkcnt;
            mmc = mmc->raid0[nextIndex];
            nextBlkcnt = 0;
            blk = 0;
            goto do_next;
        }
        return blkcntErase;
    }
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/

	return blk;
}

static ulong mmc_write_blocks(struct mmc *mmc, lbaint_t start,
		lbaint_t blkcnt, const void *src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;

	if ((start + blkcnt) > mmc->block_dev.lba) {
		printf("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       start + blkcnt, mmc->block_dev.lba);
		return 0;
	}

	if (blkcnt == 0)
		return 0;
	else if (blkcnt == 1)
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->write_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		printf("mmc write failed\n");
		return 0;
	}

	/* SPI multiblock writes terminate using a special
	 * token, not a STOP_TRANSMISSION request.
	 */
	if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			printf("mmc fail to send stop cmd\n");
			return 0;
		}
	}

	/* Waiting for the ready status */
	if (mmc_send_status(mmc, timeout))
		return 0;

	return blkcnt;
}

ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src)
{
	lbaint_t cur, blocks_todo = blkcnt;
#ifdef CONFIG_OWL_EMMC_RAID0
    int startInRaid, index, blocksPerPage;
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
		return 0;

#ifdef CONFIG_OWL_EMMC_RAID0
	if ( (mmc->raid0[0] == NULL && mmc_set_blocklen(mmc, mmc->write_bl_len))
        || (mmc->raid0[0] != NULL
            && (mmc_set_blocklen(mmc->raid0[0], mmc->write_bl_len)
                || mmc_set_blocklen(mmc->raid0[1], mmc->write_bl_len))) )
#else
	if (mmc_set_blocklen(mmc, mmc->write_bl_len))
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/
		return 0;

#ifdef CONFIG_OWL_EMMC_RAID0
    blocksPerPage = 4096 / mmc->write_bl_len;
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/

	do {
#ifdef CONFIG_OWL_EMMC_RAID0
    	if (mmc->raid0[0] != NULL) {
            if((start & blocksPerPage) == 0)
                index = 0;
            else
                index = 1;
            cur = blocksPerPage - (start & (blocksPerPage - 1));
            cur = cur > blocks_todo ? blocks_todo : cur;
            startInRaid = ((start / (blocksPerPage * 2)) * blocksPerPage) + (start & (blocksPerPage - 1));
    		if(mmc_write_blocks(mmc->raid0[index], startInRaid , cur, src) != cur)
    			return 0;
        } else {
    		cur = (blocks_todo > mmc->cfg->b_max) ?
    			mmc->cfg->b_max : blocks_todo;
    		if(mmc_write_blocks(mmc, start, cur, src) != cur)
    			return 0;
        }
#else
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_write_blocks(mmc, start, cur, src) != cur)
			return 0;
#endif  /*#ifdef CONFIG_OWL_EMMC_RAID0*/
		blocks_todo -= cur;
		start += cur;
		src += cur * mmc->write_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}
