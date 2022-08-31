#include <stdio.h>
#include <string.h>
#include "ast_adc.h"
#include "sensor.h"
#include "plat_hook.h"
#include "plat_i2c.h"
#include "plat_sensor_table.h"

sensor_cfg plat_sensor_config[] = {
};

const int SENSOR_CONFIG_SIZE = ARRAY_SIZE(plat_sensor_config);
