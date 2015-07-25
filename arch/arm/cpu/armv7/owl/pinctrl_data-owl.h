#ifndef __PINCTRL_DATA_OWLXX_H__ 
#define __PINCTRL_DATA_OWLXX_H__

#define PINCTRL_GROUP_NAME_LEN 24
#define PINCTRL_MFPCTLREGS  4
#define PINCTRL_GPIOBANKS   5
#define PINCTRL_STREGS	    2

enum owlxx_pinconf_param {
	OWLXX_PINCONF_PARAM_PULL,
	OWLXX_PINCONF_PARAM_PADDRV,
	OWLXX_PINCONF_PARAM_SCHMITT,
};

enum owlxx_pinconf_pull {
	OWLXX_PINCONF_PULL_NONE,
	OWLXX_PINCONF_PULL_DOWN,
	OWLXX_PINCONF_PULL_UP,
};

#define OWLXX_PINCONF_PACK(_param_, _arg_) ((_param_) << 16 | ((_arg_) & 0xffff))
#define OWLXX_PINCONF_UNPACK_PARAM(_conf_) ((_conf_) >> 16)
#define OWLXX_PINCONF_UNPACK_ARG(_conf_) ((_conf_) & 0xffff)

/**
 * struct owlxx_pinmux_group - describes the SOC pin group
 * @name: the name of this specific pin group
 * @pads: an array of discrete physical pins, ie, named pads in ic spec,
 *      used in this group, defined in driver-local pin enumeration space
 * @padcnt: the number of pins in this group array, i.e. the number of
 *	elements in .pads so we can iterate over that array
 * @mfpctl: fragment of mfp code
 * @gpiosw: gpio code for switch off pads
 * @schimtt: schmitt trigger setup code for this module
 */

struct owlxx_pinctrl_group {
	const char *name;
	unsigned int *pads;
	unsigned int padcnt;
	unsigned int *funcs;
	unsigned int nfuncs;

	int mfpctl_regnum;
	unsigned int mfpctl_shift;
	unsigned int mfpctl_width;

	int paddrv_regnum;
	unsigned int paddrv_shift;
	unsigned int paddrv_width;
//	u32 gpiosw[PINCTRL_GPIOBANKS];
//	u32 schimtt[PINCTRL_STREGS];
};

/*
 * struct owlxx_pinctrl_func - the SOC pinctrl mux functions
 * @name: The name of the function, exported to pinctrl core.
 * @groups: An array of pin groups that may select this function.
 * @ngroups: The number of entries in @groups.
 */
struct owlxx_pinctrl_func {
	const char *name;
	const char * const *groups;
	unsigned ngroups;
};

/*
 * struct owlxx_pinconf_reg_pull - Actions SOC pinctrl pull up/down regs
 * @reg: The index of PAD_PULLCTL regs.
 * @mask: The bit mask of PAD_PULLCTL fragment.
 * @pullup: The pullup value of PAD_PULLCTL fragment.
 * @pulldown: The pulldown value of PAD_PULLCTL fragment.
 */
struct owlxx_pinconf_reg_pull {
	int reg_num;
	unsigned int shift;
	unsigned int width;
	unsigned int pullup;
	unsigned int pulldown;
};

/*
 * struct owlxx_pinconf_schimtt - Actions SOC pinctrl PAD_ST regs
 * @reg: The index of PAD_ST regs.
 * @mask: The bit mask of PAD_ST fragment.
 */
struct owlxx_pinconf_schimtt {
	unsigned int *schimtt_funcs;
	unsigned int num_schimtt_funcs;
	int reg_num;
	unsigned int shift;
};

/*
 * struct owlxx_pinconf_pad_info - Actions SOC pinctrl pad info
 * @pad: The pin, in soc, the pad code of the silicon.
 * @gpio: The gpio number of the pad.
 * @pull: pull up/down reg, mask, and value.
 * @paddrv: pad drive strength info.
 * @schimtt: schimtt triger info.
 */
struct owlxx_pinconf_pad_info {
	int pad;
	int gpio;
	struct owlxx_pinconf_reg_pull *pull;
	struct owlxx_pinconf_schimtt *schimtt;
};

/*
 * this struct is identical to pinctrl_pin_desc.
 * struct pinctrl_pin_desc - boards/machines provide information on their
 * pins, pads or other muxable units in this struct
 * @number: unique pin number from the global pin number space
 * @name: a name for this pin
 */
struct owlxx_pinctrl_pin_desc {
	unsigned number;
	const char *name;
};

extern const struct owlxx_pinctrl_pin_desc atm7029a_pads[];
extern unsigned int atm7029a_num_pads;
extern const struct owlxx_pinctrl_group atm7029a_groups[];
extern int atm7029a_num_groups;
extern const struct owlxx_pinctrl_func atm7029a_functions[];
extern int atm7029a_num_functions;
extern struct owlxx_pinconf_pad_info atm7029a_pad_tab[];

extern const struct owlxx_pinctrl_pin_desc atm7039c_pads[];
extern unsigned int atm7039c_num_pads;
extern const struct owlxx_pinctrl_group atm7039c_groups[];
extern int atm7039c_num_groups;
extern const struct owlxx_pinctrl_func atm7039c_functions[];
extern int atm7039c_num_functions;
extern struct owlxx_pinconf_pad_info atm7039c_pad_tab[];

extern const struct owlxx_pinctrl_pin_desc atm7059tc_pads[];
extern unsigned int atm7059tc_num_pads;
extern const struct owlxx_pinctrl_group atm7059tc_groups[];
extern int atm7059tc_num_groups;
extern const struct owlxx_pinctrl_func atm7059tc_functions[];
extern int atm7059tc_num_functions;
extern struct owlxx_pinconf_pad_info atm7059tc_pad_tab[];

extern const struct owlxx_pinctrl_pin_desc atm7059_pads[];
extern unsigned int atm7059_num_pads;
extern const struct owlxx_pinctrl_group atm7059_groups[];
extern int atm7059_num_groups;
extern const struct owlxx_pinctrl_func atm7059_functions[];
extern int atm7059_num_functions;
extern struct owlxx_pinconf_pad_info atm7059_pad_tab[];

#endif /* __PINCTRL_DATA_OWLXX_H__ */
