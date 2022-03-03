// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>

#include <dt-bindings/msm-camera.h>

#include "cam_compat.h"
#include "cam_csiphy_remote_core.h"
#include "cam_csiphy_remote_dev.h"
#include "cam_csiphy_remote_soc.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "cam_mem_mgr.h"
#include "cam_rpmsg.h"

#define IS_HELIOS_ENABLED false

int32_t cam_csiphy_remote_get_instance_offset(
	struct csiphy_remote_device *csiphy_dev,
	int32_t dev_handle)
{
	int32_t i = 0;

	for (i = 0; i < csiphy_dev->acquire_count; i++) {
		if (dev_handle ==
			csiphy_dev->csiphy_info[i].hdl_data.device_hdl)
			break;
	}

	return i;
}

static void cam_csiphy_remote_reset_phyconfig_param(struct csiphy_remote_device *csiphy_dev,
	int32_t index)
{
	CAM_DBG(CAM_CSIPHY_REMOTE, "Resetting phyconfig param at index: %d", index);
	csiphy_dev->csiphy_info[index].hdl_data.device_hdl = -1;

	kfree(csiphy_dev->payload[index].phy_lane_en_config.reg_settings);
	csiphy_dev->payload[index].phy_lane_en_config.reg_settings = NULL;

	kfree(csiphy_dev->payload[index].phy_lane_config.reg_settings);
	csiphy_dev->payload[index].phy_lane_config.reg_settings = NULL;

	kfree(csiphy_dev->payload[index].phy_reset_config.reg_settings);
	csiphy_dev->payload[index].phy_reset_config.reg_settings = NULL;
}

void cam_csiphy_remote_query_cap(struct csiphy_remote_device *csiphy_dev,
	struct cam_csiphy_remote_query_cap *csiphy_cap)
{
	struct cam_hw_soc_info *soc_info = &csiphy_dev->soc_info;

	csiphy_cap->version = csiphy_dev->hw_version;
	csiphy_cap->slot_info = soc_info->index;
}

static int cam_csiphy_remote_get_lane_enable(
	struct phy_info *phy_info,
	uint16_t lane_assign, uint32_t *lane_enable)
{
	uint32_t lane_select = 0;

	if (phy_info->phy_type == CPHY) {
		switch (lane_assign & 0xF) {
		case 0x0:
			lane_select |= CPHY_LANE_0;
			break;
		case 0x1:
			lane_select |= CPHY_LANE_1;
			break;
		case 0x2:
			lane_select |= CPHY_LANE_2;
			break;
		default:
			CAM_ERR(CAM_CSIPHY_REMOTE,
				"Wrong lane configuration for CPHY: %d",
				lane_assign);
			*lane_enable = 0;
			return -EINVAL;
		}
	} else {
		switch (lane_assign & 0xF) {
		case 0x0:
			lane_select |= DPHY_LANE_0;
			lane_select |= DPHY_CLK_LN;
			break;
		case 0x1:
			lane_select |= DPHY_LANE_1;
			lane_select |= DPHY_CLK_LN;
			break;
		case 0x2:
			lane_select |= DPHY_LANE_2;
			if (phy_info->combo_mode)
				lane_select |= DPHY_LANE_3;
			else
				lane_select |= DPHY_CLK_LN;
			break;
		case 0x3:
			if (phy_info->combo_mode) {
				CAM_ERR(CAM_CSIPHY_REMOTE,
					"Wrong lane configuration for DPHYCombo: %d",
					lane_assign);
				*lane_enable = 0;
				return -EINVAL;
			}
			lane_select |= DPHY_LANE_3;
			lane_select |= DPHY_CLK_LN;
			break;
		default:
			CAM_ERR(CAM_CSIPHY_REMOTE,
				"Wrong lane configuration for DPHY: %d",
				lane_assign);
			*lane_enable = 0;
			return -EINVAL;
		}
	}

	*lane_enable = lane_select;
	CAM_INFO(CAM_CSIPHY_REMOTE, "Lane enable: 0x%x", lane_select);

	return 0;
}

static int cam_csiphy_remote_sanitize_lane_cnt(struct phy_info *phy_info)
{
	uint8_t max_supported_lanes = 0;

	if (phy_info->combo_mode) {
		max_supported_lanes = 2;
	} else {
		if (phy_info->phy_type == CPHY)
			max_supported_lanes = 3;
		else
			max_supported_lanes = 4;
	}

	if (phy_info->lane_count <= 0 || phy_info->lane_count > max_supported_lanes) {
		CAM_ERR(CAM_CSIPHY_REMOTE,
			"wrong lane_cnt configuration: max supported lane_cnt: %u received lane_cnt: %u",
			max_supported_lanes, phy_info->lane_count);
		return -EINVAL;
	}

	return 0;
}

int32_t cam_csiphy_remote_cmd_buf_parser(struct csiphy_remote_device *csiphy_dev,
	struct cam_config_dev_cmd *cfg_dev, struct phy_info *phy_info)
{
	int                      rc = 0;
	uintptr_t                generic_ptr;
	uintptr_t                generic_pkt_ptr;
	struct cam_packet       *csl_packet = NULL;
	struct cam_cmd_buf_desc *cmd_desc = NULL;
	uint32_t                *cmd_buf = NULL;
	struct cam_csiphy_remote_info  *cam_cmd_csiphy_info = NULL;
	size_t                  len;
	size_t                  remain_len;

	if (!cfg_dev || !csiphy_dev) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Invalid Args");
		return -EINVAL;
	}

	rc = cam_mem_get_cpu_buf((int32_t) cfg_dev->packet_handle,
		&generic_pkt_ptr, &len);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Failed to get packet Mem address: %d", rc);
		return rc;
	}

	remain_len = len;
	if ((sizeof(struct cam_packet) > len) ||
		((size_t)cfg_dev->offset >= len - sizeof(struct cam_packet))) {
		CAM_ERR(CAM_CSIPHY_REMOTE,
			"Inval cam_packet struct size: %zu, len: %zu",
			 sizeof(struct cam_packet), len);
		rc = -EINVAL;
		return rc;
	}

	remain_len -= (size_t)cfg_dev->offset;
	csl_packet = (struct cam_packet *)
		(generic_pkt_ptr + (uint32_t)cfg_dev->offset);

	if (cam_packet_util_validate_packet(csl_packet,
		remain_len)) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Invalid packet params");
		rc = -EINVAL;
		return rc;
	}

	cmd_desc = (struct cam_cmd_buf_desc *)
		((uint32_t *)&csl_packet->payload +
		csl_packet->cmd_buf_offset / 4);

	rc = cam_mem_get_cpu_buf(cmd_desc->mem_handle,
		&generic_ptr, &len);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY_REMOTE,
			"Failed to get cmd buf Mem address : %d", rc);
		return rc;
	}

	if ((len < sizeof(struct cam_csiphy_remote_info)) ||
		(cmd_desc->offset > (len - sizeof(struct cam_csiphy_remote_info)))) {
		CAM_ERR(CAM_CSIPHY_REMOTE,
			"Not enough buffer provided for cam_cisphy_remote_info");
		rc = -EINVAL;
		return rc;
	}

	cmd_buf = (uint32_t *)generic_ptr;
	cmd_buf += cmd_desc->offset / 4;
	cam_cmd_csiphy_info = (struct cam_csiphy_remote_info *)cmd_buf;

	phy_info->lane_count = cam_cmd_csiphy_info->lane_cnt;
	phy_info->lane_assign =
		cam_cmd_csiphy_info->lane_assign;
	phy_info->combo_mode = (cam_cmd_csiphy_info->combo_mode != 0);
	phy_info->phy_type = cam_cmd_csiphy_info->csiphy_3phase ? CPHY : DPHY;
	phy_info->phy_id = cam_cmd_csiphy_info->phy_id;

	rc = cam_csiphy_remote_sanitize_lane_cnt(phy_info);
	if (rc)
		return rc;

	CAM_INFO(CAM_CSIPHY_REMOTE,
		"phy version:0x%x idx:%d",
		csiphy_dev->hw_version,
		csiphy_dev->soc_info.index);
	CAM_INFO(CAM_CSIPHY_REMOTE,
		"phy_type:%d, combo_mode:%d lane_count:%d, lane_assign:0x%x",
		phy_info->phy_type,
		phy_info->combo_mode,
		phy_info->lane_count,
		phy_info->lane_assign);

	return rc;
}

int cam_csiphy_remote_set_payload_header(
		struct phy_header *header,
		uint32_t opcode)
{
	if (header == NULL)
		return -EINVAL;

	CAM_RPMSG_SLAVE_SET_HDR_VERSION(&(header->hpkt_header), CAM_RPMSG_V1);
	CAM_RPMSG_SLAVE_SET_HDR_DIRECTION(&(header->hpkt_header), CAM_RPMSG_DIR_MASTER_TO_SLAVE);
	CAM_RPMSG_SLAVE_SET_HDR_NUM_PACKET(&(header->hpkt_header), 1);
	CAM_RPMSG_SLAVE_SET_HDR_PACKET_SZ(&(header->hpkt_header), header->size - 4);
	CAM_RPMSG_SLAVE_SET_PAYLOAD_TYPE(&(header->hpkt_preamble), opcode);
	CAM_RPMSG_SLAVE_SET_PAYLOAD_SIZE(&(header->hpkt_preamble), header->size - 8);
	return 0;
}

int32_t cam_csiphy_remote_prepare_payload(struct csiphy_remote_device *csiphy_dev,
	struct phy_info *phy_info, int32_t offset)
{
	int32_t  rc = 0;
	uint32_t lane_enable = 0;
	uint32_t lane_select = 0;
	uint32_t cmn_size = 0, rst_size = 0;
	uint16_t i = 0, lane_cfg_size = 0;
	uint16_t lane_assign = 0;
	uint8_t  lane_cnt;
	int      max_lanes = 0;
	uint8_t  lane_pos = 0;
	int      idx = 0;
	struct   csiphy_remote_reg_t *csiphy_common_reg = NULL;
	struct   csiphy_remote_reg_t *csiphy_reset_reg = NULL;
	struct   csiphy_remote_reg_t (*reg_array)[MAX_SETTINGS_PER_LANE];
	bool     is_3phase = false;
	ssize_t  payload_size = 0;
	struct   phy_payload *payload = &(csiphy_dev->payload[offset]);

	if (phy_info->phy_type == CPHY)
		is_3phase = true;

	/* PHY id */
	payload->phy_id = phy_info->phy_id;

	/* Select reg settings */

	if (phy_info->combo_mode) {
		if (is_3phase) {
			/* CPHY combo mode */
			if (csiphy_dev->ctrl_reg->csiphy_3ph_combo_reg) {
				reg_array = csiphy_dev->ctrl_reg
					->csiphy_3ph_combo_reg;
			} else {
				reg_array =
					csiphy_dev->ctrl_reg->csiphy_3ph_reg;
			}
			lane_cfg_size = csiphy_dev->ctrl_reg->csiphy_reg
				.csiphy_3ph_config_array_size;
			max_lanes = CAM_CSIPHY_MAX_CPHY_LANES;
		} else {
			/* DPHY combo mode */
			if (csiphy_dev->ctrl_reg->csiphy_2ph_combo_mode_reg) {
				reg_array = csiphy_dev
					->ctrl_reg->csiphy_2ph_combo_mode_reg;
			} else {
				CAM_WARN(CAM_CSIPHY_REMOTE,
					"DPHY combo mode reg settings not found");
				reg_array = csiphy_dev
					->ctrl_reg->csiphy_2ph_reg;
			}
			lane_cfg_size = csiphy_dev->ctrl_reg->csiphy_reg
					.csiphy_2ph_config_array_size;
			max_lanes = MAX_LANES;
		}
	} else {
		/* CPHY/DPHY mission mode */
		if (is_3phase) {
			reg_array = csiphy_dev->ctrl_reg->csiphy_3ph_reg;
			max_lanes = CAM_CSIPHY_MAX_CPHY_LANES;
			lane_cfg_size = csiphy_dev->ctrl_reg->csiphy_reg
				.csiphy_3ph_config_array_size;
		} else {
			reg_array = csiphy_dev->ctrl_reg->csiphy_2ph_reg;
			lane_cfg_size = csiphy_dev->ctrl_reg->csiphy_reg
				.csiphy_2ph_config_array_size;
			max_lanes = MAX_LANES;
		}
	}

	/* Calculate lane enable */

	lane_cnt = phy_info->lane_count;
	lane_assign = phy_info->lane_assign;

	while (lane_cnt--) {
		rc = cam_csiphy_remote_get_lane_enable(phy_info,
			(lane_assign & 0xF), &lane_select);
		if (rc) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Wrong lane configuration: %d",
				phy_info->lane_assign);
			lane_enable = 0;
			goto reset_settings;
		}
		lane_enable |= lane_select;
		lane_assign >>= 4;
	}

	cmn_size = csiphy_dev->ctrl_reg->csiphy_reg.csiphy_common_array_size;
	rst_size = csiphy_dev->ctrl_reg->csiphy_reg.csiphy_reset_array_size;

	/* Fill reg settings */

	payload->phy_lane_en_config.num_settings = 1;
	payload->phy_lane_en_config.reg_settings = kcalloc(
		1,
		sizeof(struct phy_reg_setting),
			GFP_KERNEL);

	payload->phy_lane_config.num_settings = cmn_size + lane_cfg_size * max_lanes;
	payload->phy_lane_config.reg_settings = kcalloc(
		payload->phy_lane_config.num_settings,
		sizeof(struct phy_reg_setting),
			GFP_KERNEL);

	payload->phy_reset_config.num_settings = rst_size;
	payload->phy_reset_config.reg_settings = kcalloc(
		rst_size,
		sizeof(struct phy_reg_setting),
			GFP_KERNEL);

	for (i = 0; i < cmn_size; i++) {
		csiphy_common_reg = &csiphy_dev->ctrl_reg->csiphy_common_reg[i];
		switch (csiphy_common_reg->csiphy_param_type) {
		case CSIPHY_REMOTE_LANE_ENABLE:
			payload->phy_lane_en_config.reg_settings[0].reg_addr = csiphy_common_reg->reg_addr;
			payload->phy_lane_en_config.reg_settings[0].reg_data = lane_enable;
			payload->phy_lane_en_config.reg_settings[0].delay = csiphy_common_reg->delay;
			/* Exclude from lane cfg */
			payload->phy_lane_config.num_settings--;
			break;
		case CSIPHY_REMOTE_DEFAULT_PARAMS:
			payload->phy_lane_config.reg_settings[idx].reg_addr = csiphy_common_reg->reg_addr;
			payload->phy_lane_config.reg_settings[idx].reg_data = csiphy_common_reg->reg_data;
			payload->phy_lane_config.reg_settings[idx++].delay = csiphy_common_reg->delay;
			break;
		case CSIPHY_REMOTE_2PH_REGS:
			if (!is_3phase) {
				payload->phy_lane_config.reg_settings[idx].reg_addr = csiphy_common_reg->reg_addr;
				payload->phy_lane_config.reg_settings[idx].reg_data = csiphy_common_reg->reg_data;
				payload->phy_lane_config.reg_settings[idx++].delay = csiphy_common_reg->delay;
			} else
				payload->phy_lane_config.num_settings--;
			break;
		case CSIPHY_REMOTE_3PH_REGS:
			if (is_3phase) {
				payload->phy_lane_config.reg_settings[idx].reg_addr = csiphy_common_reg->reg_addr;
				payload->phy_lane_config.reg_settings[idx].reg_data = csiphy_common_reg->reg_data;
				payload->phy_lane_config.reg_settings[idx++].delay = csiphy_common_reg->delay;
			} else 
				payload->phy_lane_config.num_settings--;
			break;
		default:
			break;
		}
	}

	for (lane_pos = 0; lane_pos < max_lanes; lane_pos++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "lane_pos: %d is configuring", lane_pos);
		for (i = 0; i < lane_cfg_size; i++) {
			switch (reg_array[lane_pos][i].csiphy_param_type) {
			case CSIPHY_REMOTE_DEFAULT_PARAMS:
				payload->phy_lane_config.reg_settings[idx].reg_addr = reg_array[lane_pos][i].reg_addr;
				payload->phy_lane_config.reg_settings[idx].reg_data = reg_array[lane_pos][i].reg_data;
				payload->phy_lane_config.reg_settings[idx++].delay = reg_array[lane_pos][i].delay;
			break;
			default:
				payload->phy_lane_config.num_settings--;
			break;
			}
		}
	}

	idx = 0;

	for (i = 0; i < rst_size; i++) {
		csiphy_reset_reg = &csiphy_dev->ctrl_reg->csiphy_reset_regs[i];
		switch (csiphy_reset_reg->csiphy_param_type) {
		case CSIPHY_REMOTE_LANE_ENABLE:
		case CSIPHY_REMOTE_DEFAULT_PARAMS:
			payload->phy_reset_config.reg_settings[idx].reg_addr =
				csiphy_reset_reg->reg_addr;
			payload->phy_reset_config.reg_settings[idx].reg_data =
				csiphy_reset_reg->reg_data;
			payload->phy_reset_config.reg_settings[idx++].delay =
				csiphy_reset_reg->delay;
			break;
		default:
			break;
		}
	}

	/* Set header */

	payload_size = sizeof(struct phy_payload) +
		(sizeof(struct phy_reg_config) +
			(sizeof(struct phy_reg_setting) * payload->phy_lane_en_config.num_settings)) +
		(sizeof(struct phy_reg_config) +
			(sizeof(struct phy_reg_setting) * payload->phy_lane_config.num_settings)) +
		(sizeof(struct phy_reg_config) +
			(sizeof(struct phy_reg_setting) * payload->phy_reset_config.num_settings));

	payload->header.version = 0x1;
	payload->header.size    = payload_size;

	cam_csiphy_remote_set_payload_header(&(payload->header), HCM_PKT_OPCODE_CSIPHY_INIT_CONFIG);
	return rc;

reset_settings:
	cam_csiphy_remote_reset_phyconfig_param(csiphy_dev, offset);

	return rc;
}

void cam_csiphy_remote_shutdown(struct csiphy_remote_device *csiphy_dev)
{
	struct cam_hw_soc_info *soc_info;
	int i;

	soc_info = &csiphy_dev->soc_info;

	if (csiphy_dev->csiphy_state == CAM_CSIPHY_REMOTE_INIT)
		return;

	if (!csiphy_dev->acquire_count)
		return;

	if (csiphy_dev->acquire_count >= CSIPHY_MAX_INSTANCES_PER_PHY) {
		CAM_WARN(CAM_CSIPHY_REMOTE, "acquire count is invalid: %u",
			csiphy_dev->acquire_count);
		csiphy_dev->acquire_count =
			CSIPHY_MAX_INSTANCES_PER_PHY;
	}

	if (csiphy_dev->csiphy_state == CAM_CSIPHY_REMOTE_START) {
		for (i = 0; i < csiphy_dev->acquire_count; i++)
			cam_csiphy_remote_reset_phyconfig_param(csiphy_dev, i);

		csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_ACQUIRE;
	}

	if (csiphy_dev->csiphy_state == CAM_CSIPHY_REMOTE_ACQUIRE) {
		for (i = 0; i < csiphy_dev->acquire_count; i++) {
			if (csiphy_dev->csiphy_info[i].hdl_data.device_hdl
				!= -1)
				cam_destroy_device_hdl(
				csiphy_dev->csiphy_info[i]
				.hdl_data.device_hdl);
			csiphy_dev->csiphy_info[i].hdl_data.device_hdl = -1;
			csiphy_dev->csiphy_info[i].hdl_data.session_hdl = -1;
		}
	}

	csiphy_dev->acquire_count = 0;
	csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_INIT;

	cam_soc_util_disable_platform_resource(soc_info, true, true);
}

int cam_csiphy_remote_send_payload(
	struct phy_header *header)
{
	int handle, rc = 0;

	if (header == NULL)
		return -EINVAL;

	handle = cam_rpmsg_get_handle("helios");
	rc = cam_rpmsg_send(handle, header, header->size);
	return rc;
}

int cam_csiphy_remote_dump_payload_header(
		struct phy_header *header)
{
	if (header == NULL)
		return -EINVAL;

	CAM_INFO(CAM_CSIPHY_REMOTE, "version: 0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_VERSION(&(header->hpkt_header)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "direction: 0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_DIRECTION(&(header->hpkt_header)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "num_packet: 0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_NUM_PACKET(&(header->hpkt_header)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "packet_size: 0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_PACKET_SZ(&(header->hpkt_header)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "payload_type: 0x%x",
			CAM_RPMSG_SLAVE_GET_PAYLOAD_TYPE(&(header->hpkt_preamble)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "payload_size: 0x%x",
			CAM_RPMSG_SLAVE_GET_PAYLOAD_SIZE(&(header->hpkt_preamble)));
	CAM_INFO(CAM_CSIPHY_REMOTE, "header.version: 0x%x", header->version);
	CAM_INFO(CAM_CSIPHY_REMOTE, "header.size: 0x%x", header->size);
	return 0;
}

int cam_csiphy_remote_dump_payload_hex(
	struct phy_payload *payload)
{
	print_hex_dump(KERN_INFO, "CAM_INFO: CAM-CSIPHY-REMOTE: phy_lane_en_config.reg_settings:",
		DUMP_PREFIX_OFFSET,
	    16, 1,
		&(payload->phy_lane_en_config.reg_settings[0]),
		payload->phy_lane_en_config.num_settings * sizeof(struct phy_reg_setting),
		false);

	print_hex_dump(KERN_INFO, "CAM_INFO: CAM-CSIPHY-REMOTE: phy_lane_config.reg_settings:",
		DUMP_PREFIX_OFFSET,
	    16, 1,
		&(payload->phy_lane_config.reg_settings[0]),
		payload->phy_lane_config.num_settings * sizeof(struct phy_reg_setting),
		false);

	print_hex_dump(KERN_INFO, "CAM_INFO: CAM-CSIPHY-REMOTE: phy_reset_config.reg_settings:",
		DUMP_PREFIX_OFFSET,
	    16, 1,
		&(payload->phy_reset_config.reg_settings[0]),
		payload->phy_reset_config.num_settings * sizeof(struct phy_reg_setting),
		false);

	return 0;
}

int cam_csiphy_remote_regdump(
	struct phy_payload *payload)
{
	int i = 0;

	if (payload == NULL)
		return -EINVAL;

	CAM_DBG(CAM_CSIPHY_REMOTE, "Lane enable regdump:");

	for (i = 0; i < payload->phy_lane_en_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "0x%x = 0x%x",
			payload->phy_lane_en_config.reg_settings[i].reg_addr,
			payload->phy_lane_en_config.reg_settings[i].reg_data);
	}

	CAM_DBG(CAM_CSIPHY_REMOTE, "Lane regdump:");
	for (i = 0; i < payload->phy_lane_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "0x%x = 0x%x",
			payload->phy_lane_config.reg_settings[i].reg_addr,
			payload->phy_lane_config.reg_settings[i].reg_data);
	}

	CAM_DBG(CAM_CSIPHY_REMOTE, "Reset regdump:");
	for (i = 0; i < payload->phy_reset_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "0x%x = 0x%x",
			payload->phy_reset_config.reg_settings[i].reg_addr,
			payload->phy_reset_config.reg_settings[i].reg_data);
	}

	return 0;
}

int cam_csiphy_remote_dump_payload(
	struct phy_payload *payload)
{
	int i = 0;

	if (payload == NULL)
		return -EINVAL;

	cam_csiphy_remote_dump_payload_header(&payload->header);

	CAM_INFO(CAM_CSIPHY_REMOTE, "phy_id: %d", payload->phy_id);

	CAM_INFO(CAM_CSIPHY_REMOTE, "phy_lane_en_config.num_settings: %d",
		payload->phy_lane_en_config.num_settings);
	CAM_INFO(CAM_CSIPHY_REMOTE, "phy_lane_config.num_settings: %d",
		payload->phy_lane_config.num_settings);
	CAM_INFO(CAM_CSIPHY_REMOTE, "phy_reset_config.num_settings: %d",
		payload->phy_reset_config.num_settings);

	CAM_DBG(CAM_CSIPHY_REMOTE, "phy_lane_en_config.reg_settings:");

	for (i = 0; i < payload->phy_lane_en_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "%d: reg_addr:0x%x reg_data:0x%x delay:%d",
			i,
			payload->phy_lane_en_config.reg_settings[i].reg_addr,
			payload->phy_lane_en_config.reg_settings[i].reg_data,
			payload->phy_lane_en_config.reg_settings[i].delay);
	}

	CAM_DBG(CAM_CSIPHY_REMOTE, "phy_lane_config.reg_settings:");
	for (i = 0; i < payload->phy_lane_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "%d: reg_addr:0x%x reg_data:0x%x delay:%d",
			i,
			payload->phy_lane_config.reg_settings[i].reg_addr,
			payload->phy_lane_config.reg_settings[i].reg_data,
			payload->phy_lane_config.reg_settings[i].delay);
	}

	CAM_DBG(CAM_CSIPHY_REMOTE, "phy_reset_config.reg_settings:");
	for (i = 0; i < payload->phy_reset_config.num_settings; i++) {
		CAM_DBG(CAM_CSIPHY_REMOTE, "%d: reg_addr:0x%x reg_data:0x%x delay:%d",
			i,
			payload->phy_reset_config.reg_settings[i].reg_addr,
			payload->phy_reset_config.reg_settings[i].reg_data,
			payload->phy_reset_config.reg_settings[i].delay);
	}

	cam_csiphy_remote_dump_payload_hex(payload);

	return 0;
}

int32_t cam_csiphy_remote_core_cfg(void *phy_dev,
			void *arg)
{
	struct cam_control   *cmd = (struct cam_control *)arg;
	struct csiphy_remote_device *csiphy_dev = (struct csiphy_remote_device *)phy_dev;
	struct csiphy_remote_reg_parms_t *phy_reg;
	struct cam_hw_soc_info *soc_info;
	int32_t              rc = 0;

	if (!csiphy_dev || !cmd) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Invalid input args");
		return -EINVAL;
	}

	if (cmd->handle_type != CAM_HANDLE_USER_POINTER) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Invalid handle type: %d",
			cmd->handle_type);
		return -EINVAL;
	}

	soc_info = &csiphy_dev->soc_info;
	if (!soc_info) {
		CAM_ERR(CAM_CSIPHY_REMOTE, "Null Soc_info");
		return -EINVAL;
	}

	phy_reg = &csiphy_dev->ctrl_reg->csiphy_reg;
	CAM_INFO(CAM_CSIPHY_REMOTE, "Opcode received: %d", cmd->op_code);
	mutex_lock(&csiphy_dev->mutex);
	switch (cmd->op_code) {
	case CAM_ACQUIRE_DEV: {
		struct cam_sensorlite_acquire_dev csiphy_acq_dev;
		int index;
		struct cam_create_dev_hdl bridge_params;

		if ((csiphy_dev->acquire_count) &&
			(csiphy_dev->acquire_count >=
			CSIPHY_MAX_INSTANCES_PER_PHY)) {
			CAM_ERR(CAM_CSIPHY_REMOTE,
				"Exceeds max acquires allowed: %d", csiphy_dev->acquire_count);
			rc = -EINVAL;
			goto release_mutex;
		}

		rc = copy_from_user(&csiphy_acq_dev,
			u64_to_user_ptr(cmd->handle),
			sizeof(csiphy_acq_dev));
		if (rc < 0) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Failed copying from User");
			goto release_mutex;
		}

		bridge_params.ops = NULL;
		bridge_params.session_hdl = csiphy_acq_dev.session_handle;
		bridge_params.v4l2_sub_dev_flag = 0;
		bridge_params.media_entity_flag = 0;
		bridge_params.priv = csiphy_dev;
		bridge_params.dev_id = CAM_CSIPHY_REMOTE;
		index = csiphy_dev->acquire_count;
		csiphy_acq_dev.device_handle =
			cam_create_device_hdl(&bridge_params);
		if (csiphy_acq_dev.device_handle <= 0) {
			rc = -EFAULT;
			CAM_ERR(CAM_CSIPHY_REMOTE, "Can not create device handle");
			goto release_mutex;
		}

		csiphy_dev->csiphy_info[index].hdl_data.device_hdl =
			csiphy_acq_dev.device_handle;
		csiphy_dev->csiphy_info[index].hdl_data.session_hdl =
			csiphy_acq_dev.session_handle;

		CAM_DBG(CAM_CSIPHY_REMOTE, "Add dev_handle:0x%x at index: %d ",
			csiphy_dev->csiphy_info[index].hdl_data.device_hdl,
			index);
		if (copy_to_user(u64_to_user_ptr(cmd->handle),
				&csiphy_acq_dev,
				sizeof(struct cam_sensorlite_acquire_dev))) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Failed copying from User");
			rc = -EINVAL;
			goto release_mutex;
		}

		csiphy_dev->acquire_count++;

		if (csiphy_dev->csiphy_state == CAM_CSIPHY_REMOTE_INIT)
			csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_ACQUIRE;

		CAM_INFO(CAM_CSIPHY_REMOTE,
			"CAM_ACQUIRE_DEV: %u acquire_count: %d",
			soc_info->index,
			csiphy_dev->acquire_count);
	}
		break;
	case CAM_QUERY_CAP: {
		struct cam_csiphy_remote_query_cap csiphy_cap = {0};

		cam_csiphy_remote_query_cap(csiphy_dev, &csiphy_cap);
		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&csiphy_cap, sizeof(struct cam_csiphy_remote_query_cap))) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Failed copying from User");
			rc = -EINVAL;
			goto release_mutex;
		}
	}
		break;
	case CAM_STOP_DEV: {
		int32_t offset, rc = 0;
		struct cam_start_stop_dev_cmd config;

		rc = copy_from_user(&config, (void __user *)cmd->handle,
					sizeof(config));
		if (rc < 0) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Failed copying from User");
			goto release_mutex;
		}

		if (csiphy_dev->csiphy_state != CAM_CSIPHY_REMOTE_START) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Csiphy:%d Not in right state to stop : %d",
				soc_info->index, csiphy_dev->csiphy_state);
			goto release_mutex;
		}

		offset = cam_csiphy_remote_get_instance_offset(csiphy_dev,
			config.dev_handle);
		if (offset < 0 ||
			offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Index is invalid: %d", offset);
			goto release_mutex;
		}

		csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_ACQUIRE;

		CAM_INFO(CAM_CSIPHY_REMOTE,
			"CAM_STOP_PHYDEV: %u, slot: %d",
			soc_info->index,
			offset);
	}
		break;
	case CAM_RELEASE_DEV: {
		int32_t offset;
		struct cam_release_dev_cmd release;

		if (!csiphy_dev->acquire_count) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "No valid devices to release");
			rc = -EINVAL;
			goto release_mutex;
		}

		if (copy_from_user(&release,
			u64_to_user_ptr(cmd->handle),
			sizeof(release))) {
			rc = -EFAULT;
			goto release_mutex;
		}

		offset = cam_csiphy_remote_get_instance_offset(csiphy_dev,
			release.dev_handle);
		if (offset < 0 ||
			offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "index is invalid: %d", offset);
			goto release_mutex;
		}

		rc = cam_destroy_device_hdl(release.dev_handle);
		if (rc < 0)
			CAM_ERR(CAM_CSIPHY_REMOTE, "destroying the device hdl");
		csiphy_dev->csiphy_info[offset].hdl_data.device_hdl = -1;
		csiphy_dev->csiphy_info[offset].hdl_data.session_hdl = -1;

		cam_csiphy_remote_reset_phyconfig_param(csiphy_dev, offset);

		if (csiphy_dev->acquire_count) {
			csiphy_dev->acquire_count--;
			CAM_DBG(CAM_CSIPHY_REMOTE, "Acquire_cnt: %d",
				csiphy_dev->acquire_count);
		}

		if (csiphy_dev->acquire_count == 0) {
			CAM_INFO(CAM_CSIPHY_REMOTE, "All PHY devices released");
			csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_INIT;
		}

		CAM_INFO(CAM_CSIPHY_REMOTE, "CAM_RELEASE_PHYDEV: %u",
			soc_info->index);

		break;
	}
	case CAM_CONFIG_DEV: {
		int32_t offset = 0;
		struct cam_config_dev_cmd config;
		struct phy_info phy_in = {0};
		struct phy_payload *payload;

		if (copy_from_user(&config,
			u64_to_user_ptr(cmd->handle),
					sizeof(config))) {
			rc = -EFAULT;
		} else {
			rc = cam_csiphy_remote_cmd_buf_parser(csiphy_dev, &config, &phy_in);
			if (rc < 0) {
				CAM_ERR(CAM_CSIPHY_REMOTE, "Failed to parse cmd buf");
				goto release_mutex;
			}
		}

		offset = cam_csiphy_remote_get_instance_offset(csiphy_dev, config.dev_handle);
		if (offset < 0 || offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "index is invalid: %d", offset);
			goto release_mutex;
		}

		rc = cam_csiphy_remote_prepare_payload(csiphy_dev, &phy_in, offset);
		if (rc < 0) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Payload creation failed: %d", rc);
			goto release_mutex;
		}

		payload = &(csiphy_dev->payload[offset]);

		cam_csiphy_remote_regdump(payload);
		cam_csiphy_remote_dump_payload(payload);
#ifdef IS_HELIOS_ENABLED
		cam_csiphy_remote_send_payload(&(payload->header));
#endif
	}
		break;
	case CAM_START_DEV: {
		struct cam_start_stop_dev_cmd config;
		int32_t offset;

		rc = copy_from_user(&config, (void __user *)cmd->handle,
			sizeof(config));
		if (rc < 0) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "Failed copying from User");
			goto release_mutex;
		}

		offset = cam_csiphy_remote_get_instance_offset(csiphy_dev,
			config.dev_handle);
		if (offset < 0 ||
			offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
			CAM_ERR(CAM_CSIPHY_REMOTE, "index is invalid: %d", offset);
			goto release_mutex;
		}

		csiphy_dev->csiphy_state = CAM_CSIPHY_REMOTE_START;

		CAM_INFO(CAM_CSIPHY_REMOTE,
			"CAM_START_PHYDEV: %d, slot: %d",
			soc_info->index,
			offset);
	}
		break;
	default:
		CAM_ERR(CAM_CSIPHY_REMOTE, "Invalid Opcode: %d", cmd->op_code);
		rc = -EINVAL;
		goto release_mutex;
	}

	mutex_unlock(&csiphy_dev->mutex);
	return rc;

release_mutex:
	mutex_unlock(&csiphy_dev->mutex);

	return rc;
}
