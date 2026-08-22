#ifndef __SCREEN_H__
#define __SCREEN_H__
#include "u8g2.h"
#include "stdbool.h"


typedef void (*screen_draw_text_cb_t)(u8g2_t *u8g2, uint16_t x, uint16_t y, const char *text);

// ===================== 屏幕/字体配置结构体 =====================
typedef struct {
  // 屏幕基础配置
  uint16_t width;  // 屏幕宽度
  uint16_t height; // 屏幕高度
  
  // 字体配置
  const uint8_t *font;   // 使用的字体
  uint8_t font_height;   // 字体高度
  uint8_t font_baseline; // 字体基线偏移
	const uint8_t *sub_window_font;	// 子窗口使用的字体
	const uint8_t *icon_font;		// 图标字体名
  
  // 文本编码配置
  bool is_utf8;                  // UTF8编码标志：true=使用UTF8绘制，false=使用普通ASCII
  screen_draw_text_cb_t draw_text; // 自定义文字绘制回调
  
  // 布局基础配置
  uint8_t title_left_margin;       // 标题左侧边距
  uint8_t right_item_margin;       // 右侧元素右侧边距
  uint8_t right_item_left_padding; // 右侧元素左侧内边距
  
  // 滚动配置
  uint16_t scroll_pause_ticks;  // 滚动停顿时间（ms）
  uint8_t scroll_speed_divisor; // 滚动速度除数
  
  // 动画配置
  uint8_t animation_duration; // 高亮框动画时长（tick）
  
  // 高亮框配置
  uint8_t highlight_padding; // 高亮框内边距
  uint8_t highlight_height;  // 高亮框高度
  uint8_t hightlight_radius;
  
  // 右侧元素默认宽度
  uint8_t click_switch_width; // CLICK类型开关宽度
  uint8_t action_width;       // ACTION类型宽度
  uint8_t num_min_width;      // NUM类型最小宽度
} Screen_t;

// ===================== 内置文字绘制实现 =====================
// UTF8文字绘制函数
static inline void screen_draw_utf8(u8g2_t *u8g2, uint16_t x, uint16_t y, const char *text) {
  if (u8g2 && text) {
    u8g2_DrawUTF8(u8g2, x, y, text);
  }
}

// 普通ASCII文字绘制函数
static inline void screen_draw_str(u8g2_t *u8g2, uint16_t x, uint16_t y, const char *text) {
  if (u8g2 && text) {
    u8g2_DrawStr(u8g2, x, y, text);
  }
}

// ===================== 默认屏幕配置 =====================
#define DEFAULT_SCREEN_CONFIG                                                 \
  {                                                                           \
      .width = 240,                  /* 屏幕宽度 */                           \
      .height = 240,                 /* 屏幕高度 */                           \
      .font = u8g2_font_8x13_tr,     /* 默认文本字体 */                       \
      .sub_window_font = u8g2_font_6x10_tr,                                    \
      .icon_font = NULL,                            \
      .font_height = 13,             /* 字体高度 */                           \
      .font_baseline = 8,            /* 调整为标准的 8px 基线 */               \
      .is_utf8 = false,              /* UTF8 标志 */                          \
      .draw_text = screen_draw_str,  /* 文本绘制函数 */                       \
      .title_left_margin = 16,                                                \
      .right_item_margin = 8,                                                 \
      .right_item_left_padding = 8,                                           \
      .scroll_pause_ticks = 300,     /* 长文本滚动手感：停顿 300ms */           \
      .scroll_speed_divisor = 3,     /* 长文本滚动速度步长 */                 \
      .animation_duration = 10,     /* [关键修复] 高亮框过渡 120ms (约 6~8 帧) */\
      .highlight_padding = 8,        /* 高亮框内边距 */                       \
      .highlight_height = 12,        /* 高亮框高度 */                         \
      .hightlight_radius = 1,        /* 圆角半径：减少单片机 RBox 弧度计算 */    \
      .click_switch_width = 10,                                               \
      .action_width = 0,                                                      \
      .num_min_width = 30                                                     \
  }

// 全局屏幕配置实例
extern const Screen_t g_screen_cfg;

#endif