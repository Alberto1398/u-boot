#include <asm/io.h>
#include <common.h>
#include <power/atc260x/owl_atc260x.h>
#include <power/atc260x/atc260x_irkey.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/input/input.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <power/atc260x/actions_reg_atc2609a.h>

#define ATC2609A_INTS_BIT_IR (0x1 << 8)
#define ATC2609A_IRC_STAT_IIP (0x1 << 2)
#define ATC2609A_IRC_STAT_RCD (0x1 << 4)

#define IR_PROTOCOL_9012	0x0
#define IR_PROTOCOL_NEC8	0x1
#define IR_PROTOCOL_RC5		0x2
DECLARE_GLOBAL_DATA_PTR;
#if 0
static void dump_irc_reg(void){
	printf("ATC2609A_RTC_CTL = 0x%x\n", atc260x_reg_read(ATC2609A_RTC_CTL));
	printf("ATC2609A_IRC_CC = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_CC));
	printf("ATC2609A_PMU_SYS_CTL3 = 0x%x\n", atc260x_reg_read(ATC2609A_PMU_SYS_CTL3));
	printf("ATC2609A_IRC_STAT = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_STAT));
	printf("ATC2609A_IRC_KDC = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_KDC));
	printf("ATC2609A_IRC_WK = 0x%x\n", atc260x_reg_read(ATC2609A_IRC_WK));
	printf("ATC2609A_INTS_PD = 0x%x\n", atc260x_reg_read(ATC2609A_INTS_PD));
	printf("ATC2609A_INTS_MSK = 0x%x\n", atc260x_reg_read(ATC2609A_INTS_MSK));
}
#else
static inline void dump_irc_reg(void){}
#endif

struct atc260x_irkey {
	unsigned int proto;
	unsigned int customer_code;
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


static int irkey_cfg_get(void)
{
	int node;
	int ret = 0;
	int i;
	
	printf("irkey_cfg_get...\n");
	
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "actions,atc2609a-ir");
	if (node < 0)
		return -1;
	
	/* protocol */
	irkey.proto = fdtdec_get_int(gd->fdt_blob,
				node, "protocol", 1);
	printf("proto:%d\n",irkey.proto);
	irkey.customer_code = fdtdec_get_int(gd->fdt_blob,
				node, "customer_code", 0);
	printf("customer_code:%x\n",irkey.customer_code);
	if (!irkey.customer_code)
		return -1;
		
	irkey.keymapsize = fdtdec_get_int(gd->fdt_blob,
				node, "keymapsize", 50);
	printf("keymapsize=%d\n",irkey.keymapsize);
	
	irkey.ircode = malloc(4 * irkey.keymapsize);
	irkey.keycode = malloc(4 * irkey.keymapsize);
		
	ret = fdtdec_get_int_array(gd->fdt_blob, node,
					"ir_code", irkey.ircode, irkey.keymapsize);
	if (ret < 0)
		return -1;
		
	ret = fdtdec_get_int_array(gd->fdt_blob, node,
					"key_code", irkey.keycode, irkey.keymapsize);
	if (ret < 0)
		return -1;

	return 0;
}
static void irkey_init(void)
{
	atc260x_set_bits(ATC2609A_IRC_WK, 0xffff, 0x0000);
	
	/* enable external osc */
	atc260x_set_bits(ATC2609A_RTC_CTL, 0x0020, 0x0020);
	
	/* enable irq mask.*/
	atc260x_set_bits(ATC2609A_INTS_MSK, ATC2609A_INTS_BIT_IR, ATC2609A_INTS_BIT_IR);
	
	/* enable irc. */
	atc260x_set_bits(ATC2609A_IRC_CTL, 0xffff, 0x000d);

    /*mfp*/
    atc260x_reg_setbits(ATC2609A_PMU_SYS_CTL4, 0x301C, 0x000C);

    /*wake up*/
    atc260x_reg_setbits(ATC2609A_PMU_SYS_CTL0, 0x0020, 0x0020);

	/* set user code.*/
	atc260x_set_bits(ATC2609A_IRC_CC, 0xffff, irkey.customer_code & 0x0FFFF);
	
	/* clear irc state. */
	atc260x_reg_write(ATC2609A_IRC_STAT, 0xffff); 
	
	atc260x_set_bits(ATC2609A_PAD_EN, 0x0001, 0x0001);

	dump_irc_reg();
}
/*	if return -1,means something wrong with key scan or
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
	
	ret = irkey_cfg_get();
	if (ret == -1)
		return ret;
	irkey_init();

	printf("wait for irkey test...\n");
	while (count--) {
		int_pd = atc260x_reg_read(ATC2609A_INTS_PD);
		irc_stat = atc260x_reg_read(ATC2609A_IRC_STAT);
		regcode = atc260x_reg_read(ATC2609A_IRC_KDC);
		atc260x_reg_write(ATC2609A_IRC_STAT, irc_stat);
		
		if (!(int_pd & ATC2609A_INTS_BIT_IR) || (irc_stat & ATC2609A_IRC_STAT_RCD)
			 || !(irc_stat & ATC2609A_IRC_STAT_IIP)) {
			mdelay(50);
			printf("[irkey]continue check...\n");
			continue;
		}
	printf("[irkey]check over..\n");
	ircode = regcode2ircode(irkey.proto, regcode);
	ret = ircode2keycode(ircode, &keycode);
	if (ret == -1)
		return ret;
	printf("key_code:0x%x, ircode:0x%x,regcode:0x%x\n",
				keycode, ircode, regcode);
	break;
	}
	return keycode;
}

