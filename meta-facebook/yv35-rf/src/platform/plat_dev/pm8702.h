#ifndef _PLAT_MCTP_DEV_H
#define _PLAT_MCTP_DEV_H

#include "plat_i2c.h"

#define NAN (0.0f / 0.0f)

#define PM8702_DEF_EID 0x2E
#define PM8702_I2C_BUS I2C_BUS2
#define PM8702_I2C_ADDR 0b01001110
/*                            |||_ Strap pin: ASIC_M_SCAN_PCAP_SEL
 *                            ||__ Strap pin: GPIO[0]
 *                            |___ Strap pin: USOUT
 */

typedef struct {
	mctp *mctp_inst;
	bool wait_resp;
	bool available;
	bool ready;
	char revision[17]; // Add 1 byte for the string end charactor '\0'.
	uint8_t eid;
	float temperature;
} pm8702_cache_t;

extern pm8702_cache_t pm8702_cache;

void run_pm8702_tasks(void *arug0, void *arug1, void *arug2);

#endif /* _PLAT_MCTP_DEV_H */
