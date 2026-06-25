/*
 * task_hmi.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */

#include "cmsis_os.h"
#include "app_os.h"
#include "driver_conveyor.h"
#include "state.h"
#include "driver_hmi.h"
#include "driver_lcd_i2c.h"

extern volatile SystemState_t g_state;
extern volatile FaultCode_t   g_fault;

static SystemState_t prev_state = (SystemState_t)0xFF;
static FaultCode_t   prev_fault = (FaultCode_t)0xFF;

void StartHMITask(void const *argument){
    (void)argument;
    HMI_Init(); //초기화
    uint32_t t = 0;

    for(;;){
        if (g_state == STATE_FAULT){ //FAULT상태 최우선
            HMI_SetRun(LED_OFF);
            HMI_SetStop(LED_OFF);
            HMI_SetFault(LED_BLINK);

            if (g_fault != prev_fault || g_state != prev_state){ //FAULT종류가 바뀌거나 상태가 바뀌면 실행
                switch (g_fault){ //실패 상테에 따른 LCD 출력
                case FAULT_ESTOP:
                    LCD_Print2("!!!  E-STOP  !!!", "Press RESET");
                    break;
                case FAULT_SENSOR:
                    LCD_Print2("FAULT: SENSOR", "Press RESET");
                    break;
                case FAULT_SERVO:
                    LCD_Print2("FAULT: SERVO", "Press RESET");
                    break;
                default:
                    LCD_Print2("FAULT DETECTED", "Press RESET");
                    break;
                }
                prev_state = g_state; //상태 저장
                prev_fault = g_fault; //실패 저장
            }

            HMI_Tick(t); //led Blink 250ms마다 한 주기는 500ms
            osDelay(100);
            t += 100;
            continue; // 아래 상태 로직으로 내려가지 않음
        }

        if (g_state != prev_state || g_fault != prev_fault){ //비FAULT 상태: 상태/고장 변화시에만 화면 갱신
            prev_state = g_state;
            prev_fault = g_fault;

            switch (g_state){
            case STATE_IDLE:
                HMI_SetRun(LED_OFF);
                HMI_SetStop(LED_ON);
                HMI_SetFault(LED_OFF);
                LCD_Print2("IDLE", "Press START");
                break;

            case STATE_RUN:
                HMI_SetRun(LED_ON);
                HMI_SetStop(LED_OFF);
                HMI_SetFault(LED_OFF);
                LCD_Print2("RUNNING SENSOR", "WAIT...");
                break;

            case STATE_PROCESSING:
                HMI_SetRun(LED_BLINK);
                HMI_SetStop(LED_OFF);
                HMI_SetFault(LED_OFF);
                LCD_Print2("PROCESSING", "KICKING...");
                break;

            default:
                HMI_SetRun(LED_OFF);
                HMI_SetStop(LED_OFF);
                HMI_SetFault(LED_OFF);
                LCD_Print2("UNKNOWN", "CHECK STATE");
                break;
            }
        }

        // 비FAULT 구간에서는 Fault LED는 항상 꺼두기
        HMI_SetFault(LED_OFF);

        HMI_Tick(t);
        osDelay(100);
        t += 100;
    }
}

