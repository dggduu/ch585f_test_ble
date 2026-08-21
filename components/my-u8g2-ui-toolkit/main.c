// #include "Delay.h"
// #include "HList.h"
// #include "VList.h"
// #include "brick_break.h"
// #include "btn_fifo.h"
// #include "page_stack.h"
// #include "screen.h"
// #include "splash_log.h"
// #include "splash_screen.h"
// #include "stm32f10x.h"
// #include "u8g2.h"
// #include "uart.h"
// #include "ui.h"
// #include <math.h>
// #include "portal_component.h"

// extern const uint8_t icon_list[][128];

// #define ICON_SETTINGS 0x0081 // 设置图标
// #define ICON_ABOUT 0x0114    // 关于图标
// #define ICON_LOCK 0x0057     // 叉叉图标
// #define ICON_UNLOCK 0x0078   // 对勾图标

// const Screen_t g_screen_cfg = DEFAULT_SCREEN_CONFIG;
// u8g2_t u8g2;
// hlist_t g_main_hlist;
// vlist_t g_setting_main_menu;
// vlist_t g_setting_sub_menu;
// vlist_t g_about_menu;
// brick_break_ctx_t g_brick_break_ctx;

// bool g_wifi_state = false;
// bool g_bt_state = true;
// bool g_mute_mode = false;
// float g_screen_brightness = 50.0f;
// float test_num = 0;

// // ===================== 自定义SplashScreen =====================
// static void my_splash_draw(u8g2_t *u8g2, const Screen_t *screen_cfg) {
//   static uint32_t tick_count = 0;
//   tick_count++;

//   // 电量绘制
//   u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
//   u8g2_DrawFrame(u8g2, screen_cfg->width - 20, 2, 18, 10);
//   u8g2_DrawBox(u8g2, screen_cfg->width - 18, 4, 14, 6);
//   u8g2_DrawStr(u8g2, screen_cfg->width - 40, 10, "80%");

//   // 数字时钟
//   uint8_t hour = 12, min = 30, sec = (tick_count / 10) % 60;
//   char time_str[10];
//   sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);
//   u8g2_SetFont(u8g2, u8g2_font_logisoso20_tn);
//   int time_width = u8g2_GetStrWidth(u8g2, time_str);
//   u8g2_DrawStr(u8g2, (screen_cfg->width - time_width) / 2,
//                screen_cfg->height / 2 + 10, time_str);

//   // 提示文字
//   u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
//   const char *hint = "Press any btn to enter";
//   u8g2_DrawStr(u8g2, (screen_cfg->width - u8g2_GetStrWidth(u8g2, hint)) / 2,
//                screen_cfg->height - 5, hint);
// }

// // ===================== 全局按键回调 =====================
// static void global_btn_handler(btn_type_t btn) {
//   if (btn == BTN_LONG_PRESS) {
//     splash_screen_jump();
//   }
// }

// // ===================== 自定义Page =====================
// static void osc_app_draw(u8g2_t *u8g2, void *ctx) {
//   static uint8_t wave_offset = 0;
//   wave_offset = (wave_offset + 1) % 255;

//   u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
//   u8g2_DrawStr(u8g2, 2, 12, "OSCILLOSCOPE");
//   u8g2_DrawHLine(u8g2, 0, 15, 128);

//   for (uint8_t x = 0; x < 128; x++) {
//     uint8_t y = 40 + (sin((x + wave_offset) * 0.15f) * 15);
//     u8g2_DrawPixel(u8g2, x, y);
//   }
//   u8g2_DrawStr(u8g2, 10, 62, "BACK:Exit  LONG:Home");
// }
// // 自定义Page btn 消费函数
// static void osc_app_input(int btn, void *ctx) {
//   if (btn == BTN_BACK) {
//     page_stack_pop(&g_page_stack);
//   }
// }
// // 注册自定义组件
// const page_component_t OSC_APP_COMP = {.draw = osc_app_draw,
//                                        .input = osc_app_input};

// void my_long_task(void *ctx) {
//     Progress_Log(ctx, "Initializing...");
//     Delay_ms(500);

//     Progress_Log(ctx, "Erasing...");
//     Delay_ms(800);

//     for (int i = 0; i <= 100; i += 20) {
//         Progress_Log(ctx, "Writing: %d%%", i);
//         Delay_ms(200);
//     }

//     // 成功结束
//     //Progress_SetSuccess(ctx);
// 	Progress_SetFailed(ctx,"no idea");
// }
									   
// // ===================== 菜单初始化 =====================
// static void ui_menu_init(void) {
//   // 初始化列表
//   vlist_init(&g_setting_main_menu, &g_page_stack.main_tick);
//   vlist_init(&g_setting_sub_menu, &g_page_stack.main_tick);
//   vlist_init(&g_about_menu, &g_page_stack.main_tick);
//   hlist_init(&g_main_hlist, &g_page_stack.main_tick);

//   // 初始化打砖块游戏
//   brick_break_init(&g_brick_break_ctx, &g_page_stack.main_tick, &g_screen_cfg);

//   // 设置菜单
//   vlist_add_toggle(&g_setting_sub_menu, "WIFI Link", &g_wifi_state);
//   vlist_add_num(&g_setting_sub_menu, "Brightness", &g_screen_brightness, 0, 100,
//                 5);
//   vlist_add_submenu(&g_setting_main_menu, "System Config", &g_setting_sub_menu);
//   vlist_add_precise_num(&g_setting_main_menu, "Precise Num Test", &test_num,
//                         -100, 200, 3,0);
// 	vlist_add_protected_progress(&g_setting_main_menu, "Save Config", my_long_task);
//   vlist_add_toggle(&g_setting_main_menu, "Mute Mode", &g_mute_mode);
//   vlist_add_toggle(&g_setting_main_menu, "Bluetooth", &g_bt_state);
//   vlist_add_protected_submenu(&g_setting_main_menu, "Protect.false",
//                               &g_setting_sub_menu, false, "warinng",
//                               "Try Again!");
//   vlist_add_protected_submenu(&g_setting_main_menu, "Protect.true",
//                               &g_setting_sub_menu, true, "warinng",
//                               "Try Again!");

//   // 添加OSC组件入口
//   vlist_add_action(&g_setting_main_menu, "Oscilloscope", &OSC_APP_COMP, NULL);
//   // 添加打砖块游戏入口
//   vlist_add_action(&g_setting_main_menu, "Brick Break", &BRICK_BREAK_COMP,
//                    &g_brick_break_ctx);
//   vlist_add_protected_action(&g_setting_main_menu, "Brick Break (locked)",
//                              &BRICK_BREAK_COMP, &g_brick_break_ctx, false,
//                              "warinng", "This action is locked!");
//   vlist_add_protected_action(&g_setting_main_menu, "Brick Break (unlocked)",
//                              &BRICK_BREAK_COMP, &g_brick_break_ctx, true,
//                              "warinng", "This action is locked!");

//   // 关于菜单
//   vlist_add_plain_text(&g_about_menu, "Version: 0.0.1");
//   vlist_add_plain_text(&g_about_menu, "Author: dggdoo");
//   vlist_add_plain_text(&g_about_menu, "Build: 2026-01");

//   // 主菜单
//   hlist_add_glyph_item(&g_main_hlist, "SETTINGS", ICON_SETTINGS, &VLIST_COMP,
//                        &g_setting_main_menu);
//   hlist_add_xbm_item(&g_main_hlist, "OSCILLO", icon_list[3], &OSC_APP_COMP,
//                      NULL);
//   // 添加打砖块游戏到主菜单
//   hlist_add_glyph_item(&g_main_hlist, "BRICK GAME", ICON_ABOUT,
//                        &BRICK_BREAK_COMP, &g_brick_break_ctx);
//   hlist_add_glyph_item(&g_main_hlist, "ABOUT", ICON_ABOUT, &VLIST_COMP,
//                        &g_about_menu);
//   hlist_add_protected_glyph_item(&g_main_hlist, "Protected.false", ICON_LOCK,
//                                  &VLIST_COMP, &g_about_menu, false,
//                                  "Try Again!");
//   hlist_add_protected_glyph_item(&g_main_hlist, "Protected.true", ICON_UNLOCK,
//                                  &VLIST_COMP, &g_about_menu, true,
//                                  "Try Again!");

//   // 初始化SplashScreen
//   splash_screen_init(&g_main_hlist, my_splash_draw);
//   // 注册全局按键回调
//   page_stack_register_global_btn_cb(&g_page_stack, global_btn_handler);
//   // 初始页面
//   splash_screen_jump();
//   // 直接进入Hlist，需要将上面的SplashScreen相关的删除
//   // page_stack_push(&g_page_stack, &HLIST_COMP, &g_main_hlist);
// }

// // ===================== 串口按键处理 =====================
// static void uart_btn_process(void) {
//   char ch = uart_dma_read_byte();
//   if (ch != 0) {
//     switch (ch) {
//     case 'w':
//       btn_fifo_push(BTN_UP);
//       break;
//     case 's':
//       btn_fifo_push(BTN_DOWN);
//       break;
//     case 'a':
//       btn_fifo_push(BTN_ENTER);
//       break;
//     case 'd':
//       btn_fifo_push(BTN_BACK);
//       break;
//     case 'h':
//       btn_fifo_push(BTN_LONG_PRESS);
//       break;
//     case 'l':
//       btn_fifo_push(BTN_LEFT);
//       break;
//     case 'r':
//       btn_fifo_push(BTN_RIGHT);
//       break;
//     default:
//       break;
//     }
//     uart_debug_printf("rev:%c\r\n", ch);
//   }
// }

// int main(void) {
//   // 硬件初始化
//   uart_init();
//   btn_fifo_init();
//   IIC_Init();
//   u8g2Init(&u8g2);

//   // 这一段可以直接去掉
//   splash_log_init(&u8g2, g_screen_cfg.font_height, u8g2_font_5x7_tf);
//   splash_log_clear();
//   splash_log_printf("splash_log inited");
//   Delay_ms(100);
//   splash_log_printf("btn_fifo inited");
//   Delay_ms(100);
//   splash_log_printf("PWM inited");
//   Delay_ms(100);
//   splash_log_printf("UART inited");
//   Delay_ms(100);
//   splash_log_printf("...ok");
//   Delay_ms(100);
//   splash_log_printf("        /Nya!  Powered");
//   splash_log_printf("   /|/|        By");
//   splash_log_printf("  (- - |       dggduu's");
//   splash_log_printf("   |、~\        U8g2 UI");
//   splash_log_printf("  //_,)/       Toolkit");
//   Delay_ms(3000);
//   splash_log_printf("test_float %.1f", 12.8);
//   splash_log_printf("test_interger %2d", 120);
//   splash_log_printf("test_string %s", "hello world");
//   splash_log_printf("test_ovweflow %s", "hello world sdhjsdjdshj");
//   Delay_ms(200);
//   splash_log_printf("ready to test clear");
//   Delay_ms(1000);
//   splash_log_clear();
//   // 上面这些可以去掉
//   page_stack_init(&g_page_stack, &u8g2);
//   ui_menu_init();

//   while (1) {
//     uart_btn_process();
//     btn_type_t btn = btn_fifo_pop();

//     page_update(&g_page_stack, btn);
//   }
// }