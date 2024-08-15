/*
 * foo.c
 *
 *  Created on: Jul 5, 2024
 *      Author: DinhHuuNam <dinhnamuet@gmail.com>
 */
#include "foo.h"
#include <string.h>
#include <stdlib.h>

#define START_IDX 0
#define PRVER_IDX 1
#define ERRNO_IDX 2
#define MTYPE_IDX 3
#define START_DATA_IDX 12

static inline u32 get_page_base_address(u32 page) {
	return FLASH_BASE + (PAGESIZE * page);
}
static int erase_page(u32 base_page, u32 number) {
	FLASH_EraseInitTypeDef erase;
	u32 page_err;

	erase.Banks = FLASH_BANK_1;
	erase.Page = base_page;
	erase.NbPages = number;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;

	HAL_FLASH_Unlock();
	if (HAL_FLASHEx_Erase(&erase, &page_err) != HAL_OK) {
		HAL_FLASH_Lock();
		return -1;
	}
	HAL_FLASH_Lock();
	return page_err;
}
static inline u32 get_page_by_address(u32 address) {
	return (u32) (address - FLASH_BASE) / PAGESIZE;
}

static HAL_StatusTypeDef flash_write_data(u32 base_address, u32 offset,
		u8 *data, u8 length) {
	u32 i, page_idx;
	HAL_StatusTypeDef res;
	u8 page_data[PAGESIZE];
	u64 *ptr = (u64 *)&page_data[0];
	memset(page_data, 0, PAGESIZE);
	for (i = 0; i < PAGESIZE / 8; i++)
		ptr[i] = *(volatile u64*) (base_address + (8 * i));
	memcpy(&page_data[offset], data, length);
	page_idx = get_page_by_address(base_address);
	res = erase_page(page_idx, 1);
	if (res != HAL_OK)
		return res;
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

static HAL_StatusTypeDef update_firmware(u32 address, const u8 *data, u32 len) {
	u64 i, to_write;
	u64 *ptr;
	u8 program[2048];
	u32 page_idx;
	HAL_StatusTypeDef res;
	to_write = (u64) (len + 7) / 8;
	page_idx = get_page_by_address(address);
	erase_page(page_idx, 1);
	memset(program, 0, sizeof(program));
	memcpy(program, data, len);
	ptr = (u64*) &program[0];
	HAL_FLASH_Unlock();
	for (i = 0; i < to_write; i++) {
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + 8 * i,
				ptr[i]);
		if (res != HAL_OK) {
			HAL_FLASH_Lock();
			return res;
		}
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}
static void responses(struct foo_device *dev, u16 *start_frame, u8 *data,
		u32 len, u16 end) {
	u16 size = 4 * sizeof(u16) + sizeof(u32) + len + sizeof(u16);
	u8 buff[size];
	memset(buff, 0, size);
	memcpy(&buff[0], start_frame, 4 * sizeof(u16));
	memcpy(&buff[8], &len, sizeof(u32));
	memcpy(&buff[12], data, len);
	memcpy(&buff[size - 2], &end, sizeof(end));
	HAL_UART_Transmit(dev->bus, buff, size, 1000);
}
//FAFB 1000 0000 2100 08000000 2122232425262728 FCFD
error_t handle_request(struct foo_device *dev, const u8 *frame) {
	u16 start, pver, err, msgtype;
	u32 length;
	u8 res = 0;
	u8 *data_ptr = NULL;

	memcpy(&start, &frame[0], sizeof(u16));
	memcpy(&pver, &frame[2], sizeof(u16));
	memcpy(&err, &frame[4], sizeof(u16));
	memcpy(&msgtype, &frame[6], sizeof(u16));
	memcpy(&length, &frame[8], sizeof(32));

	u16 start_frame[4];
	u16 end = 0xFDFC;
	u32 len;

	start_frame[START_IDX] = 0xFBFA;
	start_frame[PRVER_IDX] = dev->protocol_version;

	if (pver != dev->protocol_version)
		start_frame[ERRNO_IDX] = INVALID;
	else
		start_frame[ERRNO_IDX] = OKAY;

	switch (msgtype) {
	case WRITE_FW_DATA:
		u64 flag;
		start_frame[MTYPE_IDX] = WRITE_FW_DATA;
		len = 1;
		if (!length) {
			flag = ON_BOOTING_APP;
			flash_write_data(VAR_BASE_ADDRESS, FLAG_OFFSET, (u8 *) &flag, sizeof(u64));
			res = 1;
			responses(dev, start_frame, &res, len, end);
			HAL_NVIC_SystemReset();
		} else if (update_firmware(dev->firmware_address, &frame[START_DATA_IDX],
				length) != HAL_OK) {
			res = 0;
		} else {
			dev->firmware_address += length;
			res = 1;
		}
		data_ptr = &res;
		break;
	default:
		start_frame[MTYPE_IDX] = msgtype;
		start_frame[ERRNO_IDX] = MSGTYPE_UKN;
		len = 0;
		data_ptr = NULL;
		break;
	}
	responses(dev, start_frame, data_ptr, len, end);
	return start_frame[ERRNO_IDX];
}
