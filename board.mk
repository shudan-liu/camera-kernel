# Build camera kernel driver
ifeq ($(TARGET_BOARD_AUTO),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/camera.ko
endif
