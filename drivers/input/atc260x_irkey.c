#include <asm/io.h>
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_irkey.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/input/input.h>
#include <libfdt.h>
#include <fdtdec.h>

#ifdef CONFIG_DM_PMIC_ATC2609A
#include <power/atc260x/actions_reg_atc2609a.h>
#define INTS_PD	ATC2609A_INTS_PD
#define IRC_STAT ATC2609A_IRC_STAT
#define IRC_KDC	ATC2609A_IRC_KDC
#elif CONFIG_DM_PMIC_ATC2603C
#include <power/atc260x/actions_reg_atc2603c.h>
#define INTS_PD	ATC2603C_INTS_PD
#define IRC_STAT	ATC2603C_IRC_STAT
#define IRC_KDC	ATC2603C_IRC_KDC
#endif


#define INTS_BIT_IR (0x1 << 8)
#define IRC_STAT_IIP (0x1 << 2)
#define IRC_STAT_RCD (0x1 << 4)
#define IRC_STAT_IREP (0x1 << 0)

#define IR_PROTOCOL_9012	0x0
#define IR_PROTOCOL_NEC8	0x1
#define IR_PROTOCOL_RC5		0x2

#define BOARDINFO_STR_LEN (16)
#define IR_IDX 13      
#define IR_LEN 1 
DECLARE_GLOBAL_DATA_PTR;

#if 0
static void dump_irc_reg(void){
#ifdef CONFIG_DM_PMIC_ATC2609A
	printf("ATC2609A_RTC_CTL = 0x%x\n", atc260x_reg_read(ATC2609A_RTC_CTL));
	printf("ATC2609A_IRC_CC = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_CC));
	printf("ATC2609A_PMU_SYS_CTL3 = 0x%x\n", atc260x_reg_read(ATC2609A_PMU_SYS_CTL3));
	printf("ATC2609A_IRC_STAT = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_STAT));
	printf("ATC2609A_IRC_KDC = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_KDC));
	printf("ATC2609A_IRC_WK = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_WK));
	printf("ATC2609A_INTS_PD = 0x%x\n", atc260x_reg_read(ATC2609A_INTS_PD));
	printf("ATC2609A_INTS_MSK = 0x%x\n", atc260x_reg_read(ATC2609A_INTS_MSK));
#elif CONFIG_DM_PMIC_ATC2603C
	printf("ATC2603C_RTC_CTL = 0x%x\n", atc260x_reg_read(ATC2603C_RTC_CTL));
	printf("ATC2603C_IRC_CTL = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_CTL));
	printf("ATC2603C_IRC_CC = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_CC));
	printf("ATC2603C_IRC_RCC = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_RCC));
	printf("ATC2603C_PMU_SYS_CTL0 = 0x%x\n", atc260x_reg_read(ATC2603C_PMU_SYS_CTL0));
	printf("ATC2603C_IRC_STAT = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_STAT));
	printf("ATC2603C_IRC_KDC = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_KDC));
	printf("ATC2603C_IRC_WK = 0x%x\n", atc260x_reg_read(ATC2603C_IRC_WK));
	printf("ATC2603C_INTS_PD = 0x%x\n", atc260x_reg_read(ATC2603C_INTS_PD));
	printf("ATC2603C_INTS_MSK = 0x%x\n", atc260x_reg_read(ATC2603C_INTS_MSK));
	printf("ATC2603C_PMU_MUX_CTL0 = 0x%x\n", atc260x_reg_read(ATC2603C_PMU_MUX_CTL0));
#endif
}
#else
static inline void dump_irc_reg(void){}
#endif
extern int read_mi_item(char *name, void *buf, unsigned int count);
extern int write_mi_item(char *name, void *buf, unsigned int count);/*for test*/

struct atc260x_irkey {
	unsigned int proto;
	unsigned int customer_code;
	unsigned int wk_code;
	u32 *ircode;
	u32 *keycode;
	u32 keymapsize;
} irkey;

static int regcode2ircode(unsigned int protocol,
		unsigned int val)
{
	switch (protocol) {
	case IR_PROTOCOL_9012:
		val &= 0x00ff;
		break;

	case IR_PROTOCOL_NEC8:
		val &= 0x00ff;
		break;

	case IR_PROTOCOL_RC5:
		val &= 0x003f;
		break;

	default:
		break;
	}

	return val;
}
static int ircode2keycode(unsigned int ircode,
		unsigned int *keycode)
{
	int i;

	for (i = 0; i < irkey.keymapsize; i++) {
		if (ircode == *(irkey.ircode + i)) {
			*keycode = *(irkey.keycode + i);
			printf("find keycode:%u\n", *keycode);
			return 0;
		}
	}
	return -1;
}
static int set_vendor_id_test()
{
	char *buf = "123456789A08D1E3";
	int ret = write_mi_item("VENDOR", buf, 17);
	if(ret < 0) {
		printf("write board info failed\n");
		return -1;
	}
	return 0;
}

static int get_irkeypad_bordinfo(char *ir_info)
{
	char buf[20];
	int str_len;
	int ret;
	/*set_vendor_id_test();*/
	memset(buf, 0, sizeof(buf));
	str_len = read_mi_item("VENDOR", buf, sizeof(buf));
	printf("str_len:%d\n", str_len);
	
	if (str_len < 0 || str_len < BOARDINFO_STR_LEN) {
		printf("read vendor id failed\n");
		ret = -1;
		return ret;
	}
	printf("vendor id is:%s",buf);
	/*get irkeypad info*/
	strncpy(ir_info, buf + (char)IR_IDX, IR_LEN);
	*(ir_info + IR_LEN) = '\0';
	printf("ir:%s", ir_info);
	ret = 0;

	return ret;
}
static int irkey_cfg_get(void)
{
	int node = -1;
	int subnode = -1;
	int ret = 0;
	int i;
	char ir_info[IR_LEN + 1] = "";
	char ircfg_node_name[10];
	bool has_vendor_id;
	printf("irkey_cfg_get...\n");
	has_vendor_id = (get_irkeypad_bordinfo(ir_info) == 0);
		
	strcpy(ircfg_node_name, "config");
	if(has_vendor_id){
		strcat(strcat(ircfg_node_name, "-"), ir_info);
	}
	printf("ircfg_node_name:%s", ircfg_node_name);
	
#ifdef CONFIG_DM_PMIC_ATC2609A
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "actions,atc2609a-ir");
#elif CONFIG_DM_PMIC_ATC2603C
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "actions,atc2603c-ir");
#endif
	if (node < 0)
		return -1;

	subnode = fdt_subnode_offset(gd->fdt_blob, node, ircfg_node_name);
	if (subnode < 0) {
		if (strcmp(ircfg_node_name, "config") != 0) {
			subnode = fdt_subnode_offset(gd->fdt_blob, node, "config");
			if (subnode < 0)
				return -1;
			else
				printf("use default cfg of irkeypad\n");
		}
	}

	/* protocol */
	irkey.proto = fdtdec_get_int(gd->fdt_blob,
				subnode, "protocol", 1);
	printf("proto:%d\n",irkey.proto);
	irkey.customer_code = fdtdec_get_int(gd->fdt_blob,
				subnode, "customer_code", 0);
	printf("customer_code:%x\n", irkey.customer_code);
	if (!irkey.customer_code)
		return -1;

	irkey.wk_code = fdtdec_get_int(gd->fdt_blob,
				subnode, "wk_code", 0);
	printf("wk_code:%x\n", irkey.wk_code);

	irkey.keymapsize = fdtdec_get_int(gd->fdt_blob,
				subnode, "keymapsize", 50);
	printf("keymapsize=%d\n", irkey.keymapsize);

	irkey.ircode = malloc(4 * irkey.keymapsize);
	irkey.keycode = malloc(4 * irkey.keymapsize);

	ret = fdtdec_get_int_array(gd->fdt_blob, subnode,
					"ir_code", irkey.ircode, irkey.keymapsize);
	if (ret < 0)
		return -1;

	ret = fdtdec_get_int_array(gd->fdt_blob, subnode,
					"key_code", irkey.keycode, irkey.keymapsize);
	if (ret < 0)
		return -1;

	return 0;
}

static u32 CONFIG_TABLE[][3] = {
/* REG		MASK		VAULE*/
#ifdef CONFIG_DM_PMIC_ATC2603C
{ATC2603C_RTC_CTL, 0x0020, 0x0020},	/* enable external osc */
{ATC2603C_IRC_WK, 0xffff, 0x0000},	/* wake up key data code,value will be changed by cfg item */
{ATC2603C_IRC_CTL, 0xffff, 0x000d},	/* enable IR,enable IRQ,IRC code mode */
{ATC2603C_IRC_CC, 0xffff, 0x0000},	/* customer code,will be changed by cfg item*/
{ATC2603C_PMU_MUX_CTL0, 0x3000, 0x1000},/* ADC channel SGPIO5*/
{ATC2603C_PMU_SYS_CTL0, 0x0020, 0x0020},/* IR_WK_EN */
{ATC2603C_INTS_MSK, 0x0100, 0x0100},	/* IR interrupt Mask bit*/
{ATC2603C_PAD_EN, 0x0001, 0x0001}	/* enable external IRQ*/
#elif CONFIG_DM_PMIC_ATC2609A
{ATC2609A_RTC_CTL, 0x0020, 0x0020},
{ATC2609A_IRC_WK, 0xffff, 0x0000},
{ATC2609A_IRC_CTL, 0xffff, 0x000d},
{ATC2609A_IRC_CC, 0xffff, 0x0000},
{ATC2609A_PMU_SYS_CTL4, 0x301C, 0x000C},
{ATC2609A_PMU_SYS_CTL0, 0x0020, 0x0020},
{ATC2609A_INTS_MSK, 0x0100, 0x0100},
{ATC2609A_PAD_EN, 0x0001, 0x0001}
#endif
};

static void irkey_init(void)
{
	int i;
	switch(irkey.proto) {
	case IR_PROTOCOL_9012:
		CONFIG_TABLE[1][2] = irkey.wk_code & 0x00ff;
		CONFIG_TABLE[2][2] = 0x000c;
		CONFIG_TABLE[3][2] = irkey.customer_code;
	break;
	case IR_PROTOCOL_NEC8:
		CONFIG_TABLE[1][2] = irkey.wk_code & 0x00ff;
		CONFIG_TABLE[2][2] = 0x000d;
		CONFIG_TABLE[3][2] = irkey.customer_code;
	break;
	case IR_PROTOCOL_RC5:
		CONFIG_TABLE[1][2] = irkey.wk_code & 0x003f;
		CONFIG_TABLE[2][2] = 0x000e;
		CONFIG_TABLE[3][2] = irkey.customer_code & 0x001f;
	break;
	default: break;
	}
	for (i = 0; i < 8; i++)
		atc260x_set_bits(CONFIG_TABLE[i][0],
						CONFIG_TABLE[i][1],
						CONFIG_TABLE[i][2]);
	dump_irc_reg();

}

/*	if return -1,means something wrong with key scan,or
	return key code
*/
int atc260x_irkey_scan()
{
	unsigned int keycode;
	unsigned int regcode;
	unsigned int ircode;
	unsigned int int_pd, irc_stat;
	int ret = 0;
	int count = 10;

	irc_stat = atc260x_reg_read(IRC_STAT);
	printf("irc_stat:%x\n",irc_stat);
	if (!(irc_stat & IRC_STAT_IREP))
		return -1;

	ret = irkey_cfg_get();
	if (ret == -1)
		return ret;
	irkey_init();

	printf("wait for irkey test...\n");
	while (count--) {
		int_pd = atc260x_reg_read(INTS_PD);
		irc_stat = atc260x_reg_read(IRC_STAT);
		regcode = atc260x_reg_read(IRC_KDC);
		atc260x_reg_write(IRC_STAT, irc_stat);

		if (!(int_pd & INTS_BIT_IR) || (irc_stat & IRC_STAT_RCD)
			 || !(irc_stat & IRC_STAT_IIP)) {
			mdelay(50);
			printf("[irkey]continue check...\n");
			continue;
		}
	printf("[irkey]check over..\n");
	ircode = regcode2ircode(irkey.proto, regcode);
	ret = ircode2keycode(ircode, &keycode);
	if (ret == -1) {
		printf("find keycode failed!\n");
		return ret;
	}
	printf("key_code:0x%x, ircode:0x%x,regcode:0x%x\n",
				keycode, ircode, regcode);
	break;
	}

	return keycode;
}

