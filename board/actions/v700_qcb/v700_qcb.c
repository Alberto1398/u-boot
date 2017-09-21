/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <fs.h>
#include <errno.h>
#include <fdtdec.h>
#include <os.h>
#include <serial.h>
#include <malloc.h>
#include <video.h>
#include <asm/arch/clk.h>
#include <asm/arch/dma.h>
#include <asm/io.h>
#include <asm/arch/regs.h>
#include <asm/arch/periph.h>
#include <asm/arch/reset.h>
#include <asm/arch/pwm.h>
#include <asm/arch/sys_proto.h>
#include "../common/board.h"


DECLARE_GLOBAL_DATA_PTR;

#define DVFSLEVEL_MAGIC    0x39500
#define OWL_CHIPID         0xc3000000L
#define OWL_GET_TYPE(dvfslevel)    (((((dvfslevel) ^ DVFSLEVEL_MAGIC) >> 10) ^ 0x3f)& 0x3f)

#define ID_V700     1

 int get_bond_opt(void)
{
	unsigned int bond_opt;
	bond_opt = (unsigned int)__invoke_fn_smc(OWL_CHIPID, 0L, 0L, 0L);
	printf("chipid :0x%x\n", bond_opt);
	bond_opt = OWL_GET_TYPE(bond_opt);
	printf("get_bond_opt :0x%x\n", bond_opt);
	return bond_opt;
}

void owl_serial_putc(struct udevice *dev, char ch)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);
	int err;
	do {
		err = ops->putc(dev, ch);
	} while (err == -EAGAIN);
}

int owl_serial_getc_overtime(struct udevice *dev)
{
	int time = 0;
	struct dm_serial_ops *ops = serial_get_ops(dev);
	// wait for data 0: not, 1: empty 
	while (ops->pending && !ops->pending(dev, true)) { 
		udelay(1000);
		time++;
		if ( time > 1000)
			return -1;
	}
	return ops->getc(dev);
}

static void owl_serial_config(int uart_index)
{

	u32 periph_id = PERIPH_ID_UART0 + uart_index;
	
	owl_clk_enable_by_perip_id(periph_id);
	switch (uart_index) {
	case 0:
		/* P_UART0_RX, P_UART0_TX */
#define UART0_RX_BITS (0x7 << 0)
#define UART0_TX_BITS (0x7 << 19)
		clrbits_le32(MFP_CTL2, UART0_RX_BITS);
		clrbits_le32(MFP_CTL3, UART0_TX_BITS);

		break;
	case 1:
		break;
	case 2:
		owl_clk_enable_by_perip_id(PERIPH_ID_UART2);
		/* 359P, P_SD0_D0 -> UART2_RX, P_SD0_D1 -> UART2_TX */
#define UART2_RX_BITS (0x4 << 17)
#define UART2_RX_BITS_MASK (0x7 << 17)
#define UART2_TX_BITS (0x4 << 14)
#define UART2_TX_BITS_MASK (0x7 << 14)
		clrsetbits_le32(MFP_CTL2,
				UART2_RX_BITS_MASK | UART2_TX_BITS_MASK,
				UART2_RX_BITS | UART2_TX_BITS);

		break;
	case 3:
		break;
	case 4:
#define UART4_RX_BITS (0x3 << 11)
#define UART4_RX_BITS_MASK (0x3 << 11)
#define UART4_TX_BITS (0x1 << 6)
#define UART4_TX_BITS_MASK (0x3 << 6)
		owl_clk_enable_by_perip_id(PERIPH_ID_UART4);
		clrsetbits_le32(MFP_CTL0,
				UART4_RX_BITS_MASK | UART4_TX_BITS_MASK ,
				UART4_RX_BITS | UART4_TX_BITS);
		/* UART4 is auto enabled */
		break;
	case 5:
		/*	P_SD0_D0 -> UART5_RX, P_SD0_D1 -> UART5_TX	*/
#define UART5_RX_BITS (0x5 << 17)
#define UART5_RX_BITS_MASK (0x7 << 17)
#define UART5_TX_BITS (0x5 << 14)
#define UART5_TX_BITS_MASK (0x7 << 14)
		clrsetbits_le32(MFP_CTL2,
				UART5_RX_BITS_MASK | UART5_TX_BITS_MASK ,
				UART5_RX_BITS | UART5_TX_BITS);
		break;
	default:
		return ;
	}

	#define CMU_UARTCLK(index)   (0xe016805cL+4*index)
	#define UART_CTL(index)   (0xe0120000L+0x2000*index)
	/* uart clock settings */
	writel(0x19, CMU_UARTCLK(uart_index));

	owl_reset_by_perip_id(periph_id);
	/* uart configure, UART0_CTL
	 * bit20 = 0, Loopback Disabled
	 * bit15=1, Uart EN
	 * bit[1:0]=11, 8bit Width
	 */
	writel(0x008003, UART_CTL(uart_index));
}

#define TEST_NUM 14
const char test_str[TEST_NUM]={0xaa,0x55,0x0a,0xa0,0x05,0x50,0xa5,0x5a,0xff,0x0, 0xf0,0x0f,0x36,0x63};
int check_comm_serial(int uart1_index, int uart2_index )
{
	int i, ch;
	struct udevice *dev1, *dev2;
	
	if ( uclass_get_device_by_seq(UCLASS_SERIAL, uart1_index, &dev1) ) {
		printf("open uart%d fail\n", uart1_index);
		return -1;
	}
	owl_serial_config(uart1_index);
	if ( uart1_index == uart2_index) {
		dev2 = dev1;		
	} else {
		if ( uclass_get_device_by_seq(UCLASS_SERIAL, uart2_index, &dev2) ) {
			printf("open uart%d fail\n", uart2_index);
			return -1;
		}
		owl_serial_config(uart2_index);
	}	

	printf("test the same uart%d send,uart%d recive:\n", uart1_index, uart2_index);

	for ( i = 0; i < TEST_NUM; i++ ) 
	{
		owl_serial_putc(dev1,test_str[i]);
		ch = owl_serial_getc_overtime(dev2);
		if ( ch < 0 || ch != test_str[i]) 
		{				
			printf("test the same uart fail, tx=%d, rx=%d\n", ch,  test_str[i]);
			return 0;
		}
		printf("i=%d, s=%x, r=%x\n",i,test_str[i], ch );
	}

	if(uart1_index != uart2_index)
	{

		printf("test the different uart%d send,uart%d recive:\n", uart2_index, uart1_index);

		for ( i = 0; i < TEST_NUM; i++ ) 
		{
			owl_serial_putc(dev2,test_str[i]);
			ch = owl_serial_getc_overtime(dev1);
			if ( ch < 0 || ch != test_str[i]) 
			{				
				printf("uart fail, tx=%d, rx=%d\n", ch,  test_str[i]);
				return 0;
			}
			printf("i=%d, s=%x, r=%x\n",i,test_str[i], ch );
		}
	}
	printf("***** test ok ******\n");
	return 1;
}

int bmp_gz_display(const char * path)
{
	char *devif, *bootdisk, *bootpart;
	char *splashimage;
	u32 splashimage_addr;
	loff_t len_read;
	int ret;
	char dev_part_str[16];

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	bootpart = getenv("bootpart");
	if (!devif || !bootdisk || !bootpart)
		return -ENOENT;

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, bootpart);
	splashimage = getenv("splashimage");
	if (splashimage == NULL)
		return -ENOENT;

	splashimage_addr = simple_strtoul(splashimage, 0, 16);
	if (splashimage_addr == 0) {
		printf("==Error: bad splashimage address specified==\n");
		return -EFAULT;
	}

	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_FAT)) {
		printf("%s: fs_set_blk_dev (%s %s) ERROR\n",
		       __func__, devif, dev_part_str);
		return -1;
	}
	video_clear();
	ret = fs_read(path, splashimage_addr, 0, 0, &len_read);
	if (ret < 0) {
		printf("%s: fs_read ERROR\n", __func__);
		return -1;
	}
	#define BMP_ALIGN_CENTER	0x7FFF
	video_display_bitmap((ulong)splashimage_addr, BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);
	return 0;
}
/*解决DDR 低频闪屏问题*/
void qcb_ddr_set(void)
{
	writel(0xff44, 0xE0290074);
	writel(0x8, 0xE0290078);
	writel(0x1, 0xE029007c);
	writel(0x00100010, 0xE029008c);
	clrsetbits_le32(0xE0290000, 1 << 2, 0); //0xE0290000的BIT2写0然后写1，上面寄存器设置才生效
	mdelay(1);
	clrsetbits_le32(0xE0290000, 1 << 2, 1 << 2);
}
void qcb_test(void)
{
	int id;

	id = get_bond_opt();
	switch(id)
	{
		case ID_V700:
			bmp_gz_display("v700_lvds.bmp.gz");
			break;
		default:
			bmp_gz_display("noid.bmp.gz");
			break;
	}
#if 0
	printf("show uart logo\n");
	mdelay(2000);

	if ( check_comm_serial(3,3) )
		bmp_gz_display("uart-ok.bmp.gz");
	else
		bmp_gz_display("uart-err.bmp.gz");
#endif
}

int board_init(void)
{
	int ret;

	owl_board_first_init();
	qcb_ddr_set();
	owl_pwm_init(gd->fdt_blob);

	ret = owl_power_init();
	if (ret) {
		printf("%s power init failed!\n", __func__);
		return ret;
	}

#ifdef CONFIG_OWL_DMA
	owl_dma_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	//int ret;

	qcb_test();

	//ret = owl_board_late_init();
	//if (ret)
		//return ret;

	return 0;
}
#endif
