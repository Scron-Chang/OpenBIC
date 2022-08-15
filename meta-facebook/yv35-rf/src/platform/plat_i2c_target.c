/* Declare all 16 buses of AST1030 here */

#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include "plat_i2c_target.h"

/* I2C target init-enable table */
const bool I2C_TARGET_ENABLE_TABLE[MAX_TARGET_NUM] = {
	TARGET_DISABLE, TARGET_ENABLE,	TARGET_DISABLE, TARGET_DISABLE,
	TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE,
	TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE,
	TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE, TARGET_DISABLE,
};

/* I2C target init-config table */
const struct _i2c_target_config I2C_TARGET_CONFIG_TABLE[MAX_TARGET_NUM] = {
	{ 0xFF, 0xA }, { 0x40, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA },
	{ 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0x4 }, { 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA },
	{ 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA }, { 0xFF, 0xA },
};
