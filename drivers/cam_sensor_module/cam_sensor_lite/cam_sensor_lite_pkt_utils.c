// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "cam_sensor_lite_pkt_utils.h"

#define SENSOR_LITE_INFO(fmt, args...) CAM_INFO(CAM_SENSOR_LITE, fmt, ##args)
#define SENSOR_LITE_DBG(fmt, args...)  CAM_DBG(CAM_SENSOR_LITE, fmt, ##args)
#define PROBE_RESPONSE_TIMEOUT 150

void __dump_slave_pkt_headers(
		struct sensor_lite_header *header)
{
	if (header == NULL)
		return;

	SENSOR_LITE_INFO("version       :    0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_VERSION(&(header->hpkt_header)));
	SENSOR_LITE_INFO("direction     :    0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_DIRECTION(&(header->hpkt_header)));
	SENSOR_LITE_INFO("num_packet    :    0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_NUM_PACKET(&(header->hpkt_header)));
	SENSOR_LITE_INFO("packet_size   :    0x%x",
			CAM_RPMSG_SLAVE_GET_HDR_PACKET_SZ(&(header->hpkt_header)));
	SENSOR_LITE_INFO("payload_type  :    0x%x",
			CAM_RPMSG_SLAVE_GET_PAYLOAD_TYPE(&(header->hpkt_preamble)));
	SENSOR_LITE_INFO("payload_size  :    0x%x",
			CAM_RPMSG_SLAVE_GET_PAYLOAD_SIZE(&(header->hpkt_preamble)));
	SENSOR_LITE_INFO("header.version : 0x%x", header->version);
	SENSOR_LITE_INFO("header.tag     : 0x%x", header->tag);
	SENSOR_LITE_INFO("header.size    : 0x%x", header->size);
}

int __set_slave_pkt_headers(
		struct sensor_lite_header *header,
		uint32_t                   opcode)
{
	if (header == NULL)
		return -EINVAL;

	CAM_RPMSG_SLAVE_SET_HDR_VERSION(&(header->hpkt_header), CAM_RPMSG_V1);
	CAM_RPMSG_SLAVE_SET_HDR_DIRECTION(&(header->hpkt_header), CAM_RPMSG_DIR_MASTER_TO_SLAVE);
	CAM_RPMSG_SLAVE_SET_HDR_NUM_PACKET(&(header->hpkt_header), 1);
	CAM_RPMSG_SLAVE_SET_HDR_PACKET_SZ(&(header->hpkt_header), header->size);
	CAM_RPMSG_SLAVE_SET_PAYLOAD_TYPE(&(header->hpkt_preamble), opcode);
	CAM_RPMSG_SLAVE_SET_PAYLOAD_SIZE(&(header->hpkt_preamble), header->size - 4);
	return 0;
}


static inline int __dump_slave_info(
	struct camera_slave_info     *slave_info)
{
	if (slave_info == NULL)
		return -EINVAL;

	SENSOR_LITE_INFO("slaveinfo.sensor_slave_addr  : 0x%x",
			slave_info->sensor_slave_addr);
	SENSOR_LITE_INFO("slaveinfo.sensor_id_reg_addr : 0x%x",
			slave_info->sensor_id_reg_addr);
	SENSOR_LITE_INFO("slaveinfo.sensor_id          : 0x%x",
			slave_info->sensor_id);
	SENSOR_LITE_INFO("slaveinfo.sensor_id_mask     : 0x%x",
			slave_info->sensor_id_mask);
	return 0;
}


static inline int __dump_pwr_settings(void *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct sensor_power_setting *pwr    = NULL;
	off_t  end_offset  = 0;
	int    i           = 0;

	pwr = (struct sensor_power_setting *)((uint8_t *)base + b_offset);
	end_offset = (off_t)((uint8_t *)base + max_size);
	for (i = 0; i < count; i++) {
		if ((off_t)(pwr + 1) > end_offset) {
			CAM_ERR(CAM_SENSOR_LITE, "dump overflow ");
			return -EINVAL;
		}

		SENSOR_LITE_INFO(
			"pwr[%d] seq_type: 0x%x seq_val: 0x%x config_val: 0x%x delay: 0x%x",
			i,
			pwr->seq_type,
			pwr->seq_val,
			pwr->config_val,
			pwr->delay);

		pwr++;
	}

	return 0;
}

static inline int __dump_pwr_on_settings_hex(void *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct sensor_power_setting *pwr    = NULL;

	pwr = (struct sensor_power_setting *)((uint8_t *)base + b_offset);
	print_hex_dump(KERN_INFO, "CAM_INFO: CAM-SENSORLITE: raw_pwr_on:", DUMP_PREFIX_OFFSET,
	     16, 1, pwr, count * sizeof(struct sensor_power_setting), false);
	return 0;
}

static inline int __dump_pwr_off_settings_hex(void *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct sensor_power_setting *pwr    = NULL;

	pwr = (struct sensor_power_setting *)((uint8_t *)base + b_offset);
	print_hex_dump(KERN_INFO, "CAM_INFO: CAM-SENSORLITE: raw_pwr_off:", DUMP_PREFIX_OFFSET,
	     16, 1, pwr, count * sizeof(struct sensor_power_setting), false);
	return 0;
}

int __copy_pwr_settings(
		void     *dst_ptr,
		void     *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct sensor_power_setting *pwr     = NULL;
	struct sensor_power_setting *dst     = NULL;
	off_t  end_offset  = 0;
	int    i           = 0;

	if ((base == NULL) || (base == NULL))
		return -EINVAL;

	dst = (struct sensor_power_setting *)dst_ptr;
	pwr = (struct sensor_power_setting *)((uint8_t *)base + b_offset);
	end_offset = (off_t)((uint8_t *)base + max_size);
	for (i = 0; i < count; i++) {
		if ((off_t)(pwr + 1) > end_offset) {
			CAM_ERR(CAM_SENSOR_LITE, "copy overflow ");
			return -EINVAL;
		}

		*dst = *pwr;
		dst++;
		pwr++;
	}

	return 0;
}

static inline int __dump_reg_settings(void *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct reg_setting *reg = NULL;
	off_t  end_offset       = 0;
	int    i                = 0;

	reg = (struct reg_setting *)((uint8_t *)base + b_offset);
	end_offset = (off_t)((uint8_t *)base + max_size);
	for (i = 0; i < count; i++) {
		if ((off_t)(reg + 1) > end_offset) {
			CAM_ERR(CAM_SENSOR_LITE, "dump overflow ");
			return -EINVAL;
		}

		SENSOR_LITE_INFO("reg[%d] slave-addr: 0x%x reg-addr: 0x%x reg-data: 0x%x",
				i,
				reg->slave_addr,
				reg->reg_addr,
				reg->reg_data);
		reg++;
	}

	return 0;
}

int __dump_probe_cmd(
	struct probe_payload_v2 *probe)
{
	if (probe == NULL)
		return -EINVAL;

	/* Append the rpmsg headers */
	__set_slave_pkt_headers(&probe->header, HCM_PKT_OPCODE_SENSOR_PROBE);
	__dump_slave_pkt_headers(&probe->header);
	__dump_slave_info(&probe->slave_info);
	CAM_INFO(CAM_SENSOR_LITE, "sensor_physical_id           : 0x%x",
			probe->sensor_physical_id);
	CAM_INFO(CAM_SENSOR_LITE, "power_up_settings_offset     : 0x%x",
			probe->power_up_settings_offset);
	CAM_INFO(CAM_SENSOR_LITE, "power_up_settings_size       : 0x%x",
			probe->power_up_settings_size);
	CAM_INFO(CAM_SENSOR_LITE, "power_down_settings_offset   : 0x%x",
			probe->power_down_settings_offset);
	CAM_INFO(CAM_SENSOR_LITE, "power_down_settings_size     : 0x%x",
			probe->power_down_settings_size);
	CAM_INFO(CAM_SENSOR_LITE, "size : 0x%x h: 0x%x si: 0x%x ",
			sizeof(*probe),
			sizeof(probe->header),
			sizeof(probe->slave_info));

	__dump_pwr_settings((void *)probe,
			probe->power_up_settings_offset,
			probe->power_up_settings_size,
			probe->header.size);
	__dump_pwr_settings((void *)probe,
			probe->power_down_settings_offset,
			probe->power_down_settings_size,
			probe->header.size);
	return 0;
}

int __dump_probe_cmd_hex(
	struct probe_payload_v2 *probe)
{

	if (probe == NULL)
		return -EINVAL;
	/* Append the rpmsg headers */
	__set_slave_pkt_headers(&probe->header, HCM_PKT_OPCODE_SENSOR_PROBE);
	__dump_slave_pkt_headers(&probe->header);
	__dump_slave_info(&probe->slave_info);
	CAM_INFO(CAM_SENSOR_LITE, "power_up_settings_offset     : 0x%x",
			probe->power_up_settings_offset);
	CAM_INFO(CAM_SENSOR_LITE, "power_up_settings_size       : 0x%x",
			probe->power_up_settings_size);
	CAM_INFO(CAM_SENSOR_LITE, "power_down_settings_offset   : 0x%x",
			probe->power_down_settings_offset);
	CAM_INFO(CAM_SENSOR_LITE, "power_down_settings_size     : 0x%x",
			probe->power_down_settings_size);
	CAM_INFO(CAM_SENSOR_LITE, "size : 0x%x h: 0x%x si: 0x%x ",
			sizeof(*probe), sizeof(probe->header), sizeof(probe->slave_info));

	__dump_pwr_on_settings_hex((void *)probe,
			probe->power_up_settings_offset,
			probe->power_up_settings_size,
			probe->header.size);
	__dump_pwr_off_settings_hex((void *)probe,
			probe->power_down_settings_offset,
			probe->power_down_settings_size,
			probe->header.size);
	return 0;
}

int __dump_host_dest_init_cmd(
	struct host_dest_camera_init_payload_v2 *init)
{

	/* Append the rpmsg headers */
	__set_slave_pkt_headers(&(init->header), HCM_PKT_OPCODE_SENSOR_INIT);
	__dump_slave_pkt_headers(&(init->header));

	CAM_INFO(CAM_SENSOR_LITE, "sensor_physical_id           : 0x%x",
			init->sensor_physical_id);
	CAM_INFO(CAM_SENSOR_LITE, "vc_map.version               : 0x%x",
			init->vc_map.version);
	CAM_INFO(CAM_SENSOR_LITE, "vc_map.sensor_vc             : 0x%x",
			init->vc_map.sensor_vc);
	CAM_INFO(CAM_SENSOR_LITE, "vc_map.aggregator_vc         : 0x%x",
			init->vc_map.aggregator_vc);

	/* TODO: validate offsets beyond size etc*/
	__dump_reg_settings((void *)init,
			init->init_setting_offset,
			init->init_setting_count,
			init->header.size);
	return 0;
}
int __dump_stream_config(
	struct sensorlite_stream_configuration *stream)
{
	if (stream == NULL)
		return -EINVAL;

	SENSOR_LITE_INFO("vc:0x%x dt:0x%x left:%d top: %d w:%d h:%d bpp:%d type:%d id: %d",
		stream->vc,
		stream->dt,
		stream->fd.x_start,
		stream->fd.y_start,
		stream->fd.width,
		stream->fd.height,
		stream->bit_width,
		stream->stream_type,
		stream->stream_configuration_id);

	return 0;
}

int __dump_resolution_cmd(
	struct sensorlite_resolution_cmd *cmd)
{
	int i = 0;

	__dump_slave_pkt_headers(&cmd->header);
	for (i = 0; i < cmd->stream_info.stream_configuration_count; i++)
		__dump_stream_config(&cmd->stream_info.stream_config[i]);

	/* dump group hold off settings */
	__dump_reg_settings((void *)cmd,
			cmd->mode_settings_offset,
			cmd->mode_settings_count,
			cmd->header.size);

	return 0;
}

static inline int __dump_resolutions(void *base,
		off_t    b_offset,
		uint32_t count,
		ssize_t  max_size)
{
	struct sensorlite_resolution_cmd *cmd = NULL;
	off_t  end_offset                     = 0;
	int    i                              = 0;

	if (base == NULL)
		return -EINVAL;

	cmd = (struct sensorlite_resolution_cmd *)((uint8_t *)base + b_offset);
	end_offset = (off_t)((uint8_t *)(base + max_size));

	for (i = 0; i < count; i++) {
		if (((off_t)((uint8_t *)cmd + cmd->header.size)) > end_offset) {
			CAM_ERR(CAM_SENSOR_LITE, "dump overflow ");
			return -EINVAL;
		}

		__dump_resolution_cmd(cmd);
		cmd = (struct sensorlite_resolution_cmd *)((uint8_t *)cmd + cmd->header.size);
	}

	return 0;
}

int __dump_slave_dest_init_cmd(
	struct slave_dest_camera_init_payload *init)
{

	if (init == NULL)
		return -EINVAL;
	/* Append the rpmsg headers */
	__set_slave_pkt_headers(&(init->header), HCM_PKT_OPCODE_SENSOR_INIT);
	__dump_slave_pkt_headers(&(init->header));


	CAM_INFO(CAM_SENSOR_LITE, "sensor_physical_id : 0x%x",
			init->sensor_physical_id);
	/* dump init settings */
	__dump_reg_settings((void *)init,
			init->init_settings_offset,
			init->init_settings_count,
			init->header.size);

	/* dump stream on settings */
	__dump_reg_settings((void *)init,
			init->streamon_settings_offset,
			init->streamon_settings_count,
			init->header.size);

	/* dump streamoff settings */
	__dump_reg_settings((void *)init,
			init->streamoff_settings_offset,
			init->streamoff_settings_count,
			init->header.size);

	/* dump group hold on settings */
	__dump_reg_settings((void *)init,
			init->groupholdon_settings_offset,
			init->groupholdon_settings_count,
			init->header.size);

	/* dump group hold off settings */
	__dump_reg_settings((void *)init,
			init->groupholdoff_settings_offset,
			init->groupholdoff_settings_count,
			init->header.size);

	/* dump resolutions */
	__dump_resolutions(init,
			init->resolution_data_offset,
			init->resolution_data_count,
			init->header.size);


	return 0;
}

int __dump_perframe_cmd(
	struct sensor_lite_perframe_cmd *pf_packet)
{
	if (pf_packet == NULL)
		return -EINVAL;
	/* Append the rpmsg headers */
	__set_slave_pkt_headers(&(pf_packet->header), HCM_PKT_OPCODE_SENSOR_CONFIG);
	__dump_slave_pkt_headers(&(pf_packet->header));

	return 0;
}

int __dump_acquire_cmd(
	struct sensor_lite_acquire_cmd *acquire)
{
	if (acquire == NULL)
		return -EINVAL;

	__dump_slave_pkt_headers(&(acquire->header));
	__dump_pwr_settings(acquire,
			acquire->power_settings_offset,
			acquire->power_settings_size,
			acquire->header.size);
	__dump_pwr_on_settings_hex(acquire,
			acquire->power_settings_offset,
			acquire->power_settings_size,
			acquire->header.size);
	return 0;
}

int __dump_release_cmd(
	struct sensor_lite_release_cmd *release)
{
	if (release == NULL)
		return -EINVAL;

	__dump_slave_pkt_headers(&(release->header));

	__dump_pwr_settings(release,
			release->power_settings_offset,
			release->power_settings_size,
			release->header.size);
	return 0;
}

int __dump_probe_response(struct sensor_probe_response *pr)
{
	CAM_INFO(CAM_SENSOR_LITE, "VT          : %d",
			pr->vt);
	CAM_INFO(CAM_SENSOR_LITE, "SID         : %d",
			pr->sensor_id);
	CAM_INFO(CAM_SENSOR_LITE, "STATUS      : %d",
			pr->status);
	CAM_INFO(CAM_SENSOR_LITE, "PID         : %d",
			pr->phy_info.phy_index);
	CAM_INFO(CAM_SENSOR_LITE, "PTYPE       : %d",
			pr->phy_info.pt);
	CAM_INFO(CAM_SENSOR_LITE, "COMBOMODE   : %d",
			pr->phy_info.combo_mode);
	CAM_INFO(CAM_SENSOR_LITE, "LANE_COUNT  : %d",
			pr->phy_info.num_lanes);
	return 0;
}

int __send_probe_pkt(
	struct sensor_lite_device *sensor_lite_dev,
	struct sensor_lite_header *header)
{
	int rc = 0;
	int handle;
	unsigned long rem_jiffies;

	reinit_completion(&(sensor_lite_dev->complete));
	if (header->tag != SENSORLITE_CMD_TYPE_PROBE) {
		CAM_ERR(CAM_SENSOR_LITE, "invalid command received to send");
		return -EINVAL;
	}

	memset(&sensor_lite_dev->probe_info, 0, sizeof(struct sensor_probe_response));
	sensor_lite_dev->probe_info.status = PROBE_FAILURE;

	handle = cam_rpmsg_get_handle("helios");
	__set_slave_pkt_headers(header, HCM_PKT_OPCODE_SENSOR_PROBE);
	rc = cam_rpmsg_send(handle, header, header->size);
	if (rc < 0) {
		CAM_ERR(CAM_SENSOR_LITE, "rpmsg send failed for probe packet");
		goto err;
	}

	CAM_INFO(CAM_SENSOR_LITE, "Waiting for probe response packet");
	rem_jiffies = wait_for_completion_timeout(
			&(sensor_lite_dev->complete),
			msecs_to_jiffies(PROBE_RESPONSE_TIMEOUT));
	if (!rem_jiffies) {
		rc = -ETIMEDOUT;
		CAM_ERR(CAM_RPMSG, "probe response timed out %d\n", rc);
		goto err;
	}

	/* Got probe response before timeout */
	if (sensor_lite_dev->probe_info.status == PROBE_SUCCESS) {
		CAM_INFO(CAM_SENSOR_LITE, "Probe success %d",
				sensor_lite_dev->soc_info.index);
		__dump_probe_response(&sensor_lite_dev->probe_info);
	} else {
		rc = -ETIMEDOUT;
		CAM_ERR(CAM_SENSOR_LITE, "Probe Failed for: %d",
				sensor_lite_dev->soc_info.index);
	}
err:
	return rc;
}

int __send_pkt(
	struct sensor_lite_header *header)
{
	int rc = 0;
#ifdef __HELIOS_ENABLED__
	int handle;

	handle = cam_rpmsg_get_handle("helios");
	rc = cam_rpmsg_send(handle, header, header->size);
#else
	switch (header->tag) {
	case SENSORLITE_CMD_TYPE_PROBE:
		__dump_probe_cmd(
			(struct probe_payload_v2 *)header);
		break;
	case SENSORLITE_CMD_TYPE_SLAVEDESTINIT:
		__dump_slave_dest_init_cmd(
			(struct slave_dest_camera_init_payload *)header);
		break;
	case SENSORLITE_CMD_TYPE_HOSTDESTINIT:
		__dump_host_dest_init_cmd(
			(struct host_dest_camera_init_payload_v2 *)header);
		break;
	case SENSORLITE_CMD_TYPE_EXPOSUREUPDATE:
		CAM_INFO(CAM_SENSOR_LITE, "Exposure update dump not enabled");
		break;
	case SENSORLITE_CMD_TYPE_RESOLUTIONINFO:
		__dump_resolution_cmd(
			(struct sensorlite_resolution_cmd *)header);
		break;
	case SENSORLITE_CMD_TYPE_DEBUG:
		CAM_INFO(CAM_SENSOR_LITE, "debug dump not enabled");
		break;
	case SENSORLITE_CMD_TYPE_PERFRAME:
		__dump_perframe_cmd(
			(struct sensor_lite_perframe_cmd *)header);
		break;
	case SENSORLITE_CMD_TYPE_START:
		break;
	case SENSORLITE_CMD_TYPE_STOP:
		break;
	case HCM_PKT_OPCODE_SENSOR_ACQUIRE:
		__dump_acquire_cmd(
			(struct sensor_lite_acquire_cmd *)header);
		break;
	case HCM_PKT_OPCODE_SENSOR_RELEASE:
		__dump_release_cmd(
			(struct sensor_lite_release_cmd *)header);
		break;

	default:
		break;
	}
#endif
	return rc;
}
