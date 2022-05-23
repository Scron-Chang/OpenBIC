#ifndef PLAT_HOOK_H
#define PLAT_HOOK_H

#define INA233_INTERNAL_VALUE 0.00512

typedef struct _isl69254iraz_t_pre_arg_ {
	uint8_t vr_page;
} isl69254iraz_t_pre_arg;

/**************************************************************************************************
 * INIT ARGS
**************************************************************************************************/
extern adc_asd_init_arg adc_asd_init_args[];
extern ina233_init_arg ina230_init_args[];

/**************************************************************************************************
 *  PRE-HOOK/POST-HOOK ARGS
 **************************************************************************************************/
extern isl69254iraz_t_pre_arg isl69254iraz_t_pre_read_args[];

/**************************************************************************************************
 *  PRE-HOOK/POST-HOOK FUNC
 **************************************************************************************************/
bool pre_ina230_read(uint8_t sensor_num, void *args);
bool pre_isl69254iraz_t_read(uint8_t sensor_num, void *args);

#endif
