#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <sys/printk.h>
#include "libutil.h"
#include "mctp.h"
#include "mctp_cci.h"
#include "mctp_ctrl.h"
#include "pm8702.h"

#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_DECLARE(plat_mctp);

#define RESERVED_CMD 0xFFFF
typedef struct {
	uint16_t cmd;
	void (*resp_handler)(void *, uint8_t *, uint16_t);
	void (*resp_timeout_handler)(void *);
	char *cmd_name;
} pm8702_cmds_t;

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

	LOG_ERR("[PM8702] Command timeout: %s ",
		(args == NULL) ? "unregisted function" : (const char *)args);
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

static void get_mctp_ver_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);

	if (!buf || !len)
		return;

	LOG_HEXDUMP_WRN(buf, len, __func__);
}

static void identify_resp_handler(void *args, uint8_t *buf, uint16_t len)
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

static void get_dev_health_resp_handler(void *args, uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(args);

	if (!buf || !len)
		return;

	LOG_HEXDUMP_WRN(buf, len, __func__);
}

static pm8702_cmds_t pm8702_mctp_ctrl_cmds[] = {
	{
		.cmd = MCTP_CTRL_CMD_SET_ENDPOINT_ID,
		.resp_handler = set_endpoint_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_Ctrl[01h]: Set endpoing ID",
	},
	{
		.cmd = MCTP_CTRL_CMD_GET_ENDPOINT_ID,
		.resp_handler = get_endpoint_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_Ctrl[02h]: Get endpoing ID",
	},
	{
		.cmd = MCTP_CTRL_CMD_GET_MCTP_VERSION_SUPPORT,
		.resp_handler = get_mctp_ver_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_Ctrl[04h]: Get MCTP Version Support",
	},
	{
		.cmd = RESERVED_CMD, //Reserved for indicating the end of the cmd table.
		.resp_handler = NULL,
		.resp_timeout_handler = NULL,
		.cmd_name = NULL,
	}
};

static pm8702_cmds_t pm8702_mctp_cci_cmds[] = {
	{
		.cmd = MCTP_CCI_CMD_IDENTIFY,
		.resp_handler = identify_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_CCI[0001h]: Information and Status: Idenfigy",
	},
	{
		.cmd = MCTP_CCI_CMD_IDENTIFY_MEM_DEV,
		.resp_handler = identify_mem_dev_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_CCI[4000h]: Identify Memory Device: Idenfify Memory Device",
	},
	{
		.cmd = MCTP_CCI_CMD_GET_HEALTH_INFO,
		.resp_handler = get_dev_health_resp_handler,
		.resp_timeout_handler = pm8702_default_resp_timeout_handler,
		.cmd_name = "MCTP_CCI[4200h]: Health Info and Alerts: Get Health Info",
	},
	{
		.cmd = RESERVED_CMD, //Reserved for indicating the end of the cmd table.
		.resp_handler = NULL,
		.resp_timeout_handler = NULL,
		.cmd_name = NULL,
	}
};

static uint8_t pm8702_mctp_ctrl_req_data_handler(void *ctrl_msg, uint8_t *data_buf)
{
	mctp_ctrl_msg *mctp_ctrl_msg_inst = (mctp_ctrl_msg *)ctrl_msg;

	switch (mctp_ctrl_msg_inst->hdr.cmd) {
	case MCTP_CTRL_CMD_SET_ENDPOINT_ID: {
		struct _set_eid_req req = {
			.op = SET_EID_REQ_OP_SET_EID,
			.eid = 0xBE, // TBD. 0xBE just for test.
		};

		if ((data_buf = (uint8_t *)malloc(sizeof(req))) == NULL) {
			return MCTP_ERROR;
		}

		memcpy(data_buf, &req, sizeof(req));

		mctp_ctrl_msg_inst->cmd_data = data_buf;
		mctp_ctrl_msg_inst->cmd_data_len = sizeof(req);
	} break;
	case MCTP_CTRL_CMD_GET_MCTP_VERSION_SUPPORT: {
		if ((data_buf = (uint8_t *)malloc(sizeof(uint8_t))) == NULL) {
			return MCTP_ERROR;
		}

		*data_buf = MCTP_MSG_TYPE_CCI;

		mctp_ctrl_msg_inst->cmd_data = data_buf;
		mctp_ctrl_msg_inst->cmd_data_len = sizeof(uint8_t);
	}
	default:
		// mctp_ctrl_send_msg always returns MCTP_ERROR when the msg->cmd_data
		// points to NULL, so assign 1 byte to it and set the data len to zero.
		if ((data_buf = (uint8_t *)malloc(sizeof(uint8_t))) == NULL) {
			return MCTP_ERROR;
		}

		mctp_ctrl_msg_inst->cmd_data = data_buf;
		mctp_ctrl_msg_inst->cmd_data_len = 0;
		break;
	}

	return MCTP_SUCCESS;
}

static uint8_t pm8702_mctp_cci_req_data_handler(void *cci_msg, uint8_t *data_buf)
{
	mctp_cci_msg *mctp_ctrl_msg_inst = (mctp_cci_msg *)cci_msg;

	switch (mctp_ctrl_msg_inst->hdr.cmd_opcode) {
	case MCTP_CCI_CMD_IDENTIFY:
	case MCTP_CCI_CMD_IDENTIFY_MEM_DEV:
	case MCTP_CCI_CMD_GET_HEALTH_INFO:
		// There is no payload for the request package.
		return mctp_cci_set_data_len(cci_msg, 0);

	default:
		LOG_ERR("[%s] Command not supported: %.4Xh", __func__,
			mctp_ctrl_msg_inst->hdr.cmd_opcode);
		return MCTP_ERROR;
	}

	return MCTP_SUCCESS;
}

static uint8_t pm8702_cmd_handler(uint8_t msg_type, uint16_t cmd)
{
	int ret = MCTP_ERROR;
	pm8702_cmds_t *cmd_ptr = NULL;
	uint8_t *req_data_buf = NULL;

	switch (msg_type) {
	case MCTP_MSG_TYPE_CTRL:
		cmd_ptr = pm8702_mctp_ctrl_cmds;
		break;
	case MCTP_MSG_TYPE_CCI:
		cmd_ptr = pm8702_mctp_cci_cmds;
		break;
	default:
		LOG_ERR("Wrong message type:[%.2X]", msg_type);
		goto out;
	}

	// Find the command.
	for (; cmd_ptr->cmd != cmd; cmd_ptr++) {
		if (cmd_ptr->cmd == RESERVED_CMD) {
			LOG_ERR("[%s] Command(Type:%.2X, Cmd:%.4X) not found", __func__, msg_type,
				cmd);
			goto out;
		}
	}

	switch (msg_type) {
	case MCTP_MSG_TYPE_CTRL: {
		mctp_ctrl_msg ctrl_msg = { 0 };

		ctrl_msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
		ctrl_msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
		ctrl_msg.ext_params.ep = pm8702_cache.eid;

		ctrl_msg.hdr.cmd = cmd_ptr->cmd;
		ctrl_msg.hdr.rq = 1; // MCTP base protocol: 0 is response, 1 is request.

		if (pm8702_mctp_ctrl_req_data_handler(&ctrl_msg, req_data_buf)) {
			LOG_ERR("[%s] Command %s: Fail to encode request package.", __func__,
				cmd_ptr->cmd_name);
			goto out;
		}

		ctrl_msg.recv_resp_cb_fn = cmd_ptr->resp_handler;
		ctrl_msg.timeout_cb_fn = cmd_ptr->resp_timeout_handler;
		ctrl_msg.timeout_cb_fn_args = (void *)cmd_ptr->cmd_name;

		ret = mctp_ctrl_send_msg(pm8702_cache.mctp_inst, &ctrl_msg);
	} break;
	case MCTP_MSG_TYPE_CCI: {
		mctp_cci_msg cci_msg = { 0 };

		cci_msg.ext_params.type = MCTP_MEDIUM_TYPE_SMBUS;
		cci_msg.ext_params.smbus_ext_params.addr = PM8702_I2C_ADDR;
		cci_msg.ext_params.ep = pm8702_cache.eid;

		cci_msg.hdr.cmd_opcode = cmd_ptr->cmd;
		cci_msg.hdr.msg_cate = MCTP_CCI_MSG_CATE_REQUEST;

		if (pm8702_mctp_cci_req_data_handler(&cci_msg, req_data_buf)) {
			LOG_ERR("[%s] Command %s: Fail to encode request package.", __func__,
				cmd_ptr->cmd_name);
			goto out;
		}

		cci_msg.msg_payload_len = 0;
		cci_msg.recv_resp_cb_fn = cmd_ptr->resp_handler;
		cci_msg.timeout_cb_fn = cmd_ptr->resp_timeout_handler;
		cci_msg.timeout_cb_fn_args = cmd_ptr->cmd_name;

		ret = mctp_cci_send_msg(pm8702_cache.mctp_inst, &cci_msg);
	} break;
	default:
		printf("Wrong message type:[%.2X]\n", msg_type);
		break;
	}

out:
	SAFE_FREE(req_data_buf);

	// Set the waiting flag if msg passed sucessfully.
	if (ret == MCTP_SUCCESS) {
		pm8702_cache.wait_resp = true;
	}

	return ret;
}

static void init_pm8702(void)
{
	enum pm8702_init_sequence {
		get_eid,
		set_eid,
		get_ready,
		get_reviseion,
		pm8702_initialized,
	};

	static uint8_t task_stage = get_eid;

	while (task_stage != pm8702_initialized) {
		if (pm8702_cache.wait_resp) {
			//Wait for the response
			k_msleep(2);
			continue;
		}

		switch (task_stage) {
		case get_eid:

			//get_dev_endpoint();
			pm8702_cmd_handler(MCTP_MSG_TYPE_CTRL, MCTP_CTRL_CMD_GET_ENDPOINT_ID);

			if (pm8702_cache.available) {
				task_stage++;
			}

			break;

		case set_eid:

			if (pm8702_cache.eid == 0) {
				LOG_INF("Try to set EID to the PM8702\n");
				pm8702_cmd_handler(MCTP_MSG_TYPE_CTRL,
						   MCTP_CTRL_CMD_SET_ENDPOINT_ID);
			} else {
				task_stage++;
			}
			break;

		case get_ready:

			pm8702_cmd_handler(MCTP_MSG_TYPE_CCI, MCTP_CCI_CMD_IDENTIFY);

			if (pm8702_cache.ready) {
				task_stage++;
			}

			break;

		case get_reviseion:

			if (strlen(pm8702_cache.revision) != 0) {
				task_stage++;
			} else {
				pm8702_cmd_handler(MCTP_MSG_TYPE_CCI,
						   MCTP_CCI_CMD_IDENTIFY_MEM_DEV);
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
		pm8702_cmd_handler(MCTP_MSG_TYPE_CCI, MCTP_CCI_CMD_IDENTIFY);
		k_msleep(1000);
		continue;
#endif
		// Do routine tasks
		k_msleep(1000);
	}
}

//////////////////////////////////////////////////////////////////////
// REMOVE FOLLOWING DEBUG CODE WHEN SUBMITTING TO OPENBIC UPSTREAM. //
//////////////////////////////////////////////////////////////////////

#include <shell/shell.h>
static int test_pm8702_identify(const struct shell *shell, size_t argc, char **argv)
{
	pm8702_cmd_handler(MCTP_MSG_TYPE_CCI, MCTP_CCI_CMD_IDENTIFY);
	return 0;
}
static int test_pm8702_set_eid(const struct shell *shell, size_t argc, char **argv)
{
	pm8702_cmd_handler(MCTP_MSG_TYPE_CTRL, MCTP_CTRL_CMD_SET_ENDPOINT_ID);
	return 0;
}
SHELL_STATIC_SUBCMD_SET_CREATE(sub_pm8702_test,
			       SHELL_CMD(identify, NULL, "Information and Status: Idenfigy(0001h)",
					 test_pm8702_identify),
			       SHELL_CMD(set_eid, NULL, "Set endpoing ID", test_pm8702_set_eid),
			       SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(pm8702, &sub_pm8702_test, "Test PM8702 Cmd", NULL);
