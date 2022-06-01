#ifndef PLAT_PWRSEQ_H
#define PLAT_PWRSEQ_H

#include "hal_gpio.h"

#define DEFAULT_POWER_ON_SEQ 0x0
#define DEFAULT_POWER_OFF_SEQ 0xB
#define POWER_STAGE_NUM 0xA
#define ASIC_PWRSEQ_NUM 4
#define DIMM_PWRSEQ_NUM 3
#define CHKPWR_DELAY_MSEC 100
#define DEV_RESET_DELAY_USEC 100

enum CONTROL_POWER_MODE {
	ENABLE_POWER_MODE = 0x00,
	DISABLE_POWER_MODE,
};

enum POWER_ON_STAGE {
	ASIC_POWER_ON_STAGE = 0x00,
	DIMM_POWER_ON_STAGE1,
	DIMM_POWER_ON_STAGE2,
	DIMM_POWER_ON_STAGE3,
	BOARD_POWER_ON_STAGE,
};

enum POWER_OFF_STAGE {
	DIMM_POWER_OFF_STAGE1 = 0x00,
	DIMM_POWER_OFF_STAGE2,
	DIMM_POWER_OFF_STAGE3,
	ASIC_POWER_OFF_STAGE1,
	ASIC_POWER_OFF_STAGE2,
	BOARD_POWER_OFF_STAGE,
};

enum CONTROL_POWER_SEQ_NUM_MAPPING {
	CONTROL_POWER_SEQ_01 = FM_P0V9_ASICD_EN, //16 X
	CONTROL_POWER_SEQ_02 = FM_P0V9_ASICA_1_EN, //110 X
	CONTROL_POWER_SEQ_03 = FM_P0V9_ASICA_2_EN, //42
	CONTROL_POWER_SEQ_04 = P1V8_VDD_EN,//74

	CONTROL_POWER_SEQ_05 = PVPP_AB_EN_R,

	CONTROL_POWER_SEQ_06 = FM_PVDDQ_AB_EN,//55 X

	CONTROL_POWER_SEQ_07 = PVTT_AB_EN_R, //24 X

	//CONTROL_POWER_SEQ_08 = CXL_CLOCK_R_OE,
};

enum CHECK_POWER_SEQ_NUM_MAPPING {
	CHECK_POWER_SEQ_01 = P0V9_ASICD_PWRGD,
	CHECK_POWER_SEQ_02 = P0V9_ASICA_1_PWRGD,
	CHECK_POWER_SEQ_03 = P0V9_ASICA_2_PWRGD,
	CHECK_POWER_SEQ_04 = P1V8_VDD_PG,
	CHECK_POWER_SEQ_05 = PVPP_AB_PG_R,
	CHECK_POWER_SEQ_06 = PWRGD_PVDDQ_AB,
	CHECK_POWER_SEQ_07 = PVTT_AB_PG_R,
	//CHECK_POWER_SEQ_08 = CXL_CLOCK_R_OE,
};

void set_CL_DC_status(uint8_t gpio_num);
void set_power_on_seq(uint8_t seq_num);
void set_power_off_seq(uint8_t seq_num);
void control_power_on_sequence();
void control_power_off_sequence();
void control_power_stage(uint8_t control_mode, uint8_t control_seq);
int check_power_stage(uint8_t check_mode, uint8_t check_seq);
bool power_on_handler(uint8_t initial_stage);
bool power_off_handler(uint8_t initial_stage);

#endif
