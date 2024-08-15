/*
 * pwm.c
 *
 *  Created on: Jul 11, 2024
 *      Author: Admin
 */
#include "pwm.h"
#include <math.h>

HAL_StatusTypeDef pwm_init(struct pwm_device *pwm, TIM_HandleTypeDef *tm, u8 channel) {
	if (!pwm || !tm)
		return HAL_ERROR;
	pwm->timer = tm;
	pwm->channel = channel;
	pwm->period = tm->Instance->ARR;
	pwm->prescaler = tm->Instance->PSC;

	switch (channel) {
		case TIM_CHANNEL_1:
			pwm->dutycycle = tm->Instance->CCR1;
			break;
		case TIM_CHANNEL_2:
			pwm->dutycycle = tm->Instance->CCR2;
			break;
		case TIM_CHANNEL_3:
			pwm->dutycycle = tm->Instance->CCR3;
			break;
		case TIM_CHANNEL_4:
			pwm->dutycycle = tm->Instance->CCR4;
			break;
		case TIM_CHANNEL_5:
			pwm->dutycycle = tm->Instance->CCR5;
			break;
		case TIM_CHANNEL_6:
			pwm->dutycycle = tm->Instance->CCR6;
			break;
		default:
			return HAL_ERROR;
	}

	return HAL_OK;
}

void pwm_set_dutycycle(struct pwm_device *pwm, float dutycycle) {
	HAL_TIM_PWM_Stop(pwm->timer, pwm->channel);
	if (dutycycle > 100)
		dutycycle = 100;
	pwm->dutycycle = pwm->period * dutycycle / 100;
	pwm->timer->Instance->CCR1 = round(pwm->dutycycle);
	pwm->timer->Instance->CNT = 0;
	HAL_TIM_PWM_Start(pwm->timer, pwm->channel);
}

HAL_StatusTypeDef pwm_set_frequency(struct pwm_device *pwm, float frequency) {
	HAL_StatusTypeDef res;
	float timer_frequency;
	TIM_TypeDef *Instance = pwm->timer->Instance;
	if (Instance == TIM2 || Instance == TIM6 || Instance == TIM7)
			timer_frequency = (float) HAL_RCC_GetPCLK1Freq();
	else if (Instance == TIM1 || Instance == TIM15 || Instance == TIM16)
			timer_frequency = (float) HAL_RCC_GetPCLK2Freq();
	else
			timer_frequency = (float) SystemCoreClock;

	if (!frequency)
		pwm->period = 0;
	else
		pwm->period = (float)(timer_frequency / (frequency * (float) pwm->prescaler + frequency) - 1);

	pwm->timer->Init.Period = round(pwm->period);
	pwm->timer->Instance->CNT = 0;
	res = HAL_TIM_PWM_Init(pwm->timer);
	return res;
}
