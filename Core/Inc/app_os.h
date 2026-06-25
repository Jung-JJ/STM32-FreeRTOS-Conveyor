/*
 * app_os.h
 *
 *  Created on: Aug 23, 2025
 *      Author: jjj
 */

#ifndef INC_APP_OS_H_
#define INC_APP_OS_H_
#pragma once
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

extern EventGroupHandle_t gEvent;

typedef enum { CONV_CMD_SET, CONV_CMD_STOP } ConvCmdType; //컨베이어 구조체 설정(큐).
typedef struct{
	ConvCmdType type;
	uint16_t duty_pct;
} ConvCmd; //타입이랑 듀티값을 넣을 수 있음.

extern QueueHandle_t gConvQ;

typedef enum { ACT_CMD_KICK } ActCmdType; //서보모터 구조체 설정(큐).
typedef struct{
	ActCmdType type;
	int32_t arg1;
	int32_t arg2;
} ActCmd; //타입이랑 각도 유지시간.

extern QueueHandle_t gActQ;

void AppOS_CreateObjects(void);
void AppOS_CreateTasks(void);
#endif /* INC_APP_OS_H_ */
