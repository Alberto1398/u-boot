/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#define PSCI_VERSION			0x84000000


static int do_smc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	u64 result, smc_arg[4];

	    /* default smc */
	smc_arg[0] = PSCI_VERSION;
	smc_arg[1] = 0;
	smc_arg[2] = 0;
	smc_arg[3] = 0;
	for (i = 1; i < argc; i++) {
		if (i < 5) {
			smc_arg[i - 1] = simple_strtoull(argv[i], NULL, 0);
		}
	}

    printf("smc call: funcid: 0x%llx args: 0x%llx 0x%llx 0x%llx\n", smc_arg[0], smc_arg[1], smc_arg[2], smc_arg[3]);

    result = __invoke_fn_smc(smc_arg[0], smc_arg[1], smc_arg[2], smc_arg[3]);

    printf("smc call: result %llx\n", result);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(smc, CONFIG_SYS_MAXARGS, 1, do_smc,
	"AARCH64 SMC test",
	"<funcid> <arg0> <arg1> <arg2>\n"
	"  - call smc with parameter\n"
);
