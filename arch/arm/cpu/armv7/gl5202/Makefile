
# Common
#obj-y += cmu.o
#obj-y += dvfs_level.o
obj-y += l2c310_misc.o
obj-y += timer.o
obj-y += chksum.o

ifdef CONFIG_SPL_BUILD
obj-y += ddr3_init/
obj-y += spl_soc_init.o
obj-y += spl_vectors.o
obj-y += spl_irq.o
else
obj-y += soc_init.o
obj-y += irq.o
endif


