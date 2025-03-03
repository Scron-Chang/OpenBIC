#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr.h>
#include "guid.h"

// size of 1 to satisfy compiler warning
__weak const EEPROM_CFG guid_config[1] = {};

uint8_t GUID_read(EEPROM_ENTRY *entry)
{
	if (entry == NULL) {
		return GUID_FAIL_TO_ACCESS;
	}

	if (entry->config.dev_id >= ARRAY_SIZE(guid_config)) {
		printf("GUID read device ID %x not exist\n", entry->config.dev_id);
		return GUID_INVALID_ID;
	}

	if ((entry->offset + entry->data_len) >=
	    (GUID_START + GUID_SIZE)) { // Check data write out of range
		printf("GUID read out of range, type: %x, ID: %x\n", entry->config.dev_type,
		       entry->config.dev_id);
		return GUID_OUT_OF_RANGE;
	}

	memcpy(&entry->config, &guid_config[entry->config.dev_id], sizeof(EEPROM_CFG));

	if (!eeprom_read(entry)) {
		return GUID_FAIL_TO_ACCESS;
	}

	return GUID_READ_SUCCESS;
}

uint8_t GUID_write(EEPROM_ENTRY *entry)
{
	if (entry == NULL) {
		return GUID_FAIL_TO_ACCESS;
	}

	if (entry->config.dev_id >= ARRAY_SIZE(guid_config)) {
		printf("GUID write device ID %x not exist\n", entry->config.dev_id);
		return GUID_INVALID_ID;
	}

	if ((entry->offset + entry->data_len) >=
	    (GUID_START + GUID_SIZE)) { // Check data write out of range
		printf("GUID write out of range, type: %x, ID: %x\n", entry->config.dev_type,
		       entry->config.dev_id);
		return GUID_OUT_OF_RANGE;
	}

	memcpy(&entry->config, &guid_config[entry->config.dev_id], sizeof(EEPROM_CFG));

	if (!eeprom_write(entry)) {
		return GUID_FAIL_TO_ACCESS;
	}

	return GUID_WRITE_SUCCESS;
}

__weak uint8_t get_system_guid(uint16_t *data_len, uint8_t *data)
{
	return GUID_FAIL_TO_ACCESS;
}

__weak uint8_t set_system_guid(uint16_t *data_len, uint8_t *data)
{
	return GUID_FAIL_TO_ACCESS;
}
