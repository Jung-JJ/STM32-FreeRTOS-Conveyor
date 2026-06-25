/*
 * driver_conveyor.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */


#include "driver_conveyor.h"
#include "hw_map.h"
#include "time.h"

void Conveyor_Init(void){
  HAL_TIM_PWM_Start(PWM_CONV_HTIM, PWM_CONV_CH);     //컨베이어 PWM 시작
  __HAL_TIM_SET_COMPARE(PWM_CONV_HTIM, PWM_CONV_CH, 0); //컨베이어 듀티를 0으로 설정함
}

void Conveyor_SetDutyPct(uint16_t pct){ //퍼센트로 받은 듀티를 CCR로 바꿈
  if (pct > 100) pct = 100; //듀티를 100%가 초과 했을 떄 100으로 설정
  uint32_t ccr = (PWM_CONV_ARR * pct) / 100; //퍼센트 듀티를 CCR 값으로 변환
  __HAL_TIM_SET_COMPARE(PWM_CONV_HTIM, PWM_CONV_CH, ccr); //PWM 비교값 갱신
}

void Conveyor_Stop(void){
  __HAL_TIM_SET_COMPARE(PWM_CONV_HTIM, PWM_CONV_CH, 0); //컨베이어 STOP
}
