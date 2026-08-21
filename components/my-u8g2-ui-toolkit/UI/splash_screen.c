#include "splash_screen.h"
#include "btn_fifo.h"
#include <string.h>

// ===================== 静态全局变量 =====================
static hlist_t *s_hlist_ctx = NULL;       // 跳转目标HList上下文
static splash_draw_cb_t s_draw_cb = NULL; // 用户注册的绘制函数

// ===================== 组件核心逻辑 =====================
/**
 * @brief SplashScreen绘制函数
 */
static void splash_screen_draw(u8g2_t *u8g2, void *ctx) {
  (void)ctx;
  if (s_draw_cb != NULL) {
    s_draw_cb(u8g2, &g_screen_cfg);
  }
}

/**
 * @brief SplashScreen输入处理函数
 */
static void splash_screen_input(int btn, void *ctx) {
  (void)ctx;
  if (btn != BTN_NONE && s_hlist_ctx != NULL) {
    page_stack_push(&g_page_stack, &HLIST_COMP, s_hlist_ctx);
  }
}

/**
 * @brief 初始化SplashScreen
 */
void splash_screen_init(hlist_t *hlist_ctx, splash_draw_cb_t draw_cb) {
  s_hlist_ctx = hlist_ctx;
  s_draw_cb = draw_cb;
}

/**
 * @brief 跳转到SplashScreen
 */
void splash_screen_jump(void) {
  g_page_stack.top = 0;
  page_stack_push(&g_page_stack, &SPLASH_SCREEN_COMP, NULL);
}

const page_component_t SPLASH_SCREEN_COMP = {
    .draw = splash_screen_draw,
    .input = splash_screen_input
};