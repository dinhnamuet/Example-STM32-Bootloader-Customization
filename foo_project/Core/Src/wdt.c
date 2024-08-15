/*
 * wdt.c
 *
 *  Created on: Jul 18, 2024
 *      Author: Admin
 */
#include "wdt.h"

HAL_StatusTypeDef watchdog_set_timeout(IWDG_HandleTypeDef *wdt,
		timeout_t timeout) {
	switch (timeout) {
	case TIMEOUT_HALF_SEC:
		wdt->Init.Prescaler = WDT_PSC_4;
		break;
	case TIMEOUT_ONE_SEC:
		wdt->Init.Prescaler = WDT_PSC_8;
		break;
	case TIMEOUT_2_SEC:
		wdt->Init.Prescaler = WDT_PSC_16;
		break;
	case TIMEOUT_4_SEC:
		wdt->Init.Prescaler = WDT_PSC_32;
		break;
	case TIMEOUT_8_SEC:
		wdt->Init.Prescaler = WDT_PSC_64;
		break;
	case TIMEOUT_16_SEC:
		wdt->Init.Prescaler = WDT_PSC_128;
		break;
	case TIMEOUT_32_SEC:
		wdt->Init.Prescaler = WDT_PSC_256;
		break;
	default:
		return HAL_ERROR;
	}
	wdt->Init.Reload = 4095;
	wdt->Init.Window = 4095;
	return HAL_IWDG_Init(wdt);
}

HAL_StatusTypeDef ping_to_watchdog(IWDG_HandleTypeDef *wdt) {
	return HAL_IWDG_Refresh(wdt);
}
