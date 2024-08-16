/*
 * foo.h
 *
 *  Created on: Jul 5, 2024
 *      Author: Dinh Huu Nam <dinhnamuet@gmail.com>
 */

#ifndef INC_FOO_H_
#define INC_FOO_H_
#include "stm32l4xx_hal.h"

#define FIRMWARE_ADDRESS 			0x0800A000U
#define FIRMWARE_PAGE				(FIRMWARE_ADDRESS - FLASH_BASE)/PAGESIZE
#define PROT_VER					0x1000

#define VAR_BASE_ADDRESS			(FLASH_BASE + 127*PAGESIZE)

#define SERIAL_NUMBER_MAX_LENGTH	24
#define SERIAL_NUMBER_OFFSET 		0
#define FLAG_MAX_LENGTH				8
#define FLAG_OFFSET					(SERIAL_NUMBER_OFFSET + SERIAL_NUMBER_MAX_LENGTH)

#define ON_UPDATE_FIRMWARE			0x12UL
#define ON_BOOTING_APP				0x00UL

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

#pragma pack(1)
struct foo_device {
	UART_HandleTypeDef *bus;
	u16 protocol_version;
	u32 firmware_address;
};
#pragma pack()

typedef enum {
	OKAY, //Success (no detectable errors)
	INVALID, //Invalid/unsupported protocol
	MSGTYPE_UKN, //Unknown message type
	MSG_LARGE, //Message too large
	DLEN_FAULT, //Data length does not match message type
	DATA_INVAL, //Data invalid
	EFAULT, //Command is valid, but desired information does not esit
	DEV_BUSY, //Device is busy
} error_t;

typedef enum {
	/* Genearal device */
	RST=0x0000, //Reset
	RST_DEF=0x0001, //Reset Default
	GET_HW_VERSION=0x0010, //Get hardware version
	GET_FW_VERSION=0x0011, //Get firmware version
	GET_SER_NO=0x0020, //Get serial number
	SET_SER_NO=0x0021, //Set serial number
	GET_USR_STR_CNT=0x0030, //Get usr string count
	GET_MAX_USR_STR=0x0031, //Get max length of user string
	GET_USR_STR=0x0032, //Get user string
	SET_USR_STR=0x0033, //Set user string
	SET_PREP_MODE=0x0002, //Set preprogramming mode
	GET_DELAY_LEDS=0x0040, //Get delay between LED and measurement
	SET_DELAY_LEDS=0x0041, //Set delay between LED and measurement
	GET_TRIGGER_BE=0x0042, //Get trigger button enable
	SET_TRIGGER_BE=0x0043, //Set trigger button enable
	WRITE_FW_DATA=0x0003, //Write firmware data
	GET_BLE_FW_VER=0x0012, //Get BLE firmware version
	READ_MCU_BLD_VER=0x0013, //Read MCU bootloader version

	/* Connection Interface */
	GET_NUM_CI=0x0100, //Get number of connection interface
	GET_IN_CI=0x0101, //Get interface connection information
	GET_IN_EN=0x0102, //Get interface enable
	SET_IN_EN=0x0103, //Set interface enable
	GET_BLE_AD_NAME=0x0140, //Get Bluetooth advertisement name
	SET_BLE_AD_NAME=0x0141, //Set Bluetooth advertisement name
	GET_BLE_MTU_SIZE=0x0142, //Get Bluetooth MTU size
	SET_CURRENT_BLE_MTU_SIZE=0x0143, //Set current bluetooth MTU size

	/* Calibration and corrections */
	GET_WAVELENGTH_COEFF=0x0210, //Get wavelength coefficients
	SET_WAVELENGTH_COEFF=0x0211, //Set wavelength coefficients
	GET_NONLINEAR_COEFF=0x0230, //Get nonlinearity coefficients
	SET_NONLINEAR_COEFF=0x0231, //Set nonlinearity coefficients
	GET_CALIB_OFFSET_VAL=0x02A0, //Get calibration offset value
	SET_CALIB_OFFSET_VAL=0x02A1, //Set calibration offset value
	GET_CALIB_GAIN_VAL=0x02A2, //Get calibration gain value
	SET_CALIB_GAIN_VAL=0x02A3, //Set calibration gain value

	/* Spectrometer feature */
	GET_INTERGRATION_TIME=0x0300, //Get integration time (us)
	GET_MIN_INTERGRATION_TIME=0x0301, //Get min integration time (us)
	GET_MAX_INTERGRATION_TIME=0x0302, //Get max integration time (us)
	SET_INTERGRATION_TIME=0x0303, //Set integration time (us)
	GET_SCANS_TO_AVERAGE=0x0330, //Get scans to average
	SET_SCANS_TO_AVERAGE=0x0331, //Set scans to average
	GET_NUM_PIXEL=0x0340, //Get number of pixel
	GET_SPECTRUM=0x0341, //Get spectrum
	GET_BOXCAR_WIDTH=0x0350, //Get boxcar width
	SET_BOXCAR_WIDTH=0x0351, //Set boxcar width
	GET_ELECTRIC_DARK_CORRECTION_EN=0x0360, //Get electric dark correction enable
	SET_ELECTRIC_DARK_CORRECTION_EN=0x0361, //Set electric dark correction enable
	GET_NONLINEAR_CORRECTION=0x0370, //Get nonlinearity correction
	SET_NONLINEAR_CORRECTION=0x0371, //Set nonlinearity correction

	/* GPIO Feature */
	GET_NUM_GPIO_PIN=0x0500, //Get number of GPIO pins
	GET_OUTPUT_EN_VECT=0x0510, //Get output enable vector
	SET_OUTPUT_EN_VECT=0x0511, //Set output enable vector
	GET_VAL_VECTOR=0x0520, //Get value vector
	SET_VAL_VECTOR=0x0521, //Set value vector

	/* LED lightsource feature */
	SET_LED_ELECTRIC_CURRENT=0x0804, //Set electric current of LED
	GET_LED_ELECTRIC_CURRENT=0x0805, //Get electric current of LED
	SET_LED_PR_MAX_ELECTRICT_CURRENT=0x0808, //Set maximum electric current that LED controller circuit can provide
	GET_LED_PR_MAX_ELECTRICT_CURRENT=0x0809, //Get maximum electric current that LED controller circuit can provide
	SET_LED_TAKE_MAX_ELECTRICT_CURRENT=0x080A, //Set maximum electric current that LED can take
	GET_LED_TAKE_MAX_ELECTRICT_CURRENT=0X080B, //Get maximum electric current that LED can take
	SET_LED_IDLE_CURRENT=0x080C, //Set idle current of LED
	GET_LED_IDLE_CURRENT=0x080D, //Get idle current of LED

	/* Reflectance customize feature */
	CONFIG_WL_MESUREMENT=0xA000, //Config wavelengths for measuremen
	GET_REFPOS_STATUS=0xA001, //Get reference position status
	GET_BATTERY_STATUS = 0xA002, //Get battery status
	GET_DARK_AND_REFDATA_STATUS=0xA003, //Get dark and references data status
	GET_SYSTEM_ERROR_CODE=0xA004, //Get system error code
	MESURE_SAMPLE_AND_CALCULATE_REFL=0xA005, //Measure sample and calculate reflectance
	MESURE_DARK_AND_SAVE=0xA006, //Measure dark and save to memory of device
	GET_DARK_THRESH_HOLD=0xA007, //Get dark thresh hold range
	SET_DARK_THRESH_HOLD=0xA008, //Set dark thresh hold range
	MESURE_REF_AND_SAVE=0xA009, //Measure references and save to memory of device
	GET_REF_THRESH_HOLD=0xA00A, //Get references thresh hold range
	SET_REF_THRESH_HOLD=0xA00B, //Set references thresh hold range
	MESURE_DARK_AND_REF_AND_SAVE=0xA00C, //Measure dark and references and save to memory of device
	GET_DARK_DATA=0xA00D, //Get dark data from spectrometer
	GET_REF_DATA=0xA00E, //Get references data from spectrometer
	GET_REFL_ME=0xA010, //Get reflectance measure enable
	SET_REFL_ME=0xA011, //Set reflectance measure enable
	GET_SYS_SERNUM=0xA020, //Get system serial number
	SET_SYS_SERNUM=0xA021, //Set system serial number
	GET_CONFIG_WL=0xA022, //Get config wavelengths for measurement
	SET_WATTING_TIME_OFF=0xA023, //Set waiting time for off when no activity
	GET_WATTING_TIME_OFF=0xA024, //Get waiting time for off when no activity
	SET_CAL_CALIB_COEF=0xA025, //Set calculated calibration coefficients
	GET_CAL_CALIB_COEF=0xA026, //Get calculated calibration coefficients data
	MESURE_AND_GET_CALIB_REFL=0xA027, //Measure and get calibrated reflectance data
	GET_MAX_REF=0xA028, //Get maximum of trying to measure references
	SET_MAX_REF=0xA029, //Set maximum of trying to measure references
	SET_CALIB_REFL=0xA030, //Set calibration reflectance which use to calculate reflectance coefficients
	GET_CALIB_REFL=0xA031, //Get calibration reflectance which use to calculate reflectance coefficients
	RESET_CALIB_REFL=0xA032, //Reset calibration reflectance which use to calculate reflectance coefficient
	RESET_CAL_CALIB_COEF=0xA033, //Reset calculated calibration coefficients

	/* Temperature feature */
	READ_BOARD_TEMP_SENSOR=0x0B02, //Read board temperature sensor
	READ_SAMPLE_TEMP=0x0B03, //Read sample temperature using thermopile
	SET_DARK_RATIO=0x0B04, //Set dark raito
	GET_DARK_RATIO=0x0B05, //Get dark raito
} msg_t;

int set_serial_number(struct foo_device *dev, const u8 *ser_num, u8 len);
void get_serial_number(struct foo_device *dev);
error_t handle_request(struct foo_device *dev, const u8 *frame);

#endif /* INC_FOO_H_ */
