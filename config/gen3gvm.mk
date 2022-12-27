# Settings for compiling Gen3 GVM camera architecture

# Localized KCONFIG settings
CONFIG_MSM_AIS_UTIL := y
CONFIG_MSM_AIS_UTIL_COMMON := y
CONFIG_V4L2_LOOPBACK_V2 := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_V4L2_LOOPBACK_V2=1

