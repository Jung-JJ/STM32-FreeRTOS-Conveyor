/*
 * driver_conveyor.h
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */

#ifndef INC_DRIVER_CONVEYOR_H_
#define INC_DRIVER_CONVEYOR_H_

#pragma once
#include <stdint.h>

void Conveyor_Init(void);
void Conveyor_SetDutyPct(uint16_t pct);
void Conveyor_Stop(void);

#endif /* INC_DRIVER_CONVEYOR_H_ */
