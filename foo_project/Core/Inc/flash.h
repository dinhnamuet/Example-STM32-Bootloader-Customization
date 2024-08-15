/*
 * flash.h
 *
 *  Created on: Jul 11, 2024
 *      Author: Admin
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_
#include "stm32l4xx_hal.h"

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
/*
 *  this function to erase a flash page
 */
HAL_StatusTypeDef flash_erase_page(u32 base_page, u32 number);
/*
 * write data to flash memory
 */
HAL_StatusTypeDef flash_write_data(u32 base_address, u32 offset, u8 *data, u16 length);

#endif /* INC_FLASH_H_ */
