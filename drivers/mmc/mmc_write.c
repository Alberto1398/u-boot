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

static ulong vir_split_write(struct mmc *mmc,lbaint_t start,
								lbaint_t blkcnt,void *src);
static ulong phy_mmc_bwrite( struct mmc *mmc,lbaint_t start,
							lbaint_t blkcnt, const void *src);
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

	if (!mmc)
		return -1;

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


static ulong phy_mmc_bwrite( struct mmc *mmc,lbaint_t start, lbaint_t blkcnt, const void *src)
{
	lbaint_t cur, blocks_todo = blkcnt;

	if (mmc_set_blocklen(mmc, mmc->write_bl_len))
		return 0;

	do {
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_write_blocks(mmc, start, cur, src) != cur)
			return 0;
		blocks_todo -= cur;
		start += cur;
		src += cur * mmc->write_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

extern ulong vir_mmc_prepare_sector(struct mmc *mmc,lbaint_t start,
								lbaint_t blkcnt);

ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src)
{
	lbaint_t realblkcnt = 0;
	struct mmc *phymmc =NULL;
	lbaint_t	blkcntbak = blkcnt;
	struct mmc *mmc = find_mmc_device(dev_num);
	if ((!mmc) ||(!src) ){
		printf("%s: mmc :%lx  dst : %lx\n",__FUNCTION__,
				(unsigned long)mmc,(unsigned long)src);
		return -1;
	}
	struct vir_mmc *vir_mmc = (struct vir_mmc *)(mmc->priv);

	if(vir_mmc->dual_mmc_en == 0){
		/* singal channel process*/
		phymmc = vir_mmc->phy_mmca;
		if(phy_mmc_bwrite( phymmc, start,  blkcnt, src)!=blkcnt){
			printf("err:%s:%d phy_mmc_bwrite \n",__FUNCTION__,__LINE__);
			return -1;
		}
	}else{
		while(1){

				if(blkcnt == 0){
				/*trasnfer finish ,resume blkcnt for return value*/
					blkcnt = blkcntbak;
					break;
				}else if(blkcnt >= (CONFIG_SYS_MMC_MAX_BLK_COUNT)){
					realblkcnt = CONFIG_SYS_MMC_MAX_BLK_COUNT;
				}else{
					realblkcnt = blkcnt ;
				}

 				vir_mmc_prepare_sector(mmc,start,realblkcnt);
				vir_split_write(mmc,start,realblkcnt,(char*)src);

				src += realblkcnt*512;
				start += realblkcnt;
				blkcnt -= realblkcnt;
			}
	}
	return blkcnt;

}

static ulong vir_split_write(struct mmc *mmc,lbaint_t start,
								lbaint_t blkcnt,void *src)
{

	struct vir_mmc *vir_mmc = (struct vir_mmc *)(mmc->priv);
	pTRANS_PAR card_a_trs_par = &(vir_mmc->phy_mmca->card_trs_par);
	pTRANS_PAR card_b_trs_par = &(vir_mmc->phy_mmcb->card_trs_par);
	char * bufa_start;
	char * bufb_start;
	char * bufa_bak	=  card_a_trs_par->buf;
	char * bufb_bak	=  card_b_trs_par->buf;

	unsigned long chunk_num =  start/DAID0_CHUCK_SECORT;
	int i = 0;

	if( chunk_num%2 == 0){
		bufa_start = src;
		/*start from carda*/
		for(i=0;i<card_a_trs_par->vailnum;i++){
			memcpy(card_a_trs_par->buf,bufa_start,512*card_a_trs_par->table[i]);
			bufa_start+=(DAID0_CHUCK_SIZE+512*card_a_trs_par->table[i]);
			card_a_trs_par->buf += 512*card_a_trs_par->table[i];
		}

		bufb_start = src+512*card_a_trs_par->table[0];
		/*start from carda*/
		for(i=0;i<card_b_trs_par->vailnum;i++){
			memcpy(card_b_trs_par->buf,bufb_start,512*card_b_trs_par->table[i]);
			bufb_start+=(DAID0_CHUCK_SIZE+512*card_b_trs_par->table[i]);
			card_b_trs_par->buf += 512*card_b_trs_par->table[i];
		}
	}else{
		/*start from cardb*/
		bufb_start = src;
		for(i=0;i<card_b_trs_par->vailnum;i++){
			memcpy(card_b_trs_par->buf,bufb_start,512*card_b_trs_par->table[i]);
			bufb_start+=(DAID0_CHUCK_SIZE+512*card_b_trs_par->table[i]);
			card_b_trs_par->buf += 512*card_b_trs_par->table[i];
		}

		/*start from carda*/
		bufa_start = src+512*card_b_trs_par->table[0];
		for(i=0;i<card_a_trs_par->vailnum;i++){
			memcpy(card_a_trs_par->buf,bufa_start,512*card_a_trs_par->table[i]);
			bufa_start+=(DAID0_CHUCK_SIZE+512*card_a_trs_par->table[i]);
			card_a_trs_par->buf += 512*card_a_trs_par->table[i];
		}
	}

	card_a_trs_par->buf = bufa_bak;
	card_b_trs_par->buf = bufb_bak;

	if( phy_mmc_bwrite( vir_mmc->phy_mmca,card_a_trs_par->start_sec,
		card_a_trs_par->sec_num,card_a_trs_par->buf)!= card_a_trs_par->sec_num){
		printf("%s:err phy_mmc_bwrite\n",__FUNCTION__);
	}
	if( phy_mmc_bwrite( vir_mmc->phy_mmcb,card_b_trs_par->start_sec,
		card_b_trs_par->sec_num,card_b_trs_par->buf)!= card_b_trs_par->sec_num){
		printf("%s:err phy_mmc_bwrite\n",__FUNCTION__);
	}

	return blkcnt;

}


