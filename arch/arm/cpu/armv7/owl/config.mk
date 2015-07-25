#
# (C) Copyright 2010,2011
# NVIDIA Corporation <www.nvidia.com>
#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#
CONFIG_ARCH_DEVICE_TREE := owl

PLATFORM_OWLXX_COMMON_CFLAGS += -I$(TOPDIR)/../../include

PLATFORM_OWLXX_CFLAGS += $(PLATFORM_OWLXX_COMMON_CFLAGS)

ifeq ($(CONFIG_OWLXX_NAND),y)
PLATFORM_LIBS += $(TOPDIR)/../common/lib/libnand_5203.a
PLATFORM_LIBS += $(TOPDIR)/../common/lib/libmiscinfos.a
endif
PLATFORM_LIBS += $(TOPDIR)/../common/lib/libdvfs.a

ifneq ($(shell if [ -f $(TOPDIR)/arch/arm/cpu/armv7/owl/ddr.c ]; then echo "y"; else echo "n"; fi;), y)
PREBUILT_LIBS += $(TOPDIR)/../common/lib/libddr.a
endif
ifneq ($(shell if [ -f $(TOPDIR)/arch/arm/cpu/armv7/owl/clocks.c ]; then echo "y"; else echo "n"; fi;), y)
PREBUILT_LIBS += $(TOPDIR)/../common/lib/libclocks.a
endif


ifeq ($(SOC),owl)
ifneq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_OF_SEPARATE),y)
ALL-y += $(obj)u-boot-dtb-owl.img
else
ALL-y += $(obj)u-boot-nodtb-owl.img
endif
endif
endif
