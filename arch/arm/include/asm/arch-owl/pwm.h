/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Lipeng<lipeng@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_PWM_H_
#define __ASM_ARCH_PWM_H_

enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};

/* stand for one pwm channel */
struct owl_pwm_device {
	unsigned int		hwpwm;
	unsigned int		period;
	enum pwm_polarity	polarity;

	/* used by pwm core */
	int			node;
	int			periph_id;
	int			mfp_config;
};

int owl_pwm_init(const void *blob);

int owl_pwm_enable(struct owl_pwm_device *pwm);
void owl_pwm_disable(struct owl_pwm_device *pwm);

int owl_pwm_config(struct owl_pwm_device *pwm, int duty_ns,
		int period_ns, enum pwm_polarity polarity);

/* get pwm device from DTS node */
int fdtdec_pwm_get(const void *blob, int node, const char *prop_name,
			struct owl_pwm_device *pwm);

#endif /* __ASM_ARCH_PWM_H_ */
