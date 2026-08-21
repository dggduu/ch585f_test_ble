#ifndef __UI_TOOLKIT_H__
#define __UI_TOOLKIT_H__
#include "screen.h"
#include "u8g2.h"

// 绘制滚动文字函数,带停顿
void draw_scroll_text_with_pause(u8g2_t *u8g2, const Screen_t *screen_cfg,
                                 const char *text, uint8_t start_x,
                                 uint8_t max_width, uint8_t y, uint32_t tick,
                                 uint8_t clip_y1, uint8_t clip_y2);
#endif