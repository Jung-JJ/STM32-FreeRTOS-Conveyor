/*
 * task_conveyor.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */
#include "cmsis_os.h"
#include "app_os.h"
#include "driver_conveyor.h"
#include "state.h"

void StartConveyorTask(void const *arg){
	(void)arg; //경고 제거
	Conveyor_Init(); //컨베이어 초기화
	ConvCmd cmd; // 컨베이어 구조체 정의
	for(;;){
		if(xQueueReceive(gConvQ, &cmd, portMAX_DELAY)==pdPASS){ //gConvQ큐에 데이터 들어오기 전 까지 무한 대기.
			if(cmd.type==CONV_CMD_STOP){ //정지 명령이면 컨베이어 멈추기
				Conveyor_Stop();
			}
			else if(cmd.type==CONV_CMD_SET){
		        for (uint16_t p=0; p<=cmd.duty_pct; p+=3){ Conveyor_SetDutyPct(p); osDelay(10); } //컨베이서 속도 천천히 올리기
			}
		}
	    if (g_state==STATE_FAULT) Conveyor_Stop(); //상태가 FAULT면 정지

	}
}

