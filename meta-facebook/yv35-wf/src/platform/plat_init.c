#include "hal_gpio.h"
#include "plat_gpio.h"
#include "plat_isr.h"
#include "plat_class.h"
#include "power_status.h"

static void dev_power_init(void)
{
	gpio_set(FM_P12V_HSC_EN, gpio_get(FM_POWER_EN));
}

void pal_pre_init()
{
	init_platform_config();

	dev_power_init();
}

void pal_set_sys_status()
{

}

#define DEF_PROJ_GPIO_PRIORITY 61

DEVICE_DEFINE(PRE_DEF_PROJ_GPIO, "PRE_DEF_PROJ_GPIO_NAME", &gpio_init, NULL, NULL, NULL,
	      POST_KERNEL, DEF_PROJ_GPIO_PRIORITY, NULL);
