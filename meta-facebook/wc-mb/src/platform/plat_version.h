#ifndef PLAT_VERSION_H
#define PLAT_VERSION_H

#include "version.h"

#define PLATFORM_NAME "Waimea Canyon"
#define PROJECT_NAME "Mainboard"
#define PROJECT_STAGE POC

/*
 * 0x01 mainboard
 */
#define BOARD_ID 0x01
#define DEVICE_ID 0x00
#define DEVICE_REVISION 0x80

#define FIRMWARE_REVISION_1 GET_FW_VERSION1(BOARD_ID, PROJECT_STAGE)
#define FIRMWARE_REVISION_2 0x01 // Count of release firmware at each stage.

#define IPMI_VERSION 0x02
#define ADDITIONAL_DEVICE_SUPPORT 0xBF
#define PRODUCT_ID 0x0000
#define AUXILIARY_FW_REVISION 0x00000000

#define BIC_FW_YEAR_MSB 0x20
#define BIC_FW_YEAR_LSB 0x22
#define BIC_FW_WEEK 0x25
#define BIC_FW_VER 0x00
#define BIC_FW_platform_0 0x77 // char: w
#define BIC_FW_platform_1 0x63 // char: c
#define BIC_FW_platform_2 0x00 // char: '\0'

#endif
