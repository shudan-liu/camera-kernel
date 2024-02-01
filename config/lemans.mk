# Settings for compiling Lemans camera architecture

# Localized KCONFIG settings
CONFIG_SPECTRA_ISP := y
CONFIG_SPECTRA_SENSOR := y
CONFIG_SPECTRA_CORE := y
CONFIG_V4L2_LOOPBACK := y
CONFIG_CAM_UTIL_COMMON := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_SPECTRA_ISP=1
ccflags-y += -DCONFIG_SPECTRA_SENSOR=1
ccflags-y += -DCONFIG_SPECTRA_CORE=1
ccflags-y += -DCONFIG_CAM_UTIL_COMMON=1
ccflags-y += -DCONFIG_V4L2_LOOPBACK=1

# External Dependencies
#KBUILD_CPPFLAGS += -DCONFIG_MSM_MMRM=1
ifeq ($(CONFIG_QCOM_VA_MINIDUMP), y)
KBUILD_CPPFLAGS += -DCONFIG_QCOM_VA_MINIDUMP=1
endif
