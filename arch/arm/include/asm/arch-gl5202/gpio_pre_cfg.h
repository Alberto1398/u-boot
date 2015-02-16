
#ifndef __XML_GPIO_H__
#define __XML_GPIO_H__

#define GPIO_CFG_MAX 32
/**
 * struct icpad - ic pad information
 * @pad_index: pad index for gpio
 * @pre_cfg: pre-configuration
 * @pad_name: name of pad
 * @status:
 * @dev: currently belongs to which device
 */
struct gpio_pre_cfg
{
    unsigned char iogroup;          // A, B, C, D, etc
    unsigned char pin_num;          // 0-31
    unsigned char gpio_dir;         // INPUT/OUTPUT
    unsigned char init_level;       // the initialize value
    unsigned char active_level;     // active level
    unsigned char unactive_level;   // unactive level
    unsigned char boot;
    unsigned char reserved[6];
    char name[64];
} __attribute__ ((packed));

#endif /* __XML_GPIO_H__ */
