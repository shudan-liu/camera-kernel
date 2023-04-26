ifneq ($(TARGET_DISABLE_AIS_DLKM),true)
# Build camera kernel driver
ifeq ($(TARGET_BOARD_AUTO),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/ais.ko
endif
endif # TARGET_DISABLE_AIS_DLKM
