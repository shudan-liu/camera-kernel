/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CPASTOP_V690_100_H_
#define _CPASTOP_V690_100_H_

#define TEST_IRQ_ENABLE 0

static struct cam_camnoc_irq_sbm cam_cpas_v690_100_irq_sbm = {
	.sbm_enable = {
		.access_type = CAM_REG_TYPE_READ_WRITE,
		.enable = true,
		.offset = 0x240, /* CAM_NOC_SBM_FAULTINEN0_LOW */
		.value = 0x4 |    /* SBM_FAULTINEN0_LOW_PORT2_MASK */
			0x10 |    /* SBM_FAULTINEN0_LOW_PORT4_MASK */
			0x20 |    /* SBM_FAULTINEN0_LOW_PORT5_MASK */
			(TEST_IRQ_ENABLE ?
			0x80 :    /* SBM_FAULTINEN0_LOW_PORT7_MASK */
			0x0),
	},
	.sbm_status = {
		.access_type = CAM_REG_TYPE_READ,
		.enable = true,
		.offset = 0x248, /* CAM_NOC_SBM_FAULTINSTATUS0_LOW */
	},
	.sbm_clear = {
		.access_type = CAM_REG_TYPE_WRITE,
		.enable = true,
		.offset = 0x280, /* CAM_NOC_SBM_FLAGOUTCLR0_LOW */
		.value = TEST_IRQ_ENABLE ? 0x5 : 0x0,
	}
};

static struct cam_camnoc_irq_err
	cam_cpas_v690_100_irq_err[] = {
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_SLAVE_ERROR,
		.enable = false,
		.sbm_port = 0x1, /* SBM_FAULTINSTATUS0_LOW_PORT0_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x8, /* CAM_NOC_ERL_MAINCTL_LOW */
			.value = 1,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x10, /* CAM_NOC_ERL_ERRVLD_LOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x18, /* CAM_NOC_ERL_ERRCLR_LOW */
			.value = 1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IFE_UBWC_ENCODE_ERROR,
		.enable = true,
		.sbm_port = 0x20, /* SBM_FAULTINSTATUS0_LOW_PORT5_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x67A0, /* CAM_NOC_RT_2_NIU_ENCERREN_lOW */
			.value = 0xF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x6790, /* CAM_NOC_RT_2_NIU_ENCERRSTATUS_lOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x6798, /* CAM_NOC_RT_2_NIU_ENCERRCLR_lOW */
			.value = 0X1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE0_UBWC_DECODE_ERROR,
		.enable = true,
		.sbm_port = 0x4, /* SBM_FAULTINSTATUS0_LOW_PORT2_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x5F20, /* CAM_NOC_NRT_1_NIU_DECERREN_lOW */
			.value = 0xFF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x5F10, /* CAM_NOC_NRT_1_NIU_DECERRSTATUS_lOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x5F18, /* CAM_NOC_NRT_1_NIU_DECERRCLR_lOW */
			.value = 0X1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE_UBWC_ENCODE_ERROR,
		.enable = true,
		.sbm_port = 0x10, /* SBM_FAULTINSTATUS0_LOW_PORT4_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x61A0, /* CAM_NOC_NRT_2_NIU_ENCERREN_lOW */
			.value = 0XF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x6190, /* CAM_NOC_NRT_2_NIU_ENCERRSTATUS_LOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x6198, /* CAM_NOC_NRT_2_NIU_ENCERRCLR_lOW */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_AHB_TIMEOUT,
		.enable = false,
		.sbm_port = 0x40, /* SBM_FAULTINSTATUS0_LOW_PORT6_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x288, /* CAM_NOC_SBM_FLAGOUTSET0_LOW */
			.value = 0x1,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x290, /* CAM_NOC_SBM_FLAGOUTSTATUS0_LOW */
		},
		.err_clear = {
			.enable = false, /* CAM_NOC_SBM_FLAGOUTCLR0_LOW */
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED1,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED2,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_CAMNOC_TEST,
		.enable = TEST_IRQ_ENABLE ? true : false,
		.sbm_port = 0x80, /* SBM_FAULTINSTATUS0_LOW_PORT7_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x288, /* CAM_NOC_SBM_FLAGOUTSET0_LOW */
			.value = 0x5,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x290, /* CAM_NOC_SBM_FLAGOUTSTATUS0_LOW */
		},
		.err_clear = {
			.enable = false, /* CAM_NOC_SBM_FLAGOUTCLR0_LOW */
		},
	},
};

static struct cam_camnoc_specific
	cam_cpas_v690_100_camnoc_specific[] = {
	{
		.port_type = CAM_CAMNOC_IFE_RDI_WR,
		.port_name = "IFE_RDI_WR",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6430, /* CAM_NOC_RT_1_NIU_PRIORITYLUT_LOW */
			.value = 0x66665433,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6434, /* CAM_NOC_RT_1_NIU_PRIORITYLUT_HIGH */
			.value = 0x66666666,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6438, /* CAM_NOC_RT_1_NIU_URGENCY_LOW */
			.value = 0x1B30,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6440, /* CAM_NOC_RT_1_NIU_DANGERLUT_LOW */
			.value = 0xffffff00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6448, /* CAM_NOC_RT_1_NIU_SAFELUT_LOW */
			.value = 0x000f,
		},
		.ubwc_ctl = {
			/*
			 * Do not explicitly set ubwc config register.
			 * Power on default values are taking care of required
			 * register settings.
			 */
			.enable = false,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5208, /* CAM_NOC_RT_1_QOSGEN_MAINCTL_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5220, /* CAM_NOC_RT_1_QOSGEN_SHAPING_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5224, /* CAM_NOC_RT_1_QOSGEN_SHAPING_hIGH */
			.value = 0x0,
		},
		.maxwr_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ,
			.masked_value = 0,
			.offset = 0x6420, /* CAM_NOC_RT_1_NIU_MAXWR_lOW */
			.value = 0x0,
		},
	},
	{
		.port_type = CAM_CAMNOC_IFE_LINEAR_STATS,
		.port_name = "IFE_LINEAR_STATS",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6630, /* CAM_NOC_RT_2_NIU_PRIORITYLUT_LOW */
			.value = 0x66665433,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6634, /* CAM_NOC_RT_2_NIU_PRIORITYLUT_HIGH */
			.value = 0x66666666,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6638, /* CAM_NOC_RT_2_NIU_URGENCY_LOW */
			.value = 0x1B30,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6640, /* CAM_NOC_RT_2_NIU_DANGERLUT_LOW */
			.value = 0xffffff00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6648, /* CAM_NOC_RT_2_NIU_SAFELUT_LOW */
			.value = 0x000f,
		},
		.ubwc_ctl = {
			/*
			 * Do not explicitly set ubwc config register.
			 * Power on default values are taking care of required
			 * register settings.
			 */
			.enable = false,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5288, /* CAM_NOC_RT_2_QOSGEN_MAINCTL_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x52A0, /* CAM_NOC_RT_2_QOSGEN_SHAPING_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x52A4, /* CAM_NOC_RT_2_QOSGEN_SHAPING_hIGH */
			.value = 0x0,
		},
		.maxwr_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ,
			.masked_value = 0,
			.offset = 0x6620, /* CAM_NOC_RT_2_NIU_MAXWR_lOW */
			.value = 0x0,
		},
	},
	{
		.port_type = CAM_CAMNOC_SFE_RD,
		.port_name = "SFE_RD",
		.enable = true,
		.priority_lut_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6230, /* CAM_NOC_RT_0_NIU_PRIORITYLUT_LOW */
			.value = 0x0,
		},
		.priority_lut_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6234, /* CAM_NOC_RT_0_NIU_PRIORITYLUT_HIGH */
			.value = 0x0,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6238, /* CAM_NOC_RT_0_NIU_URGENCY_LOW */
			.value = 0x1003,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6240, /* CAM_NOC_RT_0_NIU_DANGERLUT_LOW */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6248, /* CAM_NOC_RT_0_NIU_SAFELUT_LOW */
			.value = 0x0,
		},
		.ubwc_ctl = {
			/*
			 * Do not explicitly set ubwc config register.
			 * Power on default values are taking care of required
			 * register settings.
			 */
			.enable = false,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5188, /* CAM_NOC_RT_0_QOSGEN_MAINCTL_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x51A0, /* CAM_NOC_RT_0_QOSGEN_SHAPING_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x51A4, /* CAM_NOC_RT_0_QOSGEN_SHAPING_hIGH */
			.value = 0x0,
		},
	},
	{
		.port_type = CAM_CAMNOC_IPE_WR,
		.port_name = "IPE_WR",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6030, /* CAM_NOC_NRT_2_NIU_PRIORITYLUT_LOW */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6034, /* CAM_NOC_NRT_2_NIU_PRIORITYLUT_HIGH */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6038, /* CAM_NOC_NRT_2_NIU_URGENCY_LOW */
			.value = 0x30,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6040, /* CAM_NOC_NRT_2_NIU_DANGERLUT_LOW */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x6048, /* CAM_NOC_NRT_2_NIU_SAFELUT_LOW */
			.value = 0x0,
		},
		.ubwc_ctl = {
			.enable = false,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5108, /* CAM_NOC_NRT_2_QOSGEN_MAINCTL_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5120, /* CAM_NOC_NRT_2_QOSGEN_SHAPING_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5124, /* CAM_NOC_NRT_2_QOSGEN_SHAPING_hIGH */
			.value = 0x0,
		},
		.maxwr_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ,
			.masked_value = 0,
			.offset = 0x6020, /* CAM_NOC_NRT_2_NIU_MAXWR_lOW */
			.value = 0x0,
		},
	},
	{
		.port_type = CAM_CAMNOC_IPE0_RD,
		.port_name = "IPE0_RD",
		.enable = true,
		.priority_lut_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5E30, /* CAM_NOC_NRT_1_NIU_PRIORITYLUT_LOW */
			.value = 0x0,
		},
		.priority_lut_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5E34, /* CAM_NOC_NRT_1_NIU_PRIORITYLUT_HIGH */
			.value = 0x0,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5E38, /* CAM_NOC_NRT_1_NIU_URGENCY_LOW */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5E40, /* CAM_NOC_NRT_1_NIU_DANGERLUT_LOW */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5E48, /* CAM_NOC_NRT_1_NIU_SAFELUT_LOW */
			.value = 0x0,
		},
		.ubwc_ctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5F08, /* CAM_NOC_NRT_1_NIU_DECCTL_LOW */
			.value = 1,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5088, /* CAM_NOC_NRT_1_QOSGEN_MAINCTL_lOW */
			.value = 0x2,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x50A0, /* CAM_NOC_NRT_1_QOSGEN_SHAPING_lOW */
			.value = 0x29292929,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x50A4, /* CAM_NOC_NRT_1_QOSGEN_SHAPING_hIGH */
			.value = 0x29292929,
		},
	},
	{
		.port_type = CAM_CAMNOC_CDM,
		.port_name = "CDM",
		.enable = true,
		.priority_lut_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5C30, /* CAM_NOC_NRT_0_NIU_PRIORITYLUT_LOW */
			.value = 0x0,
		},
		.priority_lut_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5C34, /* CAM_NOC_NRT_0_NIU_PRIORITYLUT_HIGH */
			.value = 0x0,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5C38, /* CAM_NOC_NRT_0_NIU_URGENCY_LOW */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5C40, /* CAM_NOC_NRT_0_NIU_DANGERLUT_LOW */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5C48, /* CAM_NOC_NRT_0_NIU_SAFELUT_LOW */
			.value = 0x0,
		},
		.ubwc_ctl = {
			.enable = false,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5008, /* CAM_NOC_NRT_0_QOSGEN_MAINCTL_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5020, /* CAM_NOC_NRT_0_QOSGEN_SHAPING_lOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5024, /* CAM_NOC_NRT_0_QOSGEN_SHAPING_hIGH */
			.value = 0x0,
		},
	},
	{
		.port_type = CAM_CAMNOC_ICP,
		.port_name = "ICP",
		.enable = true,
		.flag_out_set0_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_WRITE,
			.masked_value = 0,
			.offset = 0x2288,
			.value = 0x100000,
		},
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5308, /* ICP_QOSGEN_MAINCTL_LOW */
			.value = 0x0,
		},
		.qosgen_shaping_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5320, /* ICP_QOSGEN_SHAPING_LOW */
			.value = 0x0,
		},
		.qosgen_shaping_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0,
			.offset = 0x5324, /* ICP_QOSGEN_SHAPING_HIGH */
			.value = 0x0,
		},
	},
};

static struct cam_camnoc_err_logger_info cam690_cpas100_err_logger_offsets = {
	.mainctrl     =  0x8, /* CAM_NOC_ERL_MAINCTL_LOW */
	.errvld       =  0x10, /* CAM_NOC_ERL_ERRVLD_LOW */
	.errlog0_low  =  0x20, /* CAM_NOC_ERL_ERRLOG0_LOW */
	.errlog0_high =  0x24, /* CAM_NOC_ERL_ERRLOG0_HIGH */
	.errlog1_low  =  0x28, /* CAM_NOC_ERL_ERRLOG1_LOW */
	.errlog1_high =  0x2c, /* CAM_NOC_ERL_ERRLOG1_HIGH */
	.errlog2_low  =  0x30, /* CAM_NOC_ERL_ERRLOG2_LOW */
	.errlog2_high =  0x34, /* CAM_NOC_ERL_ERRLOG2_HIGH */
	.errlog3_low  =  0x38, /* CAM_NOC_ERL_ERRLOG3_LOW */
	.errlog3_high =  0x3c, /* CAM_NOC_ERL_ERRLOG3_HIGH */
};

static struct cam_cpas_hw_errata_wa_list cam690_cpas100_errata_wa_list = {
	.camnoc_flush_slave_pending_trans = {
		.enable = false,
		.data.reg_info = {
			.access_type = CAM_REG_TYPE_READ,
			.offset = 0x300, /* CAM_NOC_SBM_SENSEIN0_LOW */
			.mask = 0x0,
			.value = 0, /* expected to be 0 */
		},
	},
};

static struct cam_camnoc_info cam690_cpas100_camnoc_info = {
	.specific = &cam_cpas_v690_100_camnoc_specific[0],
	.specific_size = ARRAY_SIZE(cam_cpas_v690_100_camnoc_specific),
	.irq_sbm = &cam_cpas_v690_100_irq_sbm,
	.irq_err = &cam_cpas_v690_100_irq_err[0],
	.irq_err_size = ARRAY_SIZE(cam_cpas_v690_100_irq_err),
	.err_logger = &cam690_cpas100_err_logger_offsets,
	.errata_wa_list = &cam690_cpas100_errata_wa_list,
};

static struct cam_cpas_camnoc_qchannel cam690_cpas100_qchannel_info = {
	.qchannel_ctrl   = 0x5C,
	.qchannel_status = 0x60,
};
#endif /* _CPASTOP_V690_100_H_ */

