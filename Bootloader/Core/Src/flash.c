#include "flash.h"
#include <string.h>

u32 get_page_base_address(u32 page) {
	return FLASH_BASE + (PAGESIZE * page);
}
u32 get_page_by_address(u32 address) {
	return (u32) (address - FLASH_BASE) / PAGESIZE;
}

/*
 * @bief Erase Flash memory page
 * @param base_page: Start Page
 * @param number: Number of page to be erased
 * @retval Status
 */
HAL_StatusTypeDef erase_page(u32 base_page, u32 number) {
	HAL_StatusTypeDef res = HAL_ERROR;
	FLASH_EraseInitTypeDef erase;
	u32 page_err;

	erase.Banks = FLASH_BANK_1;
	erase.Page = base_page;
	erase.NbPages = number;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;

	HAL_FLASH_Unlock();
	res = HAL_FLASHEx_Erase(&erase, &page_err);
	HAL_FLASH_Lock();
	return res;
}
/* |           |            |          |         |...
 * ^base_page  ^base addr   ^data
 * @bief Write data to flash memory
 * @param base_address: address to write
 * @param data: pointer to data
 * @param length: data length
 * @retval Status
 */
HAL_StatusTypeDef flash_write_data(u32 base_address, u8 *data, u32 length) {
	u32 page_idx, n_pages, cur_addr;
	HAL_StatusTypeDef res = HAL_OK;
	u8 page_data[PAGESIZE];
	u64 *ptr = (u64 *)&page_data[0];
	page_idx = get_page_by_address(base_address); /* Get Page of address requested */
	cur_addr = get_page_base_address(page_idx);

	n_pages = ((length + (base_address - get_page_base_address(page_idx))) / PAGESIZE) + 1; /* Number of pages to write */

	for (int i = 0; i < n_pages; i++) {
		memset(page_data, 0, PAGESIZE);
		for (int j = 0; j < PAGESIZE / 8; j++) {
			ptr[j] = *(volatile u64*) (get_page_base_address(get_page_by_address(base_address)) + (PAGESIZE * i) + (8 * j)); /* read memory page */
		}

		for (int in = 0; in < PAGESIZE; in++) {
			if (cur_addr < base_address) {
				cur_addr += 1;
			} else {
				if (length) {
					page_data[in] = *data++;
					length --;
				} else {
					break;
				}
			}
		}

		res = erase_page(page_idx + i, 1); /* Erase Page before write */
		if (res != HAL_OK)
			return res;

		HAL_FLASH_Unlock();
		for (int k = 0; k < PAGESIZE / 8; k++) {
			res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, get_page_base_address(get_page_by_address(base_address)) + (PAGESIZE * i) + (8 * k), ptr[k]); /* Write back to Flash memory */
			if (res != HAL_OK) {
				HAL_FLASH_Lock();
				return res;
			}
		}
		HAL_FLASH_Lock();
	}

	return res;
}

void flash_read(u32 address, void *desc, u32 length) {
	u8 *ptr = (u8 *)desc;
	for (int i = 0; i < length; i++) {
		ptr[i] = *(volatile u8*) (address + i);
	}
}
