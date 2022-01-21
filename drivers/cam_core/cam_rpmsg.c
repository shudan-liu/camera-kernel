// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/notifier.h>

#include "cam_rpmsg.h"
#include "cam_debug_util.h"
#include "cam_req_mgr_dev.h"
#include "media/cam_req_mgr.h"

#define LOG_CTX() \
	CAM_DBG(CAM_RPMSG, "in_irq %d, in_task %d, in_interrupt %d",\
			in_irq(), in_task(), in_interrupt())

static struct cam_rpmsg_instance_data cam_rpdev_idata[CAM_RPMSG_HANDLE_MAX];
struct cam_rpmsg_slave_pvt slave_private;

unsigned int cam_rpmsg_get_handle(char *name)
{
	if (!strcmp("helios", name))
		return CAM_RPMSG_HANDLE_SLAVE;
	else if (!strcmp("jpeg", name))
		return CAM_RPMSG_HANDLE_JPEG;
	else {
		CAM_ERR(CAM_RPMSG, "Unknown dev name %s", name);
		return CAM_RPMSG_HANDLE_MAX;
	}
}

int cam_rpmsg_is_channel_connected(unsigned int handle)
{
	struct rpmsg_device *rpdev = NULL;

	if (handle >= CAM_RPMSG_HANDLE_MAX)
		return -EINVAL;

	rpdev = cam_rpdev_idata[handle].rpdev;

	return rpdev != NULL;
}

static inline unsigned int cam_rpmsg_get_handle_from_dev(
		struct rpmsg_device *rpdev)
{
	unsigned int handle;
	struct cam_rpmsg_instance_data *idata = &cam_rpdev_idata[0];

	for (handle = 0; handle < CAM_RPMSG_HANDLE_MAX; handle++) {
		if (idata[handle].rpdev == rpdev)
			return handle;
	}
	CAM_ERR(CAM_RPMSG, "Failed to find handle for device");

	return CAM_RPMSG_HANDLE_MAX;
}

int cam_rpmsg_subscribe_slave_callback(unsigned int module_id,
	struct cam_rpmsg_slave_cbs cbs)
{
	struct cam_rpmsg_instance_data *idata =
		&cam_rpdev_idata[CAM_RPMSG_HANDLE_SLAVE];
	struct cam_rpmsg_slave_pvt *pvt =
		(struct cam_rpmsg_slave_pvt *)idata->pvt;
	unsigned long flag;

	if (module_id >= CAM_RPMSG_SLAVE_CLIENT_MAX) {
		CAM_ERR(CAM_RPMSG, "Invalid module_id %d", module_id);
		return -EINVAL;
	}

	spin_lock_irqsave(&idata->sp_lock, flag);
	if (pvt->cbs[module_id].registered) {
		spin_unlock_irqrestore(&idata->sp_lock, flag);
		CAM_ERR(CAM_RPMSG, "cb already subscribed for mid %d",
			  module_id);
		return -EALREADY;
	}

	pvt->cbs[module_id] = cbs;
	pvt->cbs[module_id].registered = 1;
	spin_unlock_irqrestore(&idata->sp_lock, flag);

	CAM_DBG(CAM_RPMSG, "rpmsg callback subscribed for mid %d", module_id);

	return 0;
}

int cam_rpmsg_unsubscribe_slave_callback(int module_id)
{
	struct cam_rpmsg_instance_data *idata =
		&cam_rpdev_idata[CAM_RPMSG_HANDLE_SLAVE];
	struct cam_rpmsg_slave_pvt *pvt =
		(struct cam_rpmsg_slave_pvt *)idata->pvt;
	unsigned long flag;

	if (module_id >= CAM_RPMSG_SLAVE_CLIENT_MAX) {
		CAM_ERR(CAM_RPMSG, "Invalid module_id %d", module_id);
		return -EINVAL;
	}

	spin_lock_irqsave(&idata->sp_lock, flag);
	if (!pvt->cbs[module_id].registered) {
		spin_unlock_irqrestore(&idata->sp_lock, flag);
		CAM_ERR(CAM_RPMSG, "cb already unsubscribed for mid %d",
			  module_id);
		return -EALREADY;
	}

	pvt->cbs[module_id].registered = 0;
	pvt->cbs[module_id].cookie = NULL;
	pvt->cbs[module_id].recv = NULL;

	spin_unlock_irqrestore(&idata->sp_lock, flag);

	CAM_DBG(CAM_RPMSG, "rpmsg callback unsubscribed for mid %d", module_id);

	return 0;
}

int cam_rpmsg_register_status_change_event(unsigned int handle,
		struct notifier_block *nb)
{
	struct cam_rpmsg_instance_data *idata;

	if (handle >= CAM_RPMSG_HANDLE_MAX)
		return -EINVAL;

	idata = &cam_rpdev_idata[handle];
	blocking_notifier_chain_register(&idata->status_change_notify, nb);

	return 0;
}

int cam_rpmsg_unregister_status_change_event(unsigned int handle,
		struct notifier_block *nb)
{
	struct cam_rpmsg_instance_data *idata;

	if (handle >= CAM_RPMSG_HANDLE_MAX)
		return -EINVAL;

	idata = &cam_rpdev_idata[handle];
	blocking_notifier_chain_unregister(&idata->status_change_notify, nb);

	return 0;
}

static void cam_rpmsg_notify_slave_status_change(
		struct cam_rpmsg_instance_data *idata, int status)
{
	struct cam_req_mgr_message msg = {0};

	msg.u.slave_status.version = 1;
	msg.u.slave_status.status = status;

	if (cam_req_mgr_notify_message(&msg,
		V4L_EVENT_CAM_REQ_MGR_SLAVE_STATUS,
		V4L_EVENT_CAM_REQ_MGR_EVENT))
		CAM_ERR(CAM_RPMSG, "Error in notifying slave status %d",
			status);

	blocking_notifier_call_chain(&idata->status_change_notify,
		status, NULL);

}

int cam_rpmsg_send(unsigned int handle, void *data, int len)
{
	int ret = 0;
	struct rpmsg_device *rpdev;

	if (handle >= CAM_RPMSG_HANDLE_MAX)
		return -EINVAL;

	rpdev = cam_rpdev_idata[handle].rpdev;

	if (!rpdev) {
		CAM_ERR(CAM_RPMSG, "Send in disconnect");
		return -EBUSY;
	}

	ret = rpmsg_send(rpdev->ept, data, len);
	if (ret) {
		CAM_ERR(CAM_RPMSG, "rpmsg_send failed dev %d, rc %d",
			handle, ret);
	}

	return ret;
}

static int cam_rpmsg_slave_cb(struct rpmsg_device *rpdev, void *data, int len,
	void *priv, u32 src)
{
	int ret = 0, processed = 0, client;
	int hdr_version, dir, payload_type, payload_len, hdr_len;
	unsigned long flag;
	struct cam_rpmsg_instance_data *idata = dev_get_drvdata(&rpdev->dev);
	struct cam_slave_pkt_hdr *hdr = data;
	struct cam_rpmsg_slave_payload_desc *payload = NULL;
	struct cam_rpmsg_slave_pvt *pvt =
		(struct cam_rpmsg_slave_pvt *)idata->pvt;
	struct cam_rpmsg_slave_cbs *cb = NULL;

	LOG_CTX();

	if (len < (sizeof(struct cam_slave_pkt_hdr) +
				sizeof(struct cam_rpmsg_slave_payload_desc))) {
		CAM_ERR(CAM_RPMSG, "malformed packet, sz %d", len);
		return 0;
	}

	hdr_version = CAM_RPMSG_SLAVE_GET_HDR_VERSION(hdr);
	hdr_len = CAM_RPMSG_SLAVE_GET_HDR_PAYLOAD_LEN(hdr);
	processed = sizeof(struct cam_slave_pkt_hdr);

	if (hdr_version != CAM_RPMSG_V1) {
		CAM_ERR(CAM_RPMSG, "Unsupported packet version %d",
				hdr_version);
		return 0;
	}

	while (processed < len) {
		payload = (struct cam_rpmsg_slave_payload_desc *)
			((uintptr_t)data + processed);
		cb = NULL;
		client = CAM_RPMSG_SLAVE_CLIENT_MAX;
		dir = CAM_RPMSG_SLAVE_GET_PAYLOAD_DIR(payload);
		payload_type = CAM_RPMSG_SLAVE_GET_PAYLOAD_TYPE(payload);
		payload_len = CAM_RPMSG_SLAVE_GET_PAYLOAD_SIZE(payload);

		if (dir != CAM_RPMSG_DIR_SLAVE_TO_MASTER) {
			CAM_ERR(CAM_RPMSG, "Invalid direction %d, payload %x", dir, payload_type);
			processed += sizeof(struct cam_rpmsg_slave_payload_desc) +
				payload_len;
			continue;
		}

		if (payload_type > CAM_RPMSG_SLAVE_PACKET_TYPE_SYSTEM_UNUSED &&
			payload_type <= CAM_RPMSG_SLAVE_PACKET_TYPE_SYSTEM_MAX) {
			/* Hand off to system pkt handler */
			client = CAM_RPMSG_SLAVE_CLIENT_SYSTEM;
		} else if (payload_type > CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_UNUSED &&
			payload_type <= CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_MAX) {
			/* Hand off to ISP pkt handler */
			client = CAM_RPMSG_SLAVE_CLIENT_ISP;
		} else if (payload_type > CAM_RPMSG_SLAVE_PACKET_TYPE_SENSOR_UNUSED &&
			payload_type <= CAM_RPMSG_SLAVE_PACKET_TYPE_SENSOR_MAX) {
			/* Hand off to SENSOR pkt handler */
			client = CAM_RPMSG_SLAVE_CLIENT_SENSOR;
		}

		spin_lock_irqsave(&idata->sp_lock, flag);
		if (client < CAM_RPMSG_SLAVE_CLIENT_MAX)
			cb = &pvt->cbs[client];

		if (cb || !cb->registered) {
			CAM_ERR(CAM_RPMSG, "cbs not registered for client %d",
					client);
		} else if (!cb->recv) {
			CAM_ERR(CAM_RPMSG, "recv not set for client %d",
					client);
		} else {
			cb->recv(cb->cookie, payload, payload_len);
		}
		spin_unlock_irqrestore(&idata->sp_lock, flag);

		processed += sizeof(struct cam_rpmsg_slave_payload_desc) +
			payload_len;
	}

	if (processed != len) {
		CAM_INFO(CAM_RPMSG, "%d bytes are left unprocessed",
				len - processed);
	}

	return ret;
}

static int cam_rpmsg_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	int rc = 0;
	unsigned int handle = cam_rpmsg_get_handle_from_dev(rpdev);
	unsigned long flag;
	struct cam_rpmsg_instance_data *idata = dev_get_drvdata(&rpdev->dev);
	cam_rpmsg_recv_cb cb;

	spin_lock_irqsave(&idata->sp_lock, flag);
	cb = idata->recv_cb;
	spin_unlock_irqrestore(&idata->sp_lock, flag);

	if (cb)
		rc = cb(rpdev, data, len, priv, src);
	else
		CAM_ERR(CAM_RPMSG, "No recv_cb for handle %d", handle);

	return rc;
}
int cam_rpmsg_set_recv_cb(unsigned int handle, cam_rpmsg_recv_cb cb)
{
	struct cam_rpmsg_instance_data *idata;
	unsigned long flags;

	if (handle >= CAM_RPMSG_HANDLE_MAX) {
		CAM_ERR(CAM_RPMSG, "Invalid handle %d", handle);
		return -EINVAL;
	}

	idata = &cam_rpdev_idata[handle];

	spin_lock_irqsave(&idata->sp_lock, flags);
	idata->recv_cb = cb;
	spin_unlock_irqrestore(&idata->sp_lock, flags);

	CAM_DBG(CAM_RPMSG, "handle %d cb %pS", handle, cb);
	return 0;
}

static int cam_rpmsg_slave_probe(struct rpmsg_device *rpdev)
{
	struct cam_rpmsg_instance_data *idata;
	unsigned int handle = CAM_RPMSG_HANDLE_SLAVE;
	unsigned long flag;

	CAM_INFO(CAM_RPMSG, "src 0x%x -> dst 0x%x", rpdev->src, rpdev->dst);
	LOG_CTX();

	idata = &cam_rpdev_idata[CAM_RPMSG_HANDLE_SLAVE];
	dev_set_drvdata(&rpdev->dev, idata);
	spin_lock_irqsave(&idata->sp_lock, flag);
	idata->rpdev = rpdev;
	idata->pvt = &slave_private;
	spin_unlock_irqrestore(&idata->sp_lock, flag);

	cam_rpmsg_set_recv_cb(handle, cam_rpmsg_slave_cb);

	cam_rpmsg_notify_slave_status_change(idata, CAM_REQ_MGR_SLAVE_UP);
	return 0;
}

static int cam_rpmsg_jpeg_probe(struct rpmsg_device *rpdev)
{
	int rc = 0;
	unsigned long flag;
	struct cam_rpmsg_instance_data *idata;

	CAM_INFO(CAM_RPMSG, "src 0x%x -> dst 0x%x", rpdev->src, rpdev->dst);
	idata = &cam_rpdev_idata[CAM_RPMSG_HANDLE_JPEG];
	dev_set_drvdata(&rpdev->dev, idata);
	spin_lock_irqsave(&idata->sp_lock, flag);
	idata->rpdev = rpdev;
	spin_unlock_irqrestore(&idata->sp_lock, flag);

	CAM_DBG(CAM_RPMSG, "rpmsg probe success for jpeg");

	return rc;
}

static void cam_rpmsg_slave_remove(struct rpmsg_device *rpdev)
{
	struct cam_rpmsg_instance_data *idata = dev_get_drvdata(&rpdev->dev);
	unsigned long flag;

	CAM_INFO(CAM_RPMSG, "Possible SSR");

	spin_lock_irqsave(&idata->sp_lock, flag);
	idata->rpdev = NULL;
	spin_unlock_irqrestore(&idata->sp_lock, flag);
	cam_rpmsg_notify_slave_status_change(idata, CAM_REQ_MGR_SLAVE_DOWN);
}

static void cam_rpmsg_jpeg_remove(struct rpmsg_device *rpdev)
{
	struct cam_rpmsg_instance_data *idata = dev_get_drvdata(&rpdev->dev);
	unsigned long flag;

	CAM_INFO(CAM_RPMSG, "jpeg rpmsg remove");

	spin_lock_irqsave(&idata->sp_lock, flag);
	idata->rpdev = NULL;
	spin_unlock_irqrestore(&idata->sp_lock, flag);
}

/* Channel name */
static struct rpmsg_device_id cam_rpmsg_slave_id_table[] = {
	{ .name = "cam-slave" },
	{ },
};

/* device tree compatible string */
static const struct of_device_id cam_rpmsg_slave_of_match[] = {
	{ .compatible = "qcom,cam-slave-rpmsg" },
	{ },
};
MODULE_DEVICE_TABLE(of, cam_rpmsg_slave_of_match);

static struct rpmsg_driver cam_rpmsg_slave_client = {
	.id_table               = cam_rpmsg_slave_id_table,
	.probe                  = cam_rpmsg_slave_probe,
	.callback               = cam_rpmsg_cb,
	.remove                 = cam_rpmsg_slave_remove,
	.drv                    = {
		.name           = "cam-slave-drv",
		.of_match_table = cam_rpmsg_slave_of_match,
	},
};

static struct rpmsg_device_id cam_rpmsg_cdsp_id_table[] = {
	{ .name = "cam-nsp-jpeg" },
	{ },
};

static const struct of_device_id cam_rpmsg_jpeg_of_match[] = {
	{ .compatible = "qcom,cam-jpeg-rpmsg" },
	{ },
};
MODULE_DEVICE_TABLE(of, cam_rpmsg_jpeg_of_match);

static struct rpmsg_driver cam_rpmsg_jpeg_client = {
	.id_table               = cam_rpmsg_cdsp_id_table,
	.probe                  = cam_rpmsg_jpeg_probe,
	.callback               = cam_rpmsg_cb,
	.remove                 = cam_rpmsg_jpeg_remove,
	.drv                    = {
		.name           = "cam-jpeg-drv",
		.of_match_table = cam_rpmsg_jpeg_of_match,
	},
};

int cam_rpmsg_init(void)
{
	int rc = 0, i;

	CAM_INFO(CAM_RPMSG, "Registering RPMSG driver");

	for (i = 0; i < CAM_RPMSG_HANDLE_MAX; i++) {
		spin_lock_init(&cam_rpdev_idata[i].sp_lock);
		BLOCKING_INIT_NOTIFIER_HEAD(
				&cam_rpdev_idata[i].status_change_notify);
	}

	rc = register_rpmsg_driver(&cam_rpmsg_slave_client);
	if (rc) {
		CAM_ERR(CAM_RPMSG, "Failed to register slave rpmsg");
		return rc;
	}

	rc = register_rpmsg_driver(&cam_rpmsg_jpeg_client);
	if (rc) {
		CAM_ERR(CAM_RPMSG, "Failed to register jpeg rpmsg");
		return rc;
	}

	return rc;
}

void cam_rpmsg_exit(void)
{
	unregister_rpmsg_driver(&cam_rpmsg_slave_client);
	unregister_rpmsg_driver(&cam_rpmsg_jpeg_client);
}

MODULE_DESCRIPTION("CAM Remote processor messaging driver");
MODULE_LICENSE("GPL v2");
