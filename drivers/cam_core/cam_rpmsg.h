// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __CAM_RPMSG_H__
#define __CAM_RPMSG_H__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/bitfield.h>
#include <linux/rpmsg.h>

typedef int (*cam_rpmsg_recv_cb)(struct rpmsg_device *rpdev, void *data,
	int len, void *priv, unsigned int src);

#define CAM_RPMSG_HANDLE_SLAVE          0
#define CAM_RPMSG_HANDLE_JPEG           1
#define CAM_RPMSG_HANDLE_MAX            2

/*
 * struct cam_slave_pkt_hdr - describes header for camera slave
 * version           : 4 bits
 * processing_engine : 2 bits
 * level             : 1 bit
 * sub_system        : 5 bits
 * reserved          : 3 bits
 * payload_length    : 16 bits, Does not include header size
 * last              : 1 bit
 */
struct cam_slave_pkt_hdr {
	uint32_t raw;
};

#define CAM_RPMSG_V1                             1

#define CAM_RPMSG_SLAVE_HDR_MASK_VERSION         GENMASK(3, 0)
#define CAM_RPMSG_SLAVE_HDR_MASK_PENGINE         GENMASK(5, 4)
#define CAM_RPMSG_SLAVE_HDR_MASK_LEVEL           BIT(6)
#define CAM_RPMSG_SLAVE_HDR_MASK_SUB_SYS         GENMASK(11, 7)
#define CAM_RPMSG_SLAVE_HDR_MASK_PAYLOAD_LEN     GENMASK(30, 15)
#define CAM_RPMSG_SLAVE_HDR_MASK_IS_LAST         BIT(31)

#define CAM_RPMSG_SLAVE_GET_HDR_VERSION(hdr) \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_VERSION, hdr->raw)
#define CAM_RPMSG_SLAVE_GET_HDR_PENGINE(hdr) \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_PENGINE, hdr->raw)
#define CAM_RPMSG_SLAVE_GET_HDR_LEVEL(hdr)   \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_LEVEL, hdr->raw)
#define CAM_RPMSG_SLAVE_GET_HDR_SUB_SYS(hdr) \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_SUB_SYS, hdr->raw)
#define CAM_RPMSG_SLAVE_GET_HDR_PAYLOAD_LEN(hdr) \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_PAYLOAD_LEN, hdr->raw)
#define CAM_RPMSG_SLAVE_GET_HDR_IS_LAST(hdr) \
	FIELD_GET(CAM_RPMSG_SLAVE_HDR_MASK_IS_LAST, hdr->raw)

#define CAM_RPMSG_SLAVE_SET_HDR_VERSION(hdr, val) \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_VERSION, val))
#define CAM_RPMSG_SLAVE_SET_HDR_PENGINE(hdr, val) \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_PENGINE, val))
#define CAM_RPMSG_SLAVE_SET_HDR_LEVEL(hdr, val)   \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_LEVEL, val))
#define CAM_RPMSG_SLAVE_SET_HDR_SUB_SYS(hdr, val) \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_SUB_SYS, val))
#define CAM_RPMSG_SLAVE_SET_HDR_PAYLOAD_LEN(hdr, val) \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_PAYLOAD_LEN, val))
#define CAM_RPMSG_SLAVE_SET_HDR_IS_LAST(hdr, val) \
	(hdr->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_HDR_MASK_IS_LAST, val))


#define CAM_RPMSG_DIR_MASTER_TO_SLAVE                        0x0
#define CAM_RPMSG_DIR_SLAVE_TO_MASTER                        0x1

#define CAM_RPMSG_SLAVE_PACKET_BASE_SYSTEM                   0x0
#define CAM_RPMSG_SLAVE_PACKET_BASE_ISP                      0x20
#define CAM_RPMSG_SLAVE_PACKET_BASE_SENSOR                   0x40

#define CAM_RPMSG_SLAVE_PACKET_TYPE_SYSTEM_UNUSED     \
	(CAM_RPMSG_SLAVE_PACKET_BASE_SYSTEM + 0x0)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_SYSTEM_PING       \
	(CAM_RPMSG_SLAVE_PACKET_BASE_SYSTEM + 0x1)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_SYSTEM_MAX        \
	(CAM_RPMSG_SLAVE_PACKET_BASE_SYSTEM + 0x19)

#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_UNUSED        \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x0)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_ACQUIRE       \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x1)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_RELEASE       \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x2)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_INIT_CONFIG   \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x3)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_START_DEV     \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x4)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_STOP_DEV      \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x5)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_ERROR         \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x6)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_ISP_MAX           \
	(CAM_RPMSG_SLAVE_PACKET_BASE_ISP + 0x19)

#define CAM_RPMSG_SLAVE_PACKET_TYPE_SENSOR_UNUSED     \
	(CAM_RPMSG_SLAVE_PACKET_BASE_SENSOR + 0)
#define CAM_RPMSG_SLAVE_PACKET_TYPE_SENSOR_MAX        \
	(CAM_RPMSG_SLAVE_PACKET_BASE_SENSOR + 0x19)

#define PACKET_VERSION_1                   1

#define ERR_TYPE_SLAVE_FATAL               1
#define ERR_TYPE_SLAVE_RECOVERY            2

/*
 * slave payload
 * type     : 8 bits
 * size     : 16 bits      Does not include payload size
 * reserved : 8 bits
 */
struct cam_rpmsg_slave_payload_desc {
	uint32_t raw;
};

#define CAM_RPMSG_SLAVE_PAYLOAD_MASK_TYPE GENMASK(6, 0)
#define CAM_RPMSG_SLAVE_PAYLOAD_MASK_DIR  BIT(7)
#define CAM_RPMSG_SLAVE_PAYLOAD_MASK_SIZE GENMASK(23, 8)

#define CAM_RPMSG_SLAVE_GET_PAYLOAD_TYPE(payload) \
	FIELD_GET(CAM_RPMSG_SLAVE_PAYLOAD_MASK_TYPE, payload->raw)
#define CAM_RPMSG_SLAVE_GET_PAYLOAD_DIR(payload)  \
	FIELD_GET(CAM_RPMSG_SLAVE_PAYLOAD_MASK_DIR, payload->raw)
#define CAM_RPMSG_SLAVE_GET_PAYLOAD_SIZE(payload) \
	FIELD_GET(CAM_RPMSG_SLAVE_PAYLOAD_MASK_SIZE, payload->raw)

#define CAM_RPMSG_SLAVE_SET_PAYLOAD_TYPE(payload, val) \
	(payload->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_PAYLOAD_MASK_TYPE, val))
#define CAM_RPMSG_SLAVE_SET_PAYLOAD_DIR(payload, val)  \
	(payload->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_PAYLOAD_MASK_DIR, val))
#define CAM_RPMSG_SLAVE_SET_PAYLOAD_SIZE(payload, val) \
	(payload->raw |= FIELD_PREP(CAM_RPMSG_SLAVE_PAYLOAD_MASK_SIZE, val))

#define CAM_RPMSG_SLAVE_CLIENT_SYSTEM    0
#define CAM_RPMSG_SLAVE_CLIENT_ISP       1
#define CAM_RPMSG_SLAVE_CLIENT_SENSOR    2
#define CAM_RPMSG_SLAVE_CLIENT_MAX       3

/** struct cam_rpmsg_slave_cbs - slave client callback
 *
 * @registered : Flag to indicate if callbacks are registered
 * @cookie     : Cookie to be passed back in callbacks
 * @recv       : Function pointer to receive callback
 */
struct cam_rpmsg_slave_cbs {
	int registered;
	void *cookie;
	int (*recv)(void *cookie, void *data, int len);
};

/** struct cam_rpmsg_slave_pvt - slave channel private data
 *
 * @cbs : slave client callback data
 */
struct cam_rpmsg_slave_pvt {
	struct cam_rpmsg_slave_cbs cbs[CAM_RPMSG_SLAVE_CLIENT_MAX];
};

/** struct cam_rpmsg_instance_data - rpmsg per channel data
 *
 * @sp_lock              : spin_lock variable
 * @recv_cb              : channel receive callback
 * @pvt                  : channel private data
 * @rpdev                : rpmsg device pointer for channel
 * @status_change_notify : notification chain
 */
struct cam_rpmsg_instance_data {
	spinlock_t sp_lock;
	cam_rpmsg_recv_cb recv_cb;
	void *pvt;
	struct rpmsg_device *rpdev;
	struct blocking_notifier_head status_change_notify;
};

/**
 * @prief : Request handle for JPEG/SLAVE rpmsg
 * @name  : name of device, can be either "helios" or "jpeg"
 *
 * @return handle on success, otherwise error codes
 */
unsigned int cam_rpmsg_get_handle(char *name);

/**
 * @brief  : Returns true if channel is connected, false otherwise
 * @handle : handle for channel
 *
 * @return 0 and 1 as channel state, otherwise error codes
 */
int cam_rpmsg_is_channel_connected(unsigned int handle);

/**
 * @brief  : Send data to rpmsg channel
 * @handle : channel handle
 * @data   : pointer to data
 * @len    : length of data
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_send(unsigned int handle, void *data, int len);

/**
 * @brief     : Subscribe callback for slave data receive
 * @module_id : onw of CAM_RPMSG_SLAVE_CLIENT_SYSTEM/ISP/SENSOR
 * @cbs       : structure for callback function and private data
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_subscribe_slave_callback(unsigned int module_id,
	struct cam_rpmsg_slave_cbs cbs);

/**
 * @brief     : Unsubscribes callback for slave data receive
 * @module_id : onw of CAM_RPMSG_SLAVE_CLIENT_SYSTEM/ISP/SENSOR
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_unsubscribe_slave_callback(int module_id);

/**
 * @brief  : start listening for status change event, channel UP/DOWN
 * @handle : Handle for channel
 * @nb     : notification block
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_register_status_change_event(unsigned int handle,
	struct notifier_block *nb);

/**
 * @brief  : stop listening for status change event, channel UP/DOWN
 * @handle : Handle for channel
 * @nb     : notification block
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_unregister_status_change_event(unsigned int handle,
	struct notifier_block *nb);

/**
 * @brief  : update receive callback for a channel
 * @handle : Handle for channel
 * @cb     : receive callback function
 *
 * @return zero on success, otherwise error codes
 */
int cam_rpmsg_set_recv_cb(unsigned int handle, cam_rpmsg_recv_cb cb);

/**
 * @brief : API to register rpmsg to platform framework.
 * @return zero on success, or ERR_PTR() on error.
 */
int cam_rpmsg_init(void);

/**
 * @brief : API to remove rpmsg from platform framework.
 */
void cam_rpmsg_exit(void);

#endif /* __CAM_RPMSG_H__ */
