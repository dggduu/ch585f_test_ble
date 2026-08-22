#include "splash_log.h"
#include "screen.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ===================== 配置 =====================
#define LOG_LINE_SPACING 5
#define MAX_CHAR_PER_LINE 50
#define SPLASH_LOG_MAX_ROWS 32   // 硬编码最大行数，确保足够

// 静态变量
static uint8_t s_font_height = 8;   // 默认字体高度
static uint8_t s_max_rows = 0;      // 根据屏幕高度动态计算
static char s_lines[SPLASH_LOG_MAX_ROWS][MAX_CHAR_PER_LINE + 1];
static uint8_t s_current_rows = 0;
static u8g2_t *s_u8g2 = NULL;

/**
 * @brief 将单行文本推入缓冲区，满屏后上移
 */
static void _push_single_line(const char *single_line) {
    if (s_current_rows < s_max_rows) {
        strncpy(s_lines[s_current_rows], single_line, MAX_CHAR_PER_LINE);
        s_lines[s_current_rows][MAX_CHAR_PER_LINE] = '\0';
        s_current_rows++;
    } else {
        memmove(&s_lines[0], &s_lines[1], sizeof(s_lines[0]) * (s_max_rows - 1));
        strncpy(s_lines[s_max_rows - 1], single_line, MAX_CHAR_PER_LINE);
        s_lines[s_max_rows - 1][MAX_CHAR_PER_LINE] = '\0';
    }
}

void splash_log_init(u8g2_t *u8g2, uint8_t font_height,
                     const uint8_t *font_name) {
    s_u8g2 = u8g2;
    s_font_height = font_height;

    // 根据 u8g2 对象的高度计算最大行数
    uint16_t screen_h = u8g2->height;
    s_max_rows = screen_h / (s_font_height + LOG_LINE_SPACING);
    if (s_max_rows > SPLASH_LOG_MAX_ROWS) {
        s_max_rows = SPLASH_LOG_MAX_ROWS;
    }
    if (s_max_rows == 0) {
        s_max_rows = 1;
    }

    s_current_rows = 0;
    memset(s_lines, 0, sizeof(s_lines));
    u8g2_SetFont(s_u8g2, font_name);
}

void splash_log_printf(const char *fmt, ...) {
    if (!s_u8g2 || !fmt)
        return;

    char long_buffer[128] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(long_buffer, sizeof(long_buffer), fmt, args);
    va_end(args);

    char *ptr = long_buffer;
    while (*ptr != '\0') {
        char tmp_seg[MAX_CHAR_PER_LINE + 1] = {0};
        strncpy(tmp_seg, ptr, MAX_CHAR_PER_LINE);
        tmp_seg[MAX_CHAR_PER_LINE] = '\0';  // 确保字符串结尾

        _push_single_line(tmp_seg);

        size_t len = strlen(ptr);
        if (len > MAX_CHAR_PER_LINE) {
            ptr += MAX_CHAR_PER_LINE;
        } else {
            break;
        }
    }

    u8g2_ClearBuffer(s_u8g2);

    for (uint8_t i = 0; i < s_current_rows; i++) {
        uint16_t y = (i + 1) * (s_font_height + LOG_LINE_SPACING);
        u8g2_DrawStr(s_u8g2, 0, y, s_lines[i]);  // 直接绘制文本
    }

    u8g2_SendBuffer(s_u8g2);
}

void splash_log_clear(void) {
    if (!s_u8g2)
        return;
    s_current_rows = 0;
    memset(s_lines, 0, sizeof(s_lines));
    u8g2_ClearBuffer(s_u8g2);
    u8g2_SendBuffer(s_u8g2);
}