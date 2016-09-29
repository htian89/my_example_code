# Makefile for the USB/IP driver
#
# $Id: Makefile 66 2008-04-20 13:19:42Z hirofuchi $
#
# Copyright (C) 2003-2008 Takahiro Hirofuchi
#
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
# USA.
#

 
# Please modify here or set environments.
# KSOURCE should be pointed to the build directory of your kernel.
# 
DEBUG ?= n
#KSOURCE ?= /usr/src/kernels/3.13.5-202.fc20.i686
KSOURCE ?= /usr/src/kernels/3.17.3-200.fc20.x86_64
#KSOURCE ?= /home/ok6410/android2.3_kernel_v1.01
#KSOURCE ?= /usr/src/kernels/linux-2.6.38



%.x:%.c
	gcc -o $@ $<

KBUILD_VERBOSE:=1

HCD_HEADER:=$(KSOURCE)/include/linux/usb/hcd.h

usbip-core-objs := usbip_common.o usbip_event.o
usbip-objs 	  := stub_dev.o stub_main.o stub_rx.o stub_tx.o

obj-m += usbip-core.o
obj-m += usbip.o

EXTRA_CFLAGS += -DHCD_HEADER=\"$(HCD_HEADER)\"


ifeq ($(DEBUG),y)
#	EXTRA_CFLAGS += -DCONFIG_USB_DEBUG 
	EXTRA_CFLAGS += -DCONFIG_USBIP_DEBUG
	EXTRA_CFLAGS += -DDEBUG
#	EXTRA_CFLAGS += -DFDEBUG
endif


default:
	make -C $(KSOURCE) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} M=`pwd` modules

check:
	#make -C $(KSOURCE) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules checkstack namespacecheck headers_check
	make -C $(KSOURCE) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules

.PHONY: cscope
cscope:
	cscope -b -k -R

.PHONY: clean
clean:
	make -C $(KSOURCE) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE}  M=`pwd` clean
	rm -f *.x *~
