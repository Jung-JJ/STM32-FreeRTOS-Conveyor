	/*
 * driver_lcd_i2c.c
 *
 *  Created on: Aug 24, 2025
 *      Author: jjj
 */

#include "driver_lcd_i2c.h"
#include "hw_map.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#define I2C_LCD_TIMEOUT_MS 50 //LCD 타임아웃 MS은 50
#define I2C_LCD_MAX_RETRY  3 //최대 재 시도는 3번
#define lcd_delay_ms(ms) vTaskDelay(pdMS_TO_TICKS(ms)) //딜레이를 해야하는데 FREERTOS떄문에 테스크 딜레이로(ms->Tick으로도 변환)
	static uint8_t g_bl = LCD_BL; //백라이트 켜는 주소

static inline HAL_StatusTypeDef i2c_send (uint8_t data){
	uint8_t b = data | g_bl; //백라이트 비트 추기
	HAL_StatusTypeDef st; //상태값 반환 하는거 정의

	for(int attempt=0;attempt<I2C_LCD_MAX_RETRY;attempt++){ //최대 3번정도 연결 시도
		st = HAL_I2C_Master_Transmit(I2C_LCD_HANDLE,
				I2C_LCD_ADDR<<1, &b, 1, I2C_LCD_TIMEOUT_MS); //LCD(PCF8574)로 1바이트 전송, HAL 규격에 맞게 7비트 주소를 <<1 하여 사용
		if(st == HAL_OK) return HAL_OK; //성공하면 HAL_OK반환
		lcd_delay_ms(2);
	}
	return HAL_ERROR;
}

static void pulse_en(uint8_t d){ //데이터 읽기 신호 보내기
	i2c_send(d | LCD_EN); //LCD en 신호 보내기
	lcd_delay_ms(1); //테스크 딜레이
	i2c_send(d & ~LCD_EN); //en 끄기
	lcd_delay_ms(1);
}

static void send4(uint8_t nib, uint8_t rs){ //4비트 데이터 보내는 함수.
	uint8_t d = (nib &0xF0) | (rs ? LCD_RS:0); //rs가 0이면 명령 1이면 데이터
	pulse_en(d);
}

static void send8(uint8_t v, uint8_t rs){//8비트 데이터 보내기 (4비트씩 I2C모듈 떄문에)
	send4(v & 0xF0, rs); //상위 4비트 보내기
	send4((uint8_t)(v<<4)&0xF0,rs); //하위 4비트 밀어서 상위 4비트가지고 보내기
}

static void cmd(uint8_t c){ //명령 데이터 보내기
	send8(c,0);
	lcd_delay_ms(1);
}

static void dat(uint8_t d){ //데이터 보내기
	send8(d,1);
	lcd_delay_ms(1);
}

void LCD_Init(void){ //LCD 초기설정 (데이터 시트 참고)
	lcd_delay_ms(50);

	send4(0x30, 0); lcd_delay_ms(5);
	send4(0x30, 0); lcd_delay_ms(5);
	send4(0x30, 0); lcd_delay_ms(5);
	send4(0x20, 0); lcd_delay_ms(5);

	cmd(0x28); //set: 4-bit,2-line, 5x8dots 00101000
	cmd(0x08); //display off
	cmd(0x01); lcd_delay_ms(3); //clear
	cmd(0x06); //Entry mode
	cmd(0x0C); //display on cursor off blink off
}

void LCD_SetCursor(uint8_t row, uint8_t col){ //커서 설정.
    if (col >= I2C_LCD_COLS) col = 0;

    // 표준 HD44780 DDRAM 베이스: 16x2 = {0x00, 0x40}
    // (20x4는 {0x00,0x40,0x14,0x54})
    static const uint8_t base[] = { 0x00, 0x40, 0x14, 0x54 };

    if (row >= I2C_LCD_ROWS) row = 0;
    uint8_t addr = (uint8_t)(base[row] + col);
    cmd((uint8_t)(0x80 | addr));
}

void LCD_Print(const char *s){ //문자열 출력
	while(*s) dat((uint8_t)*s++);
}

void LCD_Print2(const char* l1,const char* l2){// 두 줄 문자 출력
	LCD_SetCursor(0, 0);
	for(uint8_t i=0;i<I2C_LCD_COLS;i++){
		dat((uint8_t)(l1&&l1[i] ? l1[i] : ' ')); //뒤에는 공백을 채워 이전 글자 지우기
	}
	LCD_SetCursor(1, 0);
		for(uint8_t i=0;i<I2C_LCD_COLS;i++){
			dat((uint8_t)(l2&&l2[i] ? l2[i] : ' '));
		}
}

void LCD_PrintfAt(uint8_t row, uint8_t col, const char* fmt, ...){ //printf 형태를 지원하기 위해
	char buf[32]; //담아놓을 버퍼
	va_list ap; va_start(ap, fmt); //가변인자 받기 위해
	vsnprintf(buf, sizeof(buf), fmt, ap); //가변 인자 이용해서 문자열 만들어 buf에 넣기 (sizeof로 오버플로우 방지)
	va_end(ap); //가변인자 사용 종료
	LCD_SetCursor(row, col);
	LCD_Print(buf);
}

void LCD_Backlight_On(void){ g_bl = LCD_BL; i2c_send(0); }

void LCD_Backlight_Off(void){ g_bl= 0; i2c_send(0);}










