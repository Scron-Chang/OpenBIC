#ifndef PLAT_FUNC_H
#define PLAT_FUNC_H

void ISR_CL_DC_ON();
void ISR_DC_ON();
void ISR_MB_RST();
void control_power_sequence();
void init_power_on_thread();
void init_power_off_thread();
void abort_power_thread();
void e1s_power_control(void *control_type, void *dummy0, void *dummy1);
void control_e1s_power_sequence(void);

#endif
