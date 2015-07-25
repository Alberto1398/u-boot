#include <common.h>
#include <command.h>
#include <asm/arch/osboot.h>

int do_romadfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("enter brom adfu mode\n");
	owlxx_enter_adfu();
}

U_BOOT_CMD(
	romadfu,   CONFIG_SYS_MAXARGS,   0,     do_romadfu,
	"enter brom adfu mode",
	"[no args]\n"
);
