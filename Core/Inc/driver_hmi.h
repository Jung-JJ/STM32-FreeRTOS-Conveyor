/*
 * driver_hmi.h
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */

#ifndef INC_DRIVER_HMI_H_
#define INC_DRIVER_HMI_H_

#pragma once
#include <stdint.h>

typedef enum { LED_OFF, LED_ON, LED_BLINK } LedMode;

void HMI_Init(void);

void HMI_SetRun(LedMode m);
void HMI_SetStop(LedMode m);
void HMI_SetFault(LedMode m);

void HMI_Tick(uint32_t tick_ms);

#endif /* INC_DRIVER_HMI_H_ */
