#ifndef __BSP_LCD_H__
#define __BSP_LCD_H__

#include "CH58x_common.h"
#include "stdint.h"

// 屏幕尺寸
#define LCD_W   240
#define LCD_H   240

// 颜色定义（16位 RGB565）
#define LCD_WHITE         0xFFFF
#define LCD_BLACK         0x0000
#define LCD_RED           0xF800
#define LCD_GREEN         0x07E0
#define LCD_BLUE          0x001F
#define LCD_YELLOW        0xFFE0
#define LCD_CYAN          0x07FF
#define LCD_MAGENTA       0xF81F
#define LCD_GRAY          0x7BEF

// 屏幕方向（0~3，根据实际接线调整）
#define USE_HORIZONTAL 2   // 0:竖屏, 1:横屏（右转）, 2:横屏（左转）, 3:竖屏翻转

// 外部变量
extern uint16_t BACK_COLOR;   // 背景色

// 基础函数
void Lcd_Init(void);
void LCD_Clear(uint16_t Color);
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);

// 批量写像素（供 u8g2 显示框架调用）：设置窗口后一次发送 RGB565 像素数组
// x1<=x2, y1<=y2，count 必须等于 (x2-x1+1)*(y2-y1+1)
void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                     const uint16_t *pixels, uint32_t count);
// void LCD_SendBuffer(const uint8_t *index_buf, const uint16_t *palette, uint16_t width, uint16_t height, uint8_t bpp);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawPoint_big(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

// 字符/字符串显示
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color);
void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t color);
void LCD_ShowNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t color);
void LCD_ShowNum1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t color);

// 测试图案
void LCD_DispBand(void);
void LCD_DispGrayHor16(void);
void LCD_DispSnow(void);
void LCD_DispBlock(void);

#endif