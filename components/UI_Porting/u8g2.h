/*
 * u8g2.h — u8g2 风格的轻量显示框架（对接 bsp_lcd_hw 的彩色屏幕）
 *
 * 依据 components/my-u8g2-ui-toolkit 中的 ui_toolkit 实际使用的 API 子集
 * 重写实现（统计清单见 README.md），提供与 u8g2 相同的函数签名，
 * 使 ui_toolkit 的 UI/HList、VList、page_stack、splash_log、portal_component
 * 等代码可以不经修改直接编译运行。
 *
 * 与标准 u8g2 的差异：
 *   - 帧缓冲为全缓冲，像素位深通过 u8g2_porting.h 的宏配置（默认 3bit/像素）
 *   - 颜色索引 0 = 背景（黑色），u8g2_SetDrawColor(0/1) 语义与 1bpp 一致
 *   - 不提供 FirstPage/NextPage 分页缓冲模式
 *   - 不提供 u8x8 硬件抽象层（由 bsp_lcd_hw 承担）
 */
#ifndef __U8G2_H__
#define __U8G2_H__

#include "u8g2_porting.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t u8g2_uint_t;

/* ==================== 字体信息（u8g2 字体头 23 字节的解析结果） ==================== */
typedef struct {
  uint8_t glyph_cnt;
  uint8_t bbx_mode;
  uint8_t bits_per_0;
  uint8_t bits_per_1;
  uint8_t bits_per_char_width;
  uint8_t bits_per_char_height;
  uint8_t bits_per_char_x;
  uint8_t bits_per_char_y;
  uint8_t bits_per_delta_x;
  uint8_t max_char_width;
  uint8_t max_char_height;
  int8_t x_offset;
  int8_t y_offset;
  uint8_t ascent_A;
  uint8_t descent_g;
  uint8_t ascent_para;
  uint8_t descent_para;
  uint16_t start_pos_upper_A;
  uint16_t start_pos_lower_a;
  uint16_t start_pos_unicode;
} u8g2_font_info_t;

/* ==================== 字形 RLE 解码状态 ==================== */
typedef struct {
  const uint8_t *decode_ptr;
  uint8_t decode_bit_pos;
  uint8_t glyph_width;
  uint8_t glyph_height;
  uint8_t x; /* 字形内游标（局部坐标） */
  uint8_t y;
  u8g2_uint_t target_x;
  u8g2_uint_t target_y;
  uint8_t is_transparent; /* 0=实心（背景用 bg_color 填充），1=透明 */
  uint8_t fg_color;
  uint8_t bg_color;
} u8g2_font_decode_t;

/* ==================== 显示对象 ==================== */
typedef struct u8g2_struct u8g2_t;
struct u8g2_struct {
  u8g2_uint_t width;
  u8g2_uint_t height;

  /* 裁剪窗口（含边界） */
  u8g2_uint_t clip_x0;
  u8g2_uint_t clip_y0;
  u8g2_uint_t clip_x1;
  u8g2_uint_t clip_y1;

  uint8_t draw_color; /* 当前绘制颜色（调色板索引） */
  uint8_t font_mode;  /* 0: 透明文本, 1: 实心文本 */

  const uint8_t *font; /* 当前字体（u8g2 格式字体表） */
  u8g2_font_info_t font_info;
  u8g2_font_decode_t font_decode;

  uint8_t *pix_buf; /* 帧缓冲指针 */
  u8g2_uint_t pix_buf_size;

  const uint16_t *palette; /* RGB565 调色板（U8G2_NUM_COLORS 项） */
};

/* 全局显示对象实例（ui_toolkit 的 portal_component.c 引用了它） */
extern u8g2_t u8g2;

/* ==================== 移植层接口 ==================== */
/* 初始化全局显示对象：绑定帧缓冲、默认调色板、复位裁剪窗口与颜色。
 * 注意：LCD 硬件初始化（Lcd_Init）由 bsp_lcd_hw 负责，需先调用。 */
void u8g2_porting_init(u8g2_t *u8g2);

/* 自定义调色板（RGB565 数组，U8G2_NUM_COLORS 项），pal 为 NULL 时恢复默认 */
void u8g2_porting_set_palette(u8g2_t *u8g2, const uint16_t *pal);

/* 获取帧缓冲首地址（用于自检/直接操作，一般不需要） */
uint8_t *u8g2_get_buffer(u8g2_t *u8g2);

/* ==================== 初始化/缓冲（u8g2 兼容） ==================== */
/* LCD 初始化由 bsp_lcd_hw 的 Lcd_Init 完成，此函数保留为兼容空操作 */
void u8g2_InitDisplay(u8g2_t *u8g2);
/* 掉电/唤醒：本框架为全缓冲直写，保留为兼容空操作 */
void u8g2_SetPowerSave(u8g2_t *u8g2, uint8_t is_enable);

void u8g2_ClearBuffer(u8g2_t *u8g2);
void u8g2_SendBuffer(u8g2_t *u8g2);

/* ==================== 颜色/字体设置（u8g2 兼容） ==================== */
/* color: 调色板索引（0 ~ U8G2_NUM_COLORS-1），0 为背景色 */
void u8g2_SetDrawColor(u8g2_t *u8g2, uint8_t color);
/* is_transparent: 1=透明（只画前景，默认），0=实心（背景用另一色填充） */
void u8g2_SetFontMode(u8g2_t *u8g2, uint8_t is_transparent);
void u8g2_SetFont(u8g2_t *u8g2, const uint8_t *font);
/* 文本锚点固定在基线，兼容空操作 */
void u8g2_SetFontPosBaseline(u8g2_t *u8g2);

/* ==================== 裁剪窗口（u8g2 兼容） ==================== */
void u8g2_SetClipWindow(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0,
                        u8g2_uint_t x1, u8g2_uint_t y1);
void u8g2_SetMaxClipWindow(u8g2_t *u8g2);

/* ==================== 基本图元（u8g2 兼容） ==================== */
void u8g2_DrawPixel(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y);
void u8g2_DrawHLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len);
void u8g2_DrawVLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len);
void u8g2_DrawLine(u8g2_t *u8g2, u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                   u8g2_uint_t y2);
void u8g2_DrawBox(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h);
void u8g2_DrawFrame(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                    u8g2_uint_t h);
void u8g2_DrawRBox(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, u8g2_uint_t r);
void u8g2_DrawRFrame(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                     u8g2_uint_t h, u8g2_uint_t r);
void u8g2_DrawCircle(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0,
                     u8g2_uint_t rad);
void u8g2_DrawDisc(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0,
                   u8g2_uint_t rad);
void u8g2_DrawTriangle(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0,
                       u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                       u8g2_uint_t y2);

/* XBM 位图：每行像素 MSB 在前，行尾补齐到整字节（与 u8g2 一致） */
void u8g2_DrawXBM(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h, const uint8_t *bitmap);
/* 本平台无 flash/RAM 之分，与 DrawXBM 相同 */
void u8g2_DrawXBMP(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, const uint8_t *bitmap);

/* ==================== 文本（u8g2 兼容） ==================== */
/* 返回该字形的横向步进 */
u8g2_uint_t u8g2_DrawGlyph(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,
                           uint16_t encoding);
/* 返回字符串结束时的 x 位置 */
u8g2_uint_t u8g2_DrawStr(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,
                         const char *str);
u8g2_uint_t u8g2_DrawUTF8(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,
                          const char *str);

u8g2_uint_t u8g2_GetStrWidth(u8g2_t *u8g2, const char *s);
u8g2_uint_t u8g2_GetUTF8Width(u8g2_t *u8g2, const char *s);
/* 返回有符号步进 delta_x（与 u8g2 一致，可能为负） */
int8_t u8g2_GetGlyphWidth(u8g2_t *u8g2, uint16_t encoding);

uint8_t u8g2_GetFontAscent(u8g2_t *u8g2);
uint8_t u8g2_GetFontDescent(u8g2_t *u8g2);
uint8_t u8g2_GetFontHeight(u8g2_t *u8g2);

/* ==================== 颜色索引（U8G2_PORTING_BPP == 3 时） ==================== */
/* 调色板索引枚举：ui_* 系列绘制 API 的颜色参数、u8g2_SetDrawColor() 的入参。
 * 枚举类型让 IDE 自动补全可以直接列出可选颜色。 */
#if U8G2_PORTING_BPP >= 3
typedef enum {
  UI_COLOR_BLACK   = 0, /* 黑色（索引 0 恒为背景色） */
  UI_COLOR_WHITE   = 1, /* 白色 */
  UI_COLOR_RED     = 2, /* 红色 */
  UI_COLOR_GREEN   = 3, /* 绿色 */
  UI_COLOR_BLUE    = 4, /* 蓝色 */
  UI_COLOR_YELLOW  = 5, /* 黄色 */
  UI_COLOR_CYAN    = 6, /* 青色 */
  UI_COLOR_MAGENTA = 7, /* 洋红色 */
} ui_color_t;
/* 兼容宏：旧代码/二进制码仍可使用 U8G2_COLOR_* */
#define U8G2_COLOR_BLACK   UI_COLOR_BLACK
#define U8G2_COLOR_WHITE   UI_COLOR_WHITE
#define U8G2_COLOR_RED     UI_COLOR_RED
#define U8G2_COLOR_GREEN   UI_COLOR_GREEN
#define U8G2_COLOR_BLUE    UI_COLOR_BLUE
#define U8G2_COLOR_YELLOW  UI_COLOR_YELLOW
#define U8G2_COLOR_CYAN    UI_COLOR_CYAN
#define U8G2_COLOR_MAGENTA UI_COLOR_MAGENTA
#endif

/* ==================== ui_* 彩色绘制 API（操作全局 u8g2 实例） ====================
 * 所有 ui_* 绘制函数都显式携带颜色索引参数（ui_color_t），与 u8g2_* 兼容层
 * （默认白字，颜色由 u8g2_SetDrawColor 决定）对应：
 *   底层实现共用一个绘制管线，u8g2_* 内部转发到 ui_* 并传入当前 draw_color。
 * 全部遵守全局裁剪窗口（u8g2_SetClipWindow），字体绘制使用当前字体
 * （u8g2_SetFont），文本实心模式（u8g2_SetFontMode(0)）背景用另一色填充。 */

/* ---- 缓冲管理 ---- */
/* 清空帧缓冲为背景色（索引 0） */
void ui_clear(void);
/* 整屏索引缓冲一次性发送（内部调用 bsp 的 LCD_SendBuffer，无逐行窗口切换） */
void ui_send_buffer(void);

/* ---- 基本图元（color: 调色板索引 0~U8G2_NUM_COLORS-1） ---- */
void ui_draw_pixel(u8g2_uint_t x, u8g2_uint_t y, uint8_t color);
void ui_draw_hline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t color);
void ui_draw_vline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t color);
void ui_draw_line(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2,
                  uint8_t color);
void ui_draw_box(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                 uint8_t color);
void ui_draw_frame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                   uint8_t color);
void ui_draw_rbox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                  u8g2_uint_t r, uint8_t color);
void ui_draw_rframe(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                    u8g2_uint_t r, uint8_t color);
void ui_draw_circle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad,
                    uint8_t color);
void ui_draw_disc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad,
                  uint8_t color);
void ui_draw_triangle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t x1,
                      u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2,
                      uint8_t color);
/* XBM 位图：每行像素 MSB 在前，行尾补齐到整字节；1 位画 color，0 位跳过 */
void ui_draw_xbm(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                 const uint8_t *bitmap, uint8_t color);

/* ---- 文本（使用当前字体，返回横向步进/结束 x） ---- */
u8g2_uint_t ui_draw_glyph(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding,
                          uint8_t color);
u8g2_uint_t ui_draw_str(u8g2_uint_t x, u8g2_uint_t y, const char *str,
                        uint8_t color);
u8g2_uint_t ui_draw_utf8(u8g2_uint_t x, u8g2_uint_t y, const char *str,
                         uint8_t color);

/* ==================== 内置字体（u8g2 原版字体表，见 u8g2_fonts.c） ==================== */
extern const uint8_t u8g2_font_5x7_tf[];
extern const uint8_t u8g2_font_6x10_tf[];
extern const uint8_t u8g2_font_8x13_tr[];
extern const uint8_t u8g2_font_logisoso20_tn[];
extern const uint8_t u8g2_font_open_iconic_all_4x_t[];

#ifdef __cplusplus
}
#endif

#endif /* __U8G2_H__ */
