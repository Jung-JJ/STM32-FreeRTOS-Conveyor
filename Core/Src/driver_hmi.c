/*
 * driver_hmi.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */
#include "driver_hmi.h"
#include "driver_lcd_i2c.h"
#include "i2c.h"
#include <stdarg.h>
#include <stdio.h>

static LedMode runM=LED_OFF, stopM=LED_OFF, faultM=LED_OFF;


static void drive_led(GPIO_TypeDef* port, uint16_t pin, LedMode m, uint32_t t_ms){ // LED 모드(ON/OFF/BLINK)에 따라 GPIO 출력 제어
    switch (m){
    case LED_ON:
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        break;
    case LED_OFF:
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        break;
    case LED_BLINK:
        HAL_GPIO_WritePin(port, pin, ((t_ms/250U)%2U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;
    }
}


void HMI_Init(void){ //LCD 초기
	LCD_Init(); //LCD 초기화
	LCD_Backlight_On(); //백라이트 켜기
	LCD_Print2("Init...","wait"); //LCD에 글쓰기
	runM=LED_OFF, stopM=LED_OFF, faultM=LED_OFF; // 모든 LED 상태 변수 초기화
	HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Y_GPIO_Port, Y_Pin, GPIO_PIN_RESET);
}

void HMI_SetRun (LedMode m){ runM = m; } //run stop fault LED 동작 모드 설정.
void HMI_SetStop (LedMode m){ stopM = m; }
void HMI_SetFault (LedMode m){ faultM = m; }

void HMI_Tick(uint32_t tick_ms){ //Tick시간을 받고 실제 LED를 켜거나 끄게 하는 함수.
	drive_led(G_GPIO_Port, G_Pin, runM, tick_ms);
	drive_led(R_GPIO_Port, R_Pin, stopM, tick_ms);
	drive_led(Y_GPIO_Port, Y_Pin, faultM, tick_ms);
}








