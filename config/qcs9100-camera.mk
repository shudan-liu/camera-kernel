# SPDX-License-Identifier: GPL-2.0-only

# Localized KCONFIG settings
CONFIG_SPECTRA_ISP := y
CONFIG_SPECTRA_ICP := y
# CONFIG_SPECTRA_SENSOR := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_SPECTRA_ISP=1
ccflags-y += -DCONFIG_SPECTRA_ICP=1
# ccflags-y += -DCONFIG_SPECTRA_SENSOR=1

# External Dependencies
# KBUILD_CPPFLAGS += -DCONFIG_MSM_MMRM=1
ifeq ($(CONFIG_QCOM_VA_MINIDUMP), y)
KBUILD_CPPFLAGS += -DCONFIG_QCOM_VA_MINIDUMP=1
endif
