#include "portal_component.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ==================== 纯整数辅助函数（避免浮点库） ==================== */

// 纯整数 10 的 N 次幂 (10^n)
static int32_t ipow10(uint8_t exp) {
    int32_t result = 1;
    while (exp--) {
        result *= 10;
    }
    return result;
}

// 轻量级 float 转字符串（保留 1 位小数，如 12.3f -> "12.3"）
static void fast_ftoa_1dec(float val, char *buf) {
    int32_t total = (int32_t)(val * 10.0f + (val >= 0 ? 0.5f : -0.5f));
    if (total < 0) {
        *buf++ = '-';
        total = -total;
    }
    int32_t integer_part = total / 10;
    int32_t decimal_part = total % 10;
    sprintf(buf, "%ld.%ld", (long)integer_part, (long)decimal_part);
}

// 格式化固定位数的数值字符串 (取代动态 sprintf 浮点格式化)
static void format_precise_val(float val, uint8_t total_digit, uint8_t dot_pos, char *buf) {
    int32_t factor = ipow10(dot_pos);
    int32_t val_int = (int32_t)(val * factor + (val >= 0 ? 0.5f : -0.5f));
    
    if (dot_pos == 0) {
        sprintf(buf, "%0*ld", total_digit, (long)val_int);
    } else {
        int32_t integer_part = val_int / factor;
        int32_t decimal_part = val_int % factor;
        if (decimal_part < 0) decimal_part = -decimal_part;
        sprintf(buf, "%0*ld.%0*ld", total_digit - dot_pos, (long)integer_part, dot_pos, (long)decimal_part);
    }
}

/* ==================== MessageBox Portal 组件 ==================== */
static void portal_messagebox_draw(u8g2_t *u8g2, int16_t x, int16_t y,
                                   uint8_t w, uint8_t h, void *ctx) {
    if (!ctx) return;
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

const portal_component_t PORTAL_MESSAGE_BOX = {
    .draw = portal_messagebox_draw,
    .input = portal_message_box_input,
    .w = 100,
    .h = 35
};

/* ==================== NumSelector Portal 组件 ==================== */
static void portal_num_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                            uint8_t h, void *ctx) {
    if (!ctx) return;
    portal_ctx_num_t *data = (portal_ctx_num_t *)ctx;
    if (!data->val_ptr) return;

    float val = *(data->val_ptr);
    char buf[32];
    const Screen_t *sc = &g_screen_cfg;
    uint32_t current_tick = g_page_stack.main_tick;

    // 背景和边框
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, x, y, w, h);
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawFrame(u8g2, x, y, w, h);

    // 标题（带滚动）
    u8g2_SetFont(u8g2, sc->font);
    draw_scroll_text_with_pause(u8g2, sc, data->title, x + 5, w - 10, y + 12,
                                current_tick, y + 1, y + 14);

    // 当前值（无浮点 sprintf）
    fast_ftoa_1dec(val, buf);
    int val_w = u8g2_GetStrWidth(u8g2, buf);
    u8g2_DrawStr(u8g2, x + (w - val_w) / 2, y + 26, buf);

    // 进度条（转纯整数计算）
    int bx = x + 10, by = y + 30, bw = w - 20, bh = 6;
    u8g2_DrawFrame(u8g2, bx, by, bw, bh);
    
    if (data->max > data->min) {
        int32_t val_i = (int32_t)(val * 10.0f);
        int32_t min_i = (int32_t)(data->min * 10.0f);
        int32_t max_i = (int32_t)(data->max * 10.0f);
        
        if (val_i > max_i) val_i = max_i;
        if (val_i < min_i) val_i = min_i;

        int fill_w = ((val_i - min_i) * (bw - 4)) / (max_i - min_i);
        if (fill_w > 0) {
            u8g2_DrawBox(u8g2, bx + 2, by + 2, fill_w, bh - 4);
        }
    }

    // 范围信息
    u8g2_SetFont(u8g2, sc->sub_window_font);
    char s_min[10], s_step[10], s_max[10];
    fast_ftoa_1dec(data->min, s_min);
    fast_ftoa_1dec(data->step, s_step);
    fast_ftoa_1dec(data->max, s_max);
    sprintf(buf, "[%s,%s,%s]", s_min, s_step, s_max);

    int range_w = u8g2_GetStrWidth(u8g2, buf);
    u8g2_DrawStr(u8g2, x + (w - range_w) / 2, y + 45, buf);
}

static void portal_num_input(int btn, void *ctx) {
    if (!ctx) return;
    portal_ctx_num_t *data = (portal_ctx_num_t *)ctx;
    if (!data->val_ptr) return;

    if (btn == BTN_UP) {
        float next_val = *(data->val_ptr) + data->step;
        *(data->val_ptr) = (next_val > data->max) ? data->max : next_val;
    } else if (btn == BTN_DOWN) {
        float next_val = *(data->val_ptr) - data->step;
        *(data->val_ptr) = (next_val < data->min) ? data->min : next_val;
    } else if (btn == BTN_ENTER || btn == BTN_BACK) {
        page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
    }
}

const portal_component_t PORTAL_NUM = {
    .draw = portal_num_draw,
    .input = portal_num_input,
    .w = 100,
    .h = 48
};

/* ==================== 精确数值 Portal 组件 ==================== */
void portal_precise_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                         uint8_t h, void *ctx) {
    if (!ctx) return;
    portal_ctx_precise_t *data = (portal_ctx_precise_t *)ctx;

    // 背景和边框
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, x, y, w, h);
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawFrame(u8g2, x, y, w, h);

    // 标题
    u8g2_SetFont(u8g2, g_screen_cfg.font);
    u8g2_DrawStr(u8g2, x + 4, y + 10, data->title);
    u8g2_DrawHLine(u8g2, x, y + 12, w);

    // 格式化数值
    char buf[16];
    format_precise_val(*data->val_ptr, data->total_digit, data->dot_pos, buf);

    // 绘制数值
    u8g2_SetFont(u8g2, g_screen_cfg.sub_window_font);
    int str_w = u8g2_GetStrWidth(u8g2, buf);
    int num_x = x + (w - str_w) / 2;
    int num_y = y + (h / 2) + 6;
    u8g2_DrawStr(u8g2, num_x, num_y, buf);

    // 光标指示线 (防止 char_w 除零)
    size_t len = strlen(buf);
    if (len > 0) {
        int char_w = str_w / (int)len;
        int offset = data->cursor_pos;
        if (data->dot_pos > 0 && data->cursor_pos >= data->dot_pos) {
            offset += 1;
        }
        int cursor_line_x = num_x + str_w - (offset + 1) * char_w;
        u8g2_DrawHLine(u8g2, cursor_line_x, num_y + 2, char_w);
    }

    // 范围显示
    char range_buf[32];
    sprintf(range_buf, "[%ld~%ld]", (long)data->min, (long)data->max);
    u8g2_DrawStr(u8g2, x + (w - u8g2_GetStrWidth(u8g2, range_buf)) / 2,
                 y + h - 2, range_buf);
}

static void portal_precise_input(int btn, void *ctx) {
    if (!ctx) return;
    portal_ctx_precise_t *data = (portal_ctx_precise_t *)ctx;

    int32_t factor = ipow10(data->dot_pos);
    int32_t val_int = (int32_t)((*data->val_ptr) * factor + ((*data->val_ptr) >= 0 ? 0.5f : -0.5f));
    int32_t step = ipow10(data->cursor_pos);

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
    if (new_val > data->max) new_val = data->max;
    if (new_val < data->min) new_val = data->min;
    *data->val_ptr = new_val;
}

const portal_component_t PORTAL_PRECISE_NUM = {
    .draw = portal_precise_draw,
    .input = portal_precise_input,
    .w = 110,
    .h = 45
};

/* ==================== Progress Portal 组件 ==================== */
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

    p->status = PROG_STATUS_WAIT;
    /* 不再立即 force_refresh：分步模式下本函数由 portal_progress_draw 驱动，
     * 同一帧随后会由 page_update 的脏帧检测统一送显，避免重复的全屏 SPI 刷新 */
}

void Progress_SetSuccess(void *ctx) {
    portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
    p->status = PROG_STATUS_SUCCESS;
    snprintf(p->detail, sizeof(p->detail), "DONE");
}

void Progress_SetFailed(void *ctx, const char *reason) {
    portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
    p->status = PROG_STATUS_FAIL;
    snprintf(p->detail, sizeof(p->detail), "%s", reason ? reason : "ERROR");
}

static void portal_progress_draw(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                                 uint8_t h, void *ctx) {
    portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
    if (!p) return;

    /* 分步驱动：任务运行期间，每帧调用一次回调执行一小步（回调必须快速返回）。
     * 这样长任务被切分到各个 UI 帧之间，TMOS_SystemProcess 不会被连续阻塞数秒，
     * BLE 协议栈得以正常调度。任务结束时自行调用 Progress_SetSuccess/Failed。 */
    if (p->is_running && p->task_callback) {
        p->task_callback(p);
        if (p->status == PROG_STATUS_SUCCESS || p->status == PROG_STATUS_FAIL) {
            p->is_running = false;
        }
    }

    // 背景和边框
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, x, y, w, h);
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawFrame(u8g2, x, y, w, h);

    // 标题
    u8g2_SetFont(u8g2, g_screen_cfg.font);
    u8g2_DrawStr(u8g2, x + 5, y + 12, p->title);
    u8g2_DrawHLine(u8g2, x, y + 15, w);

    // 状态相关显示
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
        u8g2_DrawStr(u8g2, x + (w - u8g2_GetStrWidth(u8g2, "FAILED")) / 2,
                     y + 45, "FAILED");
    }
}

static void portal_progress_input(int btn, void *ctx) {
    portal_ctx_progress_t *p = (portal_ctx_progress_t *)ctx;
    if (!p) return;

    if (p->status == PROG_STATUS_SUCCESS || p->status == PROG_STATUS_FAIL) {
        if (btn == BTN_ENTER || btn == BTN_BACK) {
            page_stack_portal_toggle(&g_page_stack, NULL, NULL, 0);
        }
        return;
    }

    if (btn == BTN_ENTER && p->status == PROG_STATUS_WAIT) {
        /* 只启动分步任务，不在按键回调里同步执行（否则会阻塞 TMOS 整段任务）；
         * 真正的执行由 portal_progress_draw 每帧驱动一小步。 */
        p->is_running = true;
        p->task_step = 0;
        Progress_Log(p, "Starting...");
    }
}

const portal_component_t PORTAL_PROGRESS = {
    .draw = portal_progress_draw,
    .input = portal_progress_input,
    .w = 110,
    .h = 48
};

#endif /* ENABLE_VLIST_PROGRESS */