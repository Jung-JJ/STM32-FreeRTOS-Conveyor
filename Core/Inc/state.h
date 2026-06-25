/*
 * state.h
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 *      idle은 대기
 *      run 동작 중
 *      processing 동작 중
 *      fault 고장
 *
 *      none 고장x
 *      estop 이스톱
 *      sensor 센서 이상
 */

#ifndef INC_STATE_H_
#define INC_STATE_H_
#pragma once
#include <stdint.h>

typedef enum { STATE_IDLE, STATE_RUN, STATE_PROCESSING, STATE_FAULT } SystemState_t;
typedef enum { FAULT_NONE =0, FAULT_ESTOP, FAULT_SENSOR, FAULT_SERVO } FaultCode_t;

extern volatile SystemState_t g_state;
extern volatile FaultCode_t g_fault;



#endif /* INC_STATE_H_ */
