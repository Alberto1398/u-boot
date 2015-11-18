/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Some idea is from
 * "kernel/drivers/video/backlight/pwm_bl.c"
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PWM_BL_H_
#define __PWM_BL_H_

#include <asm/arch/pwm.h>

int owl_pwm_bl_init(const void *blob);

int owl_pwm_bl_get_max_brightness(void);

void owl_pwm_bl_on(void);
void owl_pwm_bl_off(void);

bool owl_pwm_bl_is_on(void);

/*
 * owl_pwm_bl_set_brightness
 *
 * @brightness,
 *	0:	turn off.
 *	> 0:	turn on.
 *	> max_brightness: limit to max_brightness
 */
void owl_pwm_bl_set_brightness(int brightness);

int owl_pwm_bl_get_brightness(void);

#endif /* __PWM_BL_H_ */
