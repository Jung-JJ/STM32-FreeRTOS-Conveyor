/*
 * hw_map.h
 *
 * Created on: Aug 23, 2025
 * Author: jjj
 */

#ifndef INC_HW_MAP_H_
#define INC_HW_MAP_H_
#pragma once

#include "tim.h"
#include "main.h"
#include "i2c.h"

/*CONVEYOR PWM*/
extern  TIM_HandleTypeDef htim3;
#define PWM_CONV_ARR      3199u
#define PWM_CONV_CH       TIM_CHANNEL_1
#define PWM_CONV_HTIM     (&htim3)

/*SERVO PWM*/
extern TIM_HandleTypeDef  htim2;
#define PWM_SERVO_HTIM    (&htim2)
#define PWM_SERVO_CHANNEL TIM_CHANNEL_1

/*LCD I2C*/
extern I2C_HandleTypeDef hi2c1;
#define I2C_LCD_HANDLE   (&hi2c1)
#define I2C_LCD_ADDR     (0x27)

/*SENSOR*/
#define BTN_START_Pin_Pin GPIO_PIN_1
#define BTN_START_Pin_GPIO_Port GPIOA
#define BTN_STOP_Pin_Pin GPIO_PIN_7
#define BTN_STOP_Pin_GPIO_Port GPIOA
#define BTN_ESTOP_Pin_Pin GPIO_PIN_4
#define BTN_ESTOP_Pin_GPIO_Port GPIOC
#define BTN_RESET_Pin_Pin GPIO_PIN_5
#define BTN_RESET_Pin_GPIO_Port GPIOC

#define IR_SENSOR_Pin_Pin GPIO_PIN_0
#define IR_SENSOR_Pin_GPIO_Port GPIOB

/*LED*/
#define R_Pin GPIO_PIN_10
#define R_GPIO_Port GPIOA
#define G_Pin GPIO_PIN_11
#define G_GPIO_Port GPIOA
#define Y_Pin GPIO_PIN_12
#define Y_GPIO_Port GPIOA

#endif /* INC_HW_MAP_H_ */
