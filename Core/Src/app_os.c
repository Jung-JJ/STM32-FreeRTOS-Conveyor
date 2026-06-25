/*
 * app_os.c
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 *      RTOS 객체 생성 + 테스크 생성 담당
 */


#include "app_os.h"

osThreadId sensorTaskHandle, conveyorTaskHandle, servoTaskHandle, hmiTaskHandle;

/*
 아래는 전역 RTOS 객체 3개 생성
 */
EventGroupHandle_t gEvent;
QueueHandle_t gConvQ;
QueueHandle_t gActQ;

void StartSensorTask(void const *arg);
void StartConveyorTask(void const *arg);
void StartServoTask(void const *arg);
void StartHMITask(void const *arg);

void AppOS_CreateObjects(void){
	gEvent = xEventGroupCreate(); //시스템 이벤트 플래그
	gActQ = xQueueCreate(8,sizeof(ActCmd)); //서보 큐
	gConvQ = xQueueCreate(8,sizeof(ConvCmd)); //컨베이어 큐
}

void AppOS_CreateTasks(void){
	osThreadDef(sensor, StartSensorTask, osPriorityNormal,0, 256); //센서
	osThreadDef(conveyor, StartConveyorTask, osPriorityNormal,0 ,256); //컨베이어
	osThreadDef(servo, StartServoTask, osPriorityNormal,0 ,256); //서보모터
	osThreadDef(hmi, StartHMITask, osPriorityLow,0 ,256); //LCD Task 위에거 보다는 크게 중요 x
	sensorTaskHandle = osThreadCreate(osThread(sensor), NULL); // Sensor Task 생성 후 해당 Task를 가리키는 Handle 저장
	conveyorTaskHandle = osThreadCreate(osThread(conveyor), NULL);
	servoTaskHandle = osThreadCreate(osThread(servo), NULL);
	hmiTaskHandle = osThreadCreate(osThread(hmi), NULL);
}



