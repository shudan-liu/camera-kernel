ifeq ($(CONFIG_QCOM_CAMERA_DEBUG), y)
$(info "CAMERA_KERNEL_ROOT is: $(CAMERA_KERNEL_ROOT)")
$(info "KERNEL_ROOT is: $(KERNEL_ROOT)")
endif

# Include Architecture configurations
ifeq ($(CONFIG_ARCH_KALAMA), y)
include $(CAMERA_KERNEL_ROOT)/config/kalama.mk
endif

ifeq ($(CONFIG_ARCH_WAIPIO), y)
include $(CAMERA_KERNEL_ROOT)/config/waipio.mk
endif

ifeq ($(CONFIG_ARCH_LAHAINA), y)
include $(CAMERA_KERNEL_ROOT)/config/lahaina.mk
endif

ifeq ($(CONFIG_ARCH_KONA), y)
include $(CAMERA_KERNEL_ROOT)/config/kona.mk
endif

ifeq ($(CONFIG_ARCH_BENGAL), y)
include $(CAMERA_KERNEL_ROOT)/config/holi.mk
endif

ifeq ($(CONFIG_ARCH_HOLI), y)
include $(CAMERA_KERNEL_ROOT)/config/holi.mk
endif

ifeq ($(CONFIG_ARCH_LITO), y)
include $(CAMERA_KERNEL_ROOT)/config/lito.mk
endif

ifeq ($(CONFIG_ARCH_SHIMA), y)
include $(CAMERA_KERNEL_ROOT)/config/shima.mk
endif

ifeq ($(CONFIG_ARCH_DIWALI), y)
include $(CAMERA_KERNEL_ROOT)/config/diwali.mk
endif

ifeq ($(CONFIG_ARCH_CAPE), y)
include $(CAMERA_KERNEL_ROOT)/config/cape.mk
endif

ifeq ($(CONFIG_ARCH_PARROT), y)
include $(CAMERA_KERNEL_ROOT)/config/parrot.mk
endif

ifeq (y, $(findstring y, $(CONFIG_ARCH_SA8155) $(CONFIG_ARCH_SA6155) $(CONFIG_ARCH_SA8195)))
include $(CAMERA_KERNEL_ROOT)/config/gen3auto.mk
endif

# List of all camera-kernel headers
cam_include_dirs := $(shell dirname `find $(CAMERA_KERNEL_ROOT) -name '*.h'` | uniq)

# Include UAPI headers
USERINCLUDE +=                              \
	-I$(CAMERA_KERNEL_ROOT)/include/uapi/
# Include Kernel headers
LINUXINCLUDE +=                                 \
	-I$(KERNEL_ROOT)                            \
	$(addprefix -I,$(cam_include_dirs))         \
	-I$(CAMERA_KERNEL_ROOT)/include/uapi/ais \
	-I$(CAMERA_KERNEL_ROOT)/
# Optional include directories
ccflags-$(CONFIG_MSM_GLOBAL_SYNX) += -I$(KERNEL_ROOT)/drivers/media/platform/msm/synx

# After creating lists, add content of 'ccflags-m' variable to 'ccflags-y' one.
ccflags-y += ${ccflags-m}

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_req_mgr/cam_req_mgr_core.o \
	drivers/cam_req_mgr/cam_req_mgr_dev.o \
	drivers/cam_req_mgr/cam_req_mgr_util.o \
	drivers/cam_req_mgr/cam_req_mgr_workq.o \
	drivers/cam_req_mgr/cam_mem_mgr.o \
	drivers/cam_req_mgr/cam_req_mgr_timer.o \
	drivers/cam_req_mgr/cam_req_mgr_debug.o

ais-$(CONFIG_MSM_AIS_UTIL_METAL) += \
	drivers/cam_utils/cam_soc_util.o \
	drivers/cam_utils/cam_io_util.o \
	drivers/cam_utils/cam_packet_util.o \
	drivers/cam_utils/cam_trace.o \
	drivers/cam_utils/cam_compat.o

ais-$(CONFIG_MSM_AIS_UTIL_COMMON) += \
	drivers/cam_utils/cam_common_util.o \
	drivers/cam_utils/cam_debug_util.o

ifdef CONFIG_QCOM_CX_IPEAK
ais-$(CONFIG_MSM_AIS_UTIL_METAL) += drivers/cam_utils/cam_cx_ipeak.o
endif

ifneq (,$(filter $(CONFIG_QCOM_BUS_SCALING),y m))
ais-$(CONFIG_MSM_AIS_UTIL_METAL) += drivers/cam_utils/cam_soc_bus.o
endif

ifneq (,$(filter $(CONFIG_INTERCONNECT_QCOM),y m))
ais-$(CONFIG_MSM_AIS_UTIL_METAL) += drivers/cam_utils/cam_soc_icc.o
endif

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_core/cam_context.o \
	drivers/cam_core/cam_context_utils.o \
	drivers/cam_core/cam_node.o \
	drivers/cam_core/cam_subdev.o

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_sync/cam_sync.o \
	drivers/cam_sync/cam_sync_util.o

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_smmu/cam_smmu_api.o

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_cpas/cpas_top/cam_cpastop_hw.o \
	drivers/cam_cpas/camss_top/cam_camsstop_hw.o \
	drivers/cam_cpas/cam_cpas_soc.o \
	drivers/cam_cpas/cam_cpas_intf.o \
	drivers/cam_cpas/cam_cpas_hw.o

ais-$(CONFIG_MSM_AIS) += \
	drivers/cam_cdm/cam_cdm_soc.o \
	drivers/cam_cdm/cam_cdm_util.o \
	drivers/cam_cdm/cam_cdm_intf.o \
	drivers/cam_cdm/cam_cdm_core_common.o \
	drivers/cam_cdm/cam_cdm_virtual_core.o \
	drivers/cam_cdm/cam_cdm_hw_core.o

ais-$(CONFIG_MSM_AIS) += \
	drivers/ais_isp/csid_hw/ais_ife_csid_dev.o \
	drivers/ais_isp/csid_hw/ais_ife_csid_soc.o \
	drivers/ais_isp/csid_hw/ais_ife_csid_core.o \
	drivers/ais_isp/csid_hw/ais_ife_csid17x.o \
	drivers/ais_isp/csid_hw/ais_ife_csid_lite17x.o \
	drivers/ais_isp/vfe_hw/ais_vfe_soc.o \
	drivers/ais_isp/vfe_hw/ais_vfe_dev.o \
	drivers/ais_isp/vfe_hw/ais_vfe_core.o \
	drivers/ais_isp/vfe_hw/vfe17x/ais_vfe17x.o \
	drivers/ais_isp/utils/ais_isp_trace.o \
	drivers/ais_isp/ais_ife_dev.o

ais-$(CONFIG_SPECTRA_ISP) += \
	drivers/cam_isp/isp_hw_mgr/hw_utils/cam_tasklet_util.o \
	drivers/cam_isp/isp_hw_mgr/hw_utils/cam_isp_packet_parser.o \
	drivers/cam_isp/isp_hw_mgr/hw_utils/irq_controller/cam_irq_controller.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_csid_ppi_dev.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_csid_ppi_core.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_csid_ppi170.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_dev.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_soc.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_core.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid17x.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_lite17x.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_soc.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_dev.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_core.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_ver2.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_rd_ver1.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_lite_ver2.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver2.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_ver2.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_rdi.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_fe_ver1.o \
	drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe17x/cam_vfe17x.o \
	drivers/cam_isp/isp_hw_mgr/cam_isp_hw_mgr.o \
	drivers/cam_isp/isp_hw_mgr/cam_ife_hw_mgr.o \
	drivers/cam_isp/cam_isp_dev.o \
	drivers/cam_isp/cam_isp_context.o

ais-$(CONFIG_SPECTRA_SENSOR) += \
	drivers/cam_sensor_module/cam_res_mgr/cam_res_mgr.o \
	drivers/cam_sensor_module/cam_sensor_utils/cam_sensor_util.o \
	drivers/cam_sensor_module/cam_cci/cam_cci_dev.o \
	drivers/cam_sensor_module/cam_cci/cam_cci_core.o \
	drivers/cam_sensor_module/cam_cci/cam_cci_soc.o \
	drivers/cam_sensor_module/cam_sensor_io/cam_sensor_io.o \
	drivers/cam_sensor_module/cam_sensor_io/cam_sensor_cci_i2c.o \
	drivers/cam_sensor_module/cam_sensor_io/cam_sensor_qup_i2c.o \
	drivers/cam_sensor_module/cam_sensor_io/cam_sensor_spi.o \
	drivers/cam_sensor_module/cam_csiphy/cam_csiphy_soc.o \
	drivers/cam_sensor_module/cam_csiphy/cam_csiphy_dev.o \
	drivers/cam_sensor_module/cam_csiphy/cam_csiphy_core.o \
	drivers/cam_sensor_module/cam_sensor/cam_sensor_dev.o \
	drivers/cam_sensor_module/cam_sensor/cam_sensor_core.o \
	drivers/cam_sensor_module/cam_sensor/cam_sensor_soc.o

ais-$(CONFIG_MSM_AIS_ACTUATOR) += \
	drivers/cam_sensor_module/cam_actuator/cam_actuator_dev.o \
	drivers/cam_sensor_module/cam_actuator/cam_actuator_core.o \
	drivers/cam_sensor_module/cam_actuator/cam_actuator_soc.o

ais-$(CONFIG_MSM_AIS_FLASH) += \
	drivers/cam_sensor_module/cam_flash/cam_flash_dev.o \
	drivers/cam_sensor_module/cam_flash/cam_flash_core.o \
	drivers/cam_sensor_module/cam_flash/cam_flash_soc.o

ais-$(CONFIG_MSM_AIS_EEPROM) += \
	drivers/cam_sensor_module/cam_eeprom/cam_eeprom_dev.o \
	drivers/cam_sensor_module/cam_eeprom/cam_eeprom_core.o \
	drivers/cam_sensor_module/cam_eeprom/cam_eeprom_soc.o

ais-$(CONFIG_MSM_AIS_OIS) += \
	drivers/cam_sensor_module/cam_ois/cam_ois_dev.o \
	drivers/cam_sensor_module/cam_ois/cam_ois_core.o \
	drivers/cam_sensor_module/cam_ois/cam_ois_soc.o

ais-$(CONFIG_MSM_AIS_IRLED) += \
	drivers/cam_sensor_module/cam_ir_led/cam_ir_led_dev.o \
	drivers/cam_sensor_module/cam_ir_led/cam_ir_led_soc.o \
	drivers/cam_sensor_module/cam_ir_led/cam_ir_led_core.o \

ais-$(CONFIG_SPECTRA_ICP) += \
	drivers/cam_icp/icp_hw/icp_hw_mgr/cam_icp_hw_mgr.o \
	drivers/cam_icp/icp_hw/a5_hw/a5_dev.o \
	drivers/cam_icp/icp_hw/a5_hw/a5_core.o \
	drivers/cam_icp/icp_hw/a5_hw/a5_soc.o \
	drivers/cam_icp/icp_hw/ipe_hw/ipe_dev.o \
	drivers/cam_icp/icp_hw/ipe_hw/ipe_core.o \
	drivers/cam_icp/icp_hw/ipe_hw/ipe_soc.o \
	drivers/cam_icp/icp_hw/bps_hw/bps_dev.o \
	drivers/cam_icp/icp_hw/bps_hw/bps_core.o \
	drivers/cam_icp/icp_hw/bps_hw/bps_soc.o \
	drivers/cam_icp/cam_icp_subdev.o \
	drivers/cam_icp/cam_icp_context.o \
	drivers/cam_icp/hfi.o

ais-$(CONFIG_SPECTRA_JPEG) += \
	drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_dev.o \
	drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_core.o \
	drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_soc.o \
	drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_dev.o \
	drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_core.o \
	drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_soc.o \
	drivers/cam_jpeg/jpeg_hw/cam_jpeg_hw_mgr.o \
	drivers/cam_jpeg/cam_jpeg_dev.o \
	drivers/cam_jpeg/cam_jpeg_context.o

ais-$(CONFIG_SPECTRA_FD) += \
	drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_dev.o \
	drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_core.o \
	drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_soc.o \
	drivers/cam_fd/fd_hw_mgr/cam_fd_hw_mgr.o \
	drivers/cam_fd/cam_fd_dev.o \
	drivers/cam_fd/cam_fd_context.o \

ais-$(CONFIG_SPECTRA_LRME) += \
	drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_dev.o \
	drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_core.o \
	drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_soc.o \
	drivers/cam_lrme/lrme_hw_mgr/cam_lrme_hw_mgr.o \
	drivers/cam_lrme/cam_lrme_dev.o \
	drivers/cam_lrme/cam_lrme_context.o \

ais-$(CONFIG_V4L2_LOOPBACK_V2) += \
	drivers/v4l2loopback-master_v2/v4l2loopback.o

ais-$(CONFIG_MSM_AIS) += drivers/ais_main.o

obj-m += ais.o
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/ais.ko
