/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>


static int boodev;

#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

noinline u64 __invoke_fn_smc(u64 function_id, u64 arg0, u64 arg1,
					 u64 arg2)
{
	__asm__ __volatile__(
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc	#0\n"
		: "+r" (function_id)
		: "r" (arg0), "r" (arg1), "r" (arg2));

	return function_id;
}
unsigned int  __read_data(int Instruction)
{
	printf("read_chipid sensor. \n");
	return (int)__invoke_fn_smc(Instruction, 0, 0, 0);;
}

void owl_bootdev_init(void)
{
	unsigned long reg;
	unsigned int bit, regv;

	reg  = CMU_DEVCLKEN0 + (CLOCK_SD2 / 32) * 4;
	bit = CLOCK_SD2 % 32;
	regv = readl(reg);
	if (regv & (1 << bit)) {
		boodev = BOOTDEV_EMMC;
		return;
	}

	reg  = CMU_DEVCLKEN0 + (CLOCK_SD0 / 32) * 4;
	bit = CLOCK_SD0 % 32;
	regv = readl(reg);
	if (regv & (1 << bit))
		boodev = BOOTDEV_SD;
	else
		boodev = BOOTDEV_NAND;
}

int owl_get_bootdev(void)
{
	return boodev;
}

int owl_bootdev_env(void)
{
	char *bootargs_env;
	char new_env[CONFIG_SYS_BARGSIZE];
	char str_bootdev[32];

	bootargs_env = getenv("bootargs.add");
	if (boodev == BOOTDEV_NAND) {
		strcpy(str_bootdev, "androidboot.bootdev=nand");
		setenv("devif", "nand");
		setenv("bootdisk", "0");
		setenv("bootcmd", "run nandboot;");
		printf("\nbootdev=nand\n");

	} else {
		strcpy(str_bootdev, "androidboot.bootdev=sd");
		setenv("devif", "mmc");
		if (boodev == BOOTDEV_EMMC) {
			setenv("bootdisk", "1");
			setenv("bootcmd", "run emmcboot;");
			printf("\nbootdev=sd  (emmc)\n");
		} else {
			setenv("bootdisk", "0");
			setenv("bootcmd", "run mmcboot;");
			printf("\nbootdev=sd  (sd)\n");
		}
	}
	if (bootargs_env != NULL) {
		snprintf(new_env, CONFIG_SYS_BARGSIZE, "%s %s",
			 bootargs_env, str_bootdev);
		setenv("bootargs.add", new_env);
	} else {
		setenv("bootargs.add", str_bootdev);
	}
	return 0;
}


