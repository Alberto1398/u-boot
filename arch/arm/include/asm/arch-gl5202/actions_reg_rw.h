/*
 * actions_reg_rw.h
 * This header is for backward compatibility only
 */

#ifndef __ACTIONS_REG_RW_H__
#define __ACTIONS_REG_RW_H__

#if !(defined(_ASSEMBLER_) || defined(__ASSEMBLY__))

#include <asm/io.h>

static inline void act_writeb(__u8 val, __u32 reg)
{
	__raw_writeb(val, reg);
}

static inline void act_writew(__u16 val, __u32 reg)
{
	__raw_writew(val, reg);
}

static inline void act_writel(__u32 val, __u32 reg)
{
	__raw_writel(val, reg);
}

static inline __u8 act_readb(__u32 reg)
{
    return __raw_readb(reg);
}

static inline __u16 act_readw(__u32 reg)
{
    return __raw_readw(reg);
}

static inline __u32 act_readl(__u32 reg)
{
    return __raw_readl(reg);
}

#endif

#endif /* __ACTIONS_REG_RW_H__ */
