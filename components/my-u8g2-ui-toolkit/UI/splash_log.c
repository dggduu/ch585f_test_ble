#include "splash_log.h"
#include "screen.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ===================== 配置 =====================
#define LOG_LINE_SPACING 5
#define MAX_LOG_LINES                                                          \
  (g_screen_cfg.height / (g_screen_cfg.font_height + LOG_LINE_SPACING))
#define MAX_CHAR_PER_LINE 50

// 静态缓冲区
static char s_lines[12][MAX_CHAR_PER_LINE + 1];
static uint8_t s_current_rows = 0;
static u8g2_t *s_u8g2 = NULL;

/**
 * @brief 将处理好的单行推入缓冲区，如果满了则整体平滑上移
 */
static void _push_single_line(const char *single_line) {
  uint8_t max_rows = MAX_LOG_LINES;

  if (s_current_rows < max_rows) {
    // 还有空位
    strncpy(s_lines[s_current_rows], single_line, MAX_CHAR_PER_LINE);
    s_lines[s_current_rows][MAX_CHAR_PER_LINE] = '\0';
    s_current_rows++;
  } else {
    // 屏幕满了
    memmove(&s_lines[0], &s_lines[1], sizeof(s_lines[0]) * (max_rows - 1));
    strncpy(s_lines[max_rows - 1], single_line, MAX_CHAR_PER_LINE);
    s_lines[max_rows - 1][MAX_CHAR_PER_LINE] = '\0';
  }
}

void splash_log_init(u8g2_t *u8g2, uint8_t font_height,
                     const uint8_t *font_name) {
  s_u8g2 = u8g2;
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

    _push_single_line(tmp_seg);

    if (strlen(ptr) > MAX_CHAR_PER_LINE) {
      ptr += MAX_CHAR_PER_LINE;
    } else {
      break;
    }
  }

  u8g2_ClearBuffer(s_u8g2);

  for (uint8_t i = 0; i < s_current_rows; i++) {
    uint16_t y = (i + 1) * (g_screen_cfg.font_height + LOG_LINE_SPACING);

    if (g_screen_cfg.draw_text) {
      g_screen_cfg.draw_text(s_u8g2, 0, y, s_lines[i]);
    }
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