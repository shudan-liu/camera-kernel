# SPDX-License-Identifier: GPL-2.0-only
# Settings for compiling lito camera architecture

# Localized KCONFIG settings
CONFIG_SPECTRA_FD := y
CONFIG_SPECTRA_ISP := y
CONFIG_SPECTRA_ICP := y
CONFIG_SPECTRA_JPEG := y
CONFIG_SPECTRA_SENSOR := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_SPECTRA_FD=1
ccflags-y += -DCONFIG_SPECTRA_ISP=1
ccflags-y += -DCONFIG_SPECTRA_ICP=1
ccflags-y += -DCONFIG_SPECTRA_JPEG=1
ccflags-y += -DCONFIG_SPECTRA_SENSOR=1
ifneq ($(TARGET_BOARD_PLATFORM), anorak61)
ccflags-y += -DCONFIG_SECURE_CAMERA_V3
endif
ccflags-y += -DCONFIG_KTHREAD_WORKER
