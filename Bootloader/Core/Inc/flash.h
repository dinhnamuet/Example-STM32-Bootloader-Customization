#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32l4xx_hal.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

HAL_StatusTypeDef erase_page(u32 base_page, u32 number);
HAL_StatusTypeDef flash_write_data(u32 base_address, u8 *data, u32 length);
u32 get_page_base_address(u32 page);
u32 get_page_by_address(u32 address);
void flash_read(u32 address, void *desc, u32 length);

#endif /* Flash */
