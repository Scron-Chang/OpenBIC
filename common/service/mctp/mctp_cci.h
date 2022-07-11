#ifndef _MCTP_CCI_H
#define _MCTP_CCI_H

#include <stdint.h>
#include <zephyr.h>

enum MCTP_CCI_CMD {
	MCTP_CCI_CMD_IDENTIFY_MEM_DEV = 0x4000,
	MCTP_CCI_CMD_GET_HEALTH_INFO = 0x4200,
};

enum MCTP_CCI_MSG_CATEGORY {
	MCTP_CCI_MSG_CATE_REQUEST = 0x00,
	MCTP_CCI_MSG_CATE_RESPONSE = 0x01,
};

typedef uint8_t (*mctp_cci_cmd_fn)(void *, uint8_t *, uint16_t, uint8_t *, uint16_t *, void *);

typedef struct _mctp_cci_cmd_handler {
	uint16_t cmd_opcode;
	mctp_cci_cmd_fn fn;
} mctp_cci_cmd_handler_t;

typedef struct __attribute__((packed)) {
	// Every MCTP package payload starts with this header.
	union {
		struct {
			uint8_t msg_type : 7;
			uint8_t ic : 1;
		};
		uint8_t msg_hdr;
	};

	struct {
		uint8_t msg_cat : 4; // Category: 0 is Req; 1: is Res
		uint8_t reserved0 : 4;
	};

	uint8_t msg_tag;
	uint8_t reserved1;

	union {
		struct {
			uint8_t cmd_opcode0_7;
			uint8_t cmd_opcode8_15;
		};
		uint16_t cmd_opcode;
	};

	struct {
		uint8_t msg_payload_len0_7;
		uint8_t msg_payload_len8_15;
		uint8_t msg_payload_len16_20 : 5;
		uint8_t reserved : 2;
		uint8_t bo : 1;
	};

	union {
		struct {
			uint8_t ret_code0_7;
			uint8_t ret_code8_15;
		};
		uint16_t ret_code;
	};

	union {
		struct {
			uint8_t vendor_state0_7;
			uint8_t vendor_state8_15;
		};
		uint16_t vendor_state;
	};
} mctp_cci_hdr;

typedef struct {
	mctp_cci_hdr hdr;
	uint8_t *msg_payload;
	uint16_t msg_payload_len;
	mctp_ext_params ext_params;

	void (*recv_resp_cb_fn)(void *, uint8_t *, uint16_t);
	void *recv_resp_cb_args;
	uint16_t timeout_ms;
	void (*timeout_cb_fn)(void *);
	void *timeout_cb_fn_args;
} mctp_cci_msg;

uint8_t mctp_cci_send_msg(void *mctp_p, mctp_cci_msg *msg);
uint8_t mctp_cci_cmd_handler(void *mctp_p, uint8_t *buf, uint32_t len, mctp_ext_params ext_params);

#endif //_MCTP_CCI_H
