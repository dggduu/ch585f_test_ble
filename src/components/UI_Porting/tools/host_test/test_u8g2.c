// /*
//  * test_u8g2.c — 主机端单元测试（不依赖 MCU 外设）
//  *
//  * 编译（在 tools/host_test/ 下）：
//  *   gcc -I ../.. -DU8G2_PORTING_SCREEN_W=128 -DU8G2_PORTING_SCREEN_H=64 \
//  *       test_u8g2.c ../../u8g2.c ../../u8g2_fonts.c -o test_u8g2
//  *
//  * 用 stub 替换 bsp 实现，验证：
//  *   1. 字形解码（ASCII art 目检 'A'、'i' 形状）
//  *   2. GetStrWidth / DrawStr 步进累加
//  *   3. 裁剪窗口
//  *   4. 图元（Box/Frame/Line/Circle/Disc/Triangle/XBM）
//  *   5. 3bpp 帧缓冲打包/解包往返（LSB-first）
//  *   6. u8g2_* 兼容层默认白字；ui_* 显式带色（含跨字节边界像素）
//  *   7. LCD_SendBuffer 整屏单次刷新（对比旧逐行路径只调一次）
//  *   8. LCD_WritePixels 备用逐行路径仍可用
//  */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>

// #include "u8g2.h"

// /* ==================== stub：LCD_SendBuffer ==================== */
// static uint16_t g_lcd_pix[128][64]; /* 模拟屏幕（RGB565） */
// static uint16_t g_lcd_w, g_lcd_h;
// static int g_flush_count = 0;   /* LCD_SendBuffer / LCD_WritePixels 调用次数 */
// static int g_flush_error = 0;

// /* 与 bsp_lcd_hw.c 的 LCD_SendBuffer 相同逻辑：LSB-first 3bpp 索引解包 + 调色板 */
// void LCD_SendBuffer(const uint8_t *index_buf, const uint16_t *palette,
//                     uint16_t width, uint16_t height, uint8_t bpp) {
//   uint32_t total = (uint32_t)width * height;
//   uint8_t mask = (uint8_t)((1u << bpp) - 1);
//   uint32_t p, byte;
//   uint8_t sh;

//   if (width != g_lcd_w || height != g_lcd_h) {
//     fprintf(stderr, "FAIL: SendBuffer 尺寸 %ux%u != 屏幕 %ux%u\n", width,
//             height, g_lcd_w, g_lcd_h);
//     g_flush_error = 1;
//     return;
//   }
//   for (p = 0; p < total; p++) {
//     uint8_t idx;
//     byte = p * bpp;
//     sh = (uint8_t)((byte) & 7);
//     byte >>= 3;
//     if (sh + bpp <= 8) {
//       idx = (uint8_t)(index_buf[byte] >> sh) & mask;
//     } else {
//       idx = (uint8_t)((index_buf[byte] >> sh) |
//                       (index_buf[byte + 1] << (8 - sh)));
//       idx &= mask;
//     }
//     g_lcd_pix[p % width][p / width] = palette[idx];
//   }
//   g_flush_count++;
// }

// /* ==================== stub：LCD_WritePixels（备用路径） ==================== */
// void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
//                      const uint16_t *pixels, uint32_t count) {
//   uint32_t i = 0;
//   uint16_t y, x;

//   if (x1 > x2 || y1 > y2) {
//     fprintf(stderr, "FAIL: 非法窗口 (%u,%u)-(%u,%u)\n", x1, y1, x2, y2);
//     g_flush_error = 1;
//     return;
//   }
//   if (count != (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1)) {
//     fprintf(stderr, "FAIL: count=%u 窗口=%u*%u\n", count, x2 - x1 + 1,
//             y2 - y1 + 1);
//     g_flush_error = 1;
//     return;
//   }
//   for (y = y1; y <= y2; y++) {
//     for (x = x1; x <= x2; x++) {
//       if (y >= g_lcd_h || x >= g_lcd_w) {
//         fprintf(stderr, "FAIL: 越界 (%u,%u)\n", x, y);
//         g_flush_error = 1;
//         return;
//       }
//       g_lcd_pix[x][y] = pixels[i++];
//     }
//   }
//   g_flush_count++;
// }

// /* ==================== 工具函数 ==================== */
// static int fail_count = 0;

// #define CHECK(cond, msg)                                                       \
//   do {                                                                         \
//     if (cond) {                                                                \
//       printf("  [PASS] %s\n", msg);                                            \
//     } else {                                                                   \
//       printf("  [FAIL] %s\n", msg);                                            \
//       fail_count++;                                                            \
//     }                                                                          \
//   } while (0)

// /* 从帧缓冲读回像素索引（LSB-first，与实现同语义；用作端到端验证） */
// static uint8_t pix_read(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
//   uint32_t bit = ((uint32_t)y * u->width + x) * U8G2_PORTING_BPP;
//   uint32_t byte = bit >> 3;
//   uint8_t sh = (uint8_t)(bit & 7);
//   uint16_t val = u->pix_buf[byte] >> sh;
//   if (sh + U8G2_PORTING_BPP > 8) {
//     val |= (uint16_t)u->pix_buf[byte + 1] << (8 - sh);
//   }
//   return (uint8_t)(val & (U8G2_NUM_COLORS - 1));
// }

// /* 把屏幕上 y 行 x0..x1 的已画像素 dump 为 ASCII art（颜色 1 = 白色） */
// static void dump_glyph(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
//                        u8g2_uint_t w, u8g2_uint_t h) {
//   u8g2_uint_t x, y;
//   for (y = y0; y < y0 + h; y++) {
//     for (x = x0; x < x0 + w; x++) {
//       putchar(pix_read(u, x, y) ? '#' : '.');
//     }
//     putchar('\n');
//   }
// }

// /* ==================== 测试用例 ==================== */
// static void test_buffer_roundtrip(u8g2_t *u) {
//   printf("[test] 3bpp 帧缓冲打包/解包往返\n");
//   u8g2_ClearBuffer(u);
//   /* 画一个 7x7 棋盘再读回 */
//   u8g2_uint_t x, y;
//   u8g2_SetDrawColor(u, 1);
//   for (y = 0; y < 7; y++)
//     for (x = 0; x < 7; x++)
//       if ((x + y) & 1) u8g2_DrawPixel(u, x, y);

//   int ok = 1;
//   for (y = 0; y < 7; y++) {
//     for (x = 0; x < 7; x++) {
//       uint8_t expect = ((x + y) & 1) ? 1 : 0;
//       if (pix_read(u, x, y) != expect) {
//         ok = 0;
//         fprintf(stderr, "  (pixel %u,%u = %u, expect %u)\n", x, y,
//                 pix_read(u, x, y), expect);
//       }
//     }
//   }
//   CHECK(ok, "棋盘图案读写一致");

//   /* 跨字节边界（8*3=24 位正好 3 字节，检查第 8、9 像素） */
//   u8g2_ClearBuffer(u);
//   u8g2_DrawPixel(u, 8, 0);
//   u8g2_DrawPixel(u, 9, 0);
//   CHECK(pix_read(u, 8, 0) == 1 && pix_read(u, 9, 0) == 1,
//         "跨字节边界像素（像素8/9）可写可读");
//   u8g2_SetDrawColor(u, 0);
//   u8g2_DrawPixel(u, 8, 0);
//   CHECK(pix_read(u, 8, 0) == 0 && pix_read(u, 9, 0) == 1,
//         "跨字节边界像素（像素8）清除后不污染像素9");
// }

// static void test_text(u8g2_t *u) {
//   u8g2_SetDrawColor(u, 1);
//   printf("[test] 文本宽度与字形形状\n");
//   u8g2_ClearBuffer(u);

//   CHECK(u8g2_GetStrWidth(u, "Hello") > 0, "GetStrWidth(\"Hello\") > 0");
//   CHECK(u8g2_GetStrWidth(u, "Hello") == 5 * u8g2_GetGlyphWidth(u, 'H'),
//         "等宽假设：5 个字形宽 = 5x 单个宽");
//   CHECK(u8g2_GetStrWidth(u, "") == 0, "GetStrWidth(\"\") == 0");
//   CHECK(u8g2_GetGlyphWidth(u, 'A') > 0, "GetGlyphWidth('A') > 0");
//   CHECK(u8g2_GetGlyphWidth(u, 0xFFFF) == 0, "缺失字形返回 0");
//   CHECK(u8g2_GetFontAscent(u) > 0, "GetFontAscent > 0");

//   /* 画 'A' 于 (0, 7)，检查形状。
//    * 5x7 'A'：glyph_height=6、y_offset=0，基线 y=7 使字形占 y=1..6：
//    *   y=1: .##..
//    *   y=2: #..#.
//    *   y=3: #..#.
//    *   y=4: ####.
//    *   y=5: #..#.
//    *   y=6: #..#.   */
//   u8g2_ClearBuffer(u);
//   u8g2_DrawStr(u, 0, 7, "A");
//   printf("  'A' 字形（基线 y=7，5x7）：\n");
//   dump_glyph(u, 0, 0, 5, 7);
//   CHECK(pix_read(u, 2, 0) == 0, "'A' 顶行留空（y=0 无像素）");
//   CHECK(pix_read(u, 1, 1) == 1 && pix_read(u, 2, 1) == 1 &&
//             pix_read(u, 0, 1) == 0 && pix_read(u, 4, 1) == 0,
//         "'A' 第 1 行：中间两点");
//   CHECK(pix_read(u, 0, 4) == 1 && pix_read(u, 1, 4) == 1 &&
//             pix_read(u, 2, 4) == 1 && pix_read(u, 3, 4) == 1 &&
//             pix_read(u, 4, 4) == 0,
//         "'A' 第 4 行：全宽横条（左起第 4 列）");
//   CHECK(pix_read(u, 0, 6) == 1 && pix_read(u, 3, 6) == 1 &&
//             pix_read(u, 1, 6) == 0 && pix_read(u, 4, 6) == 0,
//         "'A' 底行：两脚着地（不贴满基线）");

//   /* 步进累加：DrawStr 返回终点 x */
//   u8g2_ClearBuffer(u);
//   {
//     u8g2_uint_t x0 = 0, end = u8g2_DrawStr(u, x0, 7, "AB");
//     CHECK(end == (u8g2_uint_t)(x0 + u8g2_GetStrWidth(u, "AB")),
//           "DrawStr 返回 x0 + 总宽");
//   }

//   /* 字形互不重叠：'A' 与 'B' 各自独立 */
//   u8g2_ClearBuffer(u);
//   u8g2_DrawStr(u, 0, 7, "AB");
//   CHECK(pix_read(u, 5, 0) == 0, "'A' 与 'B' 不重叠（x=5 处为空）");
// }

// static void test_clip(u8g2_t *u) {
//   u8g2_SetDrawColor(u, 1);
//   printf("[test] 裁剪窗口\n");
//   u8g2_ClearBuffer(u);

//   u8g2_SetClipWindow(u, 10, 10, 19, 19);
//   u8g2_DrawBox(u, 0, 0, 40, 40);
//   CHECK(pix_read(u, 9, 10) == 0 && pix_read(u, 10, 10) == 1 &&
//             pix_read(u, 19, 19) == 1 && pix_read(u, 20, 10) == 0,
//         "DrawBox 被裁剪到 (10,10)-(19,19)");
//   u8g2_SetMaxClipWindow(u);

//   u8g2_ClearBuffer(u);
//   u8g2_DrawBox(u, 0, 0, 200, 200); /* 超出屏幕 */
//   CHECK(pix_read(u, 0, 0) == 1 && pix_read(u, 127, 63) == 1,
//         "超界 DrawBox 被裁剪到屏幕内");

//   /* 空窗口 */
//   u8g2_ClearBuffer(u);
//   u8g2_SetClipWindow(u, 20, 20, 10, 10);
//   u8g2_DrawBox(u, 0, 0, 50, 50);
//   CHECK(pix_read(u, 0, 0) == 0, "空窗口下不绘制");
//   u8g2_SetMaxClipWindow(u);
// }

// static void test_primitives(u8g2_t *u) {
//   u8g2_SetDrawColor(u, 1);
//   printf("[test] 图元\n");
//   u8g2_ClearBuffer(u);

//   u8g2_DrawFrame(u, 5, 5, 10, 10);
//   CHECK(pix_read(u, 5, 5) == 1 && pix_read(u, 14, 5) == 1 &&
//             pix_read(u, 5, 14) == 1 && pix_read(u, 14, 14) == 1,
//         "DrawFrame 四角");
//   CHECK(pix_read(u, 10, 10) == 0, "DrawFrame 内部空心");

//   u8g2_ClearBuffer(u);
//   u8g2_DrawLine(u, 0, 0, 30, 0);
//   {
//     int ok = 1;
//     for (u8g2_uint_t x = 0; x <= 30; x++)
//       if (pix_read(u, x, 0) != 1) ok = 0;
//     CHECK(ok, "DrawLine 水平线");
//   }

//   u8g2_ClearBuffer(u);
//   u8g2_DrawDisc(u, 30, 30, 5);
//   CHECK(pix_read(u, 30, 30) == 1 && pix_read(u, 35, 30) == 1 &&
//             pix_read(u, 30, 35) == 1,
//         "DrawDisc 中心与边缘");
//   CHECK(pix_read(u, 36, 30) == 0, "DrawDisc 半径外空心");

//   u8g2_ClearBuffer(u);
//   u8g2_DrawCircle(u, 30, 30, 5);
//   CHECK(pix_read(u, 35, 30) == 1 && pix_read(u, 30, 30) == 0,
//         "DrawCircle 仅圆周");

//   u8g2_ClearBuffer(u);
//   u8g2_DrawTriangle(u, 0, 0, 10, 0, 5, 10);
//   /* 扫描线填充：顶点行整条，中部按插值（半开区间约定不画底边顶点行） */
//   CHECK(pix_read(u, 5, 0) == 1 && pix_read(u, 0, 0) == 1 &&
//             pix_read(u, 10, 0) == 1,
//         "DrawTriangle 顶点行覆盖");
//   CHECK(pix_read(u, 5, 5) == 1 && pix_read(u, 3, 5) == 1 &&
//             pix_read(u, 8, 5) == 1,
//         "DrawTriangle 中部填充（y=5 时 x∈[3,8]）");

//   u8g2_ClearBuffer(u);
//   u8g2_DrawXBM(u, 0, 0, 8, 1, (const uint8_t[]){0b10101010});
//   CHECK(pix_read(u, 0, 0) == 1 && pix_read(u, 1, 0) == 0 &&
//             pix_read(u, 2, 0) == 1 && pix_read(u, 7, 0) == 0,
//         "DrawXBM MSB-first 位序");
// }

// static void test_color_indices(u8g2_t *u) {
//   printf("[test] 颜色索引\n");
//   u8g2_ClearBuffer(u);

//   /* 枚举值：与默认调色板顺序一致（供自动补全/文档） */
//   CHECK(UI_COLOR_BLACK == 0 && UI_COLOR_WHITE == 1 && UI_COLOR_RED == 2 &&
//             UI_COLOR_GREEN == 3 && UI_COLOR_BLUE == 4 && UI_COLOR_YELLOW == 5 &&
//             UI_COLOR_CYAN == 6 && UI_COLOR_MAGENTA == 7,
//         "UI_COLOR_* 枚举值与调色板索引一致");
//   CHECK(U8G2_COLOR_RED == UI_COLOR_RED, "U8G2_COLOR_* 兼容宏仍可用");

//   u8g2_SetDrawColor(u, 2); /* 红 */
//   u8g2_DrawBox(u, 0, 0, 3, 1);
//   CHECK(pix_read(u, 0, 0) == 2 && pix_read(u, 2, 0) == 2,
//         "SetDrawColor(2) 后画红色");
//   u8g2_SetDrawColor(u, 0);
//   u8g2_DrawBox(u, 1, 0, 1, 1);
//   CHECK(pix_read(u, 0, 0) == 2 && pix_read(u, 1, 0) == 0,
//         "SetDrawColor(0) 擦除为背景");
//   u8g2_SetDrawColor(u, 1);
// }

// static void test_default_white(u8g2_t *u) {
//   printf("[test] u8g2_* 默认白字\n");
//   /* 重新初始化：draw_color 默认 = 1（白） */
//   u8g2_porting_init(u);
//   u8g2_DrawPixel(u, 0, 0);
//   u8g2_DrawBox(u, 0, 1, 2, 1);
//   u8g2_DrawLine(u, 0, 3, 3, 3);
//   CHECK(pix_read(u, 0, 0) == 1 && pix_read(u, 1, 1) == 1 &&
//             pix_read(u, 3, 3) == 1,
//         "未调用 SetDrawColor 时全部为索引 1（白）");
// }

// static void test_ui_api(u8g2_t *u) {
//   printf("[test] ui_* 显式带色绘制\n");
//   (void)u; /* ui_* 操作全局实例 */
//   u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
//   ui_clear();

//   ui_draw_pixel(0, 0, UI_COLOR_RED);
//   CHECK(pix_read(&u8g2, 0, 0) == UI_COLOR_RED, "ui_draw_pixel 红色");

//   ui_draw_box(10, 10, 3, 2, UI_COLOR_GREEN);
//   CHECK(pix_read(&u8g2, 10, 10) == UI_COLOR_GREEN &&
//             pix_read(&u8g2, 12, 11) == UI_COLOR_GREEN &&
//             pix_read(&u8g2, 13, 11) == 0,
//         "ui_draw_box 绿色实心");

//   ui_draw_frame(20, 20, 4, 4, UI_COLOR_BLUE);
//   CHECK(pix_read(&u8g2, 20, 20) == UI_COLOR_BLUE &&
//             pix_read(&u8g2, 21, 21) == 0,
//         "ui_draw_frame 蓝色空心");

//   ui_draw_line(0, 5, 5, 0, UI_COLOR_YELLOW);
//   CHECK(pix_read(&u8g2, 0, 5) == UI_COLOR_YELLOW &&
//             pix_read(&u8g2, 5, 0) == UI_COLOR_YELLOW,
//         "ui_draw_line 黄色斜线");

//   ui_draw_circle(50, 30, 4, UI_COLOR_CYAN);
//   CHECK(pix_read(&u8g2, 54, 30) == UI_COLOR_CYAN &&
//             pix_read(&u8g2, 50, 30) == 0,
//         "ui_draw_circle 青色圆周");

//   ui_draw_disc(60, 30, 4, UI_COLOR_MAGENTA);
//   CHECK(pix_read(&u8g2, 60, 30) == UI_COLOR_MAGENTA &&
//             pix_read(&u8g2, 64, 30) == UI_COLOR_MAGENTA,
//         "ui_draw_disc 洋红实心");

//   ui_draw_xbm(0, 30, 8, 1, (const uint8_t[]){0b10101010}, UI_COLOR_BLUE);
//   CHECK(pix_read(&u8g2, 0, 30) == UI_COLOR_BLUE &&
//             pix_read(&u8g2, 1, 30) == 0 &&
//             pix_read(&u8g2, 7, 30) == 0,
//         "ui_draw_xbm 带色位图");

//   ui_draw_rbox(70, 10, 10, 10, 3, UI_COLOR_GREEN);
//   CHECK(pix_read(&u8g2, 71, 11) == UI_COLOR_GREEN && /* 圆角弧内 */
//             pix_read(&u8g2, 75, 15) == UI_COLOR_GREEN && /* 中部 */
//             pix_read(&u8g2, 79, 16) == UI_COLOR_GREEN, /* 右缘 */
//         "ui_draw_rbox 圆角矩形填充");
//   CHECK(pix_read(&u8g2, 70, 10) == 0, "ui_draw_rbox 切角留空（r=3 时角点在外）");

//   ui_draw_rframe(70, 30, 10, 10, 3, UI_COLOR_YELLOW);
//   CHECK(pix_read(&u8g2, 70, 33) == UI_COLOR_YELLOW && /* 圆弧上 */
//             pix_read(&u8g2, 75, 35) == 0,
//         "ui_draw_rframe 圆角矩形边框");

//   ui_draw_triangle(100, 40, 110, 40, 105, 50, UI_COLOR_CYAN);
//   CHECK(pix_read(&u8g2, 105, 40) == UI_COLOR_CYAN &&
//             pix_read(&u8g2, 105, 45) == UI_COLOR_CYAN,
//         "ui_draw_triangle 带色三角形");
// }

// static void test_ui_text(u8g2_t *u) {
//   printf("[test] ui_* 彩色文本\n");
//   (void)u;
//   u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
//   u8g2_SetFontMode(&u8g2, 1); /* 透明（默认） */

//   ui_clear();
//   ui_draw_str(0, 7, "A", UI_COLOR_RED);
//   CHECK(pix_read(&u8g2, 1, 1) == UI_COLOR_RED &&
//             pix_read(&u8g2, 0, 4) == UI_COLOR_RED,
//         "ui_draw_str 红色字形");
//   CHECK(pix_read(&u8g2, 0, 1) == 0, "透明模式下背景不填充");

//   /* 实心模式 + 前景 0：背景用另一色（fg==0 时 bg=1 白）填充 → 反色字形 */
//   ui_clear();
//   u8g2_SetFontMode(&u8g2, 0);
//   ui_draw_str(0, 7, "A", UI_COLOR_BLACK);
//   CHECK(pix_read(&u8g2, 0, 1) == 1, "实心模式：背景像素被 bg 色填充");
//   CHECK(pix_read(&u8g2, 1, 1) == 0, "实心模式：前景像素用 fg 色（黑）");
//   u8g2_SetFontMode(&u8g2, 1); /* 恢复透明 */
// }

// static void test_sendbuffer(u8g2_t *u) {
//   printf("[test] LCD_SendBuffer 整屏单次刷新\n");
//   u8g2_ClearBuffer(u);

//   /* 3bpp 下 x=2,5,... 的像素跨字节边界，覆盖 bsp 的解包逻辑 */
//   for (u8g2_uint_t x = 0; x < 8; x++) {
//     ui_draw_pixel(x, 0, (uint8_t)x);
//   }
//   ui_draw_pixel(1, 1, UI_COLOR_WHITE);
//   ui_draw_pixel(2, 1, UI_COLOR_GREEN); /* 跨字节像素 */
//   ui_draw_pixel(5, 1, UI_COLOR_BLUE);  /* 跨字节像素 */
//   g_flush_count = 0;
//   g_flush_error = 0;

//   ui_send_buffer(); /* == u8g2_SendBuffer(&u8g2) == LCD_SendBuffer(...) */

//   CHECK(g_flush_error == 0, "SendBuffer 无非法参数");
//   CHECK(g_flush_count == 1, "整屏一次调用（无逐行窗口切换）");
//   CHECK(g_lcd_pix[1][1] == 0xFFFF && g_lcd_pix[2][1] == 0x07E0 &&
//             g_lcd_pix[5][1] == 0x001F,
//         "跨字节像素 RGB565 颜色正确");
//   CHECK(g_lcd_pix[0][2] == 0x0000, "未绘制区域仍为背景色");

//   /* 每像素索引 → 调色板映射全对（期望值取全局实例的默认调色板） */
//   {
//     int ok = 1;
//     for (u8g2_uint_t x = 0; x < 8; x++) {
//       if (g_lcd_pix[x][0] != u->palette[x]) ok = 0;
//     }
//     CHECK(ok, "8 色索引全部映射正确（含跨字节 x=2,5）");
//   }

//   /* 自定义调色板 */
//   {
//     static const uint16_t pal[U8G2_NUM_COLORS] = {0x0000, 0x1111, 0x2222,
//                                                   0x3333, 0x4444, 0x5555,
//                                                   0x6666, 0x7777};
//     u8g2_porting_set_palette(u, pal);
//     ui_clear();
//     ui_draw_pixel(1, 1, UI_COLOR_RED);
//     ui_send_buffer();
//     CHECK(g_lcd_pix[1][1] == 0x2222, "自定义调色板生效");
//     u8g2_porting_set_palette(u, NULL); /* 恢复默认 */
//   }
// }

// static void test_writepixels_backup(void) {
//   printf("[test] LCD_WritePixels 备用逐行路径\n");
//   g_flush_error = 0;
//   uint16_t px[6] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0x07FF, 0xF81F};
//   LCD_WritePixels(0, 0, 2, 1, px, 6);
//   CHECK(g_flush_error == 0, "窗口/计数合法");
//   CHECK(g_lcd_pix[0][0] == 0xF800 && g_lcd_pix[2][1] == 0xF81F,
//         "逐行像素正确");
// }

// int main(void) {
//   printf("=== u8g2 porting host test (%u x %u @ %ubpp) ===\n",
//          U8G2_PORTING_SCREEN_W, U8G2_PORTING_SCREEN_H, U8G2_PORTING_BPP);
//   g_lcd_w = U8G2_PORTING_SCREEN_W;
//   g_lcd_h = U8G2_PORTING_SCREEN_H;

//   u8g2_porting_init(&u8g2);
//   u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);

//   test_buffer_roundtrip(&u8g2);
//   test_text(&u8g2);
//   test_clip(&u8g2);
//   test_primitives(&u8g2);
//   test_color_indices(&u8g2);
//   test_default_white(&u8g2);
//   test_ui_api(&u8g2);
//   test_ui_text(&u8g2);
//   test_sendbuffer(&u8g2);
//   test_writepixels_backup();

//   printf("\n=== %s (%d 失败) ===\n", fail_count == 0 ? "全部通过" : "有失败",
//          fail_count);
//   return fail_count == 0 ? 0 : 1;
// }
