#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(src),)
$(error src not defined!)
endif

ifneq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_ACTS_FOR_BOOT),y)
ifeq ($(CONFIG_ACTS_STORAGE_NAND),y)
PLATFORM_LIBS += $(src)/../common/lib/libnand_5202.a
PLATFORM_LIBS += $(src)/../common/lib/libmiscinfos.a
endif
endif
endif

ifneq ($(CONFIG_SPL_BUILD),y)
PLATFORM_LIBS += $(src)/../common/lib/libdvfs_level.a
PLATFORM_LIBS += $(src)/../common/lib/libcmu.a
endif

