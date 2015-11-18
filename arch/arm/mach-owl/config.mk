ifdef CONFIG_OWL_NAND
ifeq ($(CONFIG_S900),y)
PLATFORM_LIBS += $(srctree)/arch/arm/mach-owl/s900/nandlib/libnand_s900.lib
endif
endif
