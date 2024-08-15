/*
 * wdt.h
 *
 *  Created on: Jul 18, 2024
 *      Author: Admin
 */

#ifndef INC_WDT_H_
#define INC_WDT_H_
#include "stm32l4xx_hal.h"

#define WDT_PSC_4	IWDG_PRESCALER_4
#define WDT_PSC_8	IWDG_PRESCALER_8
#define WDT_PSC_16	IWDG_PRESCALER_16
#define WDT_PSC_32	IWDG_PRESCALER_32
#define WDT_PSC_64	IWDG_PRESCALER_64
#define WDT_PSC_128	IWDG_PRESCALER_128
#define WDT_PSC_256	IWDG_PRESCALER_256


typedef enum {
	TIMEOUT_HALF_SEC,
	TIMEOUT_ONE_SEC,
	TIMEOUT_2_SEC,
	TIMEOUT_4_SEC,
	TIMEOUT_8_SEC,
	TIMEOUT_16_SEC,
	TIMEOUT_32_SEC
} timeout_t;

HAL_StatusTypeDef watchdog_set_timeout(IWDG_HandleTypeDef *wdt, timeout_t timeout);
HAL_StatusTypeDef ping_to_watchdog(IWDG_HandleTypeDef *wdt);

#endif /* INC_WDT_H_ */
