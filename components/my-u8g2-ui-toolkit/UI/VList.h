#ifndef __VLIST_H__
#define __VLIST_H__

#include "btn_fifo.h"
#include "easing.h"
#include "page_stack.h"
#include "portal_component.h"
#include "screen.h"
#include "ui_toolkit.h"

// ========== 配置项  ==========
#define MAX_LIST_ITEMS 16
#define VLIST_ITEM_H 14
#define VLIST_ANIM_FUC QuadraticEaseOut

extern const Screen_t g_screen_cfg;

typedef enum {
  VITEM_CLICK = 0,         // 按钮
  VITEM_NUM_EDIT,          // 数字
  VITEM_SUBMENU,           // 子菜单
  VITEM_ACTION,            // 组件入口
  VITEM_PLAIN_TEXT,        // 纯文本
  VITEM_PROTECTED_SUBMENU, // 带保护的子菜单
  VITEM_PROTECTED_ACTION,  // 带保护的组件入口
  VITEM_PRECISE_EDIT,      // 按位调整整形
  VITEM_PROGRESS
} vitem_type_t;

typedef struct {
  const page_component_t *comp; // 目标组件
  void *ctx;                    // 组件上下文
} vlist_action_data_t;

typedef struct {
  vlist_action_data_t action_data; // 组件跳转数据
  bool guard_flag;                 // 保护标志
  char *alert_title;               // 保护提示标题
  char *alert_text;                // 保护提示文本
} vlist_protected_action_data_t;

typedef struct {
  const char *title;
  vitem_type_t type;
  void *user_data; // 不同类型对应不同数据：
                   // - VITEM_CLICK: bool*
                   // - VITEM_NUM_EDIT: float*
                   // - VITEM_SUBMENU/VITEM_PROTECTED_SUBMENU: vlist_t*
                   // - VITEM_ACTION: vlist_action_data_t*
                   // - VITEM_PROTECTED_ACTION: vlist_protected_action_data_t*
                   // - VITEM_PLAIN_TEXT: NULL
  float min, max, step;
  void (*callback)(void *ctx);
  // 保护子菜单扩展字段
  bool guard_flag;     // 保护标志
  char *alert_title;   // 保护提示标题
  char *alert_text;    // 保护提示文本
  uint8_t total_digit; // 总显示位数
  uint8_t dot_pos;     // 小数点位置
} vitem_t;

typedef struct {
  vitem_t items[MAX_LIST_ITEMS];
  uint8_t count;
  int from_index;
  int to_index;
  uint32_t start_tick;
  uint32_t *main_tick;

  // 数字编辑器状态
  struct {
    bool active;
    int target_idx;
  } editor;

  // 提示弹窗状态
  struct {
    bool active; // 弹窗是否激活
    char *title; // 提示标题
    char *text;  // 提示文本
  } alert;
} vlist_t;

#if ENABLE_VLIST_PROGRESS

#define PROGRESS_LOG(p, _fmt, ...)                                             \
  do {                                                                         \
    (p)->status = PROG_STATUS_RUNNING;                                         \
    snprintf((p)->detail, sizeof((p)->detail), _fmt, ##__VA_ARGS__);           \
    portal_progress_force_refresh(p);                                          \
  } while (0)

// 2. 标记任务成功完成
#define PROGRESS_SET_SUCCESS(p)                                                \
  do {                                                                         \
    (p)->status = PROG_STATUS_SUCCESS;                                         \
    snprintf((p)->detail, sizeof((p)->detail), "SUCCESS");                     \
    portal_progress_force_refresh(p);                                          \
  } while (0)

// 3. 标记任务失败并显示原因
#define PROGRESS_SET_FAILED(p, _reason)                                        \
  do {                                                                         \
    (p)->status = PROG_STATUS_FAIL;                                            \
    snprintf((p)->detail, sizeof((p)->detail), _reason);                       \
    portal_progress_force_refresh(p);                                          \
  } while (0)

typedef struct {
  const char *title;
  void (*task_cb)(void *flag_ptr);
} vlist_progress_data_t;

void vlist_add_protected_progress(vlist_t *list, const char *title,
                                  void (*cb)(void *));

#endif

extern const page_component_t VLIST_COMP;

// 注册函数
void vlist_init(vlist_t *list, uint32_t *tick_ptr);
void vlist_add_toggle(vlist_t *list, const char *title, bool *val);
void vlist_add_num(vlist_t *list, const char *title, float *val, float min,
                   float max, float step);
void vlist_add_submenu(vlist_t *list, const char *title, vlist_t *child);
void vlist_add_action(vlist_t *list, const char *title,
                      const page_component_t *comp, void *ctx);
void vlist_add_plain_text(vlist_t *list, const char *title);
void vlist_add_protected_submenu(vlist_t *list, const char *title,
                                 vlist_t *child, bool guard_flag,
                                 char *alert_title, char *alert_text);
void vlist_add_protected_action(vlist_t *list, const char *title,
                                const page_component_t *comp, void *ctx,
                                bool guard_flag, char *alert_title,
                                char *alert_text);
void vlist_add_precise_num(vlist_t *list, const char *title, float *val,
                           float min, float max, uint8_t total_digit,
                           uint8_t dot_pos);

#endif