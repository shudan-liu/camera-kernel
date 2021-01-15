# SPDX-License-Identifier: GPL-2.0-only

# auto-detect subdirs
ifeq ($(CONFIG_ARCH_KONA), y)
include $(srctree)/techpack/camera/config/konacamera.conf
endif

ifeq ($(CONFIG_ARCH_LITO), y)
include $(srctree)/techpack/camera/config/litocamera.conf
endif

ifeq ($(CONFIG_ARCH_BENGAL), y)
include $(srctree)/techpack/camera/config/bengalcamera.conf
endif

ifeq ($(CONFIG_ARCH_LAHAINA), y)
include $(srctree)/techpack/camera/config/lahainacamera.conf
endif

ifeq ($(CONFIG_ARCH_KONA), y)
LINUXINCLUDE    += \
		-include $(srctree)/techpack/camera/config/konacameraconf.h
endif

ifeq ($(CONFIG_ARCH_LITO), y)
LINUXINCLUDE    += \
		-include $(srctree)/techpack/camera/config/litocameraconf.h
endif

ifeq ($(CONFIG_ARCH_BENGAL), y)
LINUXINCLUDE    += \
		-include $(srctree)/techpack/camera/config/bengalcameraconf.h
endif

ifeq ($(CONFIG_ARCH_LAHAINA), y)
LINUXINCLUDE    += \
		-include $(srctree)/techpack/camera/config/lahainacameraconf.h
endif


ifeq (y, $(findstring y, $(CONFIG_ARCH_SA8155) $(CONFIG_ARCH_SA6155) $(CONFIG_ARCH_SA8195)))
include $(srctree)/techpack/ais/config/hanacamera.conf
endif

ifeq (y, $(findstring y, $(CONFIG_ARCH_SA8155) $(CONFIG_ARCH_SA6155) $(CONFIG_ARCH_SA8195)))
LINUXINCLUDE += \
		-include $(srctree)/techpack/ais/config/hanacameraconf.h
endif

ifeq ($(CONFIG_MSM_AIS), y)
# Use USERINCLUDE when you must reference the UAPI directories only.
USERINCLUDE += \
                -I$(srctree)/techpack/ais/include/uapi

# Use LINUXINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
LINUXINCLUDE += \
                -I$(srctree)/techpack/ais/include/uapi \
                -I$(srctree)/techpack/ais/include \
                -I$(srctree) \
                -I$(srctree)/include
obj-y += drivers/

else
$(info Target not found)
endif


