
#ifndef __XML_GET_CFG_H__
#define __XML_GET_CFG_H__

#include <asm/arch/actions_arch_common.h>

int act_xmlp_get_config(const char *key, char *buff, int len);

int act_gpio_get_pre_cfg(char *gpio_name, struct gpio_pre_cfg *m_gpio);

#endif /* __XML_GET_CFG_H__ */
