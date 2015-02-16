/*
 * Asoc matrix keypad scan in boot
 *
 * Copyright (C) 2011 Actions Semiconductor, Inc
 * Author:  chenbo <chenbo@actions-semi.com>
 *
 * Based on Samsung keypad driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_atc2603.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>
#include <asm/arch/afinfo.h>

#include "key_scan.h"

#define ATC260X_ADC0_EN     (1 << 15)
#define ADCKEY_SIZE     11
#define ADFU_DEBUG      0

/*
1   0       31
2   32      143
3   144     239
4   240     335
5   336     447
6   448     559
7   560     687
8   688     799
9   800     895
10  896     966
open    967     1024
*/
static const uint16_t left_adc[ADCKEY_SIZE] =  {  0,  32, 144, 240, 336, 448, 560, 688, 800, 896,  967};
static const uint16_t right_adc[ADCKEY_SIZE] = { 31, 143, 239, 335, 447, 559, 687, 799, 895, 966, 1023};

static const uint16_t key_code_tbl[ADCKEY_SIZE] = {
    KEY_HOME,           KEY_MENU,
    KEY_VOLUMEUP,       KEY_VOLUMEDOWN,
    KEY_RESERVED,
    KEY_BOARD_OPTION5,  KEY_BOARD_OPTION4,
    KEY_BOARD_OPTION3,  KEY_BOARD_OPTION2,
    KEY_BOARD_OPTION1,  KEY_BOARD_OPTION0
};

void act_adc_key_boot_init(void)
{
    act_260x_reg_setbits(atc2603_PMU_AuxADC_CTL0, ATC260X_ADC0_EN, ATC260X_ADC0_EN);
}

static unsigned int adckey_convert(int adc_val)
{
    int i;

    for(i = 0; i < ADCKEY_SIZE; i++)
    {
        if ((adc_val >= left_adc[i]) && (adc_val <= right_adc[i]))
        {
            return i;
        }
    }
    return 4;
}

uint act_adc_key_scan(void)
{
    uint adc_val, adckey_no, adckey_code;

    adc_val = act_260x_reg_read(atc2603_PMU_AuxADC0);
    //adckey_no = adckey_convert(adc_val);

    udelay(800); // wait next sample, 1 / (32768 / 8 / 3)

    adc_val = act_260x_reg_read(atc2603_PMU_AuxADC0);
    adckey_no = adckey_convert(adc_val);
    adckey_code = key_code_tbl[adckey_no];

    printf("adc_key: raw_val=%u key_idx=%u key_code=%u\n",
            adc_val, adckey_no, adckey_code);

    return adckey_code;
}




#define ONOFF_RESET_TIME_SEL(x)         (((x) & 0x3) << 7)
#define ONOFF_RESET_EN                  (1 << 9)
#define ONOFF_PRESS_TIME(x)             (((x) & 0x3) << 10)
#define ONOFF_INT_EN                    (1 << 12)
#define ONOFF_LONG_PRESS                (1 << 13)
#define ONOFF_SHORT_PRESS               (1 << 14)
#define ONOFF_PRESS                     (1 << 15)

static int _count_onoff_short_press(void)
{
    int i, poll_times, on_off_val;

    while(1)
    {
        on_off_val = act_260x_reg_read(atc2603_PMU_SYS_CTL2);
        if((on_off_val & ONOFF_PRESS) == 0)
            break;
        mdelay(1);
    }

    debug("%s: start count onoff times\n", __FUNCTION__);

    /* clear On/Off press pending */
    act_260x_reg_setbits(atc2603_PMU_SYS_CTL2,
        ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS,
        ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);

    for(poll_times = 0; poll_times < 3; poll_times++)
    {
        for(i = 0; i < 1000; i++)
        {
            on_off_val = act_260x_reg_read(atc2603_PMU_SYS_CTL2);
            if ((on_off_val & ONOFF_SHORT_PRESS) != 0)
                break;
            mdelay(1);
        }

        if(i == 1000)
            break;

        act_260x_reg_setbits(atc2603_PMU_SYS_CTL2,
            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS,
            ONOFF_SHORT_PRESS | ONOFF_LONG_PRESS);
    }

    debug("%s: onoff press %d times\n", __FUNCTION__, poll_times);
    return poll_times;
}

int act_key_scan(unsigned char scan_mode)
{
    int onoff_shortpress_times;
    if(scan_mode == 0)
        return act_adc_key_scan();

    onoff_shortpress_times = _count_onoff_short_press();
    if(onoff_shortpress_times == 2)
        return KEY_RECOVERY;
    else if(onoff_shortpress_times == 3)
        return KEY_ADFU;
    else
        return KEY_NULL;
}
