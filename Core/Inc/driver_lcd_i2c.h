/*
 * driver_lcd_i2c.h
 *
 *  Created on: Aug 24, 2025
 *      Author: jjj
 */

#ifndef INC_DRIVER_LCD_I2C_H_
#define INC_DRIVER_LCD_I2C_H_

#pragma once

#include <stdint.h>

#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08

#define I2C_LCD_COLS     16
#define I2C_LCD_ROWS     2
void LCD_Init(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *s);
void LCD_Print2(const char* l1,const char* l2);
void LCD_PrintfAt(uint8_t row, uint8_t col, const char* fmt, ...);
void LCD_Backlight_On(void);
void LCD_Backlight_Off(void);

#endif /* INC_DRIVER_LCD_I2C_H_ */
