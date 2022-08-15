/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <sys/printk.h>
#include "mctp.h"
#include "mctp_cci.h"
#include "mctp_ctrl.h"
#include "plat_dev/pm8702.h"

#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(plat_mctp);

#define MCTP_MSG_TYPE_MASK 0x7F

#define PROCESS_PM8702_TASK_STACK_SIZE 512

k_tid_t process_pm8702_task_tid;
struct k_thread process_pm8702_task_thread_handler;
K_THREAD_STACK_DEFINE(process_pm8702_task_thread, PROCESS_PM8702_TASK_STACK_SIZE);
static void init_process_pm8702_task_thread(void)
{
	if (process_pm8702_task_tid != NULL &&
	    strcmp(k_thread_state_str(process_pm8702_task_tid), "dead") != 0) {
		return;
	}

	process_pm8702_task_tid =
		k_thread_create(&process_pm8702_task_thread_handler, process_pm8702_task_thread,
				K_THREAD_STACK_SIZEOF(process_pm8702_task_thread), run_pm8702_tasks,
				NULL, NULL, NULL, CONFIG_MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&process_pm8702_task_thread_handler, "process_pm8702_task_thread");
}

static uint8_t mctp_msg_recv(void *mctp_p, uint8_t *buf, uint32_t len, mctp_ext_params ext_params)
{
	if (!mctp_p || !buf || !len)
		return MCTP_ERROR;

	/* first byte is message type and ic */
	uint8_t msg_type = buf[0] & MCTP_MSG_TYPE_MASK;

	switch (msg_type) {
	case MCTP_MSG_TYPE_CTRL:
		mctp_ctrl_cmd_handler(mctp_p, buf, len, ext_params);
		break;

	case MCTP_MSG_TYPE_CCI:
		mctp_cci_cmd_handler(mctp_p, buf, len, ext_params);
		break;

	default:
		LOG_WRN("Cannot find message receive function!!");
		return MCTP_ERROR;
	}

	return MCTP_SUCCESS;
}

void plat_mctp_init(void)
{
	mctp_medium_conf bic_conf = { .smbus_conf.bus = I2C_BUS2, .smbus_conf.addr = 0x40 };

	LOG_INF("Init MCTP device: bus = %x, addr = %x", bic_conf.smbus_conf.bus,
		bic_conf.smbus_conf.addr);

	if (!(pm8702_cache.mctp_inst = mctp_cci_init())) {
		LOG_ERR("[%s] Fail to init MCTP/CCI instance", __func__);
		return;
	}

	LOG_INF("Set mctp_inst[%p] medium config %s", pm8702_cache.mctp_inst,
		!mctp_set_medium_configure(pm8702_cache.mctp_inst, MCTP_MEDIUM_TYPE_SMBUS,
					   bic_conf) ?
			"success" :
			"failed");

	mctp_reg_msg_rx_func(pm8702_cache.mctp_inst, mctp_msg_recv);

	// Assign TX/RX queue
	if (mctp_start(pm8702_cache.mctp_inst)) {
		LOG_ERR("[%s] Fail to start the MCTP/CCI TX/RX queue", __func__);
		return;
	}

	return; // Now is the debug stage, so return from here.

	init_process_pm8702_task_thread();
}
