#include <power/atc260x/atc260x_core.h>
#if defined CONFIG_DM_PMIC_ATC2609A
#include <power/atc260x/actions_reg_atc2609a.h>
#elif defined CONFIG_DM_PMIC_ATC2603C
#include <power/atc260x/actions_reg_atc2603c.h>
#endif
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <i2c_eeprom.h>

#define __ffs(val) __builtin_ctzl(val)

struct atc260x_pstore_fmt {
	unsigned short regs[4];	/* destination register of each slice */
	unsigned short reg_bm[4];	/* register bitmap of each slice */
	char src_bp[4];		/* source start bit position of each slice */
};
#if defined CONFIG_DM_PMIC_ATC2603C

static const struct atc260x_pstore_fmt sc_atc2603c_pstore_fmt_tbl[] = {
	[ATC260X_PSTORE_TAG_REBOOT_ADFU] = {
					    .regs = {ATC2603C_PMU_UV_STATUS},
					    .reg_bm = {(1U << 1)},
					    .src_bp = {0},
					    },

	[ATC260X_PSTORE_TAG_REBOOT_RECOVERY] = {
						.regs = {
						ATC2603C_PMU_OV_INT_EN},
						.reg_bm = {(3U << 0)},
						.src_bp = {0},
						},

	[ATC260X_PSTORE_TAG_FW_S2] = {
				      .regs = {ATC2603C_PMU_SYS_CTL3},
				      .reg_bm = {(1U << 4)},
				      .src_bp = {0},
				      },

	[ATC260X_PSTORE_TAG_FW_BOOT_FROM_S2] = {
				      .regs = {ATC2603C_PMU_SYS_CTL3},
				      .reg_bm = {(1U << 6)},
				      .src_bp = {0},
				      },

	[ATC260X_PSTORE_TAG_DIS_MCHRG] = {
					  .regs = {ATC2603C_PMU_SYS_CTL9},
					  .reg_bm = {(1U << 0)},
					  .src_bp = {0},
					  },

	[ATC260X_PSTORE_TAG_RTC_MSALM] = {
					  .regs = {ATC2603C_PMU_FW_USE0},
					  .reg_bm = {(0xfffU << 0)},
					  .src_bp = {0},
					  },

	[ATC260X_PSTORE_TAG_RTC_HALM] = {
					 .regs = {ATC2603C_PMU_VBUS_CTL1},
					 .reg_bm = {(0x1fU << 0)},
					 .src_bp = {0},
					 },

	[ATC260X_PSTORE_TAG_RTC_YMDALM] = {
					   .regs = {ATC2603C_PMU_SYS_CTL8},
					   .reg_bm = {(0xffffU << 0)},
					   .src_bp = {0},
					   },

	[ATC260X_PSTORE_TAG_GAUGE_CAP] = {
					  .regs = {ATC2603C_PMU_FW_USE1,ATC2603C_PMU_SYS_CTL9},
					  .reg_bm = {(0xffffU << 0), (0xfU << 8)},
					  .src_bp = {0, 16},
					  },

	[ATC260X_PSTORE_TAG_GAUGE_BAT_RES] = {
					      .regs = {
									ATC2603C_PMU_OC_INT_EN,
					       ATC2603C_PMU_OC_INT_EN,
					       ATC2603C_PMU_OC_INT_EN,
					       ATC2603C_PMU_SYS_CTL9
					       },
					      .reg_bm = {
								(0x3fU << 0), (1U << 10),
					       (7U << 13), (0x3fU << 2)},
					      .src_bp = {0, 6, 7, 10},
					      },

	[ATC260X_PSTORE_TAG_GAUGE_ICM_EXIST] = {
						.regs = {ATC2603C_PMU_SYS_CTL3},
						.reg_bm = {(1U << 5)},
						.src_bp = {0},
						},

	[ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME] = {
						 .regs = {ATC2603C_PMU_FW_USE2},
						 .reg_bm = {(0x3fffU<<2)},
						 .src_bp = {0,},
						 },

	[ATC260X_PSTORE_TAG_GAUGE_S2_CONSUMP] = {
						 .regs = {0,},	/* not need for 2603c */
						 .reg_bm = {0,},
						 .src_bp = {0,},
						 },

	[ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET] = {
						 .regs = {ATC2603C_PMU_SYS_CTL9,},
						 .reg_bm = {(1U << 1),},
						 .src_bp = {0,},
						 },

	[ATC260X_PSTORE_TAG_RESUME_ADDR] = {
					    .regs = {
								ATC2603C_PMU_FW_USE0,
								ATC2603C_PMU_SYS_CTL8
					     },
					    .reg_bm = {
							(0xffffU << 0), (0xffffU << 0)},
					    .src_bp = {16, 0},
					    },
	[ATC260X_PSTORE_TAG_CAPACITY] = {
					    .regs = { ATC2603C_PMU_FW_USE3,},
					    .reg_bm = {(0xffffU << 0),},
					    .src_bp = {0,},
					    },
};
#endif
#if defined CONFIG_DM_PMIC_ATC2609A
static const struct atc260x_pstore_fmt sc_atc2609a_pstore_fmt_tbl[] = {
	[ATC260X_PSTORE_TAG_REBOOT_ADFU] = {
					    .regs = {ATC2609A_PMU_UV_STATUS},
					    .reg_bm = {(1U << 1)},
					    .src_bp = {0},
					    },

	[ATC260X_PSTORE_TAG_REBOOT_RECOVERY] = {
						.regs = {
							 ATC2609A_PMU_OV_INT_EN},
						.reg_bm = {(3U << 0)},
						.src_bp = {0},
						},

	[ATC260X_PSTORE_TAG_FW_S2] = {
				      .regs = {ATC2609A_PMU_SYS_CTL3},
				      .reg_bm = {(1U << 4)},
				      .src_bp = {0},
				      },

	[ATC260X_PSTORE_TAG_FW_BOOT_FROM_S2] = {
				      .regs = {ATC2609A_PMU_SYS_CTL3},
				      .reg_bm = {(1U << 6)},
				      .src_bp = {0},
				      },

	[ATC260X_PSTORE_TAG_DIS_MCHRG] = {
					  .regs = {ATC2609A_PMU_UV_INT_EN},
					  .reg_bm = {(1U << 0)},
					  .src_bp = {0},
					  },

	[ATC260X_PSTORE_TAG_SHDWN_NOT_DEEP] = {
					       .regs = {ATC2609A_PMU_UV_INT_EN},
					       .reg_bm = {(1U << 1)},
					       .src_bp = {0},
					       },

	[ATC260X_PSTORE_TAG_RTC_MSALM] = {
					  .regs = {ATC2609A_PMU_SYS_CTL7},
					  .reg_bm = {(0xfffU << 0)},
					  .src_bp = {0},
					  },

	[ATC260X_PSTORE_TAG_RTC_HALM] = {
					 .regs = {ATC2609A_PMU_VBUS_CTL1},
					 .reg_bm = {(0x1fU << 0)},
					 .src_bp = {0},
					 },

	[ATC260X_PSTORE_TAG_RTC_YMDALM] = {
					   .regs = {ATC2609A_PMU_SYS_CTL8},
					   .reg_bm = {(0xffffU << 0)},
					   .src_bp = {0},
					   },

	[ATC260X_PSTORE_TAG_GAUGE_CAP] = {
					  .regs = {ATC2609A_PMU_FW_USE0, ATC2609A_PMU_SYS_CTL9},
					  .reg_bm = {(0xffffU << 0), (0xfU << 8)},
					  .src_bp = {0, 16},
					  },

	[ATC260X_PSTORE_TAG_GAUGE_BAT_RES] = {
					      .regs = {0,},	/* not need for 2609a */
					      .reg_bm = {0,},
					      .src_bp = {0,},
					      },

	[ATC260X_PSTORE_TAG_GAUGE_ICM_EXIST] = {
						.regs = {0,},	/* not need for 2609a */
						.reg_bm = {0,},
						.src_bp = {0,},
						},

	[ATC260X_PSTORE_TAG_GAUGE_SHDWN_TIME] = {
						 .regs = {
							  ATC2609A_PMU_SYS_CTL9,
							  ATC2609A_PMU_BAT_CTL0,
							  ATC2609A_PMU_BAT_CTL1,
							  ATC2609A_PMU_WALL_CTL1},
						 .reg_bm = {
							    (0xffU << 0),
							    (0x3fU << 0),
							    (0x1ffU << 0),
							    (0xffU << 0)},
						 .src_bp = {0, 8, 14, 23},
						 },

	[ATC260X_PSTORE_TAG_GAUGE_S2_CONSUMP] = {
						 .regs = {
							  ATC2609A_PMU_VBUS_CTL0,},
						 .reg_bm = {(0x3fU << 0),},
						 .src_bp = {0,},
						 },

	[ATC260X_PSTORE_TAG_GAUGE_CLMT_RESET] = {
						 .regs = {
							  ATC2609A_PMU_OV_STATUS,},
						 .reg_bm = {(1U << 1),},
						 .src_bp = {0,},
						 },

	[ATC260X_PSTORE_TAG_RESUME_ADDR] = {
					    .regs = {
						     ATC2609A_PMU_SYS_CTL7,
						     ATC2609A_PMU_SYS_CTL8},
					    .reg_bm = {
						       (0xffffU << 0),
						       (0xffffU << 0)},
					    .src_bp = {16, 0},
					    },
	[ATC260X_PSTORE_TAG_CAPACITY] = {
		.regs   = {0, },
		.reg_bm = {0, },
		.src_bp = {0, },
	},
};
#endif
static const struct atc260x_pstore_fmt *_atc260x_pstore_get_fmt_desc(unsigned
								     int tag)
{
	const struct atc260x_pstore_fmt *fmt_tbl;

	/* 这样写而不是直接查表, 是为了size的优化,
	 * 当ATC260X_ICTYPE是常量时就有差别了. */
#if defined CONFIG_DM_PMIC_ATC2609A
	fmt_tbl = sc_atc2609a_pstore_fmt_tbl;
#elif defined CONFIG_DM_PMIC_ATC2603C
	fmt_tbl = sc_atc2603c_pstore_fmt_tbl;
#else
	return NULL;
#endif
	return &(fmt_tbl[tag]);
}

int atc260x_pstore_set(unsigned int tag, unsigned int value)
{
	const struct atc260x_pstore_fmt *fmt;
	unsigned int i, reg_shift;
	u32 src_bm, reg_bm, reg_val;
	int ret;

	if (tag >= ATC260X_PSTORE_TAG_NUM)
		return -1;

	fmt = _atc260x_pstore_get_fmt_desc(tag);

	if (fmt->reg_bm[0] == 0 && value != 0) {
		pr_err("pstore tag %u not available\n", tag);
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(fmt->regs); i++) {
		if (fmt->reg_bm[i] == 0)
			break;

		reg_bm = fmt->reg_bm[i];
		reg_shift = __ffs(reg_bm);
		src_bm = (reg_bm >> reg_shift) << fmt->src_bp[i];
		reg_val = ((value & src_bm) >> fmt->src_bp[i]) << reg_shift;
		ret = atc260x_reg_setbits(fmt->regs[i], reg_bm, reg_val);
		if (ret) {
			pr_err("pstore io failed, ret=%d", ret);
			return ret;
		}
	}

	return 0;
}

int atc260x_pstore_get(unsigned int tag, unsigned int *p_value)
{
	const struct atc260x_pstore_fmt *fmt;
	unsigned int i, reg_shift;
	u32 result, reg_bm;
	int ret;

	if (tag >= ATC260X_PSTORE_TAG_NUM)
		return -1;

	fmt = _atc260x_pstore_get_fmt_desc(tag);

	result = 0;
	for (i = 0; i < ARRAY_SIZE(fmt->regs); i++) {
		if (fmt->reg_bm[i] == 0)
			break;

		reg_bm = fmt->reg_bm[i];
		reg_shift = __ffs(reg_bm);

		ret = atc260x_reg_read(fmt->regs[i]);
		if (ret < 0) {
			pr_err("pstore io failed, ret=%d", ret);
			return ret;
		}
		result |= (((u32) ret & reg_bm) >> reg_shift) << fmt->src_bp[i];
	}

	*p_value = result;
	return 0;
}

int atc260x_pstore_reset_all(void)
{
	unsigned int t;
	int ret;
	for (t = 0; t < ATC260X_PSTORE_TAG_NUM; t++) {
		ret = atc260x_pstore_set(t, 0);
		if (ret)
			return ret;
	}
	return 0;
}

int atc260x_pstore_probe(struct udevice *dev)
{
	/*printf("atc260x_pstore_probe\n"); */
	return 0;
}

static const struct udevice_id atc260x_pstore_ids[] = {
	{.compatible = "actions,atc2609a-pstore"},
	{.compatible = "actions,atc2603c-pstore"},
	{}
};

U_BOOT_DRIVER(atc260x_pstore) = {
.id = UCLASS_ROOT, .name = "atc260x_pstore", .of_match =
	    atc260x_pstore_ids, .probe = atc260x_pstore_probe};
