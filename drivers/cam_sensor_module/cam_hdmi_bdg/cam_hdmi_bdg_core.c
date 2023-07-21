/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/module.h>
#include <cam_sensor_cmn_header.h>
#include "cam_sensor_core.h"
#include "cam_sensor_util.h"
#include "cam_soc_util.h"
#include "cam_trace.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "cam_hdmi_bdg_core.h"

static struct cam_sensor_ctrl_t *cam_hdmi_bdg_cam_ctrl;

int cam_hdmi_bdg_set_cam_ctrl(struct cam_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	struct cam_camera_slave_info *slave_info;

	if (s_ctrl == NULL) {
		CAM_ERR(CAM_SENSOR, " failed: %pK",
				s_ctrl);
		return -EINVAL;
	}
	if (s_ctrl->sensordata == NULL) {
		CAM_ERR(CAM_SENSOR, " failed: %pK",
				s_ctrl->sensordata);
		return -EINVAL;
	}
	slave_info = &(s_ctrl->sensordata->slave_info);
	if (!slave_info) {
		CAM_ERR(CAM_SENSOR, " failed: %pK",
				slave_info);
		return -EINVAL;
	}
	if (slave_info->sensor_id == HDMI_BDG_SENSOR_ID) {
		cam_hdmi_bdg_cam_ctrl = s_ctrl;
		CAM_ERR(CAM_SENSOR, "Setting sctrl for HDMI");
		rc = 0;
	} else {
		cam_hdmi_bdg_cam_ctrl = NULL;
		rc = -EINVAL;
	}
	return rc;
}

void cam_hdmi_bdg_unset_cam_ctrl(void)
{
	cam_hdmi_bdg_cam_ctrl = NULL;
}

int cam_hdmi_bdg_get_src_resolution(bool *signal_stable,
	int *width,
	int *height,
	int *id)
{
	int rc = 0;
	u32 hactive_h, hactive_l;
	u32 vactive_h, vactive_l;
	u32 hdmi_signal_status;
	struct cam_sensor_i2c_reg_setting m_i2cWriteSettings;
	struct cam_sensor_i2c_reg_array m_i2cWriteRegArray;

	if (!cam_hdmi_bdg_cam_ctrl) {
		CAM_ERR(CAM_SENSOR, "LT6911GXC is not ready.");
		*signal_stable = false;
		*height = -1;
		*width = -1;
		return -EINVAL;
	}
	/* camera_io_init(&(cam_hdmi_bdg_cam_ctrl->io_master_info));*/
	/* cam_sensor_power_up(cam_hdmi_bdg_cam_ctrl);*/
	mutex_lock(&(cam_hdmi_bdg_cam_ctrl->cam_sensor_mutex));
	memset(&m_i2cWriteSettings, 0, sizeof(m_i2cWriteSettings));

	m_i2cWriteSettings.reg_setting = &m_i2cWriteRegArray;
	m_i2cWriteSettings.size = 1;
	m_i2cWriteSettings.addr_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	m_i2cWriteSettings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	m_i2cWriteSettings.delay = 0;

	m_i2cWriteRegArray.reg_addr = 0xff;
	m_i2cWriteRegArray.reg_data = 0xe0;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	m_i2cWriteRegArray.reg_addr = 0xee;
	m_i2cWriteRegArray.reg_data = 0x01;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	m_i2cWriteRegArray.reg_addr = 0xff;
	m_i2cWriteRegArray.reg_data = 0xe0;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	m_i2cWriteRegArray.reg_addr = 0xb0;
	m_i2cWriteRegArray.reg_data = 0x01;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);

	/* Check HDMI signal first*/
	m_i2cWriteRegArray.reg_addr = 0xff;
	m_i2cWriteRegArray.reg_data = 0xe0;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	rc = camera_io_dev_read(
			&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			0x84, &hdmi_signal_status,
			CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_BYTE);
	CAM_INFO(CAM_SENSOR, "lt6911gxc signal stable %x", hdmi_signal_status);
	if (hdmi_signal_status == HDMI_BDG_HDMI_DISCONNECTED) {
		*signal_stable = false;
		*height = 0;
		*width = 0;
		*id = cam_hdmi_bdg_cam_ctrl->id;
		goto end;
	}
	if (hdmi_signal_status > 0xFF) {
		/* Read value from I2C should not upper than 0xFF.*/
		/* If so, it means LT6911GXC is closed.*/
		*signal_stable = false;
		*height = -1;
		*width = -1;
		*id = cam_hdmi_bdg_cam_ctrl->id;
		goto end;
	}

	m_i2cWriteRegArray.reg_addr = 0xff;
	m_i2cWriteRegArray.reg_data = 0xe0;
	rc = camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);

	rc = camera_io_dev_read(
			&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			0x8c, &hactive_h,
			CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_BYTE);
	rc = camera_io_dev_read(
			&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			0x8d, &hactive_l,
			CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_BYTE);
	rc = camera_io_dev_read(
			&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			0x8e, &vactive_h,
			CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_BYTE);
	rc = camera_io_dev_read(
			&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			0x8f, &vactive_l,
			CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_BYTE);
	*signal_stable = true;
	*height = (vactive_h << 8) | vactive_l;
	*width = (hactive_h << 8) | hactive_l;
	*id = cam_hdmi_bdg_cam_ctrl->id;
	CAM_INFO(CAM_SENSOR, "signal stable %d %d, x %d id %d",
			*signal_stable, *width,
			*height, *id);

end:
	m_i2cWriteRegArray.reg_addr = 0xff;
	m_i2cWriteRegArray.reg_data = 0xe0;
	camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	m_i2cWriteRegArray.reg_addr = 0xee;
	m_i2cWriteRegArray.reg_data = 0x00;
	camera_io_dev_write(&(cam_hdmi_bdg_cam_ctrl->io_master_info),
			&m_i2cWriteSettings);
	mutex_unlock(&(cam_hdmi_bdg_cam_ctrl->cam_sensor_mutex));
	return rc;
}
EXPORT_SYMBOL(cam_hdmi_bdg_get_src_resolution);
