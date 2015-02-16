obj-y += atc260x/
obj-y += afinfo_space.o
obj-y += key_scan.o
obj-y += reset.o

obj-y += dump_mem.o
obj-y += phy_mem.o

ifdef CONFIG_ACTS_FOR_BOOT
obj-y += brom_drv/
endif

ifdef CONFIG_SPL_BUILD
obj-y += tiny_printf/
obj-y += spl_board_init.o
obj-y += spl_console.o
obj-y += spl_gpio.o
obj-y += spl_misc.o
else
obj-y += xml_parser/
obj-y += boot_logo/
obj-y += board_init.o
obj-y += gpio.o
obj-y += boot_act_krnl.o
endif

