/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_ISP_HW_H_
#define _CAM_ISP_HW_H_

#include <linux/completion.h>
#include <media/cam_isp.h>
#include "cam_hw.h"
#include "cam_soc_util.h"
#include "cam_irq_controller.h"
#include "cam_hw_intf.h"
#include "cam_cdm_intf_api.h"
#include "cam_hw_mgr_intf.h"

/* Maximum length of tag while dumping */
#define CAM_ISP_HW_DUMP_TAG_MAX_LEN 32
/* Max isp hw pid values number */
#define CAM_ISP_HW_MAX_PID_VAL      4
/* Maximum number of output ports that map to an architecture specific input path */
#define CAM_ISP_HW_PATH_PORT_MAP_MAX    3

/*
 * MAX len of ISP Resource Name
 */
#define CAM_ISP_RES_NAME_LEN      16

enum cam_isp_bw_control_action {
	CAM_ISP_BW_CONTROL_EXCLUDE       = 0,
	CAM_ISP_BW_CONTROL_INCLUDE       = 1
};

/*
 * struct cam_isp_bw_control_args:
 *
 * @node_res:             Resource node info
 * @action:               Bandwidth control action
 */
struct cam_isp_bw_control_args {
	struct cam_isp_resource_node      *node_res;
	enum cam_isp_bw_control_action     action;
};

/*
 * struct cam_isp_apply_clk_bw_args:
 *
 * @hw_intf:             Isp hw intf pointer
 * @request_id:          Request Id
 * @skip_clk_data_rst:   Skip resetting any clk info
 */
struct cam_isp_apply_clk_bw_args {
	struct cam_hw_intf                *hw_intf;
	uint64_t                           request_id;
	bool                               skip_clk_data_rst;
};

/*
 * struct cam_isp_timestamp:
 *
 * @mono_time:          Monotonic boot time
 * @vt_time:            AV Timer time
 * @ticks:              Qtimer ticks
 */
struct cam_isp_timestamp {
	struct timespec64       mono_time;
	struct timespec64       vt_time;
	uint64_t                ticks;
};

/*
 * cam_isp_hw_get_timestamp()
 *
 * @Brief:              Get timestamp values
 *
 * @time_stamp:         Structure that holds different time values
 *
 * @Return:             Void
 */
void cam_isp_hw_get_timestamp(struct cam_isp_timestamp *time_stamp);

enum cam_isp_hw_type {
	CAM_ISP_HW_TYPE_CSID,
	CAM_ISP_HW_TYPE_ISPIF,
	CAM_ISP_HW_TYPE_VFE,
	CAM_ISP_HW_TYPE_IFE_CSID,
	CAM_ISP_HW_TYPE_TFE,
	CAM_ISP_HW_TYPE_TFE_CSID,
	CAM_ISP_HW_TYPE_TPG,
	CAM_ISP_HW_TYPE_SFE,
	CAM_ISP_HW_TYPE_VCSID,
	CAM_ISP_HW_TYPE_VIFE,
	CAM_ISP_HW_TYPE_MAX,
};

enum cam_isp_hw_split_id {
	CAM_ISP_HW_SPLIT_LEFT       = 0,
	CAM_ISP_HW_SPLIT_RIGHT,
	CAM_ISP_HW_SPLIT_MAX,
};

enum cam_isp_hw_usage_type {
	CAM_ISP_HW_USAGE_TYPE_SINGLE = 0,
	CAM_ISP_HW_USAGE_TYPE_DUAL,
	CAM_ISP_HW_USAGE_TYPE_MAX,
};

enum cam_isp_hw_sync_mode {
	CAM_ISP_HW_SYNC_NONE,
	CAM_ISP_HW_SYNC_MASTER,
	CAM_ISP_HW_SYNC_SLAVE,
	CAM_ISP_HW_SYNC_MAX,
};

enum cam_isp_resource_state {
	CAM_ISP_RESOURCE_STATE_UNAVAILABLE   = 0,
	CAM_ISP_RESOURCE_STATE_AVAILABLE     = 1,
	CAM_ISP_RESOURCE_STATE_RESERVED      = 2,
	CAM_ISP_RESOURCE_STATE_INIT_HW       = 3,
	CAM_ISP_RESOURCE_STATE_STREAMING     = 4,
};

enum cam_isp_resource_type {
	CAM_ISP_RESOURCE_UNINT,
	CAM_ISP_RESOURCE_SRC,
	CAM_ISP_RESOURCE_CID,
	CAM_ISP_RESOURCE_PIX_PATH,
	CAM_ISP_RESOURCE_VFE_IN,
	CAM_ISP_RESOURCE_VFE_BUS_RD,
	CAM_ISP_RESOURCE_VFE_OUT,
	CAM_ISP_RESOURCE_TPG,
	CAM_ISP_RESOURCE_TFE_IN,
	CAM_ISP_RESOURCE_TFE_OUT,
	CAM_ISP_RESOURCE_SFE_IN,
	CAM_ISP_RESOURCE_SFE_RD,
	CAM_ISP_RESOURCE_SFE_OUT,
	CAM_ISP_RESOURCE_MAX,
};

enum cam_isp_hw_cmd_type {
	CAM_ISP_HW_CMD_GET_CHANGE_BASE,
	CAM_ISP_HW_CMD_GET_BUF_UPDATE,
	CAM_ISP_HW_CMD_GET_BUF_UPDATE_RM,
	CAM_ISP_HW_CMD_GET_REG_UPDATE,
	CAM_ISP_HW_CMD_GET_HFR_UPDATE,
	CAM_ISP_HW_CMD_GET_HFR_UPDATE_RM,
	CAM_ISP_HW_CMD_GET_WM_SECURE_MODE,
	CAM_ISP_HW_CMD_GET_RM_SECURE_MODE,
	CAM_ISP_HW_CMD_STRIPE_UPDATE,
	CAM_ISP_HW_CMD_CLOCK_UPDATE,
	CAM_ISP_HW_CMD_BW_UPDATE,
	CAM_ISP_HW_CMD_BW_UPDATE_V2,
	CAM_ISP_HW_CMD_BW_CONTROL,
	CAM_ISP_HW_CMD_STOP_BUS_ERR_IRQ,
	CAM_ISP_HW_CMD_GET_REG_DUMP,
	CAM_ISP_HW_CMD_UBWC_UPDATE,
	CAM_ISP_HW_CMD_DUMP_BUS_INFO,
	CAM_ISP_HW_CMD_SOF_IRQ_DEBUG,
	CAM_ISP_HW_CMD_SET_CAMIF_DEBUG,
	CAM_ISP_HW_CMD_CAMIF_DATA,
	CAM_ISP_HW_CMD_CSID_CLOCK_UPDATE,
	CAM_ISP_HW_CMD_FE_UPDATE_IN_RD,
	CAM_ISP_HW_CMD_FE_UPDATE_BUS_RD,
	CAM_ISP_HW_CMD_UBWC_UPDATE_V2,
	CAM_ISP_HW_CMD_CORE_CONFIG,
	CAM_ISP_HW_CMD_WM_CONFIG_UPDATE,
	CAM_ISP_HW_CMD_CSID_QCFA_SUPPORTED,
	CAM_ISP_HW_CMD_ADD_WAIT,
	CAM_ISP_HW_CMD_ADD_WAIT_TRIGGER,
	CAM_ISP_HW_CMD_QUERY_REGSPACE_DATA,
	CAM_ISP_HW_CMD_TPG_PHY_CLOCK_UPDATE,
	CAM_ISP_HW_CMD_GET_IRQ_REGISTER_DUMP,
	CAM_ISP_HW_CMD_DUMP_HW,
	CAM_ISP_HW_CMD_FE_TRIGGER_CMD,
	CAM_ISP_HW_CMD_UNMASK_BUS_WR_IRQ,
	CAM_ISP_HW_CMD_IS_CONSUMED_ADDR_SUPPORT,
	CAM_ISP_HW_CMD_GET_RES_FOR_MID,
	CAM_ISP_HW_CMD_BLANKING_UPDATE,
	CAM_ISP_HW_CMD_CSID_CLOCK_DUMP,
	CAM_ISP_HW_CMD_TPG_CORE_CFG_CMD,
	CAM_ISP_HW_CMD_CSID_CHANGE_HALT_MODE,
	CAM_ISP_HW_CMD_SET_SFE_DEBUG_CFG,
	CAM_ISP_HW_CMD_QUERY_BUS_CAP,
	CAM_IFE_CSID_CMD_GET_TIME_STAMP,
	CAM_IFE_CSID_SET_CSID_DEBUG,
	CAM_IFE_CSID_SOF_IRQ_DEBUG,
	CAM_IFE_CSID_LOG_ACQUIRE_DATA,
	CAM_IFE_CSID_TOP_CONFIG,
	CAM_IFE_CSID_PROGRAM_OFFLINE_CMD,
	CAM_IFE_CSID_SET_DUAL_SYNC_CONFIG,
	CAM_ISP_HW_CMD_CSID_DYNAMIC_SWITCH_UPDATE,
	CAM_ISP_HW_CMD_CSID_DISCARD_INIT_FRAMES,
	CAM_ISP_HW_CMD_BUF_UPDATE,
	CAM_ISP_HW_CMD_BUF_UPDATE_RM,
	CAM_ISP_HW_NOTIFY_OVERFLOW,
	CAM_ISP_HW_CMD_IS_PDAF_RDI2_MUX_EN,
	CAM_ISP_HW_CMD_GET_PATH_PORT_MAP,
	CAM_ISP_HW_CMD_IFE_BUS_DEBUG_CFG,
	CAM_ISP_HW_SFE_SYS_CACHE_WM_CONFIG,
	CAM_ISP_HW_SFE_SYS_CACHE_RM_CONFIG,
	CAM_ISP_HW_CMD_WM_BW_LIMIT_CONFIG,
	CAM_ISP_HW_CMD_RM_ENABLE_DISABLE,
	CAM_ISP_HW_CMD_APPLY_CLK_BW_UPDATE,
	CAM_ISP_HW_CMD_INIT_CONFIG_UPDATE,
	CAM_ISP_HW_CSID_MINI_DUMP,
	CAM_ISP_HW_BUS_MINI_DUMP,
	CAM_ISP_HW_CMD_RDI_LCR_CFG,
	CAM_ISP_HW_CMD_TUNNEL_ID_UPDATE,
	CAM_ISP_VIRT_POPULATE_REGS,
	CAM_ISP_VIRT_POPULATE_OUT_PORTS,
	CAM_ISP_HW_CMD_UPDATE_CSID_RES_DATA,
	CAM_ISP_HW_CMD_UPDATE_VFE_SRC_RES_DATA,
	CAM_ISP_HW_CMD_UPDATE_VFE_OUT_RES_DATA,
	CAM_ISP_HW_CMD_UPDATE_CSID_RES_IRQ_MASK,
	CAM_ISP_HW_CMD_UPDATE_VFE_SRC_RES_IRQ_MASK,
	CAM_ISP_HW_CMD_UPDATE_VFE_OUT_RES_IRQ_MASK,
	CAM_ISP_HW_CMD_WAIT_LAST_STREAM_SOF,
	CAM_ISP_HW_CMD_CHECK_RUP_FOR_APPLIED_REQ,
	CAM_ISP_HW_CMD_GET_NUM_OUT_RES,
	CAM_ISP_HW_CMD_MAX,
};

/*
 * struct cam_isp_resource_node:
 *
 * @Brief:                        Structure representing HW resource object
 *
 * @res_type:                     Resource Type
 * @res_id:                       Unique resource ID within res_type objects
 *                                for a particular HW
 * @res_state:                    State of the resource
 * @hw_intf:                      HW Interface of HW to which this resource
 *                                belongs
 * @res_priv:                     Private data of the resource
 * @list:                         list_head node for this resource
 * @cdm_ops:                      CDM operation functions
 * @worker_info:                   Workq structure that will be used to
 *                                schedule IRQ events related to this resource
 * @irq_handle:                   handle returned on subscribing for IRQ event
 * @rdi_only_ctx:                 resource belong to rdi only context or not
 * @init:                         function pointer to init the HW resource
 * @deinit:                       function pointer to deinit the HW resource
 * @start:                        function pointer to start the HW resource
 * @stop:                         function pointer to stop the HW resource
 * @process_cmd:                  function pointer for processing commands
 *                                specific to the resource
 * @top_half_handler:             Top Half handler function
 * @bottom_half_handler:          Bottom Half handler function
 * @res_name:                     Name of resource
 * @is_per_port_start:            Indicates start_hw is called on real streamon call or
 *                                on per port streamon call
 * @is_per_port_acquire:          Indicates if resource is yet to be really acquired
 */
struct cam_isp_resource_node {
	enum cam_isp_resource_type     res_type;
	uint32_t                       res_id;
	enum cam_isp_resource_state    res_state;
	struct cam_hw_intf            *hw_intf;
	void                          *res_priv;
	struct list_head               list;
	void                          *cdm_ops;
	void                          *worker_info;
	int                            irq_handle;
	int                            rdi_only_ctx;

	int (*init)(struct cam_isp_resource_node *rsrc_node,
		void *init_args, uint32_t arg_size);
	int (*deinit)(struct cam_isp_resource_node *rsrc_node,
		void *deinit_args, uint32_t arg_size);
	int (*start)(struct cam_isp_resource_node *rsrc_node);
	int (*stop)(struct cam_isp_resource_node *rsrc_node);
	int (*process_cmd)(struct cam_isp_resource_node *rsrc_node,
		uint32_t cmd_type, void *cmd_args, uint32_t arg_size);
	CAM_IRQ_HANDLER_TOP_HALF       top_half_handler;
	CAM_IRQ_HANDLER_BOTTOM_HALF    bottom_half_handler;
	char                           res_name[CAM_ISP_RES_NAME_LEN];
	bool                           is_per_port_start;
	bool                           is_per_port_acquire;
};

/*
 * struct cam_isp_blanking_config:
 *
 * @Brief:          Structure to pass blanking details
 * @hbi:            HBI Value
 * @vbi:            VBI Value
 * node_res:        Pointer to Resource Node object
 */
struct cam_isp_blanking_config {
	uint32_t                           hbi;
	uint32_t                           vbi;
	struct cam_isp_resource_node       *node_res;
};

/**
 * struct cam_isp_hw_error_event_info:
 *
 * @brief:              Structure to pass error event details to hw mgr
 *
 * @err_type:           Type of error being reported
 *
 */
struct cam_isp_hw_error_event_info {
	uint32_t    err_type;
};

/**
 * struct cam_isp_hw_compdone_event_info:
 *
 * @brief:              Structure to pass bufdone event details to hw mgr
 *
 * @num_res:            Number of valid resource IDs in this event
 * @res_id:             Resource IDs to report buf dones
 * @last_consumed_addr: Last consumed addr for resource ID at that index
 *
 */
struct cam_isp_hw_compdone_event_info {
	uint32_t num_res;
	uint32_t res_id[CAM_NUM_OUT_PER_COMP_IRQ_MAX];
	uint32_t last_consumed_addr[CAM_NUM_OUT_PER_COMP_IRQ_MAX];
};

/*
 * struct cam_isp_hw_event_info:
 *
 * @Brief:             Structure to pass event details to hw mgr
 *
 * @res_type:          Type of IFE resource
 * @is_secondary_evt:  Indicates if event was requested by hw mgr
 * @res_id:            Unique resource ID
 * @hw_idx:            IFE hw index
 * @reg_val:           Any critical register value captured during irq handling
 * @hw_type:           Hw Type sending the event
 * @in_core_idx:       Input core type if CSID error evt
 * @event_data:        Any additional data specific to this event
 *
 */
struct cam_isp_hw_event_info {
	enum cam_isp_resource_type     res_type;
	bool                           is_secondary_evt;
	uint32_t                       res_id;
	uint32_t                       hw_idx;
	uint32_t                       reg_val;
	uint32_t                       hw_type;
	uint32_t                       in_core_idx;
	void                          *event_data;
};

/*
 * struct cam_isp_hw_cmd_buf_update:
 *
 * @Brief:           Contain the new created command buffer information
 *
 * @cmd_buf_addr:    Command buffer to store the change base command
 * @size:            Size of the buffer in bytes
 * @used_bytes:      Consumed bytes in the command buffer
 *
 */
struct cam_isp_hw_cmd_buf_update {
	uint32_t                       *cmd_buf_addr;
	uint32_t                        size;
	uint32_t                        used_bytes;
};

/*
 * struct cam_isp_hw_get_wm_update:
 *
 * @Brief:             Get cmd buffer for WM updates.
 *
 * @ image_buf:        image buffer address array
 * @ image_buf_offset: image buffer address offset array
 * @ num_buf:          Number of buffers in the image_buf array
 * @ frame_header:     frame header iova
 * @ fh_enabled:       flag to indicate if this WM enables frame header
 * @ local_id:         local id for the wm
 * @ width:            width of scratch buffer
 * @ height:           height of scratch buffer
 * @ stride:           stride of scratch buffer
 * @ slice_height:     slice height of scratch buffer
 * @ io_cfg:           IO buffer config information sent from UMD
 */
struct cam_isp_hw_get_wm_update {
	dma_addr_t                     *image_buf;
	uint32_t                        image_buf_offset[CAM_PACKET_MAX_PLANES];
	uint32_t                        num_buf;
	uint64_t                        frame_header;
	bool                            fh_enabled;
	uint32_t                        local_id;
	uint32_t                        width;
	uint32_t                        height;
	uint32_t                        stride;
	uint32_t                        slice_height;
	struct cam_buf_io_cfg          *io_cfg;
};

/*
 * struct cam_isp_hw_get_res_for_mid:
 *
 * @Brief:           Get the out resource id for given mid
 *
 * @mid:             Mid number of hw outport numb
 * @out_res_id:      Out resource id
 *
 */
struct cam_isp_hw_get_res_for_mid {
	uint32_t                       mid;
	uint32_t                       out_res_id;
};

/*
 * struct cam_isp_hw_get_cmd_update:
 *
 * @Brief:          Get cmd buffer update for different CMD types
 *
 * @res:             Resource node
 * @req_id:          Request id
 * @cmd_type:        Command type for which to get update
 * @cdm_id  :        CDM id
 * @cmd:             Command buffer information
 * @res:             Resource node
 * @cmd_type:        Command type for which to get update
 * @cmd:             Command buffer information
 * @use_scratch_cfg: To indicate if it's scratch buffer config
 * @trigger_cdm_en:  Flag to indicate if cdm is trigger
 *
 */
struct cam_isp_hw_get_cmd_update {
	struct cam_isp_resource_node     *res;
	uint64_t                          req_id;
	enum cam_isp_hw_cmd_type          cmd_type;
	enum cam_cdm_id                   cdm_id;
	struct cam_isp_hw_cmd_buf_update  cmd;
	bool                              use_scratch_cfg;
	union {
		void                                 *data;
		struct cam_isp_hw_get_wm_update      *wm_update;
		struct cam_isp_hw_get_wm_update      *rm_update;
	};
	bool trigger_cdm_en;
};

/*
 * struct cam_isp_hw_dual_isp_update_args:
 *
 * @Brief:        update the dual isp striping configuration.
 *
 * @ split_id:    spilt id to inform left or rifht
 * @ res:         resource node
 * @ dual_cfg:    dual isp configuration
 *
 */
struct cam_isp_hw_dual_isp_update_args {
	enum cam_isp_hw_split_id         split_id;
	struct cam_isp_resource_node    *res;
	struct cam_isp_dual_config      *dual_cfg;
};

/*
 * struct cam_isp_hw_dump_args:
 *
 * @Brief:        isp hw dump args
 *
 * @ req_id:         request id
 * @ cpu_addr:       cpu address
 * @ buf_len:        buf len
 * @ offset:         offset of buffer
 * @ ctxt_to_hw_map: ctx to hw map
 * @ is_dump_all:    flag to indicate if all information or just bw/clk rate
 */
struct cam_isp_hw_dump_args {
	uint64_t                req_id;
	uintptr_t               cpu_addr;
	size_t                  buf_len;
	size_t                  offset;
	void                   *ctxt_to_hw_map;
	bool                    is_dump_all;
};

/**
 * struct cam_isp_hw_dump_header - ISP context dump header
 *
 * @Brief:        isp hw dump header
 *
 * @tag:       Tag name for the header
 * @word_size: Size of word
 * @size:      Size of data
 *
 */
struct cam_isp_hw_dump_header {
	uint8_t   tag[CAM_ISP_HW_DUMP_TAG_MAX_LEN];
	uint64_t  size;
	uint32_t  word_size;
};

/**
 * struct cam_isp_hw_intf_data - ISP hw intf data
 *
 * @Brief:        isp hw intf pointer and pid list data
 *
 * @isp_hw_intf:      Isp hw intf pointer
 * @num_hw_pid:       Number of pids for this hw
 * @isp_hw_pid:       Isp hw pid values
 *
 */
struct cam_isp_hw_intf_data {
	struct cam_hw_intf     *hw_intf;
	uint32_t                num_hw_pid;
	uint32_t                hw_pid[CAM_ISP_HW_MAX_PID_VAL];
};
/**
 * struct cam_isp_hw_bus_cap:
 *
 * @Brief:         ISP hw bus capabilities
 *
 * @support_consumed_addr:  Indicate whether HW has last consumed addr reg
 * @max_out_res_type:       Maximum value of out resource type supported by hw
 * @out_fifo_depth:         Maximum output fifo depth
 *
 */
struct cam_isp_hw_bus_cap {
	bool                    support_consumed_addr;
	uint32_t                max_out_res_type;
	uint32_t                out_fifo_depth;
};

/**
 * struct cam_isp_hw_path_port_map:
 *
 * @Brief:         ISP hw bus capabilities
 *
 * @num_entries:  Number of entries
 * @entry:        Each row is an entry with the following structure:
 *                col #1: VFE IN path type
 *                col #2: ISP OUT resource type
 */
struct cam_isp_hw_path_port_map {
	uint32_t                num_entries;
	uint32_t                entry[CAM_ISP_HW_PATH_PORT_MAP_MAX][2];
};


/**
 * struct cam_isp_hw_init_config_update:
 *
 * @Brief:         Init config params for CSID/SFE/IFE resources
 *
 * @node_res:      HW Resource
 * @init_config:   Init config params from userspace
 */
struct cam_isp_hw_init_config_update {
	struct cam_isp_resource_node   *node_res;
	struct cam_isp_init_config     *init_config;
};

#endif /* _CAM_ISP_HW_H_ */
