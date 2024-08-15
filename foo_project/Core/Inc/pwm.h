/*
 * pwm.h
 *
 *  Created on: Jul 11, 2024
 *      Author: Admin
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_
#include "stm32l4xx_hal.h"

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

#pragma pack(1)
struct pwm_device {
	u32 channel;
	u32 prescaler;
	float period;
	float dutycycle;
	TIM_HandleTypeDef *timer;
};
#pragma pack()

HAL_StatusTypeDef pwm_init(struct pwm_device *pwm, TIM_HandleTypeDef *tm, u8 channel);
HAL_StatusTypeDef pwm_set_frequency(struct pwm_device *pwm, float frequency);
void pwm_set_dutycycle(struct pwm_device *pwm, float dutycycle);

#endif /* INC_PWM_H_ */
