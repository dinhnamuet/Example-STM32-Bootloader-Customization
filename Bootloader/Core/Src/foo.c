/*
 * foo.c
 *
 *  Created on: Jul 5, 2024
 *      Author: DinhHuuNam <dinhnamuet@gmail.com>
 */
#include "foo.h"
#include <string.h>
#include <stdlib.h>
#include "flash.h"

#define START_IDX 0
#define PRVER_IDX 1
#define ERRNO_IDX 2
#define MTYPE_IDX 3
#define START_DATA_IDX 12

/*
 * @bief Write Firmware data to Flash memory and verify
 * @param address: Flash memory address
 * @param data: data to write
 * @param len: data length
 * @retval Status
 */
static HAL_StatusTypeDef flash_write_firmware(u32 address, const u8 *data, u32 len) {
	u8 *verify = NULL;

	HAL_StatusTypeDef  res = flash_write_data(address, (u8 *)data, len); /* Write Firmware data */
	if (res != HAL_OK) {
		return res;
	}

	verify = (u8 *)calloc(len, sizeof(u8));
	if (!verify) {
		return HAL_ERROR;
	}

	flash_read(address, verify, len);
	if (memcmp(data, verify, len)) { /* Verify data in Flash memory */
		free(verify);
		return HAL_ERROR; /* Data miss match */
	} else {
		free(verify);
		return HAL_OK; /* Data ok */
	}
}
static void responses(struct foo_device *dev, u16 *start_frame, u8 *data, u32 len, u16 end) {
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
			flash_write_data(VAR_BASE_ADDRESS + FLAG_OFFSET, (u8 *) &flag, sizeof(u64));
			res = 1;
			responses(dev, start_frame, &res, len, end);
			HAL_NVIC_SystemReset();
		} else if (flash_write_firmware(dev->firmware_address, &frame[START_DATA_IDX],
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
