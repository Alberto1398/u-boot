obj-y += brom_api_a.o
obj-y += adfu.o

ifdef CONFIG_ACTS_STORAGE_NAND
obj-y += nand.o
else
ifdef CONFIG_ACTS_STORAGE_EMMC
obj-y += mmc.o
endif
endif
