#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "plat_isr.h"
#include "power_status.h"
#include "plat_gpio.h"
#include "plat_power_seq.h"

#define POWER_SEQ_CTRL_STACK_SIZE 1000
#define E1S_DEV_PWR_OFF 0
#define E1S_DEV_PWR_ON  1

K_THREAD_STACK_DEFINE(power_thread, POWER_SEQ_CTRL_STACK_SIZE);
struct k_thread power_thread_handler;
k_tid_t power_tid;

K_THREAD_STACK_DEFINE(e1s_power_thread, POWER_SEQ_CTRL_STACK_SIZE);
struct k_thread e1s_power_thread_handler;
k_tid_t e1s_power_tid;

void control_e1s_power_sequence(void);

void ISR_CL_DC_ON()
{
	set_CL_DC_status(FM_POWER_EN);
	//control_power_sequence();
    control_e1s_power_sequence();
}

void ISR_DC_ON()
{
	set_DC_status(PWRGD_EXP_PWROK_R);
}

void ISR_MB_RST()
{
	if (gpio_get(RST_MB_N) == POSITIVE_ACTIVATE) {
		// Enable ASIC reset pin
		gpio_set(ASIC_PERST0_N, POSITIVE_ACTIVATE);
	} else {
		// Disable ASIC reset pin
		// ASIC_PERST0_N is OD output
		gpio_set(ASIC_PERST0_N, POSITIVE_DEACTIVATE);
	}
}

void control_power_sequence()
{
	if (gpio_get(FM_POWER_EN) == POWER_ON) { // CraterLake DC on
		if (gpio_get(PWRGD_EXP_PWROK_R) == POWER_ON) {
			// If the last stage of power on sequence already power on , no need to recheck power on sequence
			// Update the flag of power on sequence number
			set_power_on_seq(POWER_STAGE_NUM);
		} else {
			// If power on sequence not finished or not started , abort power off thread before creating power on thread
			abort_power_thread();
			init_power_on_thread();
		}
    
	 } else { // CraterLake DC off
	 	if (gpio_get(CXL_CLOCK_R_OE) == POSITIVE_DEACTIVATE) {
	 		// If the last stage of power off sequence already power off , no need to recheck power off sequence
	 		// Update the flag of power off sequence number
	 		set_power_off_seq(0);
	 	} else {
	 		// If power off sequence not finished or not started , abort power on thread before creating power off thread
	 		abort_power_thread();
	 		init_power_off_thread();
	 	}
	 }
}

void init_power_on_thread()//O
{
	// Avoid re-create thread by checking thread status and thread id
	if (power_tid != NULL && strcmp(k_thread_state_str(power_tid), "dead") != 0) {
		return;
	}
	power_tid = k_thread_create(&power_thread_handler, power_thread,
				    K_THREAD_STACK_SIZEOF(power_thread), control_power_on_sequence,
				    NULL, NULL, NULL, CONFIG_MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&power_thread_handler, "power_on_sequence_thread");
}

void init_power_off_thread()
{
	// Avoid re-create thread by checking thread status and thread id
	if (power_tid != NULL && strcmp(k_thread_state_str(power_tid), "dead") != 0) {
		return;
	}
	power_tid = k_thread_create(&power_thread_handler, power_thread,
				    K_THREAD_STACK_SIZEOF(power_thread), control_power_off_sequence,
				    NULL, NULL, NULL, CONFIG_MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&power_thread_handler, "power_off_sequence_thread");
}

void abort_power_thread()
{
	if (power_tid != NULL && strcmp(k_thread_state_str(power_tid), "dead") != 0) {
		k_thread_abort(power_tid);
	}
}


void e1s_power_control(void *control_type, void *dummy0, void *dummy1)
{
	ARG_UNUSED(dummy0);
	ARG_UNUSED(dummy1);

	int type = (intptr_t)control_type;
	printf("type = %d\n", type);

	if (type == E1S_DEV_PWR_ON) {
		gpio_set(FM_P12V_HSC_EN, 1);
		gpio_set(FM_P3V3_HSC_SW_EN, 1);

		k_msleep(40);

		gpio_set(FM_CLKBUF_R_EN, 1);

		k_msleep(10);

		gpio_set(CLKBUF_E1S_0_OE_R_N, 0);
		gpio_set(CLKBUF_E1S_1_OE_R_N, 0);
		gpio_set(CLKBUF_E1S_2_OE_R_N, 0);

		k_msleep(10);

		gpio_set(RST_BIC_E1S_0_R_N, 1);
		gpio_set(RST_BIC_E1S_0_R_N, 1);
		gpio_set(RST_BIC_E1S_0_R_N, 1);
	} else {
		gpio_set(RST_BIC_E1S_0_R_N, 0);
		gpio_set(RST_BIC_E1S_0_R_N, 0);
		gpio_set(RST_BIC_E1S_0_R_N, 0);

		k_msleep(10);

		gpio_set(CLKBUF_E1S_0_OE_R_N, 1);
		gpio_set(CLKBUF_E1S_1_OE_R_N, 1);
		gpio_set(CLKBUF_E1S_2_OE_R_N, 1);

		k_msleep(10);

		gpio_set(FM_CLKBUF_R_EN, 0);

		k_msleep(10);

		gpio_set(FM_P12V_HSC_EN, 0);
		gpio_set(FM_P3V3_HSC_SW_EN, 0);
	}
}

void control_e1s_power_sequence(void)
{
	uint8_t ctrl_type = E1S_DEV_PWR_OFF;

	if (gpio_get(FM_POWER_EN) == POWER_ON)
		ctrl_type = E1S_DEV_PWR_ON;

	// Avoid re-create thread by checking thread status and thread id
	if (e1s_power_tid != NULL && strcmp(k_thread_state_str(e1s_power_tid), "dead") != 0)
		k_thread_abort(e1s_power_tid);

	e1s_power_tid = k_thread_create(&e1s_power_thread_handler, e1s_power_thread,
					K_THREAD_STACK_SIZEOF(e1s_power_thread), e1s_power_control,
					(void *)(intptr_t)ctrl_type, NULL, NULL,
					CONFIG_MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
}
