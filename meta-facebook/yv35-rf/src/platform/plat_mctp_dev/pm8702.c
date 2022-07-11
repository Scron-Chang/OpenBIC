#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <sys/printk.h>
#include "mctp.h"
#include "mctp_cci.h"
#include "mctp_ctrl.h"
#include "pm8702.h"

#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_DECLARE(plat_mctp);

pm8702_cache_t pm8702_cache = {
	.mctp_inst = NULL,
	.revision = "",
	.eid = 0,
	.temperature = NAN,
	.wait_resp = false,
	.available = false,
	.ready = false,
};

static void pm8702_default_resp_timeout_handler(void *args)
{
	pm8702_cache.wait_resp = false;
	pm8702_cache.available = false;

	LOG_ERR("PM8702 %s timeout", (const char *)args);
}

// PM8702 Tasks
static void get_endpoint_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);
	pm8702_cache.wait_resp = false;

	if (!buf || !len) {
		LOG_ERR("[%s] Received package is corrupted.", __func__);
		return;
	}

	LOG_HEXDUMP_DBG(buf, len, __func__);

	//In the response package:
	//  - Byte[0] is the complete code.
	//  - Byte[1] is the EID.
	//  TODO: Consider the endpoint type.
	if (buf[0] != MCTP_CTRL_CC_SUCCESS) {
		LOG_ERR("[%s] Error: 0x%.2X", __func__, buf[0]);
		return;
	}

	LOG_DBG("Get PM8702 EID: 0x%.2X", pm8702_cache.eid = buf[1]);
	pm8702_cache.available = true;
}
static void get_dev_endpoint(void)
{
	mctp_ctrl_msg msg = { 0 };

	msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
	msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
	msg.ext_params.ep = pm8702_cache.eid;

	msg.hdr.cmd = MCTP_CTRL_CMD_GET_ENDPOINT_ID;
	msg.hdr.rq = 1; // MCTP base protocol: 0 is response, 1 is request.

	msg.recv_resp_cb_fn = get_endpoint_resp_handler;
	msg.timeout_cb_fn = pm8702_default_resp_timeout_handler;
	msg.timeout_cb_fn_args = (void *)__func__;

	pm8702_cache.wait_resp = true;
	mctp_ctrl_send_msg(pm8702_cache.mctp_inst, &msg);
}

static void set_endpoint_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);
	pm8702_cache.wait_resp = false;

	if (!buf || !len) {
		LOG_ERR("[%s] Received package is corrupted.", __func__);
		return;
	}

	LOG_HEXDUMP_DBG(buf, len, __func__);

	//In the response package:
	//  - Byte[0] is the complete code.
	//  - Byte[2] is the EID carried in request package.
	//  TODO: Consider the endpoint type.
	if (buf[0] != MCTP_CTRL_CC_SUCCESS) {
		LOG_ERR("[%s] Error: 0x%.2X", __func__, buf[0]);
		return;
	}

	if (buf[2] != PM8702_DEF_EID) {
		LOG_ERR("[%s] Failure: rejected by the PM8702(bus:%.2d,Address:%.2X)", __func__,
			PM8702_I2C_BUS, PM8702_I2C_ADDR);
		return;
	}
	// Use the printf rather than LOG to ensure the command will be executed.
	printf("Set PM8702 EID: 0x%.2X", pm8702_cache.eid = buf[2]);
}
static void set_dev_endpoint(void)
{
	struct _set_eid_req req = { .op = SET_EID_REQ_OP_SET_EID, .eid = PM8702_DEF_EID };
	mctp_ctrl_msg msg = { 0 };

	msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
	msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
	msg.ext_params.ep = pm8702_cache.eid;

	msg.hdr.cmd = MCTP_CTRL_CMD_SET_ENDPOINT_ID;
	msg.hdr.rq = 1;

	msg.cmd_data = (uint8_t *)&req;
	msg.cmd_data_len = sizeof(req);

	msg.recv_resp_cb_fn = set_endpoint_resp_handler;
	msg.timeout_cb_fn = pm8702_default_resp_timeout_handler;
	msg.timeout_cb_fn_args = (void *)__func__;

	pm8702_cache.wait_resp = true;
	mctp_ctrl_send_msg(pm8702_cache.mctp_inst, &msg);
}

static void generic_identify_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);
	pm8702_cache.wait_resp = false;

	if (!buf || !len) {
		LOG_ERR("[%s] Received package is corrupted.", __func__);
		return;
	}

	LOG_HEXDUMP_WRN(buf, len, __func__);

	//In the response package:
	//  - Byte[0] is the complete code.
	//  - Byte[16] indicates the type of component.
	if (buf[0] != MCTP_CTRL_CC_SUCCESS) {
		LOG_ERR("[%s] Error: 0x%.2X", __func__, buf[0]);
		return;
	}

	LOG_DBG("Get PM8702 Type: 0x%.2X", buf[16]);
	pm8702_cache.ready =
		(buf[16] == 0x03); // 3 indicates the type of component is CCI Type3 Device.
}
static void generic_identify(void)
{
	mctp_cci_msg req_msg = { 0 };

	req_msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
	req_msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
	req_msg.ext_params.ep = pm8702_cache.eid;

	req_msg.hdr.cmd_opcode = PM8702_GENERIC_COMPONENT_IDENTIFY;
	req_msg.hdr.msg_cat = MCTP_CCI_MSG_CATE_REQUEST;

	req_msg.msg_payload_len = 0;

	req_msg.recv_resp_cb_fn = generic_identify_resp_handler;
	req_msg.timeout_cb_fn = pm8702_default_resp_timeout_handler;
	req_msg.timeout_cb_fn_args = (void *)__func__;

	pm8702_cache.wait_resp = true;
	mctp_cci_send_msg(pm8702_cache.mctp_inst, &req_msg);
}

static void identify_mem_dev_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);
	pm8702_cache.wait_resp = false;

	if (!buf || !len) {
		printf("[%s] Received package is corrupted.\n", __func__);
		return;
	}

	LOG_HEXDUMP_WRN(buf, len, __func__);

	//In the response package:
	//  - Byte[0] is the complete code.
	//  - Byte[16] indicates the type of component.
	if (buf[0] != MCTP_CTRL_CC_SUCCESS) {
		printf("[%s] Error: 0x%.2X\n", __func__, buf[0]);
		return;
	}

	memcpy(&pm8702_cache.revision, &buf[1], sizeof(pm8702_cache.revision));
	pm8702_cache.revision[16] = '\0'; //End the string with the '\0' charactor.
}
static uint8_t identify_mem_dev(void)
{
	mctp_cci_msg req_msg = { 0 };

	req_msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
	req_msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
	req_msg.ext_params.ep = pm8702_cache.eid;

	req_msg.hdr.cmd_opcode = MCTP_CCI_CMD_IDENTIFY_MEM_DEV;
	req_msg.hdr.msg_cat = MCTP_CCI_MSG_CATE_REQUEST;

	req_msg.msg_payload_len = 0;

	req_msg.recv_resp_cb_fn = identify_mem_dev_resp_handler;
	req_msg.timeout_cb_fn = pm8702_default_resp_timeout_handler;
	req_msg.timeout_cb_fn_args = (void *)__func__;

	pm8702_cache.wait_resp = true;
	return mctp_cci_send_msg(pm8702_cache.mctp_inst, &req_msg);
}

static void get_dev_health_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);

	if (!buf || !len)
		return;

	LOG_HEXDUMP_WRN(buf, len, __func__);
}
static uint8_t get_dev_health(void)
{
	mctp_cci_msg req_msg = { 0 };

	req_msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
	req_msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
	req_msg.ext_params.ep = pm8702_cache.eid;

	req_msg.hdr.cmd_opcode = MCTP_CCI_CMD_GET_HEALTH_INFO;
	req_msg.hdr.msg_cat = MCTP_CCI_MSG_CATE_REQUEST;

	req_msg.msg_payload_len = 0;

	req_msg.recv_resp_cb_fn = get_dev_health_resp_handler;
	req_msg.timeout_cb_fn = pm8702_default_resp_timeout_handler;
	req_msg.timeout_cb_fn_args = (void *)__func__;

	pm8702_cache.wait_resp = true;
	return mctp_cci_send_msg(pm8702_cache.mctp_inst, &req_msg);
}

static void init_pm8702(void)
{
	enum pm8702_task_stage {
		get_eid,
		set_eid,
		get_ready,
		get_reviseion,
		pm8702_initialized,
	};

	uint8_t task_stage = get_eid;

	while (task_stage != pm8702_initialized) {
		if (pm8702_cache.wait_resp) {
			//Wait for the response
			k_msleep(2);
			continue;
		}

		switch (task_stage) {
		case get_eid:

			get_dev_endpoint();

			if (pm8702_cache.available) {
				task_stage++;
			}

			break;

		case set_eid:

			if (pm8702_cache.eid == 0) {
				LOG_INF("Try to set EID to the PM8702\n");
				set_dev_endpoint();
			} else {
				task_stage++;
			}
			break;

		case get_ready:

			identify();

			if (pm8702_cache.ready) {
				task_stage++;
			}

			break;

		case get_reviseion:

			if (strlen(pm8702_cache.revision) != 0) {
				task_stage++;
			} else {
				identify_mem_dev();
			}

			break;

		default:
			LOG_ERR("Unexpected situation");
			break;
		}

		k_msleep(7000); // For debug
	}
}

void run_pm8702_tasks(void *arug0, void *arug1, void *arug2)
{
	ARG_UNUSED(arug0);
	ARG_UNUSED(arug1);
	ARG_UNUSED(arug2);

	// Init_pm8702 will block here if the initialization process fails.
	init_pm8702();

	LOG_INF("PM8702(PIONEER) is ready.");

	// TODO: Use the static list.
	while (1) {
#ifdef MCTP_BASED_CCI_TEST
		identify();
		k_msleep(1000);
		continue;
#endif
		get_dev_health();
		k_msleep(1000);
	}
}
