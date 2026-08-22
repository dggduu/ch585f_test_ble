#ifndef __SPLASH_LOG__
#define __SPLASH_LOG__

// ========== 头文件  ==========
#include "u8g2.h"
#include <stdarg.h>

// ==========  常量定义 ==========

// ==========  函数定义 ==========
// 初始化
void splash_log_init(u8g2_t *u8g2, uint8_t font_height,
                     const uint8_t *font_name);
// 打印到缓冲区
void splash_log_printf(const char *fmt, ...);
// 清屏函数
void splash_log_clear();
#endif