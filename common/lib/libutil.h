#ifndef LIBUTIL_H
#define LIBUTIL_H

#include <stdint.h>
#include "ipmb.h"
#include "hal_i2c.h"

#define SAFE_FREE(p)                                                                               \
	if (p) {                                                                                   \
		free(p);                                                                           \
		p = NULL;                                                                          \
	}

#define SETBIT(x, y) (x | (1ULL << y))
#define GETBIT(x, y) ((x & (1ULL << y)) > y)
#define CLEARBIT(x, y) (x & (~(1ULL << y)))

#define CHECK_NULL_ARG(arg_ptr)                                                                    \
	if (arg_ptr == NULL) {                                                                     \
		LOG_DBG("Parameter \"" #arg_ptr "\" passed in as NULL");                           \
		return;                                                                            \
	}

#define CHECK_NULL_ARG_WITH_RETURN(arg_ptr, ret_val)                                               \
	if (arg_ptr == NULL) {                                                                     \
		LOG_DBG("Parameter \"" #arg_ptr "\" passed in as NULL");                           \
		return ret_val;                                                                    \
	}

#define CHECK_MSGQ_INIT(msgq) CHECK_NULL_ARG((msgq)->buffer_start);

#define CHECK_MSGQ_INIT_WITH_RETURN(msgq, ret_val)                                                 \
	CHECK_NULL_ARG_WITH_RETURN((msgq)->buffer_start, ret_val);

#define CHECK_MUTEX_INIT(mutex) CHECK_NULL_ARG((mutex)->wait_q.waitq.head);

#define CHECK_MUTEX_INIT_WITH_RETURN(mutex, ret_val)                                               \
	CHECK_NULL_ARG_WITH_RETURN((mutex)->wait_q.waitq.head, ret_val);

enum BIT_SETTING_READING {
	BIT_SET = 1,
	BIT_CLEAR = 0,
};

enum BIT_SETTING_READING_N {
	BIT_SET_N = 0,
	BIT_CLEAR_N = 1,
};

ipmi_msg construct_ipmi_message(uint8_t seq_source, uint8_t netFn, uint8_t command,
				uint8_t source_inft, uint8_t target_inft, uint16_t data_len,
				uint8_t *data);

I2C_MSG construct_i2c_message(uint8_t bus_id, uint8_t address, uint8_t tx_len, uint8_t *data,
			      uint8_t rx_len);

void reverse_array(uint8_t arr[], uint8_t size);

#endif
