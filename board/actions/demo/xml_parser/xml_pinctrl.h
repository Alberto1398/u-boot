#ifndef __XML_PINCTRL_H___
#define __XML_PINCTRL_H___

#include <asm/arch/actions_arch_common.h>

typedef struct pcfg_s
{
    char name[32];
    struct file *fd;
    void *tree;
	unsigned int tree_size;
    void *xml_file_buf;
    struct pcfg_s *next;
}pcfg_t;

#define MAX_PINMUX_GROUP_PINS 32
#define MAX_PINMUX_GROUP_REGCFGS 4
#define MAX_ASOC_PINMUX_GROUPS 64
#define MAX_ASOC_PINMUX_FUNCS 64

#define PINMUX_DEV "pinctrl-asoc"
#define PINCTRL_STATE_DEFAULT "default"

#define PINCTRL_MAP_MUX_MAX 32
enum pinctrl_map_type {
	PIN_MAP_TYPE_INVALID,
	PIN_MAP_TYPE_DUMMY_STATE,
	PIN_MAP_TYPE_MUX_GROUP,
	PIN_MAP_TYPE_CONFIGS_PIN,
	PIN_MAP_TYPE_CONFIGS_GROUP,
};

struct pinctrl_map_mux {
	char *group;
	char *function;
};

struct pinctrl_map_configs {
	char *group_or_pin;
	unsigned long *configs;
	unsigned num_configs;
};

struct pinctrl_map {
	char *dev_name;
	char *name;
	enum pinctrl_map_type type;
	char *ctrl_dev_name;
	union {
		struct pinctrl_map_mux mux;
		struct pinctrl_map_configs configs;
	} data;
};

struct pinctrl_map_data {
	char dev_name[32];
	char name[32];
	char ctrl_dev_name[32];
	char group[32];
	char function[32];
};

struct asoc_regcfg {
    unsigned int reg;
    unsigned int mask;
    unsigned int val;
};

typedef int (*handle_element_attr_func)(const char *element_attr);
typedef struct
{
    char name[16];
    handle_element_attr_func func;
}handle_element_attr;


struct asoc_pinmux_group_data {
	char name[32];
	unsigned int pins[MAX_PINMUX_GROUP_PINS];
    struct asoc_regcfg regcfgs[MAX_PINMUX_GROUP_REGCFGS];
};

struct asoc_pinmux_group {
	char *name;
	unsigned int *pins;
	unsigned int npins;

    struct asoc_regcfg *regcfgs;
	unsigned int nregcfgs;
};


struct asoc_pinmux_name {
	char name[32];
	char *groups[8];
};

struct asoc_pinmux_func {
	char *name;
	char * *groups;
	unsigned ngroups;
};

struct pinctrl_args
{
	unsigned int groups_i;
	unsigned int mappings_i;
	unsigned int fuctions_i;

	unsigned int addr_asoc_pinmux_group_datas;
	unsigned int addr_asoc_pinmux_groups;
	unsigned int addr_asoc_pinmux_func_names;
	unsigned int addr_asoc_pinmux_func_groups;
	unsigned int addr_asoc_pinmux_funcs;
	unsigned int addr_pinctrl_map_datas;
	unsigned int addr_pinctrl_maps;		

	unsigned int gpios_i;
	unsigned int addr_gpio_cfgs;
};

extern int paser_config(void);
extern int paser_preconfig(void);
extern int paser_pinctrls(void);
extern int paser_gpiocfgs(void);
extern int set_boot_gpio(void);


#define LARGE_BUF_SIZE 0x10000
#define PAGE_ALIGN(x) ((((x)+4096-1)/4096)*4096)

#endif
