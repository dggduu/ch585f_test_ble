#ifndef __PORTAL_COMPONENT_H__
#define __PORTAL_COMPONENT_H__

// ==========  头部文件 ==========
#include "page_stack.h"
#include "screen.h"
#include "stdbool.h"
#include "u8g2.h"
#include "ui_toolkit.h"

// 启用VLIST_PROGERSS相关组件
#define ENABLE_VLIST_PROGRESS 1

// ========== 为了Portal组件的通用性，创建组件时需要定义该组件的上下文结构
// ==========
// ==========
// 因为定义了portal_ctx_buffer用于存放上下文，因此可以使用匿名结构体的方式定义
// ========== 消息框所需数据
typedef struct {
  const char *title;
  const char *msg;
} portal_ctx_message_box_t;

// 数字选择器所需数据
typedef struct {
  const char *title;
  float *val_ptr; // 对于需要变化的值，传入它的指针
  float min;
  float max;
  float step;
} portal_ctx_num_t;

typedef struct {
  const char *title;
  float *val_ptr;
  float min;
  float max;
  uint8_t total_digit; // 总显示位数
  uint8_t dot_pos;     // 小数点后保留位数
  uint8_t cursor_pos;  // 光标位置
} portal_ctx_precise_t;

#if ENABLE_VLIST_PROGRESS

#include <stdarg.h>

typedef enum {
  PROG_STATUS_WAIT = 0,
  PROG_STATUS_SUCCESS,
  PROG_STATUS_FAIL
} progress_status_t;

typedef struct {
  const char *title;
  void (*task_callback)(void *ctx);
  progress_status_t status;
  bool is_running;
  char detail[32];
} portal_ctx_progress_t;

void Progress_Log(void *ctx, const char *fmt, ...);
void Progress_SetSuccess(void *ctx);
void Progress_SetFailed(void *ctx, const char *reason);

extern const portal_component_t PORTAL_PROGRESS;

#endif

// ========== 外部定义  ==========
extern const portal_component_t PORTAL_MESSAGE_BOX;
extern const portal_component_t PORTAL_NUM;
extern const portal_component_t PORTAL_PRECISE_NUM;
extern const portal_component_t PORTAL_PROGRESS;

extern u8g2_t u8g2;

#endif