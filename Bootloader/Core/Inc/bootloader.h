#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__
#include "stm32l4xx_hal.h"

#define FIRMWARE_ADDRESS 			0x0800A000U
#define PROT_VER					0x1000

#ifdef FLASH_LARGE
#define FIRMWARE_TEM				get_page_base_address(90) /* temporary firmware binary code stored at page 90 */
#endif

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef enum {
    BOOTING_MODE,
    UPDATE_MODE = 0x02,
    BOOTLOADER_MODE = 0x08
} boot_options_t;

void goto_application(void);
int update_firmware(u32 len);

#endif
