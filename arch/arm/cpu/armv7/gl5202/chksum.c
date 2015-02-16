/*
 * chksum.c
 *
 *  Created on: Aug 4, 2014
 *      Author: clamshell
 */

#include <common.h>
#include <asm/arch/actions_arch_common.h>


//for 1st & 3rd mbrc checksum.
uint32_t act_calc_checksum32(uint8_t *pbuf, uint32_t size, uint32_t init_val)
{
    uint32_t sum, len, *p4;

    sum = init_val;
    len = size / sizeof(uint32_t);
    p4 = (uint32_t*)pbuf;
    while(len-- != 0)
    {
        sum += *p4++;
    }
    return sum;
}

//for 2nd mbrc checksum.
uint16_t act_calc_checksum16(uint8_t *pbuf, uint32_t size, uint16_t init_val)
{
    uint32_t sum, len;
    uint16_t *p2;

    sum = init_val;
    len = size / sizeof(uint16_t);
    p2 = (uint16_t*)pbuf;
    while(len-- != 0)
    {
        sum += *p2++;
    }
    return sum & 0xffffU;
}
