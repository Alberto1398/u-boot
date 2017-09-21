/*
* TI BQ27441 battery driver
*
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * Terry Chen chenbo@actions-semi.com
*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <i2c.h>
#include <fdtdec.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x_charger.h>
#include <power/power_battery.h>

#define BIT(nr)				(1UL << (nr))

#define DRIVER_VERSION			"1.2.1"
#define BQ27441_DEVTYPE                 0x0421

#define BQ27441_I2C_ADDRESS             0x55
#define TI_CONFIG_RSENSE                10
#define TI_CONFIG_RESERVEDCAP           0x0
#define TI_CONFIG_TERMVDATLE            100
#define TI_CONFIG_CAP                   5700
#define TI_CONFIG_TERMVOL               3400
#define TI_CONFIG_TAPCUR                350
#define TI_CONFIG_TAPERVOL              4200
#define TI_CONFIG_QMAXCELL              17299
#define TI_CONFIG_LOADSEL               0x81
#define TI_CONFIG_DESIGNENERGY          22200
#define TI_CONFIG_DSGCUR                167
#define TI_CONFIG_CHGCUR                100
#define TI_CONFIG_QUITCUR               40


/**bq27441 sub command of control*/
#define CONTROL_STATUS                  0x0000
#define DEVICE_TYPE                     0x0001
#define FW_VERSION                      0x0002
#define DM_CODE                         0x0004
#define PREV_MACWRITE                   0x0007
#define CHEM_ID                         0x0008
#define BAT_INSERT                      0x000c
#define BAT_REMOVE                      0x000d
#define SET_HIBERNATE                   0x0011
#define CLEAR_HIBERNATE                 0x0012
#define SET_CFGUPDATE                   0x0013
#define SHUTDOWN_ENABLE                 0x001b
#define SHUTDOWN                        0x001c
#define SEALED                          0x0020
#define TOGGLE_GPOUT                    0x0023
#define RESET                           0x0041
#define SOFT_RESET                      0x0042
#define UNSEALED                        0x8000
/*CONTROL_STATUS*/
#define CONTROL_STATUS_SS               BIT(13)
/*bq27441 reg addr*/
#define BQ27441_REG_CONTROL             0x00
#define BQ27441_REG_TEMP                0x02
#define BQ27441_REG_VOLT                0x04
#define BQ27441_REG_FLAGS               0x06
#define BQ27441_REG_NAC                 0x08 /*Nominal Available Capacity*/
#define BQ27441_REG_FAC                 0x0a /*Full Available Capacity*/
#define BQ27441_REG_RM                  0x0c /*Remaining Capacity*/
#define BQ27441_REG_FCC                 0x0e /*Full Charge Capacity*/
#define BQ27441_REG_AC                  0x10 /*Average Current*/
#define BQ27441_REG_SC                  0x12 /*Standby Current*/
#define BQ27441_REG_MLC                 0x14 /*Max Load Current*/
#define BQ27441_POWER_AVG               0x18 /*Average Power*/
#define BQ27441_REG_SOC                 0x1c /*State of Charge*/
#define BQ27441_REG_INTTEMP             0x1e /*Internal Temprature, unit:0.1K*/
#define BQ27441_REG_SOH                 0x20 /*State of Health*/
/*bq27441 flags reg  bit*/
#define BQ27441_FLAG_DSG                BIT(0) /*Discharge Detect*/
#define BQ27441_FLAG_SOCF               BIT(1)
#define BQ27441_FLAG_SOC1               BIT(2)
#define BQ27441_FLAG_BATDET             BIT(3)
#define BQ27441_FLAG_CFGUPMODE          BIT(4)
#define BQ27441_FLAG_ITPOR              BIT(5)
#define BQ27441_FLAG_OCVTAKEN           BIT(7)
#define BQ27441_FLAG_CHG                BIT(8)
#define BQ27441_FLAG_FC                 BIT(9)
#define BQ27441_FLAG_UT                 BIT(14)
#define BQ27441_FLAG_OT                 BIT(15)
/*bq27441 extend reg addr,0x62~0x7f reserved*/
#define BQ27441_EXT_REG_OPCONFIG        0x3a
#define BQ27441_EXT_REG_DESIGNCAPACITY  0x3c
#define BQ27441_EXT_REG_DATACLASS       0x3e
#define BQ27441_EXT_REG_DATABLOCK       0x3f
#define BQ27441_EXT_REG_BLOCKDATA_BEGIN 0x40
#define BQ27441_EXT_REG_BLOCKDATA_END   0x5f
#define BQ27441_EXT_REG_BLOCKDATACHKSUM 0x60
#define BQ27441_EXT_REG_BLOCKDATACTL    0x61
/*bq27441 data memory sub class id*/
#define BQ27441_DM_SC_SAFETY            0x02
#define BQ27441_DM_SC_CHTERM            0x24
#define BQ27441_DM_SC_CURTHRESH         0x51
#define BQ27441_DM_SC_STATE             0x52
#define BQ27441_DM_SC_RARAM             0x59


struct bq27441_device_info;
struct bq27441_access_methods {
	int (*read)(struct bq27441_device_info *di, unsigned int reg,
		int single, int le);
	int (*write)(struct bq27441_device_info *di, unsigned int reg,
		unsigned short val, int single, int le);
};

enum bq27441_chip {BQ27441};


struct bq27441_sc_state {
	unsigned int qmax_cell;
	unsigned int load_sel;
	unsigned int design_capacity;
	unsigned int term_vol;
	unsigned int term_vdatle;
	unsigned int taper_rate;
	unsigned int taper_vol;
	unsigned int reserve_cap;
	unsigned int design_energy;
};

struct bq27441_sc_cur_thresholds {
	unsigned int dsg_cur;
	unsigned int chg_cur;
	unsigned int quit_cur;
};

struct  bq27441_cfg_items {
	unsigned int *r_table;
	unsigned int taper_cur;
	unsigned int rsense;
	int design_capacity;

	struct bq27441_sc_state state;
	struct bq27441_sc_cur_thresholds cur_thresholds;
	int log_switch;
};

struct bq27441_device_info {
	struct owl_i2c_dev      *i2c_client;
	int			bus_id;

	unsigned char           bus_addr;  /* device's bus address, only for I2C, 7bit, r/w bit excluded */
	struct  bq27441_cfg_items items;
	int health;
	int detected;

	int charge_design_full;
	int bat_vol;
	int soc;
	int pwr_on;

	int online;
	struct bq27441_access_methods bus;
};

enum {
	POWER_SUPPLY_HEALTH_UNKNOWN = 0,
	POWER_SUPPLY_HEALTH_GOOD,
	POWER_SUPPLY_HEALTH_DEAD,
};

struct bq27441_device_info di = {.health = POWER_SUPPLY_HEALTH_GOOD};
static int bq27441_battery_set_unsealed(struct bq27441_device_info *di);


/*
  *  Bq27441 read function.
  *  Note: reg use little endian mode;data memory use big endian.
  */
static inline int bq27441_read(struct bq27441_device_info *di, unsigned int reg,
		int single)
{
	if (reg >= BQ27441_EXT_REG_BLOCKDATA_BEGIN
		&& reg <= BQ27441_EXT_REG_BLOCKDATA_END)
		return di->bus.read(di, reg, single, 0);
	else
		return di->bus.read(di, reg, single, 1);

}

/*
  *  Bq27441 write function.
  *  Note: reg use little endian mode;data memory use big endian.
  */
static inline int bq27441_write(struct bq27441_device_info *di, unsigned int reg,
		unsigned short val, int single)
{
	if (reg >= BQ27441_EXT_REG_BLOCKDATA_BEGIN
		&& reg <= BQ27441_EXT_REG_BLOCKDATA_END)
		return di->bus.write(di, reg, val, single, 0);
	else
		return di->bus.write(di, reg, val, single, 1);
}
/*
  *  Bq27441 set subcmd function.
  *  Note: subcmd use little endian mode fixed.
  */
static inline int bq27441_set_subcmd(struct bq27441_device_info *di, unsigned short val)
{
	return di->bus.write(di, BQ27441_REG_CONTROL, val, 0, 1);
}

static int bq27441_read_i2c(struct bq27441_device_info *di, unsigned int reg,
	int single, int le)
{
	uint8_t data[2];
	int ret;
	int data_len;

	if (single)
		data_len = 1;
	else
		data_len = 2;

	i2c_set_bus_num(di->bus_id);
	ret = i2c_read(di->bus_addr, reg, 1, data, data_len);
	if (ret) {
		printf("i2c read err, reg=0x%x ret=%d\n", reg, ret);
		return ret;
	}

	if (!single) {
		if (le)
			ret = data[0] | (data[1] << 8);
		else
			ret = (data[0] << 8) | data[1];
	} else
		ret = data[0];

	return ret;

}

static int bq27441_write_i2c(struct bq27441_device_info *di,
	unsigned int reg, unsigned short value, int single, int le)
{
	int ret;
	uint8_t data[2];
	int data_len;

	if (single) {
		data[0] = value & 0xff;
		data_len = 1;
	} else {
		if (le) {
			data[0] = value & 0xff;
			data[1] = (value >> 8) & 0xff;
		} else {
			data[0] = (value >> 8) & 0xff;
			data[1] = value & 0xff;
		}
		data_len = 2;
	}

	i2c_set_bus_num(di->bus_id);
	ret = i2c_write(di->bus_addr, reg, 1, data, data_len);
	if (ret)
		return ret;

	return 0;
}


static int bq27441_i2c_init(struct bq27441_device_info *di)
{
	int ret;

	di->bus_addr = BQ27441_I2C_ADDRESS;
	di->bus.read = bq27441_read_i2c;
	di->bus.write = bq27441_write_i2c;

	/* return no zero, if  bq27441 doesn't respond */
	i2c_set_bus_num(di->bus_id);
	ret = i2c_probe(di->bus_addr);
	if (ret) {
		di->health = POWER_SUPPLY_HEALTH_DEAD;
		printf("i2c not find bq27441 device, health:%d!\n", di->health);
		return ret;
	}

	return 0;
}

/*
 * Return the battery Voltage in uV
 * Or < 0 if something fails.
 */
static int bq27441_battery_measure_voltage(int *batv)
{
	int volt;
	int ret;

	if (di.health == POWER_SUPPLY_HEALTH_DEAD) {
		ret = atc260x_auxadc_get_translated(ATC260X_AUXADC_BATV, &volt);
		if (ret) {
			printf("[%s] get wallv auxadc err!\n", __func__);
			return ret;
		}
		*batv = volt;
		return 0;
	}

	volt = bq27441_read(&di, BQ27441_REG_VOLT, 0);
	if (volt < 0) {
		printf("error reading voltage\n");
		return volt;
	}

	*batv = volt;

	return 0;
}

static int bq27441_battery_calc_rsoc(void)
{
	int tval;

	if (di.health == POWER_SUPPLY_HEALTH_DEAD) {
		di.soc = 0;
		return di.soc;
	}

	if (di.detected)
		return di.soc;

	tval = bq27441_read(&di, BQ27441_REG_SOC, 0);
	if (tval < 0) {
		printf("error reading soc\n");
		return tval;
	}

	di.soc = tval;

	return di.soc;
}

static int bq27441_battery_check_online(void)
{
	if (di.detected)
		return di.online;
	di.online = atc260x_chk_bat_online_intermeddle();
	return di.online;
}
static int bq27441_battery_set_sealed(struct bq27441_device_info *di)
{
	int tval;

	tval = bq27441_set_subcmd(di, CONTROL_STATUS);
	if (tval < 0) {
		printf("[set sealed]send CONTROL_STATUS subcmd err, tval:%d\n", tval);
		return tval;
	}

	tval = bq27441_read(di, BQ27441_REG_CONTROL, 0);
	if (tval < 0) {
		printf("[set sealed]read BQ27441_REG_CONTROL err, tval:%d\n", tval);
		return tval;
	}
	printf("[set sealed]read BQ27441_REG_CONTROL(0x%x)\n", tval);

	if (tval & CONTROL_STATUS_SS) {
		debug("[set sealed]already sealed mode\n");
		return 0;
	}

	tval = bq27441_set_subcmd(di, SEALED);
	if (tval < 0) {
		printf("[set sealed]send SEALED subcmd err, tval:%d\n", tval);
		return tval;
	}
	mdelay(1000);

	return 0;
}

static int bq27441_battery_set_unsealed(struct bq27441_device_info *di)
{
	int tval;

	tval = bq27441_set_subcmd(di, CONTROL_STATUS);
	if (tval < 0) {
		printf("[set unsealed]send CONTROL_STATUS subcmd err, tval:%d\n", tval);
		return tval;
	}

	tval = bq27441_read(di, BQ27441_REG_CONTROL, 0);
	if (tval < 0) {
		printf("[set unsealed]read BQ27441_REG_CONTROL err, tval:%d\n", tval);
		return tval;
	}
	debug("[set unsealed]read BQ27441_REG_CONTROL(0x%x)\n", tval);

	if (!(tval & CONTROL_STATUS_SS)) {
		debug("[set unsealed]already unsealed mode\n");
		return 0;
	}

	tval = bq27441_set_subcmd(di, UNSEALED);
	if (tval < 0) {
		printf("[set unsealed]send SEALED subcmd 1st err, tval:%d\n", tval);
		return tval;
	}
	tval = bq27441_set_subcmd(di, UNSEALED);
	if (tval < 0) {
		printf("[set unsealed]send SEALED subcmd 2st err, tval:%d\n", tval);
		return tval;
	}

	return 0;
}

static int bq27441_battery_cfgupdate_finished(struct bq27441_device_info *di)
{
	int tval;

	tval = bq27441_set_subcmd(di, SOFT_RESET);
	if (tval < 0) {
		printf("[cfg finish]SOFT RESET err, tval:%d\n", tval);
		return tval;
	}

	mdelay(2000);

	tval = bq27441_read(di, BQ27441_REG_FLAGS, 0);
	if (tval < 0) {
		printf("[cfg finish] BQ27441_REG_FLAGS err, tval:%d\n", tval);
		return tval;
	} else {
		if (tval & BQ27441_FLAG_CFGUPMODE) {
			debug("[cfg finish] CFGUPDATE mode uclear! 0x%x,\n", tval);
			return -1;
		}
	}

	bq27441_battery_set_sealed(di);

	return 0;
}

static int bq27441_battery_select_subclass(struct bq27441_device_info *di,
	unsigned char subclass_id, unsigned char index)
{
	int tval;

	/*select state subclass*/
	tval = bq27441_write(di, BQ27441_EXT_REG_BLOCKDATACTL, 0x00, 1);
	if (tval < 0) {
		printf("[sel scid]enable data memory access err, tval:%d\n", tval);
		return tval;
	}
	tval = bq27441_write(di, BQ27441_EXT_REG_DATACLASS, subclass_id, 1);
	if (tval < 0) {
		printf("[sel scid]set data memory subclass id(%d) err, tval:%d\n",
			subclass_id, tval);
		return tval;
	}
	tval = bq27441_write(di, BQ27441_EXT_REG_DATABLOCK, index, 1);
	if (tval < 0) {
		printf("[sel scid]set index of data block err, tval:%d\n", tval);
		return tval;
	}

	return 0;
}

static int bq27441_battery_cfgupdate_state(struct bq27441_device_info *di)
{
	struct bq27441_cfg_items *items = &di->items;
	struct bq27441_sc_state *new_state = &items->state;
	struct bq27441_sc_state old_state;
	int tval;
	unsigned char old_chksum = 0;
	unsigned char new_chksum = 0;

	/*select state subclass*/
	tval = bq27441_battery_select_subclass(di, BQ27441_DM_SC_STATE, 0x00);
	if (tval < 0)
		return -1;
	mdelay(50);

	tval = bq27441_read(di, BQ27441_EXT_REG_BLOCKDATACHKSUM, 1);
	if (tval < 0) {
		printf("[cfg state]get block data check sum err, tval:%d\n", tval);
		return tval;
	} else {
		old_chksum = tval;
		printf("[cfg state]get block data check sum success, old_chksum:0x%x\n", old_chksum);
	}

	/*calc new checksum*/
	tval = bq27441_read(di, 0x40, 0);
	if (tval < 0) {
		printf("[cfg state]get qmax err, tval:%d\n", tval);
		return tval;
	} else {
		old_state.qmax_cell = tval;
		old_chksum +=  (old_state.qmax_cell & 0xff) +
			((old_state.qmax_cell >> 8) & 0xff);
		debug("[cfg state]get qmax success, qmax:%d\n", old_state.qmax_cell);
	}

	tval = bq27441_read(di, 0x4a, 0);
	if (tval < 0) {
		printf("[cfg state]get design_capacity err, tval:%d\n", tval);
		return tval;
	} else {
		old_state.design_capacity = tval;
		old_chksum += (old_state.design_capacity & 0xff) +
			((old_state.design_capacity >> 8) & 0xff);
		debug("[cfg state]get design_capacity success, design_capacity:%d\n", old_state.design_capacity);
	}

	tval = bq27441_read(di, 0x4c, 0);
	if (tval < 0) {
		printf("[cfg state]get design_energy err, tval:%d\n", tval);
		return tval;
	} else {
		old_state.design_energy = tval;
		old_chksum += (old_state.design_energy & 0xff) +
			((old_state.design_energy >> 8) & 0xff);
		debug("[cfg state]get design_energy success, design_energy:%d\n", old_state.design_energy);
	}

	new_chksum = ((new_state->design_capacity >> 8) & 0xFF) +
		(new_state->design_capacity & 0xFF);
	new_chksum += ((new_state->design_energy >> 8) & 0xFF) +
		(new_state->design_energy & 0xFF);

	new_chksum = ~((~(old_chksum & 0xff) + new_chksum) & 0xff);
	debug("[cfg state] new_chksum : 0x%x !\n", new_chksum);

	/*cfg update*/
	if (new_chksum == old_chksum) {
		debug("[cfg state] subclass state(%d) have already configed!\n",
			BQ27441_DM_SC_STATE);
		return 0;
	}

	tval = bq27441_write(di, 0x4a, new_state->design_capacity, 0);
	if (tval < 0) {
		printf("[cfg state]write design_capacity err, tval:%d\n", tval);
		return tval;
	}
	tval = bq27441_write(di, 0x4c, new_state->design_energy, 0);
	if (tval < 0) {
		printf("[cfg state]write design_energy err, tval:%d\n", tval);
		return tval;
	}

	/*write new chksum*/
	tval = bq27441_write(di, 0x60, new_chksum, 1);
	if (tval < 0) {
		printf("[cfg state]write new_chksum err, tval:%d\n", tval);
		return tval;
	}
	mdelay(50);

	debug("[cfg state]bq27441_battery_cfgupdate_state success\n");

	return 0;
}

static int bq27441_battery_get_pwron_state(struct bq27441_device_info *di)
{
	int tval = 0;

	 /*check sealed/unsealed mode*/
	tval = bq27441_read(di, BQ27441_REG_FLAGS, 0);
	if (tval < 0) {
		printf("[cfg update]read BQ27441_REG_FLAGS err, tval:%d\n", tval);
		return tval;
	}
	if (!(tval & BQ27441_FLAG_ITPOR)) {
		debug("[cfg update]don't need updata config, flags:0x%x\n", tval);
		di->pwr_on = 0;
	} else {
		di->pwr_on = 1;
	}

	return di->pwr_on;
}

/*
  * config parameters for gauge by writing into data memory.
  * following steps:
  * 1.set unsealed mode;
  * 2.set config update mode;
  * 3.config update;
  * 4.config finished;
  * 5.set sealed.
  */
static int bq27441_battery_update_config(struct bq27441_device_info *di)
{
	int tval = 0;

	/*if sealed, set unsealed*/
	if (bq27441_battery_set_unsealed(di))
		return -1;

	/*set cfgupdate mode*/
	tval = bq27441_set_subcmd(di, SET_CFGUPDATE);
	if (tval < 0) {
		printf("[cfg update]send SET_CFGUPDATE subcmd err, tval:%d\n", tval);
		return tval;
	}

	mdelay(1200);
	tval = bq27441_read(di, BQ27441_REG_FLAGS, 0);
	if (tval < 0) {
		printf("[cfg update]read flags err, tval:%d\n", tval);
		return tval;
	}
	if (tval & BQ27441_FLAG_CFGUPMODE)
		debug("[cfg update] set cfgupdate success\n");

	bq27441_battery_cfgupdate_state(di);
	bq27441_battery_cfgupdate_finished(di);

	return 0;
}

static int bq27441_battery_force_reset(void)
{
	int tval;

	if (di.health == POWER_SUPPLY_HEALTH_DEAD)
		return 0;

	/*if sealed, set unsealed*/
	tval = bq27441_battery_set_unsealed(&di);
	if (tval)
		return tval;

	tval = bq27441_set_subcmd(&di, RESET);
	if (tval < 0) {
		printf("[cfg update]send RESET subcmd err, tval:%d\n", tval);
		return tval;
	}

	return 0;
}

static int bq27441_battery_cfg_init(struct bq27441_device_info *di,
	const void *blob)
{
	struct bq27441_cfg_items *items = &di->items;
	int parent;
	int node;
	int error = 0;

	node = fdt_node_offset_by_compatible(blob, 0,
		"ti,bq27441-battery");
	if (node < 0) {
		printf("%s no match in dts\n", __func__);
		return -1;
	}

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	di->bus_id = i2c_get_bus_num_fdt(parent);
	printf("bq27441 bus id(%d)\n", di->bus_id);

	items->design_capacity = fdtdec_get_int(blob, node,
		"design_capacity", 0);
	error |= (items->design_capacity == -1);
	if (error) {
		printf("%s get dts value failed\n", __func__);
		return -1;
	}

	debug("%s design_capacity(%d)\n", __func__, items->design_capacity);

	return 0;
}

static void bq27441_battery_dump(void)
{
	printf("bq_bat:online(%d),vol(%dmv),soc(%d%%)\n",
		di.online, di.bat_vol, di.soc);
}

static struct power_battery bq27441_bat = {
	.measure_voltage = bq27441_battery_measure_voltage,
	.chk_online = bq27441_battery_check_online,
	.calc_soc = bq27441_battery_calc_rsoc,
	.reset = bq27441_battery_force_reset,
};

int bq27441_battery_init(const void *blob)
{
	int ret;

	ret = bq27441_battery_cfg_init(&di, blob);
	if (ret)
		return ret;

	atc260x_charger_release_guard();
	bq27441_i2c_init(&di);
	if (di.health == POWER_SUPPLY_HEALTH_DEAD)
		goto fail;
	bq27441_battery_get_pwron_state(&di);
	if (di.pwr_on)
		bq27441_battery_update_config(&di);
	bq27441_battery_check_online();
	bq27441_battery_measure_voltage(&di.bat_vol);
	bq27441_battery_calc_rsoc();
	di.detected = 1;
	bq27441_battery_dump();
fail:
	power_battery_register(&bq27441_bat);

	return 0;
}

