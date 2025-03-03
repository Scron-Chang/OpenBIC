#ifndef PLAT_PWRSEQ_H
#define PLAT_PWRSEQ_H

#include "hal_gpio.h"
#include "plat_gpio.h"

void pwr_related_pin_init(void);
uint8_t get_dev_pwrgd(uint8_t idx);
uint8_t fm_p3v3_sw_en(uint8_t idx, uint8_t val);
uint8_t fm_p12v_sw_en(uint8_t idx, uint8_t val);
uint8_t clkbuf_oe_en(uint8_t idx, uint8_t val);
uint8_t get_fm_pwrdis_status(uint8_t idx);
uint8_t fm_pwrdis_en(uint8_t idx, uint8_t val);
void check_dc_off_process(void);
uint8_t m2_dev_power_switch_with_pwrdis_chk(uint8_t idx, uint8_t enable, uint8_t chk_pwrdis,
					    uint8_t force_ctl_3v3);
uint8_t m2_dev_power_switch(uint8_t idx, uint8_t enable);
uint8_t device_all_power_set(uint8_t idx, uint8_t set_val);
void dev_pwrgd_handler(uint8_t idx);
uint8_t get_fm_p12v_sw_en(uint8_t idx);
uint8_t pwrgd_p12v_aux_100ms_get(void);
void pwrgd_p12v_aux_100ms_set(uint32_t val, uint32_t unused1);

void dev_pwrgd_handler_dev0(void);
void dev_pwrgd_handler_dev1(void);
void dev_pwrgd_handler_dev2(void);
void dev_pwrgd_handler_dev3(void);

#endif
