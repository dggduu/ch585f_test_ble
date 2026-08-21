#ifndef __SPLASH_SCREEN_H__
#define __SPLASH_SCREEN_H__

#include "HList.h"
#include "page_stack.h"
#include "screen.h"
#include "u8g2.h"

// 自定义绘制函数类型
typedef void (*splash_draw_cb_t)(u8g2_t *u8g2, const Screen_t *screen_cfg);

/**
 * @brief SplashScreen组件实例
 */
extern const page_component_t SPLASH_SCREEN_COMP;

/**
 * @brief 初始化SplashScreen
 * @param hlist_ctx 要跳转的HList上下文
 * @param draw_cb 用户自定义的绘制函数
 */
void splash_screen_init(hlist_t *hlist_ctx, splash_draw_cb_t draw_cb);

/**
 * @brief 跳转到SplashScreen
 */
void splash_screen_jump(void);

#endif