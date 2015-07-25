/*
 * (C) Copyright 2011 The Chromium OS Authors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <malloc.h>

#define OWLXX_DBG

static int dbg_switch = 1;

#ifdef OWLXX_DBG
#define dbg(fmt, args...) do { 	\
	if (dbg_switch)	\
		printf("<<DBG>>]"fmt, ##args);\
	} while (0)
#endif

struct owlxx_i2c {
	u32 ctl;		/* I2C Control Register */
	u32 clkdiv;		/* I2C Clk Divide Register */
	u32 stat;		/* I2C Status Register */
	u32 addr;		/* I2C Address Register */
	u32 txdat;		/* I2C TX Data Register */
	u32 rxdat;		/* I2C RX Data Register */
	u32 cmd;		/* I2C Command Register */
	u32 fifoctl;		/* I2C FIFO control Register */
	u32 fifostat;		/* I2C FIFO status Register */
	u32 datcnt;		/* I2C Data transmit counter */
	u32 rcnt;		/* I2C Data transmit remain counter */
};

extern void owlxx_dump_i2c_register(void);

extern int i2c_transfer(struct owlxx_i2c *i2c, unsigned char cmd_type,
		unsigned char chip, unsigned char addr[], unsigned char addr_len,
		unsigned char data[], unsigned char data_len);

/*
 * do_i2c_xfer() - Handle the "i2c transfer" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success,
 * CMD_RET_USAGE in case of misuse and negative on error.
 *
 * Syntax:
 *	i2c read {i2c_chip} {devaddr}{.0, .1, .2} {len} {memaddr}
 */
static int do_owlxx_i2c_xfer(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	u32 adap, ret = 0, i = 0;
	struct owlxx_i2c *i2c;
	u32 cmd, chip, addr, addr_len, data_len;
	unsigned char *data;

	for (i = 0; i < argc; i++)
		dbg("argv[%d] = %s\r\n", i, *(argv + i));

	i = 0;

	adap = simple_strtoul(argv[1], NULL, 16);
	dbg("use i2c_bus[%d]\r\n", adap);

	i2c = (struct owlxx_i2c *)(0xb0170000 + adap * 0x4000);
	dbg("I2C_BASE = 0x%x\r\n", (u32)i2c);

	cmd = simple_strtoul(argv[2], NULL, 16);

	if (cmd == 1) {
		dbg("Cmd : Read\r\n");
	} else if (cmd == 0) {
		dbg("Cmd : Write\r\n");
	} else {
		dbg("Cmd error\r\n");
		return CMD_RET_FAILURE;
	}

	chip = simple_strtoul(argv[3], NULL, 16);
	dbg("slave addr = 0x%x\r\n", chip);

	addr = simple_strtoul(argv[4], NULL, 16);
	dbg("register addr = 0x%x\r\n", addr);

	addr_len = simple_strtoul(argv[5], NULL, 16);
	dbg("register addr_len = %d\r\n", addr_len);

	data_len = simple_strtoul(argv[6], NULL, 16);
	dbg("data_len = %d\r\n", data_len);

	data = (unsigned char *)malloc(sizeof(data_len));
	if (!data) {
		dbg("Malloc space failed\r\n");
		return CMD_RET_FAILURE;
	}

	if (cmd == 0) {
		if (argc != 7 + data_len) {
			dbg("Write length error\r\n");
			return CMD_RET_FAILURE;
		}

		for (i = 0; i < data_len; i++) {
			data[i] = simple_strtoul(argv[7 + i], NULL, 16);
			dbg("write  data[%d] = 0x%x\r\n", i, data[i]);
		}
	}

	if (cmd == 1) {
		if (argc != 7) {
			dbg("Read length error\r\n");
			return CMD_RET_FAILURE;
		}
	}

	ret = i2c_transfer(i2c, cmd, chip, (unsigned char *)addr,
			addr_len, data, data_len);

	if (ret != 0) {
		debug("I2c read: failed %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_devmem(cmd_tbl_t *cmdtp, int flag,
		int argc, char *const argv[])
{
	u32 reg = 0, val = 0;
	reg = simple_strtoul(argv[1], NULL, 16);

	if (argc == 2) {
		val = __raw_readl(reg);
		dbg("reg[0x%8x] = 0x%8x \r\n]", reg, val);
	} else if (argc == 3) {
		val = simple_strtoul(argv[2], NULL, 16);
		__raw_writel(val, reg);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_debug(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	if (argc != 2) {
		dbg("cmd error \r\n]");
		return CMD_RET_USAGE;
	}

	dbg_switch = simple_strtoul(argv[1], NULL, 16);

	return CMD_RET_SUCCESS;
}

static int do_dump_i2c(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	owlxx_dump_i2c_register();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(i2c_xfer, CONFIG_SYS_MAXARGS, 0, do_owlxx_i2c_xfer,
		 "Owlxx i2c write utils\n",
		 "command [adap][cmd][dev] [register] [reg_len][data][]data_len\n"
		 "adap     : i2c bus num\n"
		 "cmd      : 0 write, 1 read\n"
		 "dev      : slave addr\n"
		 "register : register offset\n"
		 "reg_len  : register address lenth\n"
		 "data_len : data length\n"
		 "data     : read from or write to slave\n");

U_BOOT_CMD(devmem, CONFIG_SYS_MAXARGS, 0, do_devmem,
		 "Owlxx registor operate utils",
		 "Read : command [reg] " "Write : command [reg] [val]");

U_BOOT_CMD(debug, CONFIG_SYS_MAXARGS, 0, do_debug,
		 "Owlxx debug switch", "debug : command [num] 0 close, 1 open");

U_BOOT_CMD(dump_i2c, CONFIG_SYS_MAXARGS, 0, do_dump_i2c,
		 "Owlxx debug switch", "debug : command [num] 0 close, 1 open");

