#ifndef _PLDM_BASE_H
#define _PLDM_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pldm.h"
#include <stdint.h>

/* commands of pldm type 0x00 : PLDM_TYPE_CTRL_DISCOV */
#define PLDM_BASE_CMD_CODE_SETTID 0x01
#define PLDM_BASE_CMD_CODE_GETTID 0x02
#define PLDM_BASE_CMD_CODE_GET_PLDM_VER 0x03
#define PLDM_BASE_CMD_CODE_GET_PLDM_TYPE 0x04
#define PLDM_BASE_CMD_CODE_GET_PLDM_CMDS 0x05

#define DEFAULT_TID 0x86

enum pldm_completion_codes {
	PLDM_SUCCESS = 0x00,
	PLDM_ERROR = 0x01,
	PLDM_ERROR_INVALID_DATA = 0x02,
	PLDM_ERROR_INVALID_LENGTH = 0x03,
	PLDM_ERROR_NOT_READY = 0x04,
	PLDM_ERROR_UNSUPPORTED_PLDM_CMD = 0x05,
	PLDM_ERROR_INVALID_PLDM_TYPE = 0x20,
	PLDM_INVALID_TRANSFER_OPERATION_FLAG = 0x21,
	/* Use reserved region for oem define */
	PLDM_LATER_RESP = 0x30,
};

struct _set_tid_req {
	uint8_t tid;
} __attribute__((packed));

struct _set_tid_resp {
	uint8_t completion_code;
} __attribute__((packed));

struct _get_tid_resp {
	uint8_t completion_code;
	uint8_t tid;
} __attribute__((packed));

uint8_t pldm_base_handler_query(uint8_t code, void **ret_fn);

#ifdef __cplusplus
}
#endif

#endif /* _PLDM_BASE_H */