/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_VFE690_H_
#define _CAM_VFE690_H_
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

#define CAM_VFE_BUS_VER3_690_MAX_CLIENTS     25
#define CAM_VFE_690_NUM_DBG_REG              17

/* TODO : For LeMans porting
1. P1 - Max width , height settings
2. P1 - IPP and violation status registers
3. P2 - Debug registers
*/

static struct cam_vfe_top_ver4_module_desc vfe690_ipp_mod_desc[] = {
	{
		.id = 0,
		.desc = "CLC_DEMUX",
	},
	{
		.id = 1,
		.desc = "CLC_CHANNEL_GAIN",
	},
	{
		.id = 2,
		.desc = "CLC_BPC_PDPC",
	},
	{
		.id = 3,
		.desc = "CLC_BINCORRECT",
	},
	{
		.id = 4,
		.desc = "CLC_COMPDECOMP",
	},
	{
		.id = 5,
		.desc = "CLC_LSC",
	},
	{
		.id = 6,
		.desc = "CLC_WB_GAIN",
	},
	{
		.id = 7,
		.desc = "CLC_GIC",
	},
	{
		.id = 8,
		.desc = "CLC_BPC_ABF",
	},
	{
		.id = 9,
		.desc = "CLC_BLS",
	},
	{
		.id = 10,
		.desc = "CLC_BAYER_GTM",
	},
	{
		.id = 11,
		.desc = "CLC_BAYER_LTM",
	},
	{
		.id = 12,
		.desc = "CLC_LCAC",
	},
	{
		.id = 13,
		.desc = "CLC_DEMOSAIC",
	},
	{
		.id = 14,
		.desc = "CLC_COLOR_CORRECT",
	},
	{
		.id = 15,
		.desc = "CLC_GTM",
	},
	{
		.id = 16,
		.desc = "CLC_GLUT",
	},
	{
		.id = 17,
		.desc = "CLC_COLOR_TRANSFORM",
	},
	{
		.id = 18,
		.desc = "CLC_UVG",
	},
	{
		.id = 19,
		.desc = "CLC_PREPROCESSOR",
	},
	{
		.id = 20,
		.desc = "CLC_CHROMA_UP",
	},
	{
		.id = 21,
		.desc = "CLC_SPARSE_PD_EXT",
	},
	{
		.id = 22,
		.desc = "CLC_LCR",
	},
	{
		.id = 23,
		.desc = "CLC_COMPDECOMP_HVX_TX",
	},
	{
		.id = 24,
		.desc = "CLC_COMPDECOMP_HVX_RX",
	},
	{
		.id = 25,
		.desc = "CLC_GTM_FD_OUT",
	},
	{
		.id = 26,
		.desc = "CLC_CROP_RND_CLAMP_PIXEL_RAW_OUT",
	},
	{
		.id = 27,
		.desc = "CLC_DOWNSCALE_MN_Y_FD_OUT",
	},
	{
		.id = 28,
		.desc = "CLC_DOWNSCALE_MN_C_FD_OUT",
	},
	{
		.id = 29,
		.desc = "CLC_CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_Y_FD_OUT",
	},
	{
		.id = 30,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_C_FD_OUT",
	},
	{
		.id = 31,
		.desc = "CLC_DOWNSCALE_MN_Y_DISP_OUT",
	},
	{
		.id = 32,
		.desc = "CLC_DOWNSCALE_MN_C_DISP_OUT",
	},
	{
		.id = 33,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_Y_DISP_OUT",
	},
	{
		.id = 34,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_C_DISP_OUT",
	},
	{
		.id = 35,
		.desc = "CLC_DOWNSCALE_4TO1_Y_DISP_DS4_OUT",
	},
	{
		.id = 36,
		.desc = "CLC_DOWNSCALE_4TO1_C_DISP_DS4_OUT",
	},
	{
		.id = 37,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_Y_DISP_DS4_OUT",
	},
	{
		.id = 38,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_C_DISP_DS4_OUT",
	},
	{
		.id = 39,
		.desc = "CLC_DOWNSCALE_4TO1_Y_DISP_DS16_OUT",
	},
	{
		.id = 40,
		.desc = "CLC_DOWNSCALE_4TO1_C_DISP_DS16_OUT",
	},
	{
		.id = 41,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_Y_DISP_DS16_OUT",
	},
	{
		.id = 42,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_C_DISP_DS16_OUT",
	},
	{
		.id = 43,
		.desc = "CLC_DOWNSCALE_MN_Y_VID_OUT",
	},
	{
		.id = 44,
		.desc = "CLC_DOWNSCALE_MN_C_VID_OUT",
	},
	{
		.id = 45,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_Y_VID_OUT",
	},
	{
		.id = 46,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_MN_C_VID_OUT",
	},
	{
		.id = 47,
		.desc = "CLC_DSX_Y_VID_OUT",
	},
	{
		.id = 48,
		.desc = "CLC_DSX_C_VID_OUT",
	},
	{
		.id = 49,
		.desc = "CLC_CROP_RND_CLAMP_POST_DSX_Y_VID_OUT",
	},
	{
		.id = 50,
		.desc = "CLC_CROP_RND_CLAMP_POST_DSX_C_VID_OUT",
	},
	{
		.id = 51,
		.desc = "CLC_DOWNSCALE_4TO1_Y_VID_DS16_OUT",
	},
	{
		.id = 52,
		.desc = "CLC_DOWNSCALE_4TO1_C_VID_DS16_OUT",
	},
	{
		.id = 53,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_Y_VID_DS16_OUT",
	},
	{
		.id = 54,
		.desc = "CLC_CROP_RND_CLAMP_POST_DOWNSCALE_4TO1_C_VID_DS16_OUT",
	},
	{
		.id = 55,
		.desc = "CLC_STATS_AEC_BE",
	},
	{
		.id = 56,
		.desc = "CLC_STATS_AEC_BHIST",
	},
	{
		.id = 57,
		.desc = "CLC_STATS_BHIST",
	},
	{
		.id = 58,
		.desc = "CLC_STATS_TINTLESS_BG",
	},
	{
		.id = 59,
		.desc = "CLC_STATS_AWB_BG",
	},
	{
		.id = 60,
		.desc = "CLC_STATS_BFW",
	},
	{
		.id = 61,
		.desc = "CLC_STATS_BAF",
	},
	{
		.id = 62,
		.desc = "CLC_STATS_RS",
	},
	{
		.id = 63,
		.desc = "CLC_STATS_IHIST",
	},
};

static struct cam_vfe_top_ver4_wr_client_desc vfe690_wr_client_desc[] = {
	{
		.wm_id = 0,
		.desc = "VIDEO_FULL_Y_C0",
	},
	{
		.wm_id = 1,
		.desc = "VIDEO_FULL_UV_C1",
	},
	{
		.wm_id = 2,
		.desc = "VIDEO_FULL_C2",
	},
	{
		.wm_id = 3,
		.desc = "DISPLAY_FULL_Y",
	},
	{
		.wm_id = 4,
		.desc = "DISPLAY_FULL_C",
	},
	{
		.wm_id = 5,
		.desc = "DISPLAY_DS_4:1",
	},
	{
		.wm_id = 6,
		.desc = "DISPLAY_DS_16:1",
	},
	{
		.wm_id = 7,
		.desc = "PIXEL_RAW",
	},
	{
		.wm_id = 8,
		.desc = "LTM_STATS",
	},
	{
		.wm_id = 9,
		.desc = "STATS_AEC_BE",
	},
	{
		.wm_id = 10,
		.desc = "STATS_AEC_BHIST",
	},
	{
		.wm_id = 11,
		.desc = "STATS_TINTLESS_BG",
	},
	{
		.wm_id = 12,
		.desc = "STATS_AWB_BG",
	},
	{
		.wm_id = 13,
		.desc = "STATS_AWB_BFW",
	},
	{
		.wm_id = 14,
		.desc = "STATS_GTM_BHIST",
	},
	{
		.wm_id = 15,
		.desc = "STATS_FLICKER_RS",
	},
	{
		.wm_id = 16,
		.desc = "RDI0",
	},
	{
		.wm_id = 17,
		.desc = "RDI1",
	},
	{
		.wm_id = 18,
		.desc = "RDI2",
	},
    {
		.wm_id = 19,
		.desc = "AI_OUT1_Y_C0",
	},
	{
		.wm_id = 20,
		.desc = "AI_OUT1_UV_C1",
	},
	{
		.wm_id = 21,
		.desc = "AI_OUT1_C2",
	},
    {
		.wm_id = 22,
		.desc = "AI_OUT2_Y_C0",
	},
	{
		.wm_id = 23,
		.desc = "AI_OUT2_UV_C1",
	},
	{
		.wm_id = 24,
		.desc = "AI_OUT2_C2",
	},
};

static struct cam_vfe_top_ver4_top_err_irq_desc vfe690_top_irq_err_desc[] = {
	{
		.bitmask = BIT(0),
		.err_name = "BUS WR VIOLATION",
		.desc = "Violation error in the Bus write module",
	},
	{
		.bitmask = BIT(4),
		.err_name = "PP VIOLATION",
		.desc = "Violation error in the IPP Pixel path CLCs",
	},
	{
		.bitmask = BIT(9),
		.err_name = "DSP IFE PROTOCOL VIOLATION",
		.desc = "CCIF protocol violation on the output Data",
	},
	{
		.bitmask = BIT(10),
		.err_name = "IFE DSP TX PROTOCOL VIOLATION",
		.desc = "CCIF protocol violation on the outgoing data to the DSP interface",
	},
	{
		.bitmask = BIT(11),
		.err_name = "DSP IFE RX PROTOCOL VIOLATION",
		.desc = "CCIF protocol violation on the incoming data from DSP before processed",
	},
	{
		.bitmask = BIT(12),
		.err_name = "DSP TX FIFO OVERFLOW",
		.desc = "Overflow on DSP interface TX path FIFO",
	},
	{
		.bitmask = BIT(13),
		.err_name = "DSP RX FIFO OVERFLOW",
		.desc = "Overflow on DSP interface RX path FIFO",
	},
	{
		.bitmask = BIT(14),
		.err_name = "DSP ERROR VIOLATION",
		.desc = "When DSP sends a error signal",
	},
	{
		.bitmask = BIT(15),
		.err_name = "DIAG VIOLATION",
		.desc = "HBI is less than the minimum required HBI",
	},
};

static struct cam_vfe_top_ver4_pdaf_violation_desc vfe690_pdaf_violation_desc[] = {
	{
		.bitmask = BIT(0),
		.desc = "Sim monitor 1 violation - SAD output",
	},
	{
		.bitmask = BIT(1),
		.desc = "Sim monitor 2 violation - pre-proc output, LSB aligned",
	},
};

static struct cam_irq_register_set vfe690_top_irq_reg_set[2] = {
	{
		.mask_reg_offset   = 0x000000A8,
		.clear_reg_offset  = 0x000000B0,
		.status_reg_offset = 0x000000B8,
	},
	{
		.mask_reg_offset   = 0x000000AC,
		.clear_reg_offset  = 0x000000B4,
		.status_reg_offset = 0x000000BC,
	},
};

static struct cam_irq_controller_reg_info vfe690_top_irq_reg_info = {
	.num_registers = 2,
	.irq_reg_set = vfe690_top_irq_reg_set,
	.global_clear_offset  = 0x000000A4,
	.global_clear_bitmask = 0x00000001,
	.clear_all_bitmask = 0xFFFFFFFF,
};

//DONE FOR LeMans
static struct cam_vfe_top_ver4_reg_offset_common vfe690_top_common_reg = {
	.hw_version               = 0x00000000,
	.hw_capability            = 0x00000004,
	.lens_feature             = 0x00000008,
	.stats_feature            = 0x0000000C,
	.color_feature            = 0x00000010,
	.zoom_feature             = 0x00000014,
	.core_cfg_0               = 0x00000024,
	.core_cfg_1               = 0x00000028,
	.core_cfg_2               = 0x0000002C,
	.global_reset_cmd         = 0x000000A4,
	.diag_config              = 0x00000044,
	.diag_sensor_status_0     = 0x00000048,
	.diag_sensor_status_1     = 0x0000004C,
	.diag_frm_cnt_status_0    = 0x00000050,
	.diag_frm_cnt_status_1    = 0x00000054,
	.ipp_violation_status     = 0x00000064, //TODO: LeMans, ALSO ADD NEW VIOLATION STATUS
	.pdaf_violation_status    = 0x00000404,
	.core_cfg_3               = 0x00000068, // TODO: Remove for LeMans
	.core_cgc_ovd_0           = 0x00000018,
	.core_cgc_ovd_1           = 0x0000001C,
	.ahb_cgc_ovd              = 0x00000020,
	.dsp_status               = 0x0000006C, // TODO: Remove for LeMans
	.stats_throttle_cfg_0     = 0x0000005C,
	.stats_throttle_cfg_1     = 0x00000060,
	.stats_throttle_cfg_2     = 0x00000078, // TODO: Remove for LeMans
	.core_cfg_4               = 0x00000080, // TODO: Remove for LeMans
	.core_cfg_5               = 0x00000084, // TODO: Remove for LeMans
	.core_cfg_6               = 0x00000088, // TODO: Remove for LeMans
	.period_cfg               = 0x00000030,
	.irq_sub_pattern_cfg      = 0x00000034,
	.epoch0_pattern_cfg       = 0x00000038,
	.epoch1_pattern_cfg       = 0x0000003C,
	.epoch_height_cfg         = 0x00000040,
	.bus_violation_status     = 0x00000464,
	.bus_overflow_status      = 0x00000468,
	.top_debug_cfg            = 0x00000170,
	.num_top_debug_reg        = CAM_VFE_690_NUM_DBG_REG,
	.top_debug = {
		0x00000128,
		0x0000012C,
		0x00000130,
		0x00000134,
		0x00000138,
		0x0000013C,
		0x00000140,
		0x00000144,
		0x00000148,
		0x0000014C,
		0x00000150,
		0x00000154,
		0x00000158,
		0x0000015C,
		0x00000160,
		0x00000164,
		0x00000168,
	},
};
//Done for LeMans
static struct cam_vfe_ver4_path_reg_data vfe690_pp_common_reg_data = {
	.sof_irq_mask                    = 0x00000001,
	.epoch0_irq_mask                 = 0x10000,
	.epoch1_irq_mask                 = 0x20000,
	.eof_irq_mask                    = 0x00000002,
	.error_irq_mask                  = 0x7F050, // TODO: Get right value for LeMans
	.enable_diagnostic_hw            = 0x1,
	.top_debug_cfg_en                = 3,
	.ipp_violation_mask              = 0x10,
	//.pdaf_violation_mask             = 0x40, // Removed for LeMans. No PDAF in IPP.
};
//Done for LeMans
static struct cam_vfe_ver4_path_reg_data vfe690_vfe_full_rdi_reg_data[3] = {
	{
		.sof_irq_mask                    = 0x100,
		.eof_irq_mask                    = 0x200,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x400,
		.eof_irq_mask                    = 0x800,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x1000,
		.eof_irq_mask                    = 0x2000,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
};
//LeMans does not have pdaf. so not required.
static struct cam_vfe_ver4_path_reg_data vfe690_pdlib_reg_data = {
	.sof_irq_mask                    = 0x4,
	.eof_irq_mask                    = 0x8,
	.error_irq_mask                  = 0x0,
	.enable_diagnostic_hw            = 0x1,
	.top_debug_cfg_en                = 3,
};

struct cam_vfe_ver4_path_hw_info
	vfe690_rdi_hw_info_arr[CAM_VFE_RDI_VER2_MAX] = {
	{
		.common_reg     = &vfe690_top_common_reg,
		.reg_data       = &vfe690_vfe_full_rdi_reg_data[0],
	},
	{
		.common_reg     = &vfe690_top_common_reg,
		.reg_data       = &vfe690_vfe_full_rdi_reg_data[1],
	},
	{
		.common_reg     = &vfe690_top_common_reg,
		.reg_data       = &vfe690_vfe_full_rdi_reg_data[2],
	},
};

//TODO: Need to update for LeMans
static struct cam_vfe_top_ver4_debug_reg_info vfe690_dbg_reg_info[CAM_VFE_690_NUM_DBG_REG][8] = {
	VFE_DBG_INFO_ARRAY_4bit("test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"STATS_IHIST",
		"STATS_RS",
		"STATS_BAF",
		"GTM_BHIST",
		"TINTLESS_BG",
		"STATS_BFW",
		"STATS_BG",
		"STATS_BHIST"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"STATS_BE",
		"R2PD_DS16_C_VID",
		"R2PD_DS16_Y_VID",
		"crop_rnd_clamp_post_downscale_C_DS16_VID",
		"4to1_C_DS16_VID",
		"crop_rnd_clamp_post_downscale_Y_DS16_VID",
		"4to1_Y_DS16_VID",
		"crop_rnd_clamp_post_dsx_C_VID"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"R2PD_DS4_VID_C",
		"R2PD_DS4_VID_Y",
		"DSX_C",
		"crop_rnd_clamp_post_dsx_Y_VID",
		"DSX_Y",
		"crop_rnd_clamp_post_downscale_mn_C_VID",
		"downscale_mn_C_VID",
		"crop_rnd_clamp_post_downscale_mn_Y_VID"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"MNDS_Y_VID",
		"R2PD_DS16_C_DISP",
		"R2PD_DS16_Y_DISP",
		"crop_rnd_clamp_post_downscale_C_DS16_DISP",
		"4to1_C_DS16_DISP",
		"crop_rnd_clamp_post_downscale_Y_DS16_DISP",
		"4to1_Y_DS16_DISP",
		"R2PD_DS4_C_DISP"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"R2PD_DS4_Y_DISP",
		"crop_rnd_clamp_post_downscale_C_DS4_DISP",
		"4to1_C_DS4_DISP",
		"crop_rnd_clamp_post_downscale_Y_DS4_DISP",
		"4to1_Y_DS4_DISP",
		"crop_rnd_clamp_post_downscale_mn_C_DISP",
		"downscale_mn_C_DISP",
		"crop_rnd_clamp_post_downscale_mn_Y_DISP"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"downscale_mn_Y_DISP",
		"crop_rnd_clamp_post_downscale_mn_C_FD",
		"downscale_mn_C_FD",
		"crop_rnd_clamp_post_downscale_mn_Y_FD",
		"downscale_mn_Y_FD",
		"gtm_fd_out",
		"uvg",
		"color_xform"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"glut",
		"gtm",
		"color_correct",
		"demosaic",
		"hvx_tap2",
		"lcac",
		"bayer_ltm",
		"bayer_gtm"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"bls",
		"bpc_abf",
		"gic",
		"wb_gain",
		"lsc",
		"compdecomp_hxv_rx",
		"compdecomp_hxv_tx",
		"hvx_tap1"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"decompand",
		"reserved",
		"bincorrect",
		"bpc_pdpc",
		"channel_gain",
		"bayer_argb_ccif_converter",
		"crop_rnd_clamp_pre_argb_packer",
		"chroma_up_uv"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"chroma_up_y",
		"demux",
		"hxv_tap0",
		"preprocess",
		"sparse_pd_ext",
		"lcr",
		"bayer_ltm_bus_wr",
		"RDI2"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"RDI1",
		"RDI0",
		"lcr_bus_wr",
		"pdaf_sad_bus_wr",
		"pd_data_bus_wr",
		"sparse_pd_bus_wr",
		"ihist_bus_wr",
		"flicker_rs_bus_wr"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"gtm_bhist_bus_wr",
		"baf_bus_wr",
		"bfw_bus_wr",
		"bg_bus_wr",
		"tintless_bg_bus_wr",
		"bhist_bus_wr",
		"be_bus_wr",
		"pixel_raw_bus_wr"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"fd_c_bus_wr",
		"fd_y_bus_wr",
		"disp_ds16_bus_wr",
		"disp_ds4_bus_wr",
		"disp_c_bus_wr",
		"disp_y_bus_wr",
		"vid_ds16_bus_Wr",
		"vid_ds4_bus_Wr"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"vid_c_bus_wr",
		"vid_y_bus_wr",
		"CLC_PDAF",
		"PIX_PP",
		"reserved",
		"reserved",
		"reserved",
		"reserved"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved",
		"r2pd_reserved"
	),
};

// Done for LeMans
static struct cam_vfe_top_ver4_hw_info vfe690_top_hw_info = {
	.common_reg = &vfe690_top_common_reg,
	.vfe_full_hw_info = {
		.common_reg     = &vfe690_top_common_reg,
		.reg_data       = &vfe_pp_common_reg_data,
	},
	.pdlib_hw_info = {
		.common_reg     = &vfe690_top_common_reg,
		.reg_data       = NULL, // Removing for LeMans. Earlier value &vfe690_pdlib_reg_data,
	},
	.rdi_hw_info[0] = &vfe690_rdi_hw_info_arr[0],
	.rdi_hw_info[1] = &vfe690_rdi_hw_info_arr[1],
	.rdi_hw_info[2] = &vfe690_rdi_hw_info_arr[2],
	.wr_client_desc         = vfe690_wr_client_desc, // Done for LeMans
	.ipp_module_desc        = vfe690_ipp_mod_desc,   // TODO
	.num_mux = 4,
	.mux_type = {
		CAM_VFE_CAMIF_VER_4_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
	},
	.num_path_port_map = 0, // No PD path for LeMans
	/* No PD path for LeMans
	.path_port_map = {
		{CAM_ISP_HW_VFE_IN_PDLIB, CAM_ISP_IFE_OUT_RES_2PD},
		{CAM_ISP_HW_VFE_IN_PDLIB, CAM_ISP_IFE_OUT_RES_PREPROCESS_2PD}
	},
	*/
	.num_top_errors                  = ARRAY_SIZE(vfe690_top_irq_err_desc),
	.top_err_desc                    = vfe690_top_irq_err_desc,
	.num_pdaf_violation_errors       = ARRAY_SIZE(vfe690_pdaf_violation_desc),
	.pdaf_violation_desc             = NULL, // PDAF is not available for LeMans. vfe690_pdaf_violation_desc,
	.debug_reg_info                  = &vfe690_dbg_reg_info, // TODO for LeMans
};

//Done for LeMans
static struct cam_irq_register_set vfe690_bus_irq_reg[1] = {
		{
			.mask_reg_offset   = 0x00000418,
			.clear_reg_offset  = 0x00000420,
			.status_reg_offset = 0x00000428,
		},
};

//Done for LeMans
static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_0 = {       // VID Full Y_C0
	.meta_addr        = 0x00000640,
	.meta_cfg         = 0x00000644,
	.mode_cfg         = 0x00000648,
	.stats_ctrl       = 0x0000064C,
	.ctrl_2           = 0x00000650,
	.lossy_thresh0    = 0x00000654,
	.lossy_thresh1    = 0x00000658,
	.off_lossy_var    = 0x0000065C,
	.bw_limit         = 0x0000061C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_1 = {       // VID Full C1
	.meta_addr        = 0x00000740,
	.meta_cfg         = 0x00000744,
	.mode_cfg         = 0x00000748,
	.stats_ctrl       = 0x0000074C,
	.ctrl_2           = 0x00000750,
	.lossy_thresh0    = 0x00000754,
	.lossy_thresh1    = 0x00000758,
	.off_lossy_var    = 0x0000075C,
	.bw_limit         = 0x0000071C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_2 = {       // VID Full C2
	.meta_addr        = 0x00000840,
	.meta_cfg         = 0x00000844,
	.mode_cfg         = 0x00000848,
	.stats_ctrl       = 0x0000084C,
	.ctrl_2           = 0x00000850,
	.lossy_thresh0    = 0x00000854,
	.lossy_thresh1    = 0x00000858,
	.off_lossy_var    = 0x0000085C,
	.bw_limit         = 0x0000081C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_3 = {       // DISP Y
	.meta_addr        = 0x00000940,
	.meta_cfg         = 0x00000944,
	.mode_cfg         = 0x00000948,
	.stats_ctrl       = 0x0000094C,
	.ctrl_2           = 0x00000950,
	.lossy_thresh0    = 0x00000954,
	.lossy_thresh1    = 0x00000958,
	.off_lossy_var    = 0x0000095C,
	.bw_limit         = 0x0000091C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_4 = {       // DISP C
	.meta_addr        = 0x00000A40,
	.meta_cfg         = 0x00000A44,
	.mode_cfg         = 0x00000A48,
	.stats_ctrl       = 0x00000A4C,
	.ctrl_2           = 0x00000A50,
	.lossy_thresh0    = 0x00000A54,
	.lossy_thresh1    = 0x00000A58,
	.off_lossy_var    = 0x00000A5C,
	.bw_limit         = 0x00000A1C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_19 = {
	.meta_addr        = 0x00001940,
	.meta_cfg         = 0x00001944,
	.mode_cfg         = 0x00001948,
	.stats_ctrl       = 0x0000194C,
	.ctrl_2           = 0x00001950,
	.lossy_thresh0    = 0x00001954,
	.lossy_thresh1    = 0x00001958,
	.off_lossy_var    = 0x0000195C,
	.bw_limit         = 0x0000191C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_20 = {
	.meta_addr        = 0x00001A40,
	.meta_cfg         = 0x00001A44,
	.mode_cfg         = 0x00001A48,
	.stats_ctrl       = 0x00001A4C,
	.ctrl_2           = 0x00001A50,
	.lossy_thresh0    = 0x00001A54,
	.lossy_thresh1    = 0x00001A58,
	.off_lossy_var    = 0x00001A5C,
	.bw_limit         = 0x00001A1C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_22 = {
	.meta_addr        = 0x00001C40,
	.meta_cfg         = 0x00001C44,
	.mode_cfg         = 0x00001C48,
	.stats_ctrl       = 0x00001C4C,
	.ctrl_2           = 0x00001C50,
	.lossy_thresh0    = 0x00001C54,
	.lossy_thresh1    = 0x00001C58,
	.off_lossy_var    = 0x00001C5C,
	.bw_limit         = 0x00001C1C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	vfe690_ubwc_regs_client_23 = {
	.meta_addr        = 0x00001D40,
	.meta_cfg         = 0x00001D44,
	.mode_cfg         = 0x00001D48,
	.stats_ctrl       = 0x00001D4C,
	.ctrl_2           = 0x00001D50,
	.lossy_thresh0    = 0x00001D54,
	.lossy_thresh1    = 0x00001D58,
	.off_lossy_var    = 0x00001D5C,
	.bw_limit         = 0x00001D1C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_hw_info vfe690_bus_hw_info = {
	.common_reg = { // Done for LeMans
		.hw_version                       = 0x00000400,
		.cgc_ovd                          = 0x00000408,
		.if_frameheader_cfg               = {
			0x00000434,
			0x00000438,
			0x0000043C,
			0x00000440,
		},
		.ubwc_static_ctrl                 = 0x00000458,
		.pwr_iso_cfg                      = 0x0000045C,
		.overflow_status_clear            = 0x00000460,
		.ccif_violation_status            = 0x00000464,
		.overflow_status                  = 0x00000468,
		.image_size_violation_status      = 0x00000470,
		.debug_status_top_cfg             = 0x000004F0,
		.debug_status_top                 = 0x000004F4,
		.test_bus_ctrl                    = 0x000004FC,
		.irq_reg_info = {
			.num_registers            = 1,
			.irq_reg_set              = vfe690_bus_irq_reg,
			.global_clear_offset      = 0x00000430,
			.global_clear_bitmask     = 0x00000001,
		},
	},
	.num_client = CAM_VFE_BUS_VER3_690_MAX_CLIENTS,
	.bus_client_reg = {
		/* BUS Client 0 FULL Y */
		{
			.cfg                      = 0x00000600,
			.image_addr               = 0x00000604,
			.frame_incr               = 0x00000608,
			.image_cfg_0              = 0x0000060C,
			.image_cfg_1              = 0x00000610,
			.image_cfg_2              = 0x00000614,
			.packer_cfg               = 0x00000618,
			.frame_header_addr        = 0x00000620,
			.frame_header_incr        = 0x00000624,
			.frame_header_cfg         = 0x00000628,
			.irq_subsample_period     = 0x00000630,
			.irq_subsample_pattern    = 0x00000634,
			.framedrop_period         = 0x00000638,
			.framedrop_pattern        = 0x0000063C,
			.mmu_prefetch_cfg         = 0x00000660,
			.mmu_prefetch_max_offset  = 0x00000664,
			.system_cache_cfg         = 0x00000668,
			.addr_status_0            = 0x00000670,
			.addr_status_1            = 0x00000674,
			.addr_status_2            = 0x00000678,
			.addr_status_3            = 0x0000067C,
			.debug_status_cfg         = 0x00000680,
			.debug_status_0           = 0x00000684,
			.debug_status_1           = 0x00000688,
			.bw_limiter_addr          = 0x0000061C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_0,
		},
		/* BUS Client 1 FULL C */
		{
			.cfg                      = 0x00000700,
			.image_addr               = 0x00000704,
			.frame_incr               = 0x00000708,
			.image_cfg_0              = 0x0000070C,
			.image_cfg_1              = 0x00000710,
			.image_cfg_2              = 0x00000714,
			.packer_cfg               = 0x00000718,
			.frame_header_addr        = 0x00000720,
			.frame_header_incr        = 0x00000724,
			.frame_header_cfg         = 0x00000728,
			.irq_subsample_period     = 0x00000730,
			.irq_subsample_pattern    = 0x00000734,
			.framedrop_period         = 0x00000738,
			.framedrop_pattern        = 0x0000073C,
			.mmu_prefetch_cfg         = 0x00000760,
			.mmu_prefetch_max_offset  = 0x00000764,
			.system_cache_cfg         = 0x00000768,
			.addr_status_0            = 0x00000770,
			.addr_status_1            = 0x00000774,
			.addr_status_2            = 0x00000778,
			.addr_status_3            = 0x0000077C,
			.debug_status_cfg         = 0x00000780,
			.debug_status_0           = 0x00000784,
			.debug_status_1           = 0x00000788,
			.bw_limiter_addr          = 0x0000071C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_1,
		},
		/* BUS Client 2 FULL C2 */
		{
			.cfg                      = 0x00000800,
			.image_addr               = 0x00000804,
			.frame_incr               = 0x00000808,
			.image_cfg_0              = 0x0000080C,
			.image_cfg_1              = 0x00000810,
			.image_cfg_2              = 0x00000814,
			.packer_cfg               = 0x00000818,
			.irq_subsample_period     = 0x00000830,
			.irq_subsample_pattern    = 0x00000834,
			.framedrop_period         = 0x00000838,
			.framedrop_pattern        = 0x0000083C,
			.mmu_prefetch_cfg         = 0x00000860,
			.mmu_prefetch_max_offset  = 0x00000864,
			.system_cache_cfg         = 0x00000868,
			.addr_status_0            = 0x00000870,
			.addr_status_1            = 0x00000874,
			.addr_status_2            = 0x00000878,
			.addr_status_3            = 0x0000087C,
			.debug_status_cfg         = 0x00000880,
			.debug_status_0           = 0x00000884,
			.debug_status_1           = 0x00000888,
			.bw_limiter_addr          = 0x0000081C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_2,
		},
		/* BUS Client 3 DISP Y */
		{
			.cfg                      = 0x00000900,
			.image_addr               = 0x00000904,
			.frame_incr               = 0x00000908,
			.image_cfg_0              = 0x0000090C,
			.image_cfg_1              = 0x00000910,
			.image_cfg_2              = 0x00000914,
			.packer_cfg               = 0x00000918,
			.irq_subsample_period     = 0x00000930,
			.irq_subsample_pattern    = 0x00000934,
			.framedrop_period         = 0x00000938,
			.framedrop_pattern        = 0x0000093C,
			.mmu_prefetch_cfg         = 0x00000960,
			.mmu_prefetch_max_offset  = 0x00000964,
			.system_cache_cfg         = 0x00000968,
			.addr_status_0            = 0x00000970,
			.addr_status_1            = 0x00000974,
			.addr_status_2            = 0x00000978,
			.addr_status_3            = 0x0000097C,
			.debug_status_cfg         = 0x00000980,
			.debug_status_0           = 0x00000984,
			.debug_status_1           = 0x00000988,
			.bw_limiter_addr          = 0x0000091C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = &vfe690_ubwc_regs_client_3,
		},
		/* BUS Client 4 DISP C */
		{
			.cfg                      = 0x00000A00,
			.image_addr               = 0x00000A04,
			.frame_incr               = 0x00000A08,
			.image_cfg_0              = 0x00000A0C,
			.image_cfg_1              = 0x00000A10,
			.image_cfg_2              = 0x00000A14,
			.packer_cfg               = 0x00000A18,
			.frame_header_addr        = 0x00000A20,
			.frame_header_incr        = 0x00000A24,
			.frame_header_cfg         = 0x00000A28,
			.irq_subsample_period     = 0x00000A30,
			.irq_subsample_pattern    = 0x00000A34,
			.framedrop_period         = 0x00000A38,
			.framedrop_pattern        = 0x00000A3C,
			.mmu_prefetch_cfg         = 0x00000A60,
			.mmu_prefetch_max_offset  = 0x00000A64,
			.system_cache_cfg         = 0x00000A68,
			.addr_status_0            = 0x00000A70,
			.addr_status_1            = 0x00000A74,
			.addr_status_2            = 0x00000A78,
			.addr_status_3            = 0x00000A7C,
			.debug_status_cfg         = 0x00000A80,
			.debug_status_0           = 0x00000A84,
			.debug_status_1           = 0x00000A88,
			.bw_limiter_addr          = 0x00000A1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = &vfe690_ubwc_regs_client_4,
		},
		/* BUS Client 5 DISP DS4 */
		{
			.cfg                      = 0x00000B00,
			.image_addr               = 0x00000B04,
			.frame_incr               = 0x00000B08,
			.image_cfg_0              = 0x00000B0C,
			.image_cfg_1              = 0x00000B10,
			.image_cfg_2              = 0x00000B14,
			.packer_cfg               = 0x00000B18,
			.frame_header_addr        = 0x00000B20,
			.frame_header_incr        = 0x00000B24,
			.frame_header_cfg         = 0x00000B28,
			.irq_subsample_period     = 0x00000B30,
			.irq_subsample_pattern    = 0x00000B34,
			.framedrop_period         = 0x00000B38,
			.framedrop_pattern        = 0x00000B3C,
			.mmu_prefetch_cfg         = 0x00000B60,
			.mmu_prefetch_max_offset  = 0x00000B64,
			.system_cache_cfg         = 0x00000B68,
			.addr_status_0            = 0x00000B70,
			.addr_status_1            = 0x00000B74,
			.addr_status_2            = 0x00000B78,
			.addr_status_3            = 0x00000B7C,
			.debug_status_cfg         = 0x00000B80,
			.debug_status_0           = 0x00000B84,
			.debug_status_1           = 0x00000B88,
			.bw_limiter_addr          = 0x00000B1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 6 DISP DS16 */
		{
			.cfg                      = 0x00000C00,
			.image_addr               = 0x00000C04,
			.frame_incr               = 0x00000C08,
			.image_cfg_0              = 0x00000C0C,
			.image_cfg_1              = 0x00000C10,
			.image_cfg_2              = 0x00000C14,
			.packer_cfg               = 0x00000C18,
			.irq_subsample_period     = 0x00000C30,
			.irq_subsample_pattern    = 0x00000C34,
			.framedrop_period         = 0x00000C38,
			.framedrop_pattern        = 0x00000C3C,
			.mmu_prefetch_cfg         = 0x00000C60,
			.mmu_prefetch_max_offset  = 0x00000C64,
			.system_cache_cfg         = 0x00000C68,
			.addr_status_0            = 0x00000C70,
			.addr_status_1            = 0x00000C74,
			.addr_status_2            = 0x00000C78,
			.addr_status_3            = 0x00000C7C,
			.debug_status_cfg         = 0x00000C80,
			.debug_status_0           = 0x00000C84,
			.debug_status_1           = 0x00000C88,
			.bw_limiter_addr          = 0x00000C1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 7 PIXEL_RAW */
		{
			.cfg                      = 0x00000D00,
			.image_addr               = 0x00000D04,
			.frame_incr               = 0x00000D08,
			.image_cfg_0              = 0x00000D0C,
			.image_cfg_1              = 0x00000D10,
			.image_cfg_2              = 0x00000D14,
			.packer_cfg               = 0x00000D18,
			.irq_subsample_period     = 0x00000D30,
			.irq_subsample_pattern    = 0x00000D34,
			.framedrop_period         = 0x00000D38,
			.framedrop_pattern        = 0x00000D3C,
			.mmu_prefetch_cfg         = 0x00000D60,
			.mmu_prefetch_max_offset  = 0x00000D64,
			.system_cache_cfg         = 0x00000D68,
			.addr_status_0            = 0x00000D70,
			.addr_status_1            = 0x00000D74,
			.addr_status_2            = 0x00000D78,
			.addr_status_3            = 0x00000D7C,
			.debug_status_cfg         = 0x00000D80,
			.debug_status_0           = 0x00000D84,
			.debug_status_1           = 0x00000D88,
			.bw_limiter_addr          = 0x00000D1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 8 LTM_STATS */
		{
			.cfg                      = 0x00000E00,
			.image_addr               = 0x00000E04,
			.frame_incr               = 0x00000E08,
			.image_cfg_0              = 0x00000E0C,
			.image_cfg_1              = 0x00000E10,
			.image_cfg_2              = 0x00000E14,
			.packer_cfg               = 0x00000E18,
			.frame_header_addr        = 0x00000E20,
			.frame_header_incr        = 0x00000E24,
			.frame_header_cfg         = 0x00000E28,
			.irq_subsample_period     = 0x00000E30,
			.irq_subsample_pattern    = 0x00000E34,
			.framedrop_period         = 0x00000E38,
			.framedrop_pattern        = 0x00000E3C,
			.mmu_prefetch_cfg         = 0x00000E60,
			.mmu_prefetch_max_offset  = 0x00000E64,
			.system_cache_cfg         = 0x00000E68,
			.addr_status_0            = 0x00000E70,
			.addr_status_1            = 0x00000E74,
			.addr_status_2            = 0x00000E78,
			.addr_status_3            = 0x00000E7C,
			.debug_status_cfg         = 0x00000E80,
			.debug_status_0           = 0x00000E84,
			.debug_status_1           = 0x00000E88,
			.bw_limiter_addr          = 0x00000E1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 9 STATS_AEC_BE */
		{
			.cfg                      = 0x00000F00,
			.image_addr               = 0x00000F04,
			.frame_incr               = 0x00000F08,
			.image_cfg_0              = 0x00000F0C,
			.image_cfg_1              = 0x00000F10,
			.image_cfg_2              = 0x00000F14,
			.packer_cfg               = 0x00000F18,
			.irq_subsample_period     = 0x00000F30,
			.irq_subsample_pattern    = 0x00000F34,
			.framedrop_period         = 0x00000F38,
			.framedrop_pattern        = 0x00000F3C,
			.mmu_prefetch_cfg         = 0x00000F60,
			.mmu_prefetch_max_offset  = 0x00000F64,
			.system_cache_cfg         = 0x00000F68,
			.addr_status_0            = 0x00000F70,
			.addr_status_1            = 0x00000F74,
			.addr_status_2            = 0x00000F78,
			.addr_status_3            = 0x00000F7C,
			.debug_status_cfg         = 0x00000F80,
			.debug_status_0           = 0x00000F84,
			.debug_status_1           = 0x00000F88,
			.bw_limiter_addr          = 0x00000F1C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 10 STATS_AEC_BHIST */
		{
			.cfg                      = 0x00001000,
			.image_addr               = 0x00001004,
			.frame_incr               = 0x00001008,
			.image_cfg_0              = 0x0000100C,
			.image_cfg_1              = 0x00001010,
			.image_cfg_2              = 0x00001014,
			.packer_cfg               = 0x00001018,
			.frame_header_addr        = 0x00001020,
			.frame_header_incr        = 0x00001024,
			.frame_header_cfg         = 0x00001028,
			.irq_subsample_period     = 0x00001030,
			.irq_subsample_pattern    = 0x00001034,
			.framedrop_period         = 0x00001038,
			.framedrop_pattern        = 0x0000103C,
			.mmu_prefetch_cfg         = 0x00001060,
			.mmu_prefetch_max_offset  = 0x00001064,
			.system_cache_cfg         = 0x00001068,
			.addr_status_0            = 0x00001070,
			.addr_status_1            = 0x00001074,
			.addr_status_2            = 0x00001078,
			.addr_status_3            = 0x0000107C,
			.debug_status_cfg         = 0x00001080,
			.debug_status_0           = 0x00001084,
			.debug_status_1           = 0x00001088,
			.bw_limiter_addr          = 0x0000101C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 11 STATS_TINTLESS_BG */
		{
			.cfg                      = 0x00001100,
			.image_addr               = 0x00001104,
			.frame_incr               = 0x00001108,
			.image_cfg_0              = 0x0000110C,
			.image_cfg_1              = 0x00001110,
			.image_cfg_2              = 0x00001114,
			.packer_cfg               = 0x00001118,
			.frame_header_addr        = 0x00001120,
			.frame_header_incr        = 0x00001124,
			.frame_header_cfg         = 0x00001128,
			.irq_subsample_period     = 0x00001130,
			.irq_subsample_pattern    = 0x00001134,
			.framedrop_period         = 0x00001138,
			.framedrop_pattern        = 0x0000113C,
			.mmu_prefetch_cfg         = 0x00001160,
			.mmu_prefetch_max_offset  = 0x00001164,
			.system_cache_cfg         = 0x00001168,
			.addr_status_0            = 0x00001170,
			.addr_status_1            = 0x00001174,
			.addr_status_2            = 0x00001178,
			.addr_status_3            = 0x0000117C,
			.debug_status_cfg         = 0x00001180,
			.debug_status_0           = 0x00001184,
			.debug_status_1           = 0x00001188,
			.bw_limiter_addr          = 0x0000111C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 12 STATS_AWB_BG */
		{
			.cfg                      = 0x00001200,
			.image_addr               = 0x00001204,
			.frame_incr               = 0x00001208,
			.image_cfg_0              = 0x0000120C,
			.image_cfg_1              = 0x00001210,
			.image_cfg_2              = 0x00001214,
			.packer_cfg               = 0x00001218,
			.frame_header_addr        = 0x00001220,
			.frame_header_incr        = 0x00001224,
			.frame_header_cfg         = 0x00001228,
			.irq_subsample_period     = 0x00001230,
			.irq_subsample_pattern    = 0x00001234,
			.framedrop_period         = 0x00001238,
			.framedrop_pattern        = 0x0000123C,
			.mmu_prefetch_cfg         = 0x00001260,
			.mmu_prefetch_max_offset  = 0x00001264,
			.system_cache_cfg         = 0x00001268,
			.addr_status_0            = 0x00001270,
			.addr_status_1            = 0x00001274,
			.addr_status_2            = 0x00001278,
			.addr_status_3            = 0x0000127C,
			.debug_status_cfg         = 0x00001280,
			.debug_status_0           = 0x00001284,
			.debug_status_1           = 0x00001288,
			.bw_limiter_addr          = 0x0000121C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_5,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 13 STATS_AWB_BFW */
		{
			.cfg                      = 0x00001300,
			.image_addr               = 0x00001304,
			.frame_incr               = 0x00001308,
			.image_cfg_0              = 0x0000130C,
			.image_cfg_1              = 0x00001310,
			.image_cfg_2              = 0x00001314,
			.packer_cfg               = 0x00001318,
			.frame_header_addr        = 0x00001320,
			.frame_header_incr        = 0x00001324,
			.frame_header_cfg         = 0x00001328,
			.irq_subsample_period     = 0x00001330,
			.irq_subsample_pattern    = 0x00001334,
			.framedrop_period         = 0x00001338,
			.framedrop_pattern        = 0x0000133C,
			.mmu_prefetch_cfg         = 0x00001360,
			.mmu_prefetch_max_offset  = 0x00001364,
			.system_cache_cfg         = 0x00001368,
			.addr_status_0            = 0x00001370,
			.addr_status_1            = 0x00001374,
			.addr_status_2            = 0x00001378,
			.addr_status_3            = 0x0000137C,
			.debug_status_cfg         = 0x00001380,
			.debug_status_0           = 0x00001384,
			.debug_status_1           = 0x00001388,
			.bw_limiter_addr          = 0x0000131C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_5,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 14 STATS_GTM_BHIST */
		{
			.cfg                      = 0x00001400,
			.image_addr               = 0x00001404,
			.frame_incr               = 0x00001408,
			.image_cfg_0              = 0x0000140C,
			.image_cfg_1              = 0x00001410,
			.image_cfg_2              = 0x00001414,
			.packer_cfg               = 0x00001418,
			.frame_header_addr        = 0x00001420,
			.frame_header_incr        = 0x00001424,
			.frame_header_cfg         = 0x00001428,
			.irq_subsample_period     = 0x00001430,
			.irq_subsample_pattern    = 0x00001434,
			.framedrop_period         = 0x00001438,
			.framedrop_pattern        = 0x0000143C,
			.mmu_prefetch_cfg         = 0x00001460,
			.mmu_prefetch_max_offset  = 0x00001464,
			.system_cache_cfg         = 0x00001468,
			.addr_status_0            = 0x00001470,
			.addr_status_1            = 0x00001474,
			.addr_status_2            = 0x00001478,
			.addr_status_3            = 0x0000147C,
			.debug_status_cfg         = 0x00001480,
			.debug_status_0           = 0x00001484,
			.debug_status_1           = 0x00001488,
			.bw_limiter_addr          = 0x0000141C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_6,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 15 STATS_FLICKER_RS */
		{
			.cfg                      = 0x00001500,
			.image_addr               = 0x00001504,
			.frame_incr               = 0x00001508,
			.image_cfg_0              = 0x0000150C,
			.image_cfg_1              = 0x00001510,
			.image_cfg_2              = 0x00001514,
			.packer_cfg               = 0x00001518,
			.frame_header_addr        = 0x00001520,
			.frame_header_incr        = 0x00001524,
			.frame_header_cfg         = 0x00001528,
			.irq_subsample_period     = 0x00001530,
			.irq_subsample_pattern    = 0x00001534,
			.framedrop_period         = 0x00001538,
			.framedrop_pattern        = 0x0000153C,
			.mmu_prefetch_cfg         = 0x00001560,
			.mmu_prefetch_max_offset  = 0x00001564,
			.system_cache_cfg         = 0x00001568,
			.addr_status_0            = 0x00001570,
			.addr_status_1            = 0x00001574,
			.addr_status_2            = 0x00001578,
			.addr_status_3            = 0x0000157C,
			.debug_status_cfg         = 0x00001580,
			.debug_status_0           = 0x00001584,
			.debug_status_1           = 0x00001588,
			.bw_limiter_addr          = 0x0000151C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_7,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 16 RDI 0 */
		{
			.cfg                      = 0x00001600,
			.image_addr               = 0x00001604,
			.frame_incr               = 0x00001608,
			.image_cfg_0              = 0x0000160C,
			.image_cfg_1              = 0x00001610,
			.image_cfg_2              = 0x00001614,
			.packer_cfg               = 0x00001618,
			.frame_header_addr        = 0x00001620,
			.frame_header_incr        = 0x00001624,
			.frame_header_cfg         = 0x00001628,
			.irq_subsample_period     = 0x00001630,
			.irq_subsample_pattern    = 0x00001634,
			.framedrop_period         = 0x00001638,
			.framedrop_pattern        = 0x0000163C,
			.mmu_prefetch_cfg         = 0x00001660,
			.mmu_prefetch_max_offset  = 0x00001664,
			.system_cache_cfg         = 0x00001668,
			.addr_status_0            = 0x00001670,
			.addr_status_1            = 0x00001674,
			.addr_status_2            = 0x00001678,
			.addr_status_3            = 0x0000167C,
			.debug_status_cfg         = 0x00001680,
			.debug_status_0           = 0x00001684,
			.debug_status_1           = 0x00001688,
			.bw_limiter_addr          = 0x0000161C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_8,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 17 RDI 1 */
		{
			.cfg                      = 0x00001700,
			.image_addr               = 0x00001704,
			.frame_incr               = 0x00001708,
			.image_cfg_0              = 0x0000170C,
			.image_cfg_1              = 0x00001710,
			.image_cfg_2              = 0x00001714,
			.packer_cfg               = 0x00001718,
			.frame_header_addr        = 0x00001720,
			.frame_header_incr        = 0x00001724,
			.frame_header_cfg         = 0x00001728,
			.irq_subsample_period     = 0x00001730,
			.irq_subsample_pattern    = 0x00001734,
			.framedrop_period         = 0x00001738,
			.framedrop_pattern        = 0x0000173C,
			.mmu_prefetch_cfg         = 0x00001760,
			.mmu_prefetch_max_offset  = 0x00001764,
			.system_cache_cfg         = 0x00001768,
			.addr_status_0            = 0x00001770,
			.addr_status_1            = 0x00001774,
			.addr_status_2            = 0x00001778,
			.addr_status_3            = 0x0000177C,
			.debug_status_cfg         = 0x00001780,
			.debug_status_0           = 0x00001784,
			.debug_status_1           = 0x00001788,
			.bw_limiter_addr          = 0x0000171C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_9,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 18 RDI 2 */
		{
			.cfg                      = 0x00001800,
			.image_addr               = 0x00001804,
			.frame_incr               = 0x00001808,
			.image_cfg_0              = 0x0000180C,
			.image_cfg_1              = 0x00001810,
			.image_cfg_2              = 0x00001814,
			.packer_cfg               = 0x00001818,
			.frame_header_addr        = 0x00001820,
			.frame_header_incr        = 0x00001824,
			.frame_header_cfg         = 0x00001828,
			.irq_subsample_period     = 0x00001830,
			.irq_subsample_pattern    = 0x00001834,
			.framedrop_period         = 0x00001838,
			.framedrop_pattern        = 0x0000183C,
			.mmu_prefetch_cfg         = 0x00001860,
			.mmu_prefetch_max_offset  = 0x00001864,
			.system_cache_cfg         = 0x00001868,
			.addr_status_0            = 0x00001870,
			.addr_status_1            = 0x00001874,
			.addr_status_2            = 0x00001878,
			.addr_status_3            = 0x0000187C,
			.debug_status_cfg         = 0x00001880,
			.debug_status_0           = 0x00001884,
			.debug_status_1           = 0x00001888,
			.bw_limiter_addr          = 0x0000181C,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_10,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 19 AI OUT 1 Y C0 */
		{
			.cfg                      = 0x00001900,
			.image_addr               = 0x00001904,
			.frame_incr               = 0x00001908,
			.image_cfg_0              = 0x0000190C,
			.image_cfg_1              = 0x00001910,
			.image_cfg_2              = 0x00001914,
			.packer_cfg               = 0x00001918,
			.frame_header_addr        = 0x00001920,
			.frame_header_incr        = 0x00001924,
			.frame_header_cfg         = 0x00001928,
			.irq_subsample_period     = 0x00001930,
			.irq_subsample_pattern    = 0x00001934,
			.framedrop_period         = 0x00001938,
			.framedrop_pattern        = 0x0000193C,
			.mmu_prefetch_cfg         = 0x00001960,
			.mmu_prefetch_max_offset  = 0x00001964,
			.system_cache_cfg         = 0x00001968,
			.addr_status_0            = 0x00001970,
			.addr_status_1            = 0x00001974,
			.addr_status_2            = 0x00001978,
			.addr_status_3            = 0x0000197C,
			.debug_status_cfg         = 0x00001980,
			.debug_status_0           = 0x00001984,
			.debug_status_1           = 0x00001988,
			.bw_limiter_addr          = 0x0000191C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_19,
		},
		/* BUS Client 20 AI OUT 1 UV C1 */
		{
			.cfg                      = 0x00001A00,
			.image_addr               = 0x00001A04,
			.frame_incr               = 0x00001A08,
			.image_cfg_0              = 0x00001A0C,
			.image_cfg_1              = 0x00001A10,
			.image_cfg_2              = 0x00001A14,
			.packer_cfg               = 0x00001A18,
			.frame_header_addr        = 0x00001A20,
			.frame_header_incr        = 0x00001A24,
			.frame_header_cfg         = 0x00001A28,
			.irq_subsample_period     = 0x00001A30,
			.irq_subsample_pattern    = 0x00001A34,
			.framedrop_period         = 0x00001A38,
			.framedrop_pattern        = 0x00001A3C,
			.mmu_prefetch_cfg         = 0x00001A60,
			.mmu_prefetch_max_offset  = 0x00001A64,
			.system_cache_cfg         = 0x00001A68,
			.addr_status_0            = 0x00001A70,
			.addr_status_1            = 0x00001A74,
			.addr_status_2            = 0x00001A78,
			.addr_status_3            = 0x00001A7C,
			.debug_status_cfg         = 0x00001A80,
			.debug_status_0           = 0x00001A84,
			.debug_status_1           = 0x00001A88,
			.bw_limiter_addr          = 0x00001A1C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_20,
		},
		/* BUS Client 21 AI OUT 1 C2 */
		{
			.cfg                      = 0x00001B00,
			.image_addr               = 0x00001B04,
			.frame_incr               = 0x00001B08,
			.image_cfg_0              = 0x00001B0C,
			.image_cfg_1              = 0x00001B10,
			.image_cfg_2              = 0x00001B14,
			.packer_cfg               = 0x00001B18,
			.frame_header_addr        = 0x00001B20,
			.frame_header_incr        = 0x00001B24,
			.frame_header_cfg         = 0x00001B28,
			.irq_subsample_period     = 0x00001B30,
			.irq_subsample_pattern    = 0x00001B34,
			.framedrop_period         = 0x00001B38,
			.framedrop_pattern        = 0x00001B3C,
			.mmu_prefetch_cfg         = 0x00001B60,
			.mmu_prefetch_max_offset  = 0x00001B64,
			.system_cache_cfg         = 0x00001B68,
			.addr_status_0            = 0x00001B70,
			.addr_status_1            = 0x00001B74,
			.addr_status_2            = 0x00001B78,
			.addr_status_3            = 0x00001B7C,
			.debug_status_cfg         = 0x00001B80,
			.debug_status_0           = 0x00001B84,
			.debug_status_1           = 0x00001B88,
			.bw_limiter_addr          = 0x00001B1C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = NULL,
		},
		/* BUS Client 22 AI OUT 2 Y C0 */
		{
			.cfg                      = 0x00001C00,
			.image_addr               = 0x00001C04,
			.frame_incr               = 0x00001C08,
			.image_cfg_0              = 0x00001C0C,
			.image_cfg_1              = 0x00001C10,
			.image_cfg_2              = 0x00001C14,
			.packer_cfg               = 0x00001C18,
			.frame_header_addr        = 0x00001C20,
			.frame_header_incr        = 0x00001C24,
			.frame_header_cfg         = 0x00001C28,
			.irq_subsample_period     = 0x00001C30,
			.irq_subsample_pattern    = 0x00001C34,
			.framedrop_period         = 0x00001C38,
			.framedrop_pattern        = 0x00001C3C,
			.mmu_prefetch_cfg         = 0x00001C60,
			.mmu_prefetch_max_offset  = 0x00001C64,
			.system_cache_cfg         = 0x00001C68,
			.addr_status_0            = 0x00001C70,
			.addr_status_1            = 0x00001C74,
			.addr_status_2            = 0x00001C78,
			.addr_status_3            = 0x00001C7C,
			.debug_status_cfg         = 0x00001C80,
			.debug_status_0           = 0x00001C84,
			.debug_status_1           = 0x00001C88,
			.bw_limiter_addr          = 0x00001C1C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_22,
		},
		/* BUS Client 23 AI OUT 2 UV C1 */
		{
			.cfg                      = 0x00001D00,
			.image_addr               = 0x00001D04,
			.frame_incr               = 0x00001D08,
			.image_cfg_0              = 0x00001D0C,
			.image_cfg_1              = 0x00001D10,
			.image_cfg_2              = 0x00001D14,
			.packer_cfg               = 0x00001D18,
			.frame_header_addr        = 0x00001D20,
			.frame_header_incr        = 0x00001D24,
			.frame_header_cfg         = 0x00001D28,
			.irq_subsample_period     = 0x00001D30,
			.irq_subsample_pattern    = 0x00001D34,
			.framedrop_period         = 0x00001D38,
			.framedrop_pattern        = 0x00001D3C,
			.mmu_prefetch_cfg         = 0x00001D60,
			.mmu_prefetch_max_offset  = 0x00001D64,
			.system_cache_cfg         = 0x00001D68,
			.addr_status_0            = 0x00001D70,
			.addr_status_1            = 0x00001D74,
			.addr_status_2            = 0x00001D78,
			.addr_status_3            = 0x00001D7C,
			.debug_status_cfg         = 0x00001D80,
			.debug_status_0           = 0x00001D84,
			.debug_status_1           = 0x00001D88,
			.bw_limiter_addr          = 0x00001D1C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = &vfe690_ubwc_regs_client_23,
		},
		/* BUS Client 24 AI OUT 2 C2 */
		{
			.cfg                      = 0x00001E00,
			.image_addr               = 0x00001E04,
			.frame_incr               = 0x00001E08,
			.image_cfg_0              = 0x00001E0C,
			.image_cfg_1              = 0x00001E10,
			.image_cfg_2              = 0x00001E14,
			.packer_cfg               = 0x00001E18,
			.frame_header_addr        = 0x00001E20,
			.frame_header_incr        = 0x00001E24,
			.frame_header_cfg         = 0x00001E28,
			.irq_subsample_period     = 0x00001E30,
			.irq_subsample_pattern    = 0x00001E34,
			.framedrop_period         = 0x00001E38,
			.framedrop_pattern        = 0x00001E3C,
			.mmu_prefetch_cfg         = 0x00001E60,
			.mmu_prefetch_max_offset  = 0x00001E64,
			.system_cache_cfg         = 0x00001E68,
			.addr_status_0            = 0x00001E70,
			.addr_status_1            = 0x00001E74,
			.addr_status_2            = 0x00001E78,
			.addr_status_3            = 0x00001E7C,
			.debug_status_cfg         = 0x00001E80,
			.debug_status_0           = 0x00001E84,
			.debug_status_1           = 0x00001E88,
			.bw_limiter_addr          = 0x00001E1C,
			.comp_group              = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = NULL,
		},
	},
	.num_out = 18,
	.vfe_out_hw_info = {
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_1,
			.mid[0]        = 18,
			.num_wm        = 1,
			.line_based    = 1,
			.wm_idx        = {
				16,
			},
			.name          = {
				"RDI_0",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_2,
			.mid[0]        = 19,
			.num_wm        = 1,
			.line_based    = 1,
			.wm_idx        = {
				17,
			},
			.name          = {
				"RDI_1",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_3,
			.mid[0]        = 20,
			.num_wm        = 1,
			.line_based    = 1,
			.wm_idx        = {
				18,
			},
			.name          = {
				"RDI_2",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_FULL,
			.max_width     = 4096,
			.max_height    = 4096,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 8,
			.mid[1]        = 9,
			.mid[2]        = 10,
			.mid[3]        = 11,
			.num_wm        = 3,
			.wm_idx        = {
				0,
				1,
				2,
			},
			.name          = {
				"VID_FULL_YC0",
				"VID_FULL_UVC1",
				"VID_FULL_C2",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_FULL_DISP,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 32,
			.num_wm        = 2,
			.wm_idx        = {
				3,
				4,
			},
			.name          = {
				"DISP_FULL_Y",
				"DISP_FULL_C",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_DS4_DISP,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 32,
			.num_wm        = 1,
			.wm_idx        = {
				5,
			},
			.name          = {
				"DISP DS_4",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_DS16_DISP,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 33,
			.num_wm        = 1,
			.wm_idx        = {
				6,
			},
			.name          = {
				"DISP DS_16",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RAW_DUMP,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 16,
			.mid[1]        = 17,
			.num_wm        = 1,
			.wm_idx        = {
				7,
			},
			.name          = {
				"PIXEL_RAW",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_LTM_STATS,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 49,
			.mid[1]        = 50,
			.num_wm        = 1,
			.wm_idx        = {
				8,
			},
			.name          = {
				"LTM",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AEC_BE,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 39,
			.num_wm        = 1,
			.wm_idx        = {
				9,
			},
			.name          = {
				"AEC_BE",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_STATS_BHIST,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 40,
			.num_wm        = 1,
			.wm_idx        = {
				10,
			},
			.name          = {
				"STATS_BHIST",
			},
		},
		{
			.vfe_out_type  =
				CAM_VFE_BUS_VER3_VFE_OUT_STATS_TL_BG,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 41,
			.num_wm        = 1,
			.wm_idx        = {
				11,
			},
			.name          = {
				"STATS_TL_BG",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AWB_BG,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 42,
			.num_wm        = 1,
			.wm_idx        = {
				12,
			},
			.name          = {
				"STATS_AWB_BGB",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_AWB_BFW,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 43,
			.num_wm        = 1,
			.wm_idx        = {
				13,
			},
			.name          = {
				"AWB_BFW",
			},
		},
		{
			.vfe_out_type  =
				CAM_VFE_BUS_VER3_VFE_OUT_STATS_GTM_BHIST,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 45,
			.num_wm        = 1,
			.wm_idx        = {
				14,
			},
			.name          = {
				"GTM_BHIST",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_STATS_RS,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 46,
			.num_wm        = 1,
			.wm_idx        = {
				15,
			},
			.name          = {
				"STATS_RS",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_AIOUT1,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 46,
			.num_wm        = 3,
			.wm_idx        = {
				19,
				20,
				21,
			},
			.name          = {
				"AIOUT1_YC0",
				"AIOUT1_UVC1",
				"AIOUT1_C2",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_AIOUT2,
			.max_width     = 1920,
			.max_height    = 1080,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_0,
			.mid[0]        = 46,
			.num_wm        = 3,
			.wm_idx        = {
				22,
				23,
				24,
			},
			.name          = {
				"AIOUT2_YC0",
				"AIOUT2_UVC1",
				"AIOUT2_C2",
			},
		},
	},
	.num_cons_err = 29,
	.constraint_error_list = {
		{
			.bitmask = BIT(0),
			.error_description = "PPC 1x1 input not supported"
		},
		{
			.bitmask = BIT(1),
			.error_description = "PPC 1x2 input not supported"
		},
		{
			.bitmask = BIT(2),
			.error_description = "PPC 2x1 input not supported"
		},
		{
			.bitmask = BIT(3),
			.error_description = "PPC 2x2 input not supported"
		},
		{
			.bitmask = BIT(4),
			.error_description = "Pack 8 BPP format not supported"
		},
		{
			.bitmask = BIT(5),
			.error_description = "Pack 16 format not supported"
		},
		{
			.bitmask = BIT(6),
			.error_description = "Pack 32 BPP format not supported"
		},
		{
			.bitmask = BIT(7),
			.error_description = "Pack 64 BPP format not supported"
		},
		{
			.bitmask = BIT(8),
			.error_description = "Pack MIPI 20 format not supported"
		},
		{
			.bitmask = BIT(9),
			.error_description = "Pack MIPI 14 format not supported"
		},
		{
			.bitmask = BIT(10),
			.error_description = "Pack MIPI 12 format not supported"
		},
		{
			.bitmask = BIT(11),
			.error_description = "Pack MIPI 10 format not supported"
		},
		{
			.bitmask = BIT(12),
			.error_description = "Pack 128 BPP format not supported"
		},
		{
			.bitmask = BIT(13),
			.error_description = "UBWC NV12 format not supported"
		},
		{
			.bitmask = BIT(14),
			.error_description = "UBWC NV12 4R format not supported"
		},
		{
			.bitmask = BIT(15),
			.error_description = "UBWC TP10 format not supported"
		},
		{
			.bitmask = BIT(16),
			.error_description = "Frame based Mode not supported"
		},
		{
			.bitmask = BIT(17),
			.error_description = "Index based Mode not supported"
		},
		{
			.bitmask = BIT(18),
			.error_description = "FIFO image addr unalign"
		},
		{
			.bitmask = BIT(19),
			.error_description = "FIFO ubwc addr unalign"
		},
		{
			.bitmask = BIT(20),
			.error_description = "FIFO frmheader addr unalign"
		},
		{
			.bitmask = BIT(21),
			.error_description = "Image address unalign"
		},
		{
			.bitmask = BIT(22),
			.error_description = "UBWC address unalign"
		},
		{
			.bitmask = BIT(23),
			.error_description = "Frame Header address unalign"
		},
		{
			.bitmask = BIT(24),
			.error_description = "Stride unalign"
		},
		{
			.bitmask = BIT(25),
			.error_description = "X Initialization unalign"
		},
		{
			.bitmask = BIT(26),
			.error_description = "Image Width unalign"
		},
		{
			.bitmask = BIT(27),
			.error_description = "Image Height unalign"
		},
		{
			.bitmask = BIT(28),
			.error_description = "Meta Stride unalign"
		},
	},
	.bus_error_irq_mask = {
		0xD0000000,
		0x00000000,
	},
	.num_comp_grp          = 11,
	.support_consumed_addr = true,
	.comp_done_shift       = 0,
	.top_irq_shift         = 0,
	.max_out_res           = CAM_ISP_IFE_OUT_RES_BASE + 33, // TODO check for LeMans
	.pack_align_shift      = 5,
	.max_bw_counter_limit  = 0xFF,
	.support_tunneling = false,
	.fifo_depth = 2,
};

static struct cam_vfe_irq_hw_info vfe690_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_EXT_CSID,
	.top_irq_reg   = &vfe690_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_vfe690_hw_info = {
	.irq_hw_info                  = &vfe690_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &vfe690_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &vfe690_top_hw_info,
};

#endif /* _CAM_VFE690_H_ */
