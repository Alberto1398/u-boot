#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>


#define GPIO_REG_BASE               (GPIO_MFP_PWM_BASE)
#define GPIO_BANK(gpio)             ((gpio) / 32)
#define GPIO_IN_BANK(gpio)          ((gpio) % 32)
#define GPIO_BIT(gpio)              (1 << GPIO_IN_BANK(gpio))

#define GPIO_REG_OUTEN(gpio)        (GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x0)
#define GPIO_REG_INEN(gpio)         (GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x4)
#define GPIO_REG_DAT(gpio)          (GPIO_REG_BASE + GPIO_BANK(gpio) * 0xc + 0x8)
#define GPIO_REG_INTC_PD(gpio)      (GPIO_REG_BASE + GPIO_BANK(gpio) * 0x8 + 0x208)
#define GPIO_REG_INTC_MASK(gpio)    (GPIO_REG_BASE + GPIO_BANK(gpio) * 0x8 + 0x20c)



DECLARE_GLOBAL_DATA_PTR;

static void _set_gpio_output(unsigned int gpio, int val)
{
    unsigned int dat;

    act_writel(act_readl(GPIO_REG_INEN(gpio)) & ~GPIO_BIT(gpio),
        GPIO_REG_INEN(gpio));

    act_writel(act_readl(GPIO_REG_OUTEN(gpio)) | GPIO_BIT(gpio),
        GPIO_REG_OUTEN(gpio));

    dat = act_readl(GPIO_REG_DAT(gpio));

    if (val)
        dat |= GPIO_BIT(gpio);
    else
        dat &= ~GPIO_BIT(gpio);

    act_writel(dat, GPIO_REG_DAT(gpio));
}

static void _set_gpio_input(unsigned int gpio)
{
    act_writel(act_readl(GPIO_REG_OUTEN(gpio)) & ~GPIO_BIT(gpio),
        GPIO_REG_OUTEN(gpio));
    act_writel(act_readl(GPIO_REG_INEN(gpio)) | GPIO_BIT(gpio),
        GPIO_REG_INEN(gpio));
}

void act_spl_init_gpio_cfg(void)
{
	afinfo_t const *p_afinfo;
	gpio_cfg_t const *gpio_cfg;
	uint32_t reg_val;
    int i, num;

    /* enable MFP */
    reg_val = act_readl(CMU_DEVCLKEN0);
    if((reg_val & (1U << 18)) == 0)
    {
        act_writel(reg_val | (1U<<18), CMU_DEVCLKEN0);
    }
    reg_val = act_readl(PAD_CTL);
    if((reg_val & (1U << 1)) == 0)
    {
        act_writel(reg_val | (1U<<1), PAD_CTL);
    }
    if((reg_val & (1U << 2)) == 0)
    {
        act_writel(reg_val | (1U<<2), PAD_CTL);
    }
    if((reg_val & (1U << 3)) == 0)
    {
        act_writel(reg_val | (1U<<3), PAD_CTL);
    }
    act_readl(PAD_CTL);

    p_afinfo = ACTS_AFINFO_PTR;

    num = sizeof(p_afinfo->gpio_cfg) / sizeof(p_afinfo->gpio_cfg[0]);
    for (i = 0; i < num; i++)
    {
        gpio_cfg = &p_afinfo->gpio_cfg[i];
        switch (gpio_cfg->type)
        {
        case GPIO_TYPE_OUTPUT_LOW:
            _set_gpio_output(gpio_cfg->gpio, 0);
            break;
        case GPIO_TYPE_OUTPUT_HIGH:
            _set_gpio_output(gpio_cfg->gpio, 1);
            break;
        case GPIO_TYPE_INPUT:
            _set_gpio_input(gpio_cfg->gpio);
            break;
        case GPIO_TYPE_NULL:
            continue;
        default:
//            INFO("%s: invalid gpio type %d (gpio 0x%x)\n", __FUNCTION__,
//                gpio_cfg->type, gpio_cfg->gpio);
            hang();
        }
    }
}

