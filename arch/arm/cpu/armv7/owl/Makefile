#
# (C) Copyright 2000-2012
# Actions Semi .Inc
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

include $(TOPDIR)/config.mk

LIB	=  $(obj)lib$(SOC).o


COBJS	+= scan.o
ifeq ($(shell if [ -f $(TOPDIR)/arch/arm/cpu/armv7/owl/ddr.c ]; then echo "y"; else echo "n"; fi;), y)
COBJS	+= ddr.o
endif
ifeq ($(shell if [ -f $(TOPDIR)/arch/arm/cpu/armv7/owl/clocks.c ]; then echo "y"; else echo "n"; fi;), y)
COBJS	+= clocks.o
endif
COBJS	+= pmu.o
ifndef CONFIG_SPL_BUILD
COBJS	+= atc260x_pstore.o
endif
COBJS	+= soc.o
COBJS	+= timer.o
COBJS	+= gpio.o
SOBJS   += checksum_ddr.o
SOBJS-$(CONFIG_OWLXX_QUICKBOOT) += hibernate.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_OWLXX_PINCTRL) += pinctrl.o
ifdef CONFIG_ATM7059TC
COBJS-$(CONFIG_OWLXX_PINCTRL) += pinctrl_data-atm7059tc.o
endif
ifdef CONFIG_ATM7059A
COBJS-$(CONFIG_OWLXX_PINCTRL) += pinctrl_data-atm7059.o
endif
ifdef CONFIG_ATM7039C
COBJS-$(CONFIG_OWLXX_PINCTRL) += pinctrl_data-atm7039c.o
endif
else
COBJS-$(CONFIG_SPL_MTEST) += mtest.o
endif

COBJS-$(CONFIG_OWLXX_PWM)	+= pwm.o

COBJS-$(CONFIG_USE_IRQ)	+= irq.o

SRCS	:= $(SOBJS:.o=.S) $(SOBJS-y:.o=.S) $(COBJS:.o=.c) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS) $(SOBJS-y))
all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
