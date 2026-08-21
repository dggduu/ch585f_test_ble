#include "portal_component.h"
#include <math.h>
#include <stdio.h>

// --- MessageBox Portal 组件 ---
static void portal_messagebox_draw(u8g2_t *u8g2, int16_t x, int16_t y,
                                   uint8_t w, uint8_t h, void *ctx) {
  if (!ctx)
    return;
  portal_ctx_message_box_t *data = (portal_ctx_message_box_t *)ctx;

  // 绘制背景和外框
  u8g2_SetDrawColor(u8g2, 0);
  u8g2_DrawBox(u8g2, x, y, w, h);
  u8g2_SetDrawColor(u8g2, 1);
  u8g2_DrawFrame(u8g2, x, y, w, h);

  u8g2_SetFont(u8g2, g_screen_cfg.sub_window_font);

  // 标题居中绘制
  if (data->title) {
    int title_w = u8g2_GetStrWidth(u8g2, data->title);
    g_screen_cfg.draw_text(u8g2, x + (w - title_w) / 2, y + 12, data->title);
  }

  u8g2_DrawHLine(u8g2, x + 5, y + 15, w - 10);

  // 内容绘制
  if (data->msg) {
    g_screen_cfg.draw_text(u8g2, x + 5, y + 27, data->msg);
  }
}

static void portal_message_box_input(int btn, void *ctx) {
  if (btn == BTN_ENTER || btn == BTN_BACK) {
    page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
  }
}

const portal_component_t PORTAL_MESSAGE_BOX = {.draw = portal_messagebox_draw,
                                               .input =
                                                   portal_message_box_input,
                                               .w = 100,
                                               .h = 35};

// --- NumSelector Portal 组件 ---
static void portal_num_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                            uint8_t h, void *ctx) {
  if (!ctx)
    return;
  portal_ctx_num_t *data = (portal_ctx_num_t *)ctx;
  if (!data->val_ptr)
    return;

  float val = *(data->val_ptr);
  char buf[32];
  const Screen_t *sc = &g_screen_cfg;
  uint32_t current_tick = g_page_stack.main_tick;

  u8g2_SetDrawColor(u8g2, 0);
  u8g2_DrawBox(u8g2, x, y, w, h);
  u8g2_SetDrawColor(u8g2, 1);
  u8g2_DrawFrame(u8g2, x, y, w, h);

  u8g2_SetFont(u8g2, sc->font);
  draw_scroll_text_with_pause(u8g2, sc, data->title, x + 5, w - 10, y + 12,
                              current_tick, y + 1, y + 14);

  u8g2_SetFont(u8g2, sc->font);
  sprintf(buf, "%.1f", val);
  int val_w = u8g2_GetStrWidth(u8g2, buf);
  u8g2_DrawStr(u8g2, x + (w - val_w) / 2, y + 26, buf);

  int bx = x + 10, by = y + 30, bw = w - 20, bh = 6;
  u8g2_DrawFrame(u8g2, bx, by, bw, bh);
  float ratio = 0.0f;
  if (data->max > data->min) {
    ratio = (val - data->min) / (data->max - data->min);
    ratio = fmaxf(0.0f, fminf(1.0f, ratio));
  }
  if (ratio > 0) {
    u8g2_DrawBox(u8g2, bx + 2, by + 2, (int)((bw - 4) * ratio), bh - 4);
  }

  u8g2_SetFont(u8g2, sc->sub_window_font);
  sprintf(buf, "[%.1f,%.1f,%.1f]", data->min, data->step, data->max);
  int range_w = u8g2_GetStrWidth(u8g2, buf);
  u8g2_DrawStr(u8g2, x + (w - range_w) / 2, y + 45, buf);
}

static void portal_num_input(int btn, void *ctx) {
  if (!ctx)
    return;
  portal_ctx_num_t *data = (portal_ctx_num_t *)ctx;
  if (!data->val_ptr)
    return;

  if (btn == BTN_UP) {
    *(data->val_ptr) = fminf(*(data->val_ptr) + data->step, data->max);
  } else if (btn == BTN_DOWN) {
    *(data->val_ptr) = fmaxf(*(data->val_ptr) - data->step, data->min);
  } else if (btn == BTN_ENTER || btn == BTN_BACK) {
    page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
  }
}

const portal_component_t PORTAL_NUM = {
    .draw = portal_num_draw, .input = portal_num_input, .w = 100, .h = 48};

// 按位调整整形数值Portal
void portal_precise_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                         uint8_t h, void *ctx) {
  if (!ctx)
    return;
  portal_ctx_precise_t *data = (portal_ctx_precise_t *)ctx;

  u8g2_SetDrawColor(u8g2, 0);
  u8g2_DrawBox(u8g2, x, y, w, h);
  u8g2_SetDrawColor(u8g2, 1);
  u8g2_DrawFrame(u8g2, x, y, w, h);
  u8g2_SetFont(u8g2, g_screen_cfg.font);
  u8g2_DrawStr(u8g2, x + 4, y + 10, data->title);
  u8g2_DrawHLine(u8g2, x, y + 12, w);

  char buf[16];
  char fmt[10];
  if (data->dot_pos > 0)
    sprintf(fmt, "%%0%d.%df", data->total_digit + 1, data->dot_pos);
  else
    sprintf(fmt, "%%0%d.0f", data->total_digit);
  sprintf(buf, fmt, *data->val_ptr);

  u8g2_SetFont(u8g2, g_screen_cfg.sub_window_font);
  int str_w = u8g2_GetStrWidth(u8g2, buf);
  int num_x = x + (w - str_w) / 2;
  int num_y = y + (h / 2) + 6;
  u8g2_DrawStr(u8g2, num_x, num_y, buf);

  int char_w = str_w / strlen(buf);
  int offset = data->cursor_pos;
  if (data->dot_pos > 0 && data->cursor_pos >= data->dot_pos)
    offset += 1;
  int cursor_line_x = num_x + str_w - (offset + 1) * char_w;
  u8g2_DrawHLine(u8g2, cursor_line_x, num_y + 2, char_w);

  char range_buf[32];
  sprintf(range_buf, "[%.0f~%.0f]", data->min, data->max);
  u8g2_SetFont(u8g2, g_screen_cfg.sub_window_font);
  u8g2_DrawStr(u8g2, x + (w - u8g2_GetStrWidth(u8g2, range_buf)) / 2, y + h - 2,
               range_buf);
}

static void portal_precise_input(int btn, void *ctx) {
  if (!ctx)
    return;
  portal_ctx_precise_t *data = (portal_ctx_precise_t *)ctx;

  float factor = powf(10.0f, data->dot_pos);
  int32_t val_int = (int32_t)roundf((*data->val_ptr) * factor);

  int32_t step = 1;
  for (uint8_t i = 0; i < data->cursor_pos; i++)
    step *= 10;

  switch (btn) {
  case BTN_LEFT:
    if (data->cursor_pos < data->total_digit - 1)
      data->cursor_pos++;
    break;
  case BTN_RIGHT:
    if (data->cursor_pos > 0)
      data->cursor_pos--;
    break;
  case BTN_UP:
    val_int += step;
    break;
  case BTN_DOWN:
    val_int -= step;
    break;
  case BTN_ENTER:
  case BTN_BACK:
    page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
    return;
  }

  float new_val = (float)val_int / factor;
  if (new_val > data->max)
    new_val = data->max;
  if (new_val < data->min)
    new_val = data->min;
  *data->val_ptr = new_val;
}

const portal_component_t PORTAL_PRECISE_NUM = {.draw = portal_precise_draw,
                                               .input = portal_precise_input,
                                               .w = 110,
                                               .h = 45};

#if ENABLE_VLIST_PROGRESS
void portal_progress_force_refresh(void *ctx) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  int16_t w = PORTAL_PROGRESS.w;
  int16_t h = PORTAL_PROGRESS.h;
  int16_t x = (g_screen_cfg.width - w) / 2;
  int16_t y = (g_screen_cfg.height - h) / 2;

  u8g2_SetClipWindow(&u8g2, x, y, x + w, y + h);

  PORTAL_PROGRESS.draw(&u8g2, x, y, w, h, p);
  u8g2_SendBuffer(&u8g2);

  u8g2_SetMaxClipWindow(&u8g2);
}

void Progress_Log(void *ctx, const char *fmt, ...) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  va_list args;
  va_start(args, fmt);
  vsnprintf(p->detail, sizeof(p->detail), fmt, args);
  va_end(args);

  if (p->status == PROG_STATUS_WAIT)
    p->status = PROG_STATUS_WAIT;
  portal_progress_force_refresh(ctx);
}

void Progress_SetSuccess(void *ctx) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  p->status = PROG_STATUS_SUCCESS;
  snprintf(p->detail, sizeof(p->detail), "DONE");
  portal_progress_force_refresh(ctx);
}

void Progress_SetFailed(void *ctx, const char *reason) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  p->status = PROG_STATUS_FAIL;
  snprintf(p->detail, sizeof(p->detail), reason);
  portal_progress_force_refresh(ctx);
}

static void portal_progress_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                                 uint8_t h, void *ctx) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  if (!p)
    return;

  u8g2_SetDrawColor(u8g2, 0);
  u8g2_DrawBox(u8g2, x, y, w, h);
  u8g2_SetDrawColor(u8g2, 1);
  u8g2_DrawFrame(u8g2, x, y, w, h);

  u8g2_SetFont(u8g2, g_screen_cfg.font);
  u8g2_DrawStr(u8g2, x + 5, y + 12, p->title);
  u8g2_DrawHLine(u8g2, x, y + 15, w);

  if (p->status == PROG_STATUS_WAIT && !p->is_running) {
    u8g2_SetFont(u8g2, g_screen_cfg.sub_window_font);
    const char *tip = "Press[ENTER]to Start";
    int tw = u8g2_GetStrWidth(u8g2, tip);
    u8g2_DrawStr(u8g2, x + (w - tw) / 2, y + 34, tip);
  } else {
    u8g2_SetFont(u8g2, g_screen_cfg.font);
    int dw = u8g2_GetStrWidth(u8g2, p->detail);
    u8g2_DrawStr(u8g2, x + (w - dw) / 2, y + 34, p->detail);
  }

  if (p->status == PROG_STATUS_FAIL) {
    u8g2_SetFont(u8g2, g_screen_cfg.font);
    u8g2_DrawStr(u8g2, x + (w - u8g2_GetStrWidth(u8g2, "FAILED")) / 2, y + 45,
                 "FAILED");
  }
}

static void portal_progress_input(int btn, void *ctx) {
  portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
  if (!p)
    return;

  if (p->status != PROG_STATUS_WAIT && !p->is_running) {
    if (btn == BTN_ENTER || btn == BTN_BACK) {
      page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
    }
    return;
  }

  if (btn == BTN_ENTER && p->status == PROG_STATUS_WAIT) {
    p->is_running = true;
    Progress_Log(p, "Starting...");

    if (p->task_callback) {
      p->task_callback(p);
    }

    p->is_running = false;
  }
}

const portal_component_t PORTAL_PROGRESS = {.draw = portal_progress_draw,
                                            .input = portal_progress_input,
                                            .w = 110,
                                            .h = 48};
#endif