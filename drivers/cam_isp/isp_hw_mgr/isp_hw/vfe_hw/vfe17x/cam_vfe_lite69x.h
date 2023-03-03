/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */


#ifndef _CAM_VFE_LITE69X_H_
#define _CAM_VFE_LITE69X_H_
#include "cam_vfe_camif_ver3.h"
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

#define CAM_VFE_69X_NUM_DBG_REG 4

static struct cam_vfe_top_ver4_module_desc vfe69x_ipp_mod_desc[] = {
	{
		.id = 0,
		.desc = "CLC_BLS",
	},
	{
		.id = 1,
		.desc = "CLC_GLUT",
	},
	{
		.id = 2,
		.desc = "CLC_STATS_BG",
	},
};

static struct cam_vfe_top_ver4_wr_client_desc vfe690x_wr_client_desc[] = {
	{
		.wm_id = 0,
		.desc = "RDI_0",
	},
	{
		.wm_id = 1,
		.desc = "RDI_1",
	},
	{
		.wm_id = 2,
		.desc = "RDI_2",
	},
	{
		.wm_id = 3,
		.desc = "RDI_3",
	},
        {
		.wm_id = 4,
		.desc = "RDI_4",
	},
        {
		.wm_id = 5,
		.desc = "RDI_5",
	},
};

static struct cam_irq_register_set vfe69x_top_irq_reg_set[3] = {
	{
		.mask_reg_offset   = 0x00002068,
		.clear_reg_offset  = 0x00002070,
		.status_reg_offset = 0x00002078,
	},
	{
		.mask_reg_offset   = 0x0000206C,
		.clear_reg_offset  = 0x00002074,
		.status_reg_offset = 0x0000207C,
	},
};

static struct cam_irq_controller_reg_info vfe69x_top_irq_reg_info = {
	.num_registers = 2,
	.irq_reg_set = vfe69x_top_irq_reg_set,
	.global_clear_offset  = 0x00002064,
	.global_clear_bitmask = 0x00000001,
	.clear_all_bitmask = 0xFFFFFFFF,
};

static struct cam_vfe_top_ver4_reg_offset_common vfe69x_top_common_reg = {
	.hw_version               = 0x00002000,
	.hw_capability            = 0x00002004,
	.core_cgc_ovd_0           = 0x00002008,
	.ahb_cgc_ovd              = 0x0000200C,
	//.core_cfg_0               = 0x0000103C,   // TODO : Reg not available due to no IPP in Lite
	.diag_config              = 0x00002010,
	.diag_sensor_status_0     = 0x00002014,
	.diag_sensor_status_1     = 0x00002018,
	//.ipp_violation_status     = 0x00001054,   // TODO : Reg not available due to no IPP in Lite
	.bus_violation_status     = 0x000024E4,
	.bus_overflow_status      = 0x000024E8,
	.top_debug_cfg            = 0x000020FC,
	.num_top_debug_reg        = CAM_VFE_69X_NUM_DBG_REG,
	.top_debug                = {
		0x000020E4,
		0x000020E8,
		0x000020EC,
		0x000020F0,
	},
};

static struct cam_vfe_ver4_path_reg_data vfe69x_ipp_reg_data =
{
	.sof_irq_mask                    = 0x1,
	.eof_irq_mask                    = 0x2,
	.error_irq_mask                  = 0x2,
	.enable_diagnostic_hw            = 0x1,
	.top_debug_cfg_en                = 0x3,
	.ipp_violation_mask              = 0x10,
};

static struct cam_vfe_ver4_path_reg_data vfe69x_rdi_reg_data[6] = {

	{
		.sof_irq_mask                    = 0x1,
		.eof_irq_mask                    = 0x2,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x4,
		.eof_irq_mask                    = 0x8,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x10,
		.eof_irq_mask                    = 0x20,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x40,
		.eof_irq_mask                    = 0x80,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x100,
		.eof_irq_mask                    = 0x200,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x400,
		.eof_irq_mask                    = 0x800,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
};

static struct cam_vfe_ver4_path_hw_info
	vfe69x_rdi_hw_info[6] = {    // TODO : Define new version macro for CAM_VFE_RDI_VER2_MAX for array index
	{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[0],
	},
	{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[1],
	},
	{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[2],
	},
	{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[3],
	},
		{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[4],
	},
	{
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = &vfe69x_rdi_reg_data[5],
	},
};

static struct cam_vfe_top_ver4_debug_reg_info vfe69x_dbg_reg_info[CAM_VFE_69X_NUM_DBG_REG][8] = {
	VFE_DBG_INFO_ARRAY_4bit(
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	{
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
	},
	VFE_DBG_INFO_ARRAY_4bit(
		"PP_THROTTLE",
		"STATS_BG_THROTTLE",
		"STATS_BG",
		"BLS",
		"GLUT",
		"unused",
		"unused",
		"unused"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"RDI_0",
		"RDI_1",
		"RDI_2",
		"RDI_3",
		"PP_STATS_BG",
		"PP_GLUT",
		"PP_STATS_BG",
		"PP_GLUT"
	),
};

static struct cam_vfe_top_ver4_hw_info vfe69x_top_hw_info = {
	.common_reg = &vfe69x_top_common_reg,
	.rdi_hw_info[0] = &vfe69x_rdi_hw_info[0],
	.rdi_hw_info[1] = &vfe69x_rdi_hw_info[1],
	.rdi_hw_info[2] = &vfe69x_rdi_hw_info[2],
	.rdi_hw_info[3] = &vfe69x_rdi_hw_info[3],
	.rdi_hw_info[4] = &vfe69x_rdi_hw_info[4],
	.rdi_hw_info[5] = &vfe69x_rdi_hw_info[5],
	.vfe_full_hw_info = {
		.common_reg     = &vfe69x_top_common_reg,
		.reg_data       = NULL,   //Spectra 690: No IPP in IFE Lite. Hence set to NULL
	},
	.ipp_module_desc        = NULL, // TODO: Spectra 690: No IPP in IFE Lite. Hence set to NULL
	.wr_client_desc         = vfe690x_wr_client_desc,
	.num_mux = 6,
	.mux_type = {
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
	},
	.debug_reg_info = NULL, // TODO : configure Debug registers for Spectra 690. earlier value : &vfe69x_dbg_reg_info,
};

static struct cam_irq_register_set vfe690x_bus_irq_reg[1] = {
		{
			.mask_reg_offset   = 0x00002498,
			.clear_reg_offset  = 0x000024A0,
			.status_reg_offset = 0x000024A8,
		},
};

static struct cam_vfe_bus_ver3_hw_info vfe690x_bus_hw_info = {
	.common_reg = {
		.hw_version                       = 0x00002480,
		.cgc_ovd                          = 0x00002488,
		.if_frameheader_cfg               = {
			0x000024B4,
			0x000024B8,
			0x000024BC,
			0x000024C0,
			0x000024C4,
			0x000024C8,
		},
		.pwr_iso_cfg                      = 0x000024DC,
		.overflow_status_clear            = 0x000024E0,
		.ccif_violation_status            = 0x000024E4,
		.overflow_status                  = 0x000024E8,
		.image_size_violation_status      = 0x000024F0,
		.debug_status_top_cfg             = 0x00002570,
		.debug_status_top                 = 0x00002574,
		.test_bus_ctrl                    = 0x0000257C,
		.irq_reg_info = {
			.num_registers            = 1,
			.irq_reg_set              = vfe690x_bus_irq_reg,
			.global_clear_offset      = 0x000024B0,
			.global_clear_bitmask     = 0x00000001,
		},
	},
	.num_client = 6,
	.bus_client_reg = {
		/* BUS Client 0 RDI0 */
		{
			.cfg                      = 0x00002680,
			.image_addr               = 0x00002684,
			.frame_incr               = 0x00002688,
			.image_cfg_0              = 0x0000268C,
			.image_cfg_1              = 0x00002690,
			.image_cfg_2              = 0x00002694,
			.packer_cfg               = 0x00002698,
			.frame_header_addr        = 0x000026A0,
			.frame_header_incr        = 0x000026A4,
			.frame_header_cfg         = 0x000026A8,
			.irq_subsample_period     = 0x000026B0,
			.irq_subsample_pattern    = 0x000026B4,
			.framedrop_period         = 0x000026B8, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x000026BC, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x000026E0,
			.mmu_prefetch_max_offset  = 0x000026E4,
			.system_cache_cfg         = 0x000026E8,
			.addr_status_0            = 0x000026F0,
			.addr_status_1            = 0x000026F4,
			.addr_status_2            = 0x000026F8,
			.addr_status_3            = 0x000026FC,
			.debug_status_cfg         = 0x00002700,
			.debug_status_0           = 0x00002704,
			.debug_status_1           = 0x00002708,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 1 RDI1 */
		{
			.cfg                      = 0x00002780,
			.image_addr               = 0x00002784,
			.frame_incr               = 0x00002788,
			.image_cfg_0              = 0x0000278C,
			.image_cfg_1              = 0x00002790,
			.image_cfg_2              = 0x00002794,
			.packer_cfg               = 0x00002798,
			.frame_header_addr        = 0x000027A0,
			.frame_header_incr        = 0x000027A4,
			.frame_header_cfg         = 0x000027A8,
			.irq_subsample_period     = 0x000027B0,
			.irq_subsample_pattern    = 0x000027B4,
			.framedrop_period         = 0x00002726, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x00002726, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x000027E0,
			.mmu_prefetch_max_offset  = 0x000027E4,
			.system_cache_cfg         = 0x000027E8,
			.addr_status_0            = 0x000027F0,
			.addr_status_1            = 0x000027F4,
			.addr_status_2            = 0x000027F8,
			.addr_status_3            = 0x000027FC,
			.debug_status_cfg         = 0x00002800,
			.debug_status_0           = 0x00002804,
			.debug_status_1           = 0x00002808,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 2 RDI2 */
		{
			.cfg                      = 0x00002880,
			.image_addr               = 0x00002884,
			.frame_incr               = 0x00002888,
			.image_cfg_0              = 0x0000288C,
			.image_cfg_1              = 0x00002890,
			.image_cfg_2              = 0x00002894,
			.packer_cfg               = 0x00002898,
			.frame_header_addr        = 0x000028A0,
			.frame_header_incr        = 0x000028A4,
			.frame_header_cfg         = 0x000028A8,
			.irq_subsample_period     = 0x000028B0,
			.irq_subsample_pattern    = 0x000028B4,
			.framedrop_period         = 0x000028B8, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x000028BC, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x000028E0,
			.mmu_prefetch_max_offset  = 0x000028E4,
			.system_cache_cfg         = 0x000028E8,
			.addr_status_0            = 0x000028F0,
			.addr_status_1            = 0x000028F4,
			.addr_status_2            = 0x000028F8,
			.addr_status_3            = 0x000028FC,
			.debug_status_cfg         = 0x00002900,
			.debug_status_0           = 0x00002904,
			.debug_status_1           = 0x00002908,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 3 RDI3 */
		{
			.cfg                      = 0x00002980,
			.image_addr               = 0x00002984,
			.frame_incr               = 0x00002988,
			.image_cfg_0              = 0x0000298C,
			.image_cfg_1              = 0x00002990,
			.image_cfg_2              = 0x00002994,
			.packer_cfg               = 0x00002998,
			.frame_header_addr        = 0x000029A0,
			.frame_header_incr        = 0x000029A4,
			.frame_header_cfg         = 0x000029A8,
			.irq_subsample_period     = 0x000029B0,
			.irq_subsample_pattern    = 0x000029B4,
			.framedrop_period         = 0x000029B8, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x000029BC, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x000029E0,
			.mmu_prefetch_max_offset  = 0x000029E4,
			.system_cache_cfg         = 0x000029E8,
			.addr_status_0            = 0x000029F0,
			.addr_status_1            = 0x000029F4,
			.addr_status_2            = 0x000029F8,
			.addr_status_3            = 0x000029FC,
			.debug_status_cfg         = 0x00002A00,
			.debug_status_0           = 0x00002A04,
			.debug_status_1           = 0x00002A08,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.ubwc_regs                = NULL,
		},

		/* BUS Client 4 RDI4 */
		{
			.cfg                      = 0x00002A80,
			.image_addr               = 0x00002A84,
			.frame_incr               = 0x00002A88,
			.image_cfg_0              = 0x00002A8C,
			.image_cfg_1              = 0x00002A90,
			.image_cfg_2              = 0x00002A94,
			.packer_cfg               = 0x00002A98,
			.frame_header_addr        = 0x00002AA0,
			.frame_header_incr        = 0x00002AA4,
			.frame_header_cfg         = 0x00002AA8,
			.irq_subsample_period     = 0x00002AB0,
			.irq_subsample_pattern    = 0x00002AB4,
			.framedrop_period         = 0x00002AB8, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x00002ABC, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x00002AE0,
			.mmu_prefetch_max_offset  = 0x00002AE4,
			.system_cache_cfg         = 0x00002AE8,
			.addr_status_0            = 0x00002AF0,
			.addr_status_1            = 0x00002AF4,
			.addr_status_2            = 0x00002AF8,
			.addr_status_3            = 0x00002AFC,
			.debug_status_cfg         = 0x00002B00,
			.debug_status_0           = 0x00002B04,
			.debug_status_1           = 0x00002B08,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 5 RDI5 */
		{
			.cfg                      = 0x00002B80,
			.image_addr               = 0x00002B84,
			.frame_incr               = 0x00002B88,
			.image_cfg_0              = 0x00002B8C,
			.image_cfg_1              = 0x00002B90,
			.image_cfg_2              = 0x00002B94,
			.packer_cfg               = 0x00002B98,
			.frame_header_addr        = 0x00002BA0,
			.frame_header_incr        = 0x00002BA4,
			.frame_header_cfg         = 0x00002BA8,
			.irq_subsample_period     = 0x00002BB0,
			.irq_subsample_pattern    = 0x00002BB4,
			.framedrop_period         = 0x00002BB8, //TODO: Not applicable for LeMans. setting the member for legacy
			.framedrop_pattern        = 0x00002BBC, //TODO: Not applicable for LeMans. setting the member for legacy
			.mmu_prefetch_cfg         = 0x00002BE0,
			.mmu_prefetch_max_offset  = 0x00002BE4,
			.system_cache_cfg         = 0x00002BE8,
			.addr_status_0            = 0x00002BF0,
			.addr_status_1            = 0x00002BF4,
			.addr_status_2            = 0x00002BF8,
			.addr_status_3            = 0x00002BFC,
			.debug_status_cfg         = 0x00002C00,
			.debug_status_0           = 0x00002C04,
			.debug_status_1           = 0x00002C08,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_5,
			.ubwc_regs                = NULL,
		},

	},
	.num_out = 6,
	.vfe_out_hw_info = {
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.num_wm        = 1,
			.mid[0]        = 16,
			.line_based    = 1,
			.wm_idx        = {
				0,
			},
			.name          = {
				"LITE_0",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_1,
			.num_wm        = 1,
			.mid[0]        = 17,
			.line_based    = 1,
			.wm_idx        = {
				1,
			},
			.name          = {
				"LITE_1",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_2,
			.num_wm        = 1,
			.mid[0]        = 18,
			.line_based    = 1,
			.wm_idx        = {
				2,
			},
			.name          = {
				"LITE_2",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI3,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_3,
			.num_wm        = 1,
			.mid[0]        = 19,
			.line_based    = 1,
			.wm_idx        = {
				3,
			},
			.name          = {
				"LITE_3",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI4,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_4,
			.num_wm        = 1,
			.mid[0]        = 20,
			.line_based    = 1,
			.wm_idx        = {
				4,
			},
			.name          = {
				"LITE_4",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI5,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_5,
			.num_wm        = 1,
			.mid[0]        = 21,
			.line_based    = 1,
			.wm_idx        = {
				5,
			},
			.name          = {
				"LITE_5",
			},
		},
	},
	.bus_error_irq_mask = {
		0xDC000000,
		0x00000000,
	},
	.num_comp_grp    = 6,
	.support_consumed_addr = true,
	.comp_done_shift = 0,
	.top_irq_shift   = 0,
	.max_out_res = CAM_ISP_IFE_OUT_RES_BASE + 33,
	.support_tunneling = false,
	.fifo_depth = 2,
};

static struct cam_vfe_irq_hw_info vfe69x_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_LITE_EXT_CSID,
	.top_irq_reg   = &vfe69x_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_vfe_lite69x_hw_info = {
	.irq_hw_info                   = &vfe69x_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &vfe690x_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &vfe69x_top_hw_info,
};

#endif /* _CAM_VFE_LITE68X_H_ */
