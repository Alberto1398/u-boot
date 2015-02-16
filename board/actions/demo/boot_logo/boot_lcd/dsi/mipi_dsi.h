#ifndef __LEOPARD_MIPI_H__
#define __LEOPARD_MIPI_H__

enum DSI_CLK
{
	CLK_436M = 0x102,
	CLK_654M = 0x101,
	CLK_564M = 0x01,
	CLK_376M = 0x02
};

extern int set_dsiclk(void);

extern void send_short_packet(unsigned int data_type, unsigned int sp_data, unsigned int trans_mode);
extern void send_normal_long_packet(unsigned int data_type, unsigned int word_cnt, unsigned int * send_data, unsigned int trans_mode);

extern void send_cmd(void);
#endif
