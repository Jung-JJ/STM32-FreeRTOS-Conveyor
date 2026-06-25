/*
 * task_sensor.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */

#include "cmsis_os.h"
#include "events.h"
#include "app_os.h"
#include "belt.h"
#include "stm32f1xx_hal.h"
#include "state.h"   // g_state 사용

//IR: Active-Low → RESET이면 감지(1)
static inline uint8_t ir_is_detected(void){
  return (HAL_GPIO_ReadPin(IR_SENSOR_Pin_GPIO_Port, IR_SENSOR_Pin_Pin) == GPIO_PIN_RESET) ? 1u : 0u;
}

//버튼 읽기: 눌림=LOW(0), 뗌=HIGH(1)
#define READ_BTN(port,pin) ((HAL_GPIO_ReadPin((port),(pin))==GPIO_PIN_RESET) ? 0u : 1u)

void StartSensorTask(void const *arg)
{
  (void)arg;

  //버튼 이전 상태 기록
  uint8_t ps_prev = READ_BTN(BTN_START_Pin_GPIO_Port, BTN_START_Pin_Pin);
  uint8_t pt_prev = READ_BTN(BTN_STOP_Pin_GPIO_Port,  BTN_STOP_Pin_Pin);
  uint8_t pr_prev = READ_BTN(BTN_RESET_Pin_GPIO_Port, BTN_RESET_Pin_Pin);

  //디바운싱 시간
  const uint32_t BTN_HOLD_MS = 15;     // 10~30ms 권장
  uint32_t ps_hold_ts = 0, pt_hold_ts = 0, pr_hold_ts = 0;

  //IR 상태
  uint8_t  ir_prev        = ir_is_detected(); // 1=감지, 0=비감지
  uint32_t clear_start    = 0;
  uint32_t det_hold_ts    = 0;

  /* - 감지 게이팅 플래그
     - RUN 상태에서 영역이 '충분히 비워짐'이 확인되면 armed=1 로 재장전
     - 감지 이벤트(EV_OBJ_DETECTED)를 한 번 발행하면 armed=0 으로 내려 자동 재트리거 방지 */
  uint8_t  detect_armed   = 1;


  const uint32_t CLEAR_MS       = 50;  // 비감지 지속 AREA_CLEAR
  const uint32_t DETECT_HOLD_MS = 30;  // 감지 지속 OBJ_DETECTED (엣지 누락 대비)

  for(;;){
    // START 처리구간
    {
      uint8_t now = READ_BTN(BTN_START_Pin_GPIO_Port, BTN_START_Pin_Pin); //현제 버튼 핀 기록
      if (now != ps_prev) { //지금이랑 이전이 다르면 실
        if (ps_hold_ts == 0) ps_hold_ts = xTaskGetTickCount(); // 변화 시작 시간 저장
        else if (xTaskGetTickCount() - ps_hold_ts >= pdMS_TO_TICKS(BTN_HOLD_MS)) { //아니면 누른 시간에서 현제 시간 뺴서 15ms보다 크면
          ps_prev = now;  // 이전 상태 갱신
          ps_hold_ts = 0; // 디바운스 타이머 초기화
          if (now == 0) xEventGroupSetBits(gEvent, EV_START); // 버튼 눌림 확정 START 이벤트 발생
        }
      } else ps_hold_ts = 0; // 상태 변화가 없으면 디바운스 타이머 초기화
    }
    // STOP
    {
      uint8_t now = READ_BTN(BTN_STOP_Pin_GPIO_Port, BTN_STOP_Pin_Pin);
      if (now != pt_prev) {
        if (pt_hold_ts == 0) pt_hold_ts = xTaskGetTickCount();
        else if (xTaskGetTickCount() - pt_hold_ts >= pdMS_TO_TICKS(BTN_HOLD_MS)) {
          pt_prev = now; pt_hold_ts = 0;
          if (now == 0) xEventGroupSetBits(gEvent, EV_STOP);
        }
      } else pt_hold_ts = 0;
    }
    // RESET
    {
      uint8_t now = READ_BTN(BTN_RESET_Pin_GPIO_Port, BTN_RESET_Pin_Pin);
      if (now != pr_prev) {
        if (pr_hold_ts == 0) pr_hold_ts = xTaskGetTickCount();
        else if (xTaskGetTickCount() - pr_hold_ts >= pdMS_TO_TICKS(BTN_HOLD_MS)) {
          pr_prev = now; pr_hold_ts = 0;
          if (now == 0) xEventGroupSetBits(gEvent, EV_RESET);
        }
      } else pr_hold_ts = 0;
    }

    //IR: 감지 게이팅 + RUN 상태에서만 트리거
    uint8_t run_now = (g_state == STATE_RUN) ? 1u : 0u;
    uint8_t ir_now  = ir_is_detected();  // 1=감지, 0=비감지

    // 비감지(0) 지속이면 AREA_CLEAR 1회 발행 + (RUN 상태일 때) armed 재장전
    if (ir_now == 0) {
      if (clear_start == 0) {
        clear_start = xTaskGetTickCount(); //물체가 없어진 시각 기록
      } else if (xTaskGetTickCount() - clear_start >= pdMS_TO_TICKS(CLEAR_MS)) { //50ms 이상 물체 감지 안 하면(노이즈 처리)
        xEventGroupSetBits(gEvent, EV_AREA_CLEAR); //클리어 이벤트
        clear_start  = 0;
        if (run_now) detect_armed = 1;   // 다음 감지 허용
      }
    }
    else {
      clear_start = 0;
    }

    // 감지(1): RUN 상태 && armed=1 일 때만 OBJ_DETECTED 발행
    if (ir_now == 1 && run_now && detect_armed) {
      if (ir_prev == 0) { //이전 IR값이 0이였으면(물체가 없다가 생겼을 때)
        xEventGroupSetBits(gEvent, EV_OBJ_DETECTED);
        detect_armed = 0;           // 한 번만 트리거 하기 위함
        det_hold_ts  = 0;           // 홀드 타이머 무효화
      }
      else {
        //엣지 누락 대비로 단 1회만
        if (det_hold_ts == 0) det_hold_ts = xTaskGetTickCount();
        else if (xTaskGetTickCount() - det_hold_ts >= pdMS_TO_TICKS(DETECT_HOLD_MS)) { //물체가 30ms로 계속 감지 도면
          xEventGroupSetBits(gEvent, EV_OBJ_DETECTED);
          detect_armed = 0;         // 한 번만 트리거
          det_hold_ts  = 0;         // 재발행 금지
        }
      }
    }
    else {
      det_hold_ts = 0;              // 조건 불충족 시 시간 리셋
    }

    ir_prev = ir_now;


    Belt_StateMachine_Step(); //이벤트 처리

    osDelay(10);
  }
}
