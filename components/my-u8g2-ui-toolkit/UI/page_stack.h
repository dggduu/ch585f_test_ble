#ifndef __PAGE_STACK_H__
#define __PAGE_STACK_H__

#include "btn_fifo.h"
#include "easing.h"
#include "u8g2.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 页面函数类型
typedef void (*page_draw_func_t)(u8g2_t *u8g2, void *ctx);
typedef void (*page_input_func_t)(int btn, void *ctx);
// 全局按键回调类型
typedef void (*global_btn_cb_t)(btn_type_t btn);
typedef void (*portal_draw_cb_t)(u8g2_t *u8g2, int16_t x, int16_t y, uint8_t w,
                                 uint8_t h, void *ctx);

// 页面组件结构体
typedef struct {
  page_draw_func_t draw;
  page_input_func_t input;
} page_component_t;

// Portal 组件结构体
typedef struct {
  portal_draw_cb_t draw;
  page_input_func_t input;
  uint8_t w;
  uint8_t h;
} portal_component_t;

// 页面节点结构体
typedef struct {
  const page_component_t *comp;
  void *ctx;
} page_t;

#define PAGE_STACK_MAX_DEPTH 8
#define PORTAL_CTX_BUFFER_SIZE 64

// 页面栈核心结构体
typedef struct {
  page_t stack[PAGE_STACK_MAX_DEPTH];
  uint8_t top;
  u8g2_t *u8g2;
  uint32_t main_tick;
  global_btn_cb_t global_btn_handler; // 全局按键回调
  // portal 相关
  const portal_component_t *active_portal;
  uint8_t portal_ctx_buffer[PORTAL_CTX_BUFFER_SIZE];
  void *portal_ctx;
  bool is_portal_running;
  bool is_exiting;
  float ani_progress;
} page_stack_t;

extern page_stack_t g_page_stack; // 全局页面栈实例

// 核心接口
void page_stack_init(page_stack_t *ps, u8g2_t *u8g2);
int page_stack_push(page_stack_t *ps, const page_component_t *comp, void *ctx);
int page_stack_pop(page_stack_t *ps);
page_t *page_stack_current(page_stack_t *ps);
void page_update(page_stack_t *ps, btn_type_t btn);
// 注册全局按键回调
void page_stack_register_global_btn_cb(page_stack_t *ps, global_btn_cb_t cb);
void page_stack_portal_toggle(page_stack_t *ps, const portal_component_t *comp,
                              void *ctx, size_t ctx_size);

#endif