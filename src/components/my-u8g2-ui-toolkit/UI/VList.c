#include "VList.h"
#include "u8g2.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define PREFIX_SUBMENU "-"
#define PREFIX_ACTION ">"

#if ENABLE_VLIST_PROGRESS
#define PREFIX_PROGRESS	"~"
#endif

/**
 * @brief 工具函数，获取右侧元素宽度
 * @return 宽度（u8）
 */
static uint8_t get_right_item_width(u8g2_t *u8g2, const Screen_t *screen_cfg,
                                    const vitem_t *item) {
  if (u8g2 == NULL || screen_cfg == NULL || item == NULL)
    return 0;

  switch (item->type) {
  case VITEM_CLICK:
    return screen_cfg->click_switch_width;
  case VITEM_NUM_EDIT: {
    if (item->user_data == NULL)
      return screen_cfg->num_min_width;
    char buf[16];
    sprintf(buf, "%.1f", *(float *)item->user_data);
    int num_width = u8g2_GetStrWidth(u8g2, buf);
    return (num_width > screen_cfg->num_min_width ? num_width
                                                  : screen_cfg->num_min_width);
  }
  case VITEM_PRECISE_EDIT: {
    if (item->user_data == NULL)
      return screen_cfg->num_min_width;
    char buf[16];
    sprintf(buf, "%u", *(uint16_t *)item->user_data);
    int num_width = u8g2_GetStrWidth(u8g2, buf);
    return (num_width > screen_cfg->num_min_width ? num_width
                                                  : screen_cfg->num_min_width);
  }
  case VITEM_ACTION:
    return 0;
  case VITEM_PROTECTED_ACTION:
    return 0;
  case VITEM_SUBMENU:
  case VITEM_PROTECTED_SUBMENU:
    return 0;
  case VITEM_PLAIN_TEXT:
    return 0;
  default:
    return 0;
  }
}

/**
 * @brief  核心绘制逻辑
 *
 * @param[in]  a
 * @param[in]  b
 *
 * @return
 */
void vlist_draw(u8g2_t *u8g2, void *ctx) {
  if (u8g2 == NULL || ctx == NULL) return;

  vlist_t *list = (vlist_t *)ctx;
  const Screen_t *screen_cfg = &g_screen_cfg;
  if (list->count == 0) return;

  float p = fminf((float)(*list->main_tick - list->start_tick) / screen_cfg->animation_duration, 1.0f);
  float ease_p = VLIST_ANIM_FUC(p);
  float ease_idx = list->from_index + (list->to_index - list->from_index) * ease_p;

  // 滚动偏移计算
  int scroll_y = 0;
  if (ease_idx > 3.0f) {
    scroll_y = (int)((ease_idx - 3.0f) * (screen_cfg->font_height + 3));
  }

  // 绘制右侧滚动条
  u8g2_SetDrawColor(u8g2, 1);
  int from_bar_len = (int)(64.0f * ((float)(list->from_index + 1) / list->count));
  int to_bar_len = (int)(64.0f * ((float)(list->to_index + 1) / list->count));
  int curr_bar_len = (int)(from_bar_len + (to_bar_len - from_bar_len) * ease_p);
  u8g2_DrawVLine(u8g2, screen_cfg->width - 1, 0, curr_bar_len);

  u8g2_SetFont(u8g2, screen_cfg->font);

  for (int i = 0; i < list->count; i++) {
    vitem_t *curr_item = &list->items[i];
    int item_y = (i * (screen_cfg->font_height + 3)) - scroll_y + screen_cfg->font_baseline + 2;

    if (item_y < -screen_cfg->font_height || item_y > screen_cfg->height + 10) continue;
    uint8_t right_item_w = get_right_item_width(u8g2, screen_cfg, curr_item);
    uint8_t right_item_x = screen_cfg->width - right_item_w - screen_cfg->right_item_margin;
    uint8_t base_avail_width = right_item_x - screen_cfg->title_left_margin - 5;

    bool is_highlighted = (i == list->to_index);
    uint8_t clip_y1, clip_y2;

    if (is_highlighted) {
      int box_y = (int)(ease_idx * (screen_cfg->font_height + 3)) - scroll_y + 2;
      int target_w = u8g2_GetStrWidth(u8g2, curr_item->title) + screen_cfg->highlight_padding;
      int start_w = 20;
      if (list->from_index >= 0 && list->from_index < list->count) {
          start_w = u8g2_GetStrWidth(u8g2, list->items[list->from_index].title) + screen_cfg->highlight_padding;
      }

      // 限制不超出右侧元素
      int max_w = right_item_x - screen_cfg->title_left_margin + (screen_cfg->highlight_padding / 2);
      if (target_w > max_w) target_w = max_w;
      if (start_w > max_w) start_w = max_w;

      int cur_box_w = (int)(start_w + (target_w - start_w) * ease_p);
      int box_x = screen_cfg->title_left_margin - (screen_cfg->highlight_padding / 2);

      // 绘制高亮背景
      u8g2_SetDrawColor(u8g2, 1);
      u8g2_DrawRBox(u8g2, box_x, box_y, cur_box_w, screen_cfg->highlight_height, g_screen_cfg.hightlight_radius);

      int text_clip_x = box_x + 2;
      int text_clip_w = cur_box_w - 8;
      clip_y1 = box_y + 1;
      clip_y2 = box_y + screen_cfg->highlight_height - 1;

      // 绘制前缀
      u8g2_SetDrawColor(u8g2, 0);
      const char* prefix = "";
      if (curr_item->type == VITEM_SUBMENU || curr_item->type == VITEM_PROTECTED_SUBMENU) prefix = PREFIX_SUBMENU;
      else if (curr_item->type == VITEM_ACTION || curr_item->type == VITEM_PROTECTED_ACTION) prefix = PREFIX_ACTION;
      else if (curr_item->type == VITEM_PROGRESS) prefix = PREFIX_PROGRESS;
      g_screen_cfg.draw_text(u8g2, 5, item_y, prefix);

      // 绘制标题
      draw_scroll_text_with_pause(u8g2, screen_cfg, curr_item->title, 
                                  screen_cfg->title_left_margin, text_clip_w, 
                                  item_y, *list->main_tick, clip_y1, clip_y2);
    } else {
      u8g2_SetDrawColor(u8g2, 1);
      clip_y1 = item_y - screen_cfg->font_height + 2;
      clip_y2 = item_y + 2;

      const char* prefix = "";
      if (curr_item->type == VITEM_SUBMENU || curr_item->type == VITEM_PROTECTED_SUBMENU) prefix = PREFIX_SUBMENU;
      else if (curr_item->type == VITEM_ACTION || curr_item->type == VITEM_PROTECTED_ACTION) prefix = PREFIX_ACTION;
      else if (curr_item->type == VITEM_PROGRESS) prefix = PREFIX_PROGRESS;
      g_screen_cfg.draw_text(u8g2, 5, item_y, prefix);

      draw_scroll_text_with_pause(u8g2, screen_cfg, curr_item->title, 
                                  screen_cfg->title_left_margin, base_avail_width, 
                                  item_y, *list->main_tick, clip_y1, clip_y2);
    }

    // --- 绘制右侧交互元素---
    u8g2_SetDrawColor(u8g2, 1); // 恢复颜色
    if (curr_item->type == VITEM_CLICK && curr_item->user_data != NULL) {
      bool v = *(bool *)curr_item->user_data;
      if (v) u8g2_DrawBox(u8g2, right_item_x, item_y - 8, 6, 6);
      else u8g2_DrawFrame(u8g2, right_item_x, item_y - 8, 6, 6);
    } 
    else if ((curr_item->type == VITEM_NUM_EDIT || curr_item->type == VITEM_PRECISE_EDIT) && curr_item->user_data != NULL) {
      char b[16];
      sprintf(b, "%.1f", *(float *)curr_item->user_data);
      draw_scroll_text_with_pause(u8g2, screen_cfg, b, right_item_x, right_item_w, 
                                  item_y, *list->main_tick, clip_y1, clip_y2);
    }
  }
}

/**
 * @brief  初始化/注册函数
 *
 * @param[in]  a
 * @param[in]  b
 *
 * @return
 */
void vlist_init(vlist_t *list, uint32_t *tick_ptr) {
  if (list == NULL || tick_ptr == NULL) {
    return;
  }

  memset(list, 0, sizeof(vlist_t));
  list->main_tick = tick_ptr;
  list->from_index = 0;
  list->to_index = 0;
  list->start_tick = *tick_ptr;
  list->alert.active = false;
  list->alert.title = NULL;
  list->alert.text = NULL;
}

void vlist_add_action(vlist_t *list, const char *title,
                      const page_component_t *comp, void *ctx) {
  if (list == NULL || title == NULL || comp == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    static vlist_action_data_t action_data_pool[MAX_LIST_ITEMS] = {0};
    vlist_action_data_t *action_data = &action_data_pool[list->count];
    action_data->comp = comp;
    action_data->ctx = ctx;

    list->items[list->count].title = title;
    list->items[list->count].type = VITEM_ACTION;
    list->items[list->count].user_data = action_data;
    list->items[list->count].callback = NULL;
    list->count++;
  }
}

void vlist_add_plain_text(vlist_t *list, const char *title) {
  if (list == NULL || title == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    list->items[list->count].title = title;
    list->items[list->count].type = VITEM_PLAIN_TEXT;
    list->items[list->count].user_data = NULL;
    list->items[list->count].callback = NULL;
    list->count++;
  }
}

void vlist_add_toggle(vlist_t *list, const char *title, bool *val) {
  if (list == NULL || title == NULL || val == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    list->items[list->count].title = title;
    list->items[list->count].type = VITEM_CLICK;
    list->items[list->count].user_data = val;
    list->count++;
  }
}

void vlist_add_num(vlist_t *list, const char *title, float *val, float min,
                   float max, float step) {
  if (list == NULL || title == NULL || val == NULL || step <= 0) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    vitem_t *it = &list->items[list->count++];
    it->title = title;
    it->type = VITEM_NUM_EDIT;
    it->user_data = val;
    it->min = min;
    it->max = max;
    it->step = step;
  }
}

void vlist_add_submenu(vlist_t *list, const char *title, vlist_t *child) {
  if (list == NULL || title == NULL || child == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    list->items[list->count].title = title;
    list->items[list->count].type = VITEM_SUBMENU;
    list->items[list->count].user_data = child;
    list->count++;
  }
}

void vlist_add_protected_action(vlist_t *list, const char *title,
                                const page_component_t *comp, void *ctx,
                                bool guard_flag, char *alert_title,
                                char *alert_text) {
  if (list == NULL || title == NULL || comp == NULL || alert_text == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    static vlist_protected_action_data_t protected_action_pool[MAX_LIST_ITEMS] =
        {0};
    vlist_protected_action_data_t *prot_action_data =
        &protected_action_pool[list->count];

    prot_action_data->action_data.comp = comp;
    prot_action_data->action_data.ctx = ctx;
    prot_action_data->guard_flag = guard_flag;
    prot_action_data->alert_title = alert_title;
    prot_action_data->alert_text = alert_text;

    vitem_t *it = &list->items[list->count++];
    it->title = title;
    it->type = VITEM_PROTECTED_ACTION;
    it->user_data = prot_action_data;
    it->callback = NULL;
    it->guard_flag = guard_flag;
    it->alert_title = alert_title;
    it->alert_text = alert_text;
  }
}

void vlist_add_protected_submenu(vlist_t *list, const char *title,
                                 vlist_t *child, bool guard_flag,
                                 char *alert_title, char *alert_text) {
  if (list == NULL || title == NULL || child == NULL || alert_text == NULL) {
    return;
  }

  if (list->count < MAX_LIST_ITEMS) {
    vitem_t *it = &list->items[list->count++];
    it->title = title;
    it->type = VITEM_PROTECTED_SUBMENU;
    it->user_data = child;
    it->guard_flag = guard_flag;
    it->alert_title = alert_title;
    it->alert_text = alert_text;
  }
}

void vlist_add_precise_num(vlist_t *list, const char *title, float *val,
                           float min, float max, uint8_t total_digit,
                           uint8_t dot_pos) {
  if (list == NULL || list->count >= MAX_LIST_ITEMS)
    return;

  vitem_t *it = &list->items[list->count++];
  it->title = title;
  it->type = VITEM_PRECISE_EDIT;
  it->user_data = val;
  it->min = min;
  it->max = max;

  it->total_digit = total_digit;
  it->dot_pos = dot_pos;
}

#if ENABLE_VLIST_PROGRESS

void vlist_add_protected_progress(vlist_t *list, const char *title,
                                  void (*cb)(void *)) {
  if (list->count >= MAX_LIST_ITEMS)
    return;
  vitem_t *it = &list->items[list->count++];
  it->title = title;
  it->type = VITEM_PROGRESS;
  it->callback = (void (*)(void *))cb; // 借用 callback 指针存任务函数
}

#endif

/**
 * @brief  输入处理函数
 *
 * @param[in]  a
 * @param[in]  b
 *
 * @return
 */
void vlist_input_handler(int btn, void *ctx) {
  if (ctx == NULL)
    return;

  vlist_t *list = (vlist_t *)ctx;

  // 处理上下键滚动
  bool animation_triggered = false;
  if (btn == BTN_UP && list->to_index > 0) {
    list->from_index = list->to_index;
    list->to_index--;
    animation_triggered = true;
  } else if (btn == BTN_DOWN && list->to_index < list->count - 1) {
    list->from_index = list->to_index;
    list->to_index++;
    animation_triggered = true;
  }

  if (animation_triggered) {
    list->start_tick = *list->main_tick;
    return;
  }

  // 处理返回键
  if (btn == BTN_BACK) {
    page_stack_pop(&g_page_stack);
    return;
  }

  // 处理确认键
  if (btn == BTN_ENTER) {
    vitem_t *it = &list->items[list->to_index];
    switch (it->type) {
    case VITEM_CLICK:
      if (it->user_data)
        *(bool *)it->user_data = !*(bool *)it->user_data;
      break;
    case VITEM_NUM_EDIT:
      page_stack_portal_toggle(&g_page_stack, &PORTAL_NUM,
                               &(portal_ctx_num_t){
                                   .title = "num_select",
                                   .val_ptr = (float *)it->user_data,
                                   .min = -100,
                                   .max = 100,
                                   .step = 3,
                               },
                               sizeof(portal_ctx_num_t));
      break;
    case VITEM_SUBMENU:
      if (it->user_data) {
        vlist_t *child = (vlist_t *)it->user_data;
        child->from_index = 0;
        child->to_index = 0;
        child->start_tick = *child->main_tick;
        page_stack_push(&g_page_stack, &VLIST_COMP, child);
      }
      break;
    case VITEM_PROTECTED_SUBMENU:
      if (it->guard_flag) {
        if (it->user_data) {
          vlist_t *child = (vlist_t *)it->user_data;
          child->from_index = 0;
          child->to_index = 0;
          child->start_tick = *child->main_tick;
          page_stack_push(&g_page_stack, &VLIST_COMP, child);
        }
      } else {
        page_stack_portal_toggle(&g_page_stack, &PORTAL_MESSAGE_BOX,
                                 &(portal_ctx_message_box_t){
                                     .title = "Warning", .msg = it->alert_text},
                                 sizeof(portal_ctx_message_box_t));
      }
      break;
      break;
    case VITEM_ACTION:
      if (it->user_data && ((vlist_action_data_t *)it->user_data)->comp) {
        vlist_action_data_t *action_data = (vlist_action_data_t *)it->user_data;
        page_stack_push(&g_page_stack, action_data->comp, action_data->ctx);
      }
      break;
    // 带保护的组件入口处理逻辑
    case VITEM_PROTECTED_ACTION:
      if (it->user_data) {
        vlist_protected_action_data_t *prot_action =
            (vlist_protected_action_data_t *)it->user_data;
        if (prot_action->guard_flag) {
          page_stack_push(&g_page_stack, prot_action->action_data.comp,
                          prot_action->action_data.ctx);
        } else {
          page_stack_portal_toggle(
              &g_page_stack, &PORTAL_MESSAGE_BOX,
              &(portal_ctx_message_box_t){.title = "Warning",
                                          .msg = it->alert_text},
              sizeof(portal_ctx_message_box_t));
        }
      }
      break;
    case VITEM_PLAIN_TEXT:
      break;
#if ENABLE_VLIST_PROGRESS

    case VITEM_PROGRESS:
      page_stack_portal_toggle(
          &g_page_stack, &PORTAL_PROGRESS,
          &(portal_ctx_progress_t){.title = it->title,
                                   .task_callback =
                                       (void (*)(void *))it->callback,
                                   .status = PROG_STATUS_WAIT,
                                   .is_running = false},
          sizeof(portal_ctx_progress_t));
      break;

#endif
    case VITEM_PRECISE_EDIT:
      page_stack_portal_toggle(&g_page_stack, &PORTAL_PRECISE_NUM,
                               &(portal_ctx_precise_t){
                                   .title = it->title,
                                   .val_ptr = (float *)it->user_data,
                                   .min = it->min,
                                   .max = it->max,
                                   .total_digit = it->total_digit,
                                   .dot_pos = it->dot_pos,
                                   .cursor_pos = 0,
                               },
                               sizeof(portal_ctx_precise_t));
      break;
    }
  }
}

const page_component_t VLIST_COMP = {vlist_draw, vlist_input_handler};