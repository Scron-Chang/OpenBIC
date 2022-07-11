#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/util.h>

#include "mctp.h"
#include "mctp_cci.h"
#include "zephyr.h"

#include <stdio.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(mctp);

#define DEFAULT_WAIT_TO_MS 3000
#define TO_CHK_INTERVAL_MS 1000
#define RESP_MSG_PROC_MUTEX_WAIT_TO_MS 1000

typedef struct _wait_msg {
	sys_snode_t node;
	mctp *mctp_inst;
	int64_t exp_to_ms;
	mctp_cci_msg msg;
} wait_msg;

static K_MUTEX_DEFINE(wait_recv_resp_mutex);
static sys_slist_t wait_recv_resp_list = SYS_SLIST_STATIC_INIT(&wait_recv_resp_list);

static uint8_t mctp_cci_msg_timeout_check(sys_slist_t *list, struct k_mutex *mutex)
{
	if (!list || !mutex)
		return MCTP_ERROR;

	if (k_mutex_lock(mutex, K_MSEC(RESP_MSG_PROC_MUTEX_WAIT_TO_MS))) {
		LOG_WRN("MCTP/CCI mutex is locked over %d ms!!", RESP_MSG_PROC_MUTEX_WAIT_TO_MS);
		return MCTP_ERROR;
	}

	sys_snode_t *node;
	sys_snode_t *s_node;
	sys_snode_t *pre_node = NULL;
	int64_t cur_uptime = k_uptime_get();

	SYS_SLIST_FOR_EACH_NODE_SAFE (list, node, s_node) {
		wait_msg *p = (wait_msg *)node;

		if ((p->exp_to_ms <= cur_uptime)) {
			printk("mctp cci msg timeout!!\n");
			printk("cmd_opcode %.4x, msg_tag %.2x\n", p->msg.hdr.cmd_opcode,
			       p->msg.hdr.msg_tag);
			sys_slist_remove(list, pre_node, node);

			if (p->msg.timeout_cb_fn)
				p->msg.timeout_cb_fn(p->msg.timeout_cb_fn_args);

			free(p);
		} else {
			pre_node = node;
		}
	}

	k_mutex_unlock(mutex);
	return MCTP_SUCCESS;
}

static void mctp_cci_msg_timeout_monitor(void *dummy0, void *dummy1, void *dummy2)
{
	ARG_UNUSED(dummy0);
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);

	while (1) {
		k_msleep(TO_CHK_INTERVAL_MS);

		mctp_cci_msg_timeout_check(&wait_recv_resp_list, &wait_recv_resp_mutex);
	}
}

K_THREAD_DEFINE(def_mctp_cci_msg_timeout_monitor, 1024, mctp_cci_msg_timeout_monitor, NULL, NULL,
		NULL, 7, 0, 0);

static uint8_t mctp_cci_cmd_resp_process(mctp *mctp_inst, uint8_t *buf, uint32_t len,
					 mctp_ext_params ext_params)
{
	ARG_UNUSED(ext_params);

	if (!mctp_inst || !buf || !len)
		return MCTP_ERROR;

	if (k_mutex_lock(&wait_recv_resp_mutex, K_MSEC(RESP_MSG_PROC_MUTEX_WAIT_TO_MS))) {
		LOG_WRN("mutex is locked over %d ms!", RESP_MSG_PROC_MUTEX_WAIT_TO_MS);
		return MCTP_ERROR;
	}

	mctp_cci_hdr *hdr = (mctp_cci_hdr *)buf;
	sys_snode_t *node;
	sys_snode_t *s_node;
	sys_snode_t *pre_node = NULL;
	sys_snode_t *found_node = NULL;

	SYS_SLIST_FOR_EACH_NODE_SAFE (&wait_recv_resp_list, node, s_node) {
		wait_msg *p = (wait_msg *)node;
		/* found the proper handler */
		if ((p->msg.hdr.msg_tag == hdr->msg_tag) && (p->mctp_inst == mctp_inst) &&
		    (p->msg.hdr.cmd_opcode == hdr->cmd_opcode)) {
			found_node = node;
			sys_slist_remove(&wait_recv_resp_list, pre_node, node);
			break;
		} else {
			pre_node = node;
		}
	}
	k_mutex_unlock(&wait_recv_resp_mutex);

	if (found_node) {
		/* invoke resp handler */
		wait_msg *p = (wait_msg *)found_node;
		if (p->msg.recv_resp_cb_fn)
			/* remove mctp cci header for handler */
			p->msg.recv_resp_cb_fn(p->msg.recv_resp_cb_args, buf + sizeof(p->msg.hdr),
					       len - sizeof(p->msg.hdr));
		free(p);
	}

	return MCTP_SUCCESS;
}

uint8_t mctp_cci_cmd_handler(void *mctp_p, uint8_t *buf, uint32_t len, mctp_ext_params ext_params)
{
	uint8_t ret = MCTP_ERROR;

	if (!mctp_p || !buf || !len)
		return MCTP_ERROR;

	mctp *mctp_inst = (mctp *)mctp_p;
	mctp_cci_hdr *hdr = (mctp_cci_hdr *)buf;

	switch (hdr->msg_cat) {
	case MCTP_CCI_MSG_CATE_RESPONSE:
		ret = mctp_cci_cmd_resp_process(mctp_inst, buf, len, ext_params);
		break;

	case MCTP_CCI_MSG_CATE_REQUEST:
		printf("Now Yv35-rf doesn't support request package\n");
		break;

	default:
		printf("[%s] Invalid msg cat: %.2x\n", __func__, hdr->msg_cat);
		break;
	}

	return ret;
}

uint8_t mctp_cci_send_msg(void *mctp_p, mctp_cci_msg *msg)
{
	if (!mctp_p || !msg)
		return MCTP_ERROR;

	mctp *mctp_inst = (mctp *)mctp_p;

	if (msg->hdr.msg_cat == MCTP_CCI_MSG_CATE_REQUEST) {
		static uint8_t _msg_tag;

		msg->hdr.msg_tag = _msg_tag++;
		msg->hdr.msg_type = MCTP_MSG_TYPE_CCI;

		msg->ext_params.tag_owner = 1;
	}

	uint16_t len = sizeof(msg->hdr) + msg->msg_payload_len;
	uint8_t buf[len];

	memcpy(buf, &msg->hdr, sizeof(msg->hdr));
	memcpy(buf + sizeof(msg->hdr), msg->msg_payload, msg->msg_payload_len);

	LOG_HEXDUMP_DBG(buf, len, __func__);

	uint8_t rc = mctp_send_msg(mctp_inst, buf, len, msg->ext_params);
	if (rc == MCTP_ERROR) {
		LOG_WRN("mctp_send_msg error!!");
		return MCTP_ERROR;
	}

	if (msg->hdr.msg_cat == MCTP_CCI_MSG_CATE_REQUEST) {
		wait_msg *p = (wait_msg *)malloc(sizeof(*p));
		if (!p) {
			LOG_WRN("wait_msg alloc failed!");
			return MCTP_ERROR;
		}

		memset(p, 0, sizeof(*p));
		p->mctp_inst = mctp_inst;
		p->msg = *msg;
		p->exp_to_ms =
			k_uptime_get() + (msg->timeout_ms ? msg->timeout_ms : DEFAULT_WAIT_TO_MS);

		k_mutex_lock(&wait_recv_resp_mutex, K_FOREVER);
		sys_slist_append(&wait_recv_resp_list, &p->node);
		k_mutex_unlock(&wait_recv_resp_mutex);
	}

	return MCTP_SUCCESS;
}
