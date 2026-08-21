// /*
//  * test_u8g2.c — 主机端单元测试（不依赖 MCU 外设）
//  *
//  * 编译（在 tools/host_test/ 下）：
//  *   gcc -I ../.. -DU8G2_PORTING_SCREEN_W=128 -DU8G2_PORTING_SCREEN_H=64 \
//  *       test_u8g2.c ../../u8g2.c ../../u8g2_fonts.c -o test_u8g2
//  *
//  * 用 stub 的 LCD_WritePixels 替换 bsp 实现，验证：
//  *   1. 字形解码（ASCII art 目检 'A'、'i' 形状）
//  *   2. GetStrWidth / DrawStr 步进累加
//  *   3. 裁剪窗口
//  *   4. 图元（Box/Frame/Line/Circle/Disc/Triangle/XBM）
//  *   5. SendBuffer 逐行输出（每行都到达 stub 且像素颜色正确）
//  *   6. 3bpp 帧缓冲打包/解包往返
//  */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>

// #include "u8g2.h"

// /* ==================== stub：LCD_WritePixels ==================== */
// static uint16_t g_lcd_pix[128][64]; /* 模拟屏幕（RGB565） */
// static uint16_t g_lcd_w, g_lcd_h;
// static int g_flush_count = 0;
// static int g_flush_error = 0;

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

// /* 从 3bpp 帧缓冲读回像素（测试用，直接调用 u8g2 的公开接口再验证） */
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

//   /* 只验证偶数位是对的、奇数位也是对的 */
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

// static void test_flush(u8g2_t *u) {
//   printf("[test] SendBuffer 逐行输出\n");
//   u8g2_ClearBuffer(u);
//   u8g2_SetDrawColor(u, 1);
//   u8g2_DrawPixel(u, 3, 4); /* 白 */
//   u8g2_SetDrawColor(u, 2);
//   u8g2_DrawPixel(u, 5, 6); /* 红 */
//   g_flush_count = 0;
//   g_flush_error = 0;

//   u8g2_SendBuffer(u);

//   CHECK(g_flush_error == 0, "所有 flush 窗口/计数合法");
//   CHECK(g_flush_count == (int)u->height, "每个屏幕行 flush 一次");
//   CHECK(g_lcd_pix[3][4] == 0xFFFF && g_lcd_pix[5][6] == 0xF800 &&
//             g_lcd_pix[0][0] == 0x0000,
//         "RGB565 颜色映射正确（白/红/黑）");

//   /* 自定义调色板 */
//   {
//     static const uint16_t pal[U8G2_NUM_COLORS] = {0x0000, 0x1111, 0x2222,
//                                                   0x3333, 0x4444, 0x5555,
//                                                   0x6666, 0x7777};
//     u8g2_porting_set_palette(u, pal);
//     u8g2_ClearBuffer(u);
//     u8g2_SetDrawColor(u, 2);
//     u8g2_DrawPixel(u, 1, 1);
//     u8g2_SendBuffer(u);
//     CHECK(g_lcd_pix[1][1] == 0x2222, "自定义调色板生效");
//     u8g2_porting_set_palette(u, NULL); /* 恢复默认 */
//   }
// }

// static void test_color(u8g2_t *u) {
//   printf("[test] 颜色索引\n");
//   u8g2_ClearBuffer(u);
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
//   test_color(&u8g2);
//   test_flush(&u8g2);

//   printf("\n=== %s (%d 失败) ===\n", fail_count == 0 ? "全部通过" : "有失败",
//          fail_count);
//   return fail_count == 0 ? 0 : 1;
// }
