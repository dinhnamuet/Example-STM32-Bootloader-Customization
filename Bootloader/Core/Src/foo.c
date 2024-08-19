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
#include "bootloader.h"
#include "CRC.h"

#define START_IDX 0
#define PRVER_IDX 1
#define ERRNO_IDX 2
#define MTYPE_IDX 3
#define START_DATA_IDX 12

void get_serial_number(struct foo_device *dev) {
	u8 foo[24] = {};
	memset(&dev->serial_number, 0, sizeof(dev->serial_number));
	flash_read(VAR_BASE_ADDRESS + SERIAL_NUMBER_OFFSET, foo, 24);
	memcpy(&dev->serial_number.len, foo, 4);
	if (dev->serial_number.len > 20) {
		memcpy(dev->serial_number.number, (u8*) "ABCDE", 5);
		dev->serial_number.len = 5;
		return;
	} else {
		memcpy(dev->serial_number.number, &foo[4], 20);
	}
}

/*
 * @bief Write Firmware data to Flash memory and verify
 * @param address: Flash memory address
 * @param data: data to write
 * @param len: data length
 * @retval Status
 */
static HAL_StatusTypeDef flash_write_firmware(u32 address, const u8 *data, u32 len) {
	u8 *verify = NULL;
	u32 crc_value = 0;

	memcpy(&crc_value, &data[len - 4], sizeof(u32)); /* Get CRC at the end of data */
	len -= 4;

	if (CRC_CalculateCRC32(data, len) != crc_value) {
		return HAL_ERROR; /* CRC is incorrect */
	}

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
	serial_send(&dev->serial, buff, size);
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
	case GET_HW_VERSION:
			start_frame[MTYPE_IDX] = GET_HW_VERSION;
			len = 2;
			data_ptr = (u8*) &dev->hardware_version;
			break;

	case GET_FW_VERSION:
			start_frame[MTYPE_IDX] = GET_FW_VERSION;
			len = 2;
			data_ptr = (u8*) &dev->firmware_version;
			break;

	case GET_SER_NO:
			start_frame[MTYPE_IDX] = GET_SER_NO;
			get_serial_number(dev);
			len = dev->serial_number.len;
			data_ptr = &dev->serial_number.number[0];
			break;

	case GET_TRIGGER_BE:
			start_frame[MTYPE_IDX] = GET_TRIGGER_BE;
			len = 1;
			data_ptr = &dev->button_trigger;
			break;

	case SET_TRIGGER_BE:
			start_frame[MTYPE_IDX] = SET_TRIGGER_BE;
			len = 0;
			data_ptr = NULL;
			break;

	case GET_WATTING_TIME_OFF:
			start_frame[MTYPE_IDX] = GET_WATTING_TIME_OFF;
			len = 4;
			data_ptr = (u8*) &dev->watting_time_off;
			break;

	case SET_PREP_MODE:
			u8 buf[2];
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &frame[12], 2);
			start_frame[MTYPE_IDX] = SET_PREP_MODE;
			len = 0;
			if (buf[0] == 1 && buf[1] == 5) {
				responses(dev, start_frame, NULL, len, end);
				HAL_NVIC_SystemReset();
			}
			break;

	case WRITE_FW_DATA:
		boot_options_t flag;
		start_frame[MTYPE_IDX] = WRITE_FW_DATA;
		static u32 fw_len;
		len = 1;
#ifdef FLASH_LARGE
		flash_read(VAR_BASE_ADDRESS + FLAG_OFFSET, &flag, sizeof(boot_options_t));
		if (length && flag != BOOTING_MODE) {
			flag = BOOTING_MODE;
			flash_write_data(VAR_BASE_ADDRESS + FLAG_OFFSET, &flag, sizeof(boot_options_t)); /* If update firmware process fault, Bootloader boot old firmware */
		}
#endif
		if (!length) {
#ifdef FLASH_LARGE
			flag = UPDATE_MODE;
			flash_write_data((VAR_BASE_ADDRESS + FW_LENGTH_OFFSET), (u8 *)&fw_len, sizeof(u32));
			flash_write_data((VAR_BASE_ADDRESS + FLAG_OFFSET), (u8 *)&flag, sizeof(boot_options_t));
			res = 1;
			responses(dev, start_frame, &res, len, end);
			HAL_NVIC_SystemReset();
#else
			flag = BOOTING_MODE;
			flash_write_data(VAR_BASE_ADDRESS + FLAG_OFFSET, &flag, sizeof(boot_options_t));
			res = 1;
			responses(dev, start_frame, &res, len, end);
			HAL_NVIC_SystemReset();
#endif
		} else if (flash_write_firmware(dev->firmware_address, &frame[START_DATA_IDX], length) != HAL_OK) {
			res = 0;
		} else {
			dev->firmware_address += length;
			fw_len += length;
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
