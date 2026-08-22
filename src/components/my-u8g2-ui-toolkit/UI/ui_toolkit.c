#include "ui_toolkit.h"

void draw_scroll_text_with_pause(u8g2_t *u8g2, const Screen_t *screen_cfg,
                                 const char *text, uint8_t start_x,
                                 uint8_t max_width, uint8_t y, uint32_t tick,
                                 uint8_t clip_y1, uint8_t clip_y2) {
  if (u8g2 == NULL || screen_cfg == NULL || text == NULL || max_width == 0)
    return;

  int text_width = u8g2_GetStrWidth(u8g2, text);
  if (text_width <= max_width) {
    g_screen_cfg.draw_text(u8g2, start_x, y, text);
    return;
  }

  int gap = 20;
  int total_len = text_width + gap;
  int pause_ticks = screen_cfg->scroll_pause_ticks;
  int scroll_ticks = total_len * screen_cfg->scroll_speed_divisor;
  int total_cycle_ticks = pause_ticks + scroll_ticks;
  uint32_t cycle_tick = tick % total_cycle_ticks;

  if (cycle_tick < pause_ticks) {
    u8g2_SetClipWindow(u8g2, start_x, clip_y1, start_x + max_width, clip_y2);
    g_screen_cfg.draw_text(u8g2, start_x, y, text);
    u8g2_SetMaxClipWindow(u8g2);
    return;
  }

  uint32_t scroll_start_tick = cycle_tick - pause_ticks;
  int offset = scroll_start_tick / screen_cfg->scroll_speed_divisor;
  int draw_x = start_x - offset;

  u8g2_SetClipWindow(u8g2, start_x, clip_y1, start_x + max_width, clip_y2);
  g_screen_cfg.draw_text(u8g2, draw_x, y, text);
  g_screen_cfg.draw_text(u8g2, draw_x + total_len, y, text);
  u8g2_SetMaxClipWindow(u8g2);
}