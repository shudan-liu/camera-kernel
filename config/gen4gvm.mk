# Settings for compiling Gen3 GVM camera architecture

# Localized KCONFIG settings
CONFIG_MSM_QCX_UTIL_COMMON := y
CONFIG_V4L2_LOOPBACK := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_V4L2_LOOPBACK=1

