#ifndef PLAT_VERSION_H
#define PLAT_VERSION_H

#include "version.h"

#define PLATFORM_NAME "Yosemite 3"
#define PROJECT_NAME "Delta Lake"
#define PROJECT_STAGE MP

/*
 * 0x01 motherboard
 */
#define BOARD_ID 0x01
#define DEVICE_ID 0x00
#define DEVICE_REVISION 0x80

// Firmware revision manually set
#define FIRMWARE_REVISION_1 0x51
#define FIRMWARE_REVISION_2 0x02

#define IPMI_VERSION 0x02
#define ADDITIONAL_DEVICE_SUPPORT 0xBF
#define PRODUCT_ID 0x0000
#define AUXILIARY_FW_REVISION 0x00000000
#define IANA_ID 0x009C9C // same as TI BIC

#define BIC_FW_YEAR_MSB 0x20
#define BIC_FW_YEAR_LSB 0x22
#define BIC_FW_WEEK 0x34
#define BIC_FW_VER 0x01
#define BIC_FW_platform_0 0x64 // char: d
#define BIC_FW_platform_1 0x6c // char: l
#define BIC_FW_platform_2 0x00 // char: '\0'

#endif
