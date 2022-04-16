// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>

#include "cam_sensor_lite_dev.h"
#include "cam_sensor_lite_core.h"
#include "cam_sensor_lite_soc.h"
#include "cam_req_mgr_dev.h"
#include <media/cam_sensor.h>
#include "camera_main.h"
#include <dt-bindings/msm-camera.h>

static void cam_sensor_lite_subdev_handle_message(
		struct v4l2_subdev *sd,
		enum cam_subdev_message_type_t message_type,
		void *data)
{
	struct sensor_lite_device *sensor_lite_dev = v4l2_get_subdevdata(sd);
	uint32_t data_idx;

	switch (message_type) {
	case CAM_SUBDEV_MESSAGE_REG_DUMP:
		data_idx = *(uint32_t *)data;
		CAM_INFO(CAM_SENSOR_LITE, "subdev index : %d Sensor Lite index: %d",
				sensor_lite_dev->soc_info.index, data_idx);
				break;
	default:
		break;
	}
}

static int cam_sensor_lite_subdev_close_internal(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct sensor_lite_device *sensor_lite_dev =
		v4l2_get_subdevdata(sd);

	if (!sensor_lite_dev) {
		CAM_ERR(CAM_SENSOR_LITE, "sensor_lite_dev ptr is NULL");
		return -EINVAL;
	}

	mutex_lock(&sensor_lite_dev->mutex);
	cam_sensor_lite_shutdown(sensor_lite_dev);
	mutex_unlock(&sensor_lite_dev->mutex);

	return 0;
}

static int cam_sensor_lite_subdev_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	bool crm_active = cam_req_mgr_is_open();

	if (crm_active) {
		CAM_DBG(CAM_SENSOR_LITE, "CRM is ACTIVE, close should be from CRM");
		return 0;
	}

	return cam_sensor_lite_subdev_close_internal(sd, fh);
}

static long cam_sensor_lite_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	struct sensor_lite_device *sensor_lite_dev = v4l2_get_subdevdata(sd);
	int rc = 0;

	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		rc = cam_sensor_lite_core_cfg(sensor_lite_dev, arg);
		if (rc)
			CAM_ERR(CAM_SENSOR_LITE,
				"Failed in configuring the device: %d", rc);
		break;
	case CAM_SD_SHUTDOWN:
		if (!cam_req_mgr_is_shutdown()) {
			CAM_ERR(CAM_SENSOR_LITE, "SD shouldn't come from user space");
			return 0;
		}

		rc = cam_sensor_lite_subdev_close_internal(sd, NULL);
		break;
	default:
		CAM_ERR(CAM_SENSOR_LITE, "Wrong ioctl : %d", cmd);
		rc = -ENOIOCTLCMD;
		break;
	}

	return rc;
}

#ifdef CONFIG_COMPAT
static long cam_sensor_lite_subdev_compat_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, unsigned long arg)
{
	int32_t rc = 0;
	struct cam_control cmd_data;

	if (copy_from_user(&cmd_data, (void __user *)arg,
		sizeof(cmd_data))) {
		CAM_ERR(CAM_SENSOR_LITE, "Failed to copy from user_ptr=%pK size=%zu",
			(void __user *)arg, sizeof(cmd_data));
		return -EFAULT;
	}

	/* All the arguments converted to 64 bit here
	 * Passed to the api in core.c
	 */
	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		rc = cam_sensor_lite_subdev_ioctl(sd, cmd, &cmd_data);
		if (rc)
			CAM_ERR(CAM_SENSOR_LITE,
				"Failed in subdev_ioctl: %d", rc);
		break;
	default:
		CAM_ERR(CAM_SENSOR_LITE, "Invalid compat ioctl cmd: %d", cmd);
		rc = -ENOIOCTLCMD;
		break;
	}

	if (!rc) {
		if (copy_to_user((void __user *)arg, &cmd_data,
			sizeof(cmd_data))) {
			CAM_ERR(CAM_SENSOR_LITE,
				"Failed to copy to user_ptr=%pK size=%zu",
				(void __user *)arg, sizeof(cmd_data));
			rc = -EFAULT;
		}
	}

	return rc;
}
#endif

static struct v4l2_subdev_core_ops sensor_lite_subdev_core_ops = {
	.ioctl = cam_sensor_lite_subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = cam_sensor_lite_subdev_compat_ioctl,
#endif
};

static const struct v4l2_subdev_ops sensor_lite_subdev_ops = {
	.core = &sensor_lite_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops sensor_lite_subdev_intern_ops = {
	.close = cam_sensor_lite_subdev_close,
};

static int cam_sensor_lite_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct sensor_lite_device *sensor_lite_dev = NULL;
	int32_t               rc = 0;
	struct platform_device *pdev = to_platform_device(dev);

	sensor_lite_dev = devm_kzalloc(&pdev->dev,
		sizeof(struct sensor_lite_device), GFP_KERNEL);
	if (!sensor_lite_dev)
		return -ENOMEM;

	mutex_init(&sensor_lite_dev->mutex);
	sensor_lite_dev->v4l2_dev_str.pdev = pdev;

	sensor_lite_dev->soc_info.pdev = pdev;
	sensor_lite_dev->soc_info.dev = &pdev->dev;
	sensor_lite_dev->soc_info.dev_name = pdev->name;
	sensor_lite_dev->ref_count = 0;

	rc = cam_sensor_lite_parse_dt_info(pdev, sensor_lite_dev);
	if (rc < 0)
		CAM_ERR(CAM_SENSOR_LITE,  "DT parsing failed: %d", rc);

	sensor_lite_dev->v4l2_dev_str.internal_ops =
		&sensor_lite_subdev_intern_ops;
	sensor_lite_dev->v4l2_dev_str.ops =
		&sensor_lite_subdev_ops;
	snprintf(sensor_lite_dev->device_name,
		CAM_CTX_DEV_NAME_MAX_LENGTH,
		"%s%d", CAMX_SENOSR_LITE_DEV_NAME,
		sensor_lite_dev->soc_info.index);
	sensor_lite_dev->v4l2_dev_str.name =
		sensor_lite_dev->device_name;
	sensor_lite_dev->v4l2_dev_str.sd_flags =
		(V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS);
	sensor_lite_dev->v4l2_dev_str.ent_function =
		CAM_SENSORLITE_DEVICE_TYPE;
	sensor_lite_dev->v4l2_dev_str.msg_cb =
		cam_sensor_lite_subdev_handle_message;
	sensor_lite_dev->v4l2_dev_str.token =
		sensor_lite_dev;
	sensor_lite_dev->v4l2_dev_str.close_seq_prior =
		CAM_SD_CLOSE_MEDIUM_PRIORITY;

	rc = cam_register_subdev(&(sensor_lite_dev->v4l2_dev_str));
	if (rc < 0)
		CAM_ERR(CAM_SENSOR_LITE, "cam_register_subdev Failed rc: %d", rc);

	platform_set_drvdata(pdev, &(sensor_lite_dev->v4l2_dev_str.sd));

	/* Non real time device */
	sensor_lite_dev->ops.get_dev_info = NULL;
	sensor_lite_dev->ops.link_setup   = NULL;
	sensor_lite_dev->ops.apply_req    = NULL;
	sensor_lite_dev->acquire_cmd      = NULL;
	sensor_lite_dev->release_cmd      = NULL;
	sensor_lite_dev->start_cmd        = NULL;
	sensor_lite_dev->stop_cmd         = NULL;
	return rc;
}

static void cam_sensor_lite_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
}

const static struct component_ops cam_sensor_lite_component_ops = {
	.bind = cam_sensor_lite_component_bind,
	.unbind = cam_sensor_lite_component_unbind,
};

static int32_t cam_sensor_lite_platform_probe(struct platform_device *pdev)
{
	int rc = 0;

	CAM_DBG(CAM_SENSOR_LITE, "Adding SENSOR LITE component");
	rc = component_add(&pdev->dev, &cam_sensor_lite_component_ops);
	if (rc)
		CAM_ERR(CAM_SENSOR_LITE, "failed to add component rc: %d", rc);
	return rc;
}


static int32_t cam_sensor_lite_device_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &cam_sensor_lite_component_ops);
	return 0;
}

static const struct of_device_id cam_sensor_lite_dt_match[] = {
	{.compatible = "qcom,cam-sensor-lite"},
	{}
};

MODULE_DEVICE_TABLE(of, cam_sensor_lite_dt_match);

struct platform_driver cam_sensor_lite_driver = {
	.probe  = cam_sensor_lite_platform_probe,
	.remove = cam_sensor_lite_device_remove,
	.driver = {
		.name  = CAMX_SENOSR_LITE_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = cam_sensor_lite_dt_match,
		.suppress_bind_attrs = true,
	},
};

int32_t cam_sensor_lite_init_module(void)
{
	return platform_driver_register(&cam_sensor_lite_driver);
}

void cam_sensor_lite_exit_module(void)
{
	platform_driver_unregister(&cam_sensor_lite_driver);
}

MODULE_DESCRIPTION("CAM Sensor Lite driver");
MODULE_LICENSE("GPL v2");
