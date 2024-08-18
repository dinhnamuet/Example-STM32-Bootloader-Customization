/*
 * foo.c
 *
 *  Created on: Jul 2, 2024
 *      Author: DinhHuuNam <dinhnamuet@gmail.com>
 */
#include "foo.h"
#include <string.h>
#include <stdlib.h>
#include "flash.h"
#include "pwm.h"

#define START_IDX 0
#define PRVER_IDX 1
#define ERRNO_IDX 2
#define MTYPE_IDX 3

#define FLAG_ADDR 		(FLASH_BASE + (PAGESIZE*64))
#define FLAG_PAGE		(FLAG_ADDR - FLASH_BASE)/PAGESIZE

static void set_pwm_led(struct foo_device *dev) {
	float dutycycle = 100 * (float)dev->led_current / (float)dev->max_led_current;
	pwm_set_dutycycle(&dev->led, dutycycle);
}

int set_serial_number(struct foo_device *dev, const u8 *ser_num, u8 len) {
	u8 to_write;
	u8 buf[24];
	to_write = (len > 20) ? 20 : len;

	memset(dev->serial_number.number, 0, sizeof(dev->serial_number));
	memcpy(dev->serial_number.number, ser_num, to_write);
	dev->serial_number.len = to_write;
	memcpy(buf, &dev->serial_number.len, 4);
	memcpy(&buf[4], dev->serial_number.number, 20);

	if (flash_write_data(VAR_BASE_ADDRESS, buf, SERIAL_NUMBER_MAX_LENGTH)!= HAL_OK)
		return -1;
	return 0;
}

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

static void responses(struct foo_device *dev, u16 *start_frame, u8 *data,
		u32 len, u16 end) {
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
	else if (msgtype == SET_SER_NO && length > 20)
		start_frame[ERRNO_IDX] = MSG_LARGE;
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
	case SET_SER_NO:
		start_frame[MTYPE_IDX] = SET_SER_NO;
		set_serial_number(dev, &frame[12], length);
		len = 0;
		data_ptr = NULL;
		break;
	case GET_WATTING_TIME_OFF:
		start_frame[MTYPE_IDX] = GET_WATTING_TIME_OFF;
		len = 4;
		data_ptr = (u8*) &dev->watting_time_off;
		break;
	case SET_PREP_MODE:
		boot_options_t flag = BOOTLOADER_MODE;
		u8 buf[2];
		memset(buf, 0, sizeof(buf));
		memcpy(buf, &frame[12], 2);
		start_frame[MTYPE_IDX] = SET_PREP_MODE;
		len = 0;
		if (buf[0] == 1 && buf[1] == 5) {
			flash_write_data(VAR_BASE_ADDRESS + FLAG_OFFSET, (u8*) &flag, sizeof(boot_options_t));
			responses(dev, start_frame, NULL, len, end);
			HAL_NVIC_SystemReset();
		}
		break;
	case SET_LED_ELECTRIC_CURRENT:
		len = 0;
		start_frame[MTYPE_IDX] = SET_LED_ELECTRIC_CURRENT;
		memcpy(&dev->led_current, &frame[12], 2);
		set_pwm_led(dev);
		data_ptr = NULL;
		break;
	case GET_LED_ELECTRIC_CURRENT:
		start_frame[MTYPE_IDX] = GET_LED_ELECTRIC_CURRENT;
		len = 2;
		data_ptr = (u8*) &dev->led_current;
		break;
	case SET_LED_PR_MAX_ELECTRICT_CURRENT:
		len = 0;
		start_frame[MTYPE_IDX] = SET_LED_PR_MAX_ELECTRICT_CURRENT;
		memcpy(&dev->max_led_current, &frame[12], 2);
		set_pwm_led(dev);
		data_ptr = NULL;
		break;
	case GET_LED_PR_MAX_ELECTRICT_CURRENT:
		start_frame[MTYPE_IDX] = GET_LED_PR_MAX_ELECTRICT_CURRENT;
		len = 2;
		data_ptr = (u8 *) &dev->max_led_current;
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
