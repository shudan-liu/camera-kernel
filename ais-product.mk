ifneq ($(TARGET_DISABLE_AIS_DLKM),true)
PRODUCT_PACKAGES += ais.ko
endif # TARGET_DISABLE_AIS_DLKM
