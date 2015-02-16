/*
 *  linux/include/asm/setup.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Structure passed to kernel to tell it about the
 *  hardware it's running on.  See linux/Documentation/arm/Setup
 *  for more info.
 *
 * NOTE:
 *  This file contains two ways to pass information from the boot
 *  loader to the kernel. The old struct param_struct is deprecated,
 *  but it will be kept in the kernel for 5 years from now
 *  (2001). This will allow boot loaders to convert to the new struct
 *  tag way.
 */
#ifndef __ASMARM_SETUP_ACTIONS_H
#define __ASMARM_SETUP_ACTIONS_H

#define ATAG_XML	0x41000501

struct tag_xml {
	unsigned int xml_buf_start;
	unsigned int xml_buf_len;
	unsigned int bin_cfg_xml_buf_start;	
	unsigned int bin_cfg_xml_buf_len;		
};

#define ATAG_PINCTRL	0x41000502
struct tag_pinctrl {
	unsigned int pinctrl_buf_start;
	unsigned int pinctrl_buf_len;	
};

#define ATAG_DVFSLEVEL     0x41000503
struct tag_dvfslevel {
	unsigned int dvfslevel;
	unsigned int icversion;
};

#define ATAG_BOARD_OPTION     0x41000504
struct tag_board_opt {
	unsigned int board_opt;
	unsigned int board_opt_flags;
};

#define ATAG_PMEM_INFO     0x41000505
struct tag_pmem_info {
	unsigned int ddr_size;
	unsigned int fb_size;
	unsigned int gpu_size;
	unsigned int ion_size;
};

#define ATAG_BOOT_DEV     0x41000506
struct tag_boot_dev {
	unsigned int boot_dev;
};

#define ATAG_BOOT_AFINFO    0x41000507
struct tag_boot_afinfo {
	unsigned int afinfo_buf_start;
	unsigned int afinfo_buf_len;	
};


#endif /* __ASMARM_SETUP_ACTIONS_H */
