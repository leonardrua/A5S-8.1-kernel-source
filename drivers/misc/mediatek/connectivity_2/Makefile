#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

# Connectivity combo driver
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    subdir-ccflags-y += -I$(srctree)/
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/clkbuf_v1
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/clkbuf_v1/$(MTK_PLATFORM)
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
    subdir-ccflags-y += -I$(srctree)/drivers/mmc/core
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/$(MTK_PLATFORM)
    subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/

    # Do Nothing, move to standalone repo
    MODULE_NAME := connadp
    obj-y += $(MODULE_NAME).o
    $(MODULE_NAME)-objs += common/connectivity_build_in_adapter.o
    $(MODULE_NAME)-objs += common/wmt_build_in_adapter.o


    # CONFIG_WLAN_DRV_BUILD_IN comes from Kconfig; the sub-Makefiles decide
    # obj-y (built-in) versus obj-m (module). Use the sources in this tree,
    # not the old standalone vendor repo.
    export CONFIG_MTK_COMBO_WIFI_HIF=axi
    export MTK_COMBO_CHIP=CONNAC
    export WLAN_CHIP_ID=6765
    export MTK_ANDROID_WMT=y

    ifeq ($(CONFIG_WLAN_DRV_BUILD_IN),y)
        subdir-ccflags-y += -DMTK_WCN_BUILT_IN_DRIVER
        # Suppress module-style init/exit stubs in the built-in drivers.
        # CFG_BUILT_IN_DRIVER in wlan_drv_gen4m is now keyed directly off
        # CONFIG_WLAN_DRV_BUILD_IN.
        subdir-ccflags-y += -DMTK_WCN_REMOVE_KERNEL_MODULE
    endif

    obj-y += wmt_drv/
    obj-y += wmt_chrdev_wifi/
    obj-y += wlan_drv_gen4m/
    obj-y += fmradio/
    obj-y += bt/
    obj-y += gps_drv/

# Otherwise we were called directly from the command line;
# invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
