/*
 * task_servo.c
 *
 * Created on: Aug 22, 2025
 * Author: jjj
 */

#include "cmsis_os.h"
#include "app_os.h"
#include "events.h"
#include "driver_servo.h"

void StartServoTask(void const *arg)
{
  (void)arg;
  Servo_Init();// 서보 초기화

  const uint32_t PRE_KICK_DELAY_MS  = 150;
  const uint16_t SERVO_WORK_DEG     = 90;
  const uint32_t POST_MOVE_SETTLEMS = 150;
  const uint32_t CLEAR_TIMEOUT_MS   = 2500;

  ActCmd cmd;
  for(;;){
    if (xQueueReceive(gActQ, &cmd, portMAX_DELAY) == pdPASS){ //큐에 명령어 올 떄 까지 무한 대기.
      if (cmd.type == ACT_CMD_KICK){ //킥 명령이면
        uint16_t work = (uint16_t)(cmd.arg1 ? cmd.arg1 : SERVO_WORK_DEG); //cmd.arg1이 있으면 cmd.arg1값 사용 아니면 90도
        uint32_t dwell= (uint32_t)(cmd.arg2 ? cmd.arg2 : POST_MOVE_SETTLEMS); //위와 같음

        osDelay(PRE_KICK_DELAY_MS); //서보 움직이기 전 대기
        Servo_WriteDeg(work); //서보 움직이기.
        osDelay(dwell); //서보 이동할 떄 까지 대기

        EventBits_t got = xEventGroupWaitBits(
            gEvent, EV_AREA_CLEAR, pdTRUE, pdFALSE,
            pdMS_TO_TICKS(CLEAR_TIMEOUT_MS)); //EV_AREA_CLEAR이벤트를 기다림 최대 2.5초 동안

        if ((got & EV_AREA_CLEAR) == 0) { //실패
          xEventGroupSetBits(gEvent, EV_FAULT_SERVO);   // STOP
        }
        else { //성공
          Servo_WriteDeg(0); //서보 0도로
          osDelay(dwell); //이동할 때 까지 대기
          xEventGroupSetBits(gEvent, EV_ACT_DONE);
        }
      }
    }
  }
}
