
#include <common.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>
#include <asm/arch/afinfo.h>

#if defined(CONFIG_SPL_BUILD)
#define ACT_AFINFO_SPACE_SIZE   (2048+8)
/* SPL阶段这段空间是由量产脚本或upgrade_app填写的, 规格上是2KB(写2KB,实际有效数据小于2KB)
 * 为避免莫名的越界, 多预留了一点空间. */
#else
#define ACT_AFINFO_SPACE_SIZE   (2048+8) //(sizeof(afinfo_t))
#endif

uint8_t __attribute__((used, aligned(16))) g_afinfo_space[ACT_AFINFO_SPACE_SIZE] = {
    // to make this table into .data section, do not fill 0 here.
    'F', 'I', 'L', 'L', '_', 'A', 'F', 'I', 'N', 'F', 'O'
};
