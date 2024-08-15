/*
 * flash.c
 *
 *  Created on: Jul 11, 2024
 *      Author: Admin
 */

#include "flash.h"
#include <string.h>

static inline u32 get_page_base_address(u32 page) {
	return FLASH_BASE + (PAGESIZE * page);
}
static inline u32 get_page_by_address(u32 address) {
	return (address - FLASH_BASE) / PAGESIZE;
}

HAL_StatusTypeDef flash_erase_page(u32 base_page, u32 number) {
	FLASH_EraseInitTypeDef erase;
	u32 page_err;
	HAL_StatusTypeDef res;

	erase.Banks = FLASH_BANK_1;
	erase.NbPages = number;
	erase.Page = base_page;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;

	HAL_FLASH_Unlock();
	res = HAL_FLASHEx_Erase(&erase, &page_err);
	if (res != HAL_OK) {
		HAL_FLASH_Lock();
		return res;
	}
	HAL_FLASH_Lock();
	return res;
}

HAL_StatusTypeDef flash_write_data(u32 base_address, u32 offset, u8 *data, u16 length) {
	u32 i, page_idx;
	HAL_StatusTypeDef res;
	u8 page_data[PAGESIZE];
	u64 *ptr = (u64 *)&page_data[0];
	memset(page_data, 0, PAGESIZE);
	for (i = 0; i < PAGESIZE / 8; i++)
		ptr[i] = *(volatile u64*) (base_address + (8 * i));
	memcpy(&page_data[offset], data, length);
	page_idx = get_page_by_address(base_address);
	while (flash_erase_page(page_idx, 1) != HAL_OK);
	HAL_FLASH_Unlock();
	for (i = 0; i < PAGESIZE / 8; i++) {
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, base_address + (8*i), ptr[i]);
		if (res != HAL_OK) {
			HAL_FLASH_Lock();
			return res;
		}
	}
	HAL_FLASH_Lock();
	return res;
}
