#include "hal_gpio.h"
#include "plat_gpio.h"
#include "plat_class.h"
#include "plat_isr.h"
#include "plat_i2c_target.h"

#include "plat_power_seq.h"
#include "power_status.h"
#include "plat_mctp.h"

void pal_pre_init()
{
	init_platform_config();
	/* init i2c target */
	for (int index = 0; index < MAX_TARGET_NUM; index++) {
		if (I2C_TARGET_ENABLE_TABLE[index])
			i2c_target_control(
				index, (struct _i2c_target_config *)&I2C_TARGET_CONFIG_TABLE[index],
				1);
	}
}

void pal_set_sys_status()
{
	set_MB_DC_status(FM_POWER_EN);
	set_DC_status(PWRGD_CARD_PWROK);
	control_power_sequence();
}

void pal_post_init()
{
	gpio_set(SPI_RST_FLASH_N, GPIO_HIGH);

	k_usleep(100);

	gpio_set(ASIC_DEV_RST_N, GPIO_HIGH);

	plat_mctp_init();
}

#define DEF_PROJ_GPIO_PRIORITY 61

DEVICE_DEFINE(PRE_DEF_PROJ_GPIO, "PRE_DEF_PROJ_GPIO_NAME", &gpio_init, NULL, NULL, NULL,
	      POST_KERNEL, DEF_PROJ_GPIO_PRIORITY, NULL);
