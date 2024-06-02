# SPDX-License-Identifier: GPL-2.0-only

$(info "CAMERA_KERNEL_ROOT is: $(CAMERA_KERNEL_ROOT)")
$(info "KERNEL_ROOT is: $(KERNEL_ROOT)")
$(info "MACHINE is: $(MACHINE)")
$(info "CAMERA_ARCH is: $(CAMERA_ARCH)")

# Include Architecture configurations
ifeq ($(CAMERA_ARCH), qcm6490)
include $(CAMERA_KERNEL_ROOT)/config/qcm6490-camera.mk
else
include $(CAMERA_KERNEL_ROOT)/config/qcs8550-camera.mk
endif
ifneq ($(KBUILD_EXTRA_CONFIGS),)
include $(KBUILD_EXTRA_CONFIGS)
endif

# List of all camera-kernel headers

ifeq ($(CAMERA_ARCH), qcm6490)
cam_include_dirs := $(shell dirname `find $(CAMERA_KERNEL_ROOT)/camera_kt -name '*.h'` | uniq)
cam_include_dirs += $(shell dirname `find $(CAMERA_KERNEL_ROOT)/common -name '*.h'` | uniq)
else
cam_include_dirs := $(shell dirname `find $(CAMERA_KERNEL_ROOT)/camera -name '*.h'` | uniq)
cam_include_dirs += $(shell dirname `find $(CAMERA_KERNEL_ROOT)/common -name '*.h'` | uniq)
endif


ifeq ($(CAMERA_ARCH), qcm6490)
# Include UAPI headers
USERINCLUDE +=                              \
	-I$(CAMERA_KERNEL_ROOT)/camera_kt/include/uapi/
# Include Kernel headers
LINUXINCLUDE +=                                                  \
	-I$(KERNEL_ROOT)                                         \
	$(addprefix -I,$(cam_include_dirs))                      \
	-I$(CAMERA_KERNEL_ROOT)/camera_kt/include/uapi/camera/media \
	-I$(CAMERA_KERNEL_ROOT)/camera_kt/include/uapi/camera       \
	-I$(CAMERA_KERNEL_ROOT)/camera_kt/include/uapi              \
	-I$(CAMERA_KERNEL_ROOT)/camera_kt                           \
	-I$(CAMERA_KERNEL_ROOT)/

# After creating lists, add content of 'ccflags-m' variable to 'ccflags-y' one.
ccflags-y += ${ccflags-m}

camera-y := \
	camera_kt/drivers/cam_req_mgr/cam_req_mgr_core.o \
	camera_kt/drivers/cam_req_mgr/cam_req_mgr_dev.o \
	camera_kt/drivers/cam_req_mgr/cam_req_mgr_debug.o \
	camera_kt/drivers/cam_utils/cam_soc_util.o \
	camera_kt/drivers/cam_utils/cam_packet_util.o \
	camera_kt/drivers/cam_utils/cam_trace.o \
	camera_kt/drivers/cam_core/cam_context.o \
	camera_kt/drivers/cam_core/cam_context_utils.o \
	camera_kt/drivers/cam_core/cam_node.o \
	camera_kt/drivers/cam_core/cam_subdev.o \
	camera_kt/drivers/cam_sync/cam_sync.o \
	camera_kt/drivers/cam_sync/cam_sync_util.o \
	camera_kt/drivers/cam_cpas/cpas_top/cam_cpastop_hw.o \
	camera_kt/drivers/cam_cpas/camss_top/cam_camsstop_hw.o \
	camera_kt/drivers/cam_cpas/cam_cpas_soc.o \
	camera_kt/drivers/cam_cpas/cam_cpas_intf.o \
	camera_kt/drivers/cam_cpas/cam_cpas_hw.o \
	camera_kt/drivers/cam_cdm/cam_cdm_soc.o \
	camera_kt/drivers/cam_cdm/cam_cdm_util.o \
	camera_kt/drivers/cam_cdm/cam_cdm_intf.o \
	camera_kt/drivers/cam_cdm/cam_cdm_core_common.o \
	camera_kt/drivers/cam_cdm/cam_cdm_virtual_core.o \
	camera_kt/drivers/cam_cdm/cam_cdm_hw_core.o \
	camera_kt/drivers/cam_req_mgr/cam_mem_mgr.o \
	camera_kt/drivers/cam_smmu/cam_smmu_api.o \
	common/cam_common_util.o \
	common/cam_compat.o \
	common/cam_debug_util.o \
	common/cam_io_util.o \
	common/cam_req_mgr_timer.o \
	common/cam_req_mgr_util.o \
	common/cam_req_mgr_workq.o \
	common/cam_soc_icc.o

camera-$(CONFIG_QCOM_CX_IPEAK) += camera_kt/drivers/cam_utils/cam_cx_ipeak.o
camera-$(CONFIG_QCOM_BUS_SCALING) += camera_kt/drivers/cam_utils/cam_soc_bus.o

camera-$(CONFIG_SPECTRA_ISP) += \
	camera_kt/drivers/cam_isp/isp_hw_mgr/hw_utils/cam_tasklet_util.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/hw_utils/cam_isp_packet_parser.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/hw_utils/irq_controller/cam_irq_controller.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid17x.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_lite17x.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_top/cam_sfe_top.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus_rd.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus_wr.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_ver2.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_rd_ver1.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_ver3.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_lite_ver2.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_common.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver3.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver2.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_ver2.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_ver3.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_rdi.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_fe_ver1.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_lite_ver3.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe17x/cam_vfe.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/cam_isp_hw_mgr.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/cam_ife_hw_mgr.o \
	camera_kt/drivers/cam_isp/cam_isp_dev.o \
	camera_kt/drivers/cam_isp/cam_isp_context.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_ver2.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_ver3.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/top_tpg/cam_top_tpg_ver1.o

camera-$(CONFIG_SPECTRA_ICP) += \
	camera_kt/drivers/cam_icp/icp_hw/icp_hw_mgr/cam_icp_hw_mgr.o \
	camera_kt/drivers/cam_icp/icp_hw/ipe_hw/ipe_dev.o \
	camera_kt/drivers/cam_icp/icp_hw/ipe_hw/ipe_core.o \
	camera_kt/drivers/cam_icp/icp_hw/ipe_hw/ipe_soc.o \
	camera_kt/drivers/cam_icp/icp_hw/a5_hw/a5_dev.o \
	camera_kt/drivers/cam_icp/icp_hw/a5_hw/a5_core.o \
	camera_kt/drivers/cam_icp/icp_hw/a5_hw/a5_soc.o \
	camera_kt/drivers/cam_icp/icp_hw/bps_hw/bps_dev.o \
	camera_kt/drivers/cam_icp/icp_hw/bps_hw/bps_core.o \
	camera_kt/drivers/cam_icp/icp_hw/bps_hw/bps_soc.o \
	camera_kt/drivers/cam_icp/cam_icp_subdev.o \
	camera_kt/drivers/cam_icp/cam_icp_context.o \
	camera_kt/drivers/cam_icp/hfi.o

camera-$(CONFIG_SPECTRA_JPEG) += \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_dev.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_core.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_soc.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_dev.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_core.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_soc.o \
	camera_kt/drivers/cam_jpeg/jpeg_hw/cam_jpeg_hw_mgr.o \
	camera_kt/drivers/cam_jpeg/cam_jpeg_dev.o \
	camera_kt/drivers/cam_jpeg/cam_jpeg_context.o

camera-$(CONFIG_SPECTRA_FD) += \
	camera_kt/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_dev.o \
	camera_kt/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_core.o \
	camera_kt/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_soc.o \
	camera_kt/drivers/cam_fd/fd_hw_mgr/cam_fd_hw_mgr.o \
	camera_kt/drivers/cam_fd/cam_fd_dev.o \
	camera_kt/drivers/cam_fd/cam_fd_context.o

camera-$(CONFIG_SPECTRA_LRME) += \
	camera_kt/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_dev.o \
	camera_kt/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_core.o \
	camera_kt/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_soc.o \
	camera_kt/drivers/cam_lrme/lrme_hw_mgr/cam_lrme_hw_mgr.o \
	camera_kt/drivers/cam_lrme/cam_lrme_dev.o \
	camera_kt/drivers/cam_lrme/cam_lrme_context.o

camera-$(CONFIG_SPECTRA_SENSOR) += \
	camera_kt/drivers/cam_sensor_module/cam_actuator/cam_actuator_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_actuator/cam_actuator_core.o \
	camera_kt/drivers/cam_sensor_module/cam_actuator/cam_actuator_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_cci/cam_cci_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_cci/cam_cci_core.o \
	camera_kt/drivers/cam_sensor_module/cam_cci/cam_cci_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_core.o \
	camera_kt/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_core.o  \
	camera_kt/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_ois/cam_ois_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_ois/cam_ois_core.o \
	camera_kt/drivers/cam_sensor_module/cam_ois/cam_ois_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor/cam_sensor_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor/cam_sensor_core.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor/cam_sensor_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_io.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_cci_i2c.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_qup_i2c.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_spi.o \
	camera_kt/drivers/cam_sensor_module/cam_sensor_utils/cam_sensor_util.o \
	camera_kt/drivers/cam_sensor_module/cam_res_mgr/cam_res_mgr.o \
	camera_kt/drivers/cam_sensor_module/cam_flash/cam_flash_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_flash/cam_flash_core.o \
	camera_kt/drivers/cam_sensor_module/cam_flash/cam_flash_soc.o \
	camera_kt/drivers/cam_sensor_module/cam_dp_bdg/cam_dp_bdg_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_dp_bdg/cam_dp_bdg_core.o \
	camera_kt/drivers/cam_sensor_module/cam_hdmi_bdg/cam_hdmi_bdg_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_hdmi_bdg/cam_hdmi_bdg_core.o \
	camera_kt/drivers/cam_sensor_module/cam_ir_led/cam_ir_led_dev.o \
	camera_kt/drivers/cam_sensor_module/cam_ir_led/cam_ir_led_core.o \
	camera_kt/drivers/cam_sensor_module/cam_ir_led/cam_ir_led_soc.o

camera-$(CONFIG_SPECTRA_CUSTOM) += \
	camera_kt/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_soc.o \
	camera_kt/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_dev.o \
	camera_kt/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_core.o \
	camera_kt/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_csid/cam_custom_csid_dev.o \
	camera_kt/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw_mgr.o \
	camera_kt/drivers/cam_cust/cam_custom_dev.o \
	camera_kt/drivers/cam_cust/cam_custom_context.o

camera-$(CONFIG_SPECTRA_OPE) += \
	camera_kt/drivers/cam_ope/cam_ope_subdev.o \
	camera_kt/drivers/cam_ope/cam_ope_context.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/cam_ope_hw_mgr.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_dev.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_soc.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_core.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/top/ope_top.o \
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/bus_rd/ope_bus_rd.o\
	camera_kt/drivers/cam_ope/ope_hw_mgr/ope_hw/bus_wr/ope_bus_wr.o

camera-$(CONFIG_SPECTRA_TFE) += \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi100.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_bus.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_dev.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_soc.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_core.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid530.o \
	camera_kt/drivers/cam_isp/isp_hw_mgr/cam_tfe_hw_mgr.o

camera-y += camera_kt/drivers/camera_main.o

obj-m += camera.o
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/camera.ko
else
# Include UAPI headers
USERINCLUDE +=                              \
	-I$(CAMERA_KERNEL_ROOT)/camera/include/uapi/
# Include Kernel headers
LINUXINCLUDE +=                                                       \
	-I$(KERNEL_ROOT)                                              \
	$(addprefix -I,$(cam_include_dirs))                           \
	-I$(CAMERA_KERNEL_ROOT)/camera/include/uapi/camera/media \
	-I$(CAMERA_KERNEL_ROOT)/camera/include/uapi/camera       \
	-I$(CAMERA_KERNEL_ROOT)/camera/include/uapi              \
	-I$(CAMERA_KERNEL_ROOT)/camera                           \
	-I$(CAMERA_KERNEL_ROOT)/

# Optional include directories
# SYNXVENDORDIR=$(CAMERA_KERNEL_ROOT)/../synx-kernel
# ccflags-$(CONFIG_MSM_GLOBAL_SYNX) += -I$(KERNEL_ROOT)/camera/drivers/media/platform/msm/synx
# ccflags-$(TARGET_SYNX_ENABLE) += -I$(SYNXVENDORDIR)/include/uapi/synx/media
# ccflags-$(TARGET_SYNX_ENABLE) += -I$(SYNXVENDORDIR)/msm/synx
# ccflags-$(TARGET_SYNX_ENABLE) += -DCONFIG_TARGET_SYNX_ENABLE=1
# ccflags-y += -I$(CAMERA_KERNEL_ROOT)/../securemsm-kernel/
# ccflags-y += -I$(CAMERA_KERNEL_ROOT)/../securemsm-kernel/include/

# After creating lists, add content of 'ccflags-m' variable to 'ccflags-y' one.
ccflags-y += ${ccflags-m}
camera-y := \
	camera/drivers/cam_req_mgr/cam_req_mgr_core.o \
	camera/drivers/cam_req_mgr/cam_req_mgr_dev.o \
	camera/drivers/cam_req_mgr/cam_mem_mgr.o \
	camera/drivers/cam_req_mgr/cam_req_mgr_debug.o \
	camera/drivers/cam_utils/cam_soc_util.o \
	camera/drivers/cam_utils/cam_packet_util.o \
	camera/drivers/cam_utils/cam_trace.o \
	camera/drivers/cam_core/cam_context.o \
	camera/drivers/cam_core/cam_context_utils.o \
	camera/drivers/cam_core/cam_node.o \
	camera/drivers/cam_core/cam_subdev.o \
	camera/drivers/cam_sync/cam_sync.o \
	camera/drivers/cam_sync/cam_sync_util.o \
	camera/drivers/cam_sync/cam_sync_dma_fence.o \
	camera/drivers/cam_cpas/cpas_top/cam_cpastop_hw.o \
	camera/drivers/cam_cpas/camss_top/cam_camsstop_hw.o \
	camera/drivers/cam_cpas/cam_cpas_soc.o \
	camera/drivers/cam_cpas/cam_cpas_intf.o \
	camera/drivers/cam_cpas/cam_cpas_hw.o \
	camera/drivers/cam_cdm/cam_cdm_soc.o \
	camera/drivers/cam_cdm/cam_cdm_util.o \
	camera/drivers/cam_cdm/cam_cdm_intf.o \
	camera/drivers/cam_cdm/cam_cdm_core_common.o \
	camera/drivers/cam_cdm/cam_cdm_virtual_core.o \
	camera/drivers/cam_cdm/cam_cdm_hw_core.o \
	camera/drivers/cam_smmu/cam_smmu_api.o \
	common/cam_common_util.o \
	common/cam_compat.o \
	common/cam_debug_util.o \
	common/cam_io_util.o \
	common/cam_req_mgr_timer.o \
	common/cam_req_mgr_util.o \
	common/cam_req_mgr_workq.o \
	common/cam_soc_icc.o

ifeq (,$(filter $(CONFIG_CAM_PRESIL),y m))
	camera-y += camera/drivers/cam_presil/stub/cam_presil_hw_access_stub.o
else
	ccflags-y += -DCONFIG_CAM_PRESIL=1
endif

camera-$(TARGET_SYNX_ENABLE) += camera/drivers/cam_sync/cam_sync_synx.o
camera-$(CONFIG_QCOM_CX_IPEAK) += camera/drivers/cam_utils/cam_cx_ipeak.o
camera-$(CONFIG_QCOM_BUS_SCALING) += camera/drivers/cam_utils/cam_soc_bus.o

camera-$(CONFIG_SPECTRA_ISP) += \
	camera/drivers/cam_isp/isp_hw_mgr/hw_utils/cam_tasklet_util.o \
	camera/drivers/cam_isp/isp_hw_mgr/hw_utils/cam_isp_packet_parser.o \
	camera/drivers/cam_isp/isp_hw_mgr/hw_utils/irq_controller/cam_irq_controller.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_soc.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_common.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_hw_ver1.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_hw_ver2.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_mod.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ife_csid_hw/cam_ife_csid_lite_mod.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_soc.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/cam_sfe_core.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_top/cam_sfe_top.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus_rd.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/sfe_hw/sfe_bus/cam_sfe_bus_wr.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_soc.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/cam_vfe_core.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_ver2.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_rd_ver1.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_bus/cam_vfe_bus_ver3.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_lite_ver2.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_common.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver4.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver3.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_top_ver2.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_ver2.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_ver3.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_rdi.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_fe_ver1.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe_top/cam_vfe_camif_lite_ver3.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/vfe_hw/vfe17x/cam_vfe.o \
	camera/drivers/cam_isp/isp_hw_mgr/cam_isp_hw_mgr.o \
	camera/drivers/cam_isp/isp_hw_mgr/cam_ife_hw_mgr.o \
	camera/drivers/cam_isp/cam_isp_dev.o \
	camera/drivers/cam_isp/cam_isp_context.o

camera-$(CONFIG_SPECTRA_ICP) += \
	camera/drivers/cam_icp/icp_hw/icp_hw_mgr/cam_icp_hw_mgr.o \
	camera/drivers/cam_icp/icp_hw/ipe_hw/ipe_dev.o \
	camera/drivers/cam_icp/icp_hw/ipe_hw/ipe_core.o \
	camera/drivers/cam_icp/icp_hw/ipe_hw/ipe_soc.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_v1_hw/cam_icp_v1_dev.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_v1_hw/cam_icp_v1_core.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_v2_hw/cam_icp_v2_dev.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_v2_hw/cam_icp_v2_core.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_common/cam_icp_proc_common.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/icp_common/cam_icp_soc_common.o \
	camera/drivers/cam_icp/icp_hw/icp_proc/cam_icp_proc.o \
	camera/drivers/cam_icp/icp_hw/bps_hw/bps_dev.o \
	camera/drivers/cam_icp/icp_hw/bps_hw/bps_core.o \
	camera/drivers/cam_icp/icp_hw/bps_hw/bps_soc.o \
	camera/drivers/cam_icp/icp_hw/ofe_hw/ofe_dev.o \
	camera/drivers/cam_icp/icp_hw/ofe_hw/ofe_core.o \
	camera/drivers/cam_icp/icp_hw/ofe_hw/ofe_soc.o \
	camera/drivers/cam_icp/cam_icp_subdev.o \
	camera/drivers/cam_icp/cam_icp_context.o \
	camera/drivers/cam_icp/hfi.o

camera-$(CONFIG_SPECTRA_JPEG) += \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_dev.o \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_core.o \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_enc_hw/jpeg_enc_soc.o \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_dev.o \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_core.o \
	camera/drivers/cam_jpeg/jpeg_hw/jpeg_dma_hw/jpeg_dma_soc.o \
	camera/drivers/cam_jpeg/jpeg_hw/cam_jpeg_hw_mgr.o \
	camera/drivers/cam_jpeg/cam_jpeg_dev.o \
	camera/drivers/cam_jpeg/cam_jpeg_context.o

camera-$(CONFIG_SPECTRA_FD) += \
	camera/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_dev.o \
	camera/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_core.o \
	camera/drivers/cam_fd/fd_hw_mgr/fd_hw/cam_fd_hw_soc.o \
	camera/drivers/cam_fd/fd_hw_mgr/cam_fd_hw_mgr.o \
	camera/drivers/cam_fd/cam_fd_dev.o \
	camera/drivers/cam_fd/cam_fd_context.o

camera-$(CONFIG_SPECTRA_LRME) += \
	camera/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_dev.o \
	camera/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_core.o \
	camera/drivers/cam_lrme/lrme_hw_mgr/lrme_hw/cam_lrme_hw_soc.o \
	camera/drivers/cam_lrme/lrme_hw_mgr/cam_lrme_hw_mgr.o \
	camera/drivers/cam_lrme/cam_lrme_dev.o \
	camera/drivers/cam_lrme/cam_lrme_context.o

camera-$(CONFIG_SPECTRA_SENSOR) += \
	camera/drivers/cam_sensor_module/cam_actuator/cam_actuator_dev.o \
	camera/drivers/cam_sensor_module/cam_actuator/cam_actuator_core.o \
	camera/drivers/cam_sensor_module/cam_actuator/cam_actuator_soc.o \
	camera/drivers/cam_sensor_module/cam_cci/cam_cci_dev.o \
	camera/drivers/cam_sensor_module/cam_cci/cam_cci_core.o \
	camera/drivers/cam_sensor_module/cam_cci/cam_cci_soc.o \
	camera/drivers/cam_sensor_module/cam_tpg/cam_tpg_dev.o \
	camera/drivers/cam_sensor_module/cam_tpg/cam_tpg_core.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw_common.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw_v_1_0/tpg_hw_v_1_0.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw_v_1_2/tpg_hw_v_1_2.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw_v_1_3/tpg_hw_v_1_3.o \
	camera/drivers/cam_sensor_module/cam_tpg/tpg_hw/tpg_hw_v_1_4/tpg_hw_v_1_4.o \
	camera/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_soc.o \
	camera/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_dev.o \
	camera/drivers/cam_sensor_module/cam_csiphy/cam_csiphy_core.o \
	camera/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_dev.o \
	camera/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_core.o  \
	camera/drivers/cam_sensor_module/cam_eeprom/cam_eeprom_soc.o \
	camera/drivers/cam_sensor_module/cam_ois/cam_ois_dev.o \
	camera/drivers/cam_sensor_module/cam_ois/cam_ois_core.o \
	camera/drivers/cam_sensor_module/cam_ois/cam_ois_soc.o \
	camera/drivers/cam_sensor_module/cam_sensor/cam_sensor_dev.o \
	camera/drivers/cam_sensor_module/cam_sensor/cam_sensor_core.o \
	camera/drivers/cam_sensor_module/cam_sensor/cam_sensor_soc.o \
	camera/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_io.o \
	camera/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_cci_i2c.o \
	camera/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_qup_i2c.o \
	camera/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_qup_i3c.o \
	camera/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_spi.o \
	camera/drivers/cam_sensor_module/cam_sensor_utils/cam_sensor_util.o \
	camera/drivers/cam_sensor_module/cam_res_mgr/cam_res_mgr.o \
	camera/drivers/cam_sensor_module/cam_flash/cam_flash_dev.o \
	camera/drivers/cam_sensor_module/cam_flash/cam_flash_core.o \
	camera/drivers/cam_sensor_module/cam_flash/cam_flash_soc.o \
	camera/drivers/cam_sensor_module/cam_sensor_module_debug.o

camera-$(CONFIG_SPECTRA_CUSTOM) += \
	camera/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_soc.o \
	camera/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_dev.o \
	camera/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw1/cam_custom_sub_mod_core.o \
	camera/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_csid/cam_custom_csid_dev.o \
	camera/drivers/cam_cust/cam_custom_hw_mgr/cam_custom_hw_mgr.o \
	camera/drivers/cam_cust/cam_custom_dev.o \
	camera/drivers/cam_cust/cam_custom_context.o

camera-$(CONFIG_SPECTRA_OPE) += \
	camera/drivers/cam_ope/cam_ope_subdev.o \
	camera/drivers/cam_ope/cam_ope_context.o \
	camera/drivers/cam_ope/ope_hw_mgr/cam_ope_hw_mgr.o \
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_dev.o \
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_soc.o \
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/ope_core.o \
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/top/ope_top.o \
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/bus_rd/ope_bus_rd.o\
	camera/drivers/cam_ope/ope_hw_mgr/ope_hw/bus_wr/ope_bus_wr.o

camera-$(CONFIG_SPECTRA_CRE) += \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/cre_core.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/cre_soc.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/cre_dev.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/top/cre_top.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/bus_rd/cre_bus_rd.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cre_hw/bus_wr/cre_bus_wr.o \
	camera/drivers/cam_cre/cam_cre_hw_mgr/cam_cre_hw_mgr.o \
	camera/drivers/cam_cre/cam_cre_dev.o \
	camera/drivers/cam_cre/cam_cre_context.o

camera-$(CONFIG_SPECTRA_TFE) += \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi_core.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/ppi_hw/cam_csid_ppi100.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_soc.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_core.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe_bus.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_hw/cam_tfe.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_dev.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_soc.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid_core.o \
	camera/drivers/cam_isp/isp_hw_mgr/isp_hw/tfe_csid_hw/cam_tfe_csid.o \
	camera/drivers/cam_isp/isp_hw_mgr/cam_tfe_hw_mgr.o

camera-y += camera/drivers/camera_main.o

obj-m += camera.o
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/camera.ko
endif
