/*
 * brom_api.h
 *
 *  Created on: 2014-7-15
 *      Author: liangzhixuan
 */

#ifndef __BROM_API_H__
#define __BROM_API_H__

typedef struct
{
    uint16_t    col_addr;
    uint16_t    row_addr_l;
    uint8_t     row_addr_h;
    uint8_t     __pad0[3];
    uint32_t    buf_addr;
} act_brom_nand_read_param_t;



extern void act_brom_nand_lb_read_unit(act_brom_nand_read_param_t *p_param);
extern void act_brom_nand_lb_ttlc_read_unit(act_brom_nand_read_param_t *p_param);
extern void act_brom_nand_exit(void);

extern void act_brom_sdmmc_read_lba(uint32_t lba, uint32_t len, uint32_t buf_addr);
extern void act_brom_sdmmc_exit(void);

extern void __attribute__((noreturn)) act_brom_enter_adfu(void);


#endif /* __BROM_API_H__ */
