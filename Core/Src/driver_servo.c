/*
 * driver_servo.c
 *
 * Created on: Aug 22, 2025
 * Author: jjj
 */

#include "driver_servo.h"
#include "tim.h"
#include "hw_map.h"

// PWM 신호 주파수 및 펄스 폭 상수
#define PWM_FREQUENCY_HZ 50       // 50Hz (20ms 주기)
#define SERVO_MAX_US     2500     // 2.5ms (최대 각도)
#define SERVO_MIN_US     500      // 0.5ms (최소 각도)
#define SERVO_NEUTRAL_US 1500     // 1.5ms (중립)


void Servo_Init(void){ //서보 초기화
	HAL_TIM_PWM_Start(PWM_SERVO_HTIM, PWM_SERVO_CHANNEL);
	Servo_WriteUS(SERVO_NEUTRAL_US);
}

void Servo_WriteUS(uint16_t us){
    if(us < SERVO_MIN_US) us = SERVO_MIN_US; //서보 보호
    if(us > SERVO_MAX_US) us = SERVO_MAX_US;

    // 펄스 폭을 Period(ARR) 값에 비례하여 계산
    // ex) 1.5ms / 20ms * 20000 = 1500
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(PWM_SERVO_HTIM) + 1; //타이머의 PWM 한 주기 카운트 수 가져오기(전체 주기 카운트 수)
    uint32_t compare = (uint32_t)((float)us / (1000000.0f / PWM_FREQUENCY_HZ) * period); //pwm주기 us로 계산. CCR= (HIGHTIME/주기 초 us단위)(듀티비) * 카운트 수

    __HAL_TIM_SET_COMPARE(PWM_SERVO_HTIM, PWM_SERVO_CHANNEL, compare);
}

void Servo_WriteDeg(uint16_t deg){ //입력 받은 각도를 PWM에 넣음
    if(deg > 180) deg = 180;
    uint16_t us = SERVO_MIN_US + (uint16_t)((float)deg / 180.0f * (SERVO_MAX_US-SERVO_MIN_US)); //0~180도 각도를 500~2500us로 매핑(선형 보간식 이용).
    Servo_WriteUS(us);
}
