ifneq ($(TARGET_IS_HEADLESS),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_CAMERA),true)
# Make target to specify building the camera.ko from within Android build system.
LOCAL_PATH := $(call my-dir)
# Path to DLKM make scripts
DLKM_DIR := $(TOP)/device/qcom/common/dlkm

# Kbuild options
KBUILD_OPTIONS := CAMERA_KERNEL_ROOT=$(shell pwd)/$(LOCAL_PATH)
KBUILD_OPTIONS += KERNEL_ROOT=$(shell pwd)/kernel/msm-$(TARGET_KERNEL_VERSION)/
KBUILD_OPTIONS += MODNAME=ais
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)


# Clear shell environment variables from previous android module during build
include $(CLEAR_VARS)
# For incremental compilation support.
LOCAL_SRC_FILES             :=  \
                                $(shell find $(LOCAL_PATH)/config -L -type f)      \
                                $(shell find $(LOCAL_PATH)/drivers -L -type f)     \
                                $(shell find $(LOCAL_PATH)/include -L -type f)     \
                                $(LOCAL_PATH)/Android.mk \
                                $(LOCAL_PATH)/ais-board.mk   \
                                $(LOCAL_PATH)/ais-product.mk \
                                $(LOCAL_PATH)/Kbuild
LOCAL_MODULE_PATH           := $(KERNEL_MODULES_OUT)
LOCAL_MODULE                := ais.ko
LOCAL_MODULE_TAGS           := optional

include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif #TARGET_USES_QMAA_OVERRIDE_CAMERA
endif #TARGET_IS_HEADLESS
