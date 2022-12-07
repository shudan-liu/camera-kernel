/* Copyright (c) 2022, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include "cam_subdev.h"
#include "cam_debug_util.h"

static int cam_v4l_nodes_probe(struct platform_device *pdev)
{
	int rc;

    rc = cam_dev_mgr_create_subdev_nodes();
	if (rc) {
		CAM_ERR(CAM_CRM, "cam_dev_mgr_create_subdev_nodes failed, rc = %d", rc);
	}
	return rc;
}

static const struct of_device_id cam_v4l_nodes_dt_match[] = {
	{.compatible = "qcom,cam-v4l-nodes"},
	{}
};

static struct platform_driver cam_v4l_nodes_driver = {
	.probe = cam_v4l_nodes_probe,
	.driver = {
		.name = "cam_v4l_nodes",
		.owner = THIS_MODULE,
		.of_match_table = cam_v4l_nodes_dt_match,
		.suppress_bind_attrs = true,
	},
};

static int __init cam_v4l_nodes_init_module(void)
{
	return platform_driver_register(&cam_v4l_nodes_driver);
}

static void __exit cam_v4l_nodes_exit_module(void)
{
	platform_driver_unregister(&cam_v4l_nodes_driver);
}

module_init(cam_v4l_nodes_init_module);
module_exit(cam_v4l_nodes_exit_module);
MODULE_DESCRIPTION("CAMERA V4L Nodes driver");
MODULE_LICENSE("GPL v2");
