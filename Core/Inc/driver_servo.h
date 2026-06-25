/*
 * driver_servo.h
 *
 * Created on: Aug 22, 2025
 * Author: jjj
 */

#ifndef INC_DRIVER_SERVO_H_
#define INC_DRIVER_SERVO_H_

#include "main.h"

#define PWM_FREQUENCY_HZ 50       // 50Hz (20ms 주기)


void Servo_Init(void);
void Servo_WriteUS(uint16_t us);
void Servo_WriteDeg(uint16_t deg);

#endif /* INC_DRIVER_SERVO_H_ */
