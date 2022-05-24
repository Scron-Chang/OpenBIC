#include "plat_sensor_table.h"

#include <stdio.h>
#include <string.h>

#include "ast_adc.h"
#include "sensor.h"
#include "plat_hook.h"
#include "plat_i2c.h"
#include "plat_def.h"

sensor_cfg plat_sensor_config[] = {
	/* number,                  type,       port,      address,      offset,
     access check arg0, arg1, cache, cache_status, mux_ADDRess, mux_offset,
     pre_sensor_read_fn, pre_sensor_read_args, post_sensor_read_fn, post_sensor_read_fn  */

	// temperature *	  	
	{ SENSOR_NUM_TEMP_CXL_CNTR, sensor_dev_tmp75, I2C_BUS2, TMP75_ASIC_ADDR, TMP75_TEMP_OFFSET,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  NULL },
	{ SENSOR_NUM_TEMP_TMP75, sensor_dev_tmp75, I2C_BUS3, TMP75_MB_ADDR, TMP75_TEMP_OFFSET,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  NULL },
	{ SENSOR_NUM_TEMP_SSD0, sensor_dev_nvme, I2C_BUS5, SSD0_ADDR, SSD0_OFFSET, post_access, 0,
	  0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_nvme_read, &mux_conf_addr_0xe2[1],
	  NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_SSD1, sensor_dev_nvme, I2C_BUS6, SSD0_ADDR, SSD0_OFFSET, post_access, 0,
	  0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_nvme_read, &mux_conf_addr_0xe2[1],
	  NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_SSD2, sensor_dev_nvme, I2C_BUS7, SSD0_ADDR, SSD0_OFFSET, post_access, 0,
	  0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_nvme_read, &mux_conf_addr_0xe2[1],
	  NULL, NULL, NULL },

	// ADC *
	{ SENSOR_NUM_VOL_P12V_STBY, sensor_dev_ast_adc, ADC_PORT1, NONE, NONE, stby_access, 66, 10,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P3V3_STBY, sensor_dev_ast_adc, ADC_PORT2, NONE, NONE, stby_access, 2, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P12V_E1S, sensor_dev_ast_adc, ADC_PORT3, NONE, NONE, stby_access, 66, 10,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_VDD_P1V2, sensor_dev_ast_adc, ADC_PORT6, NONE, NONE, stby_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_VDD_P1V8, sensor_dev_ast_adc, ADC_PORT7, NONE, NONE, stby_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P3V3_E1S, sensor_dev_ast_adc, ADC_PORT13, NONE, NONE, stby_access, 2, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_PVPP_AB, sensor_dev_ast_adc, ADC_PORT11, NONE, NONE, stby_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_PVTT_AB, sensor_dev_ast_adc, ADC_PORT9, NONE, NONE, stby_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P5V_STBY, sensor_dev_ast_adc, ADC_PORT8, NONE, NONE, stby_access, 711, 200,
	  SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adc_asd_init_args[0] },	  

	// INA230 *
	{ SENSOR_NUM_VOL_P1V2_E1S, sensor_dev_ina233, I2C_BUS3, INA230_12V_ADDR, SMBUS_VOL_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[0] },
	{ SENSOR_NUM_VOL_PVPP_AB_R, sensor_dev_ina233, I2C_BUS3, INA230_PVPP_ADDR, SMBUS_VOL_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[1] },
	{ SENSOR_NUM_CUR_P12V_E1S, sensor_dev_ina233, I2C_BUS3, INA230_12V_ADDR, SMBUS_CUR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[0] },
	{ SENSOR_NUM_CUR_PVPP_AB_R, sensor_dev_ina233, I2C_BUS3, INA230_PVPP_ADDR, SMBUS_CUR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[1] },	  	  
	{ SENSOR_NUM_PWR_P12V_E1S, sensor_dev_ina233, I2C_BUS3, INA230_12V_ADDR, SMBUS_PWR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[0] },
	{ SENSOR_NUM_PWR_PVPP_AB_R, sensor_dev_ina233, I2C_BUS3, INA230_PVPP_ADDR, SMBUS_PWR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_ina233_read, NULL,
	  NULL, NULL, &ina233_init_args[1] },


	// VR temperature
	{ SENSOR_NUM_TEMP_P0V9A_ASICD, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_TEMP_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read, 
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_P0V9A_ASICA1, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_TEMP_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read, 
	  &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_P0V9A_ASICA2, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR, SMBUS_TEMP_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read, 
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_PVDDQ_AB, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR, SMBUS_TEMP_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read, 
	  &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },	  

	// VR Voltage
    { SENSOR_NUM_VOL_P0V9A_ASICD, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_VOL_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P0V9A_ASICA1, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_VOL_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P0V9A_ASICA2, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR, SMBUS_VOL_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_PVDDQ_AB, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR,
	  SMBUS_VOL_CMD, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS,
	  pre_isl69254iraz_t_read, &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },


	// VR Current
    { SENSOR_NUM_CUR_P0V9A_ASICD, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_CUR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P0V9A_ASICA1, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_CUR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P0V9A_ASICA2, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR, SMBUS_CUR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_PVDDQ_AB, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR,
	  SMBUS_CUR_CMD, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS,
	  pre_isl69254iraz_t_read, &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },


	// VR Power
	{ SENSOR_NUM_PWR_P0V9A_ASICD, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_PWR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P0V9A_ASICA1, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA1_ADDR, SMBUS_PWR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P0V9A_ASICA2, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR, SMBUS_PWR_CMD,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS, pre_isl69254iraz_t_read,
	  &isl69254iraz_t_pre_read_args[0], NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_PVDDQ_AB, sensor_dev_isl69254iraz_t, I2C_BUS10, VR_P0V9A_ASICA2_ADDR,
	  SMBUS_PWR_CMD, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, 0, SENSOR_INIT_STATUS,
	  pre_isl69254iraz_t_read, &isl69254iraz_t_pre_read_args[1], NULL, NULL, NULL },

};

uint8_t load_sensor_config(void)
{
	memcpy(sensor_config, plat_sensor_config, sizeof(plat_sensor_config));
	return ARRAY_SIZE(plat_sensor_config);
}
