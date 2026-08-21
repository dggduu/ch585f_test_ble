/*
 * u8g2_porting.h — u8g2 风格显示框架的移植配置层
 *
 * 该框架对接 src/bsp/bsp_lcd_hw.c（ST7789V 240x240，SPI，RGB565），
 * 提供 u8g2 兼容的 API（见 u8g2.h）。颜色深度通过宏配置，默认 3bit/像素
 * （8 色调色板），适合 CH585 的 RAM 资源。
 *
 * 颜色深度与帧缓冲大小：
 *   U8G2_PORTING_BPP   像素位深（1/2/3/4/8），默认 3
 *   帧缓冲字节数 = W * H * BPP / 8
 *     240x240 @3bpp = 21600 字节（CH585F SRAM 128KB，可承受）
 *
 * 调色板：颜色索引（0 ~ 2^BPP-1）通过 u8g2_SetDrawColor() 选择，
 * 索引 0 恒为背景色（黑色）。默认 8 色调色板对应 LCD 驱动的
 * LCD_WHITE/BLACK/RED/GREEN/BLUE/YELLOW/CYAN/MAGENTA。
 */
#ifndef __U8G2_PORTING_H__
#define __U8G2_PORTING_H__

#include <stdint.h>

/* ============ 屏幕尺寸（与 bsp_lcd_hw.h 的 LCD_W/LCD_H 保持一致） ============ */
#ifndef U8G2_PORTING_SCREEN_W
#define U8G2_PORTING_SCREEN_W 240
#endif
#ifndef U8G2_PORTING_SCREEN_H
#define U8G2_PORTING_SCREEN_H 240
#endif

/* ============ 像素位深：1/2/3/4/8，默认 3（8 色） ============ */
#ifndef U8G2_PORTING_BPP
#define U8G2_PORTING_BPP 3
#endif

#if U8G2_PORTING_BPP != 1 && U8G2_PORTING_BPP != 2 && \
    U8G2_PORTING_BPP != 3 && U8G2_PORTING_BPP != 4 && \
    U8G2_PORTING_BPP != 8
#error "U8G2_PORTING_BPP must be 1, 2, 3, 4 or 8"
#endif

/* ============ 帧缓冲 ============ */
#define U8G2_PIX_BUF_BYTES                                                     \
  ((U8G2_PORTING_SCREEN_W * U8G2_PORTING_SCREEN_H * U8G2_PORTING_BPP + 7) / 8)

/* 颜色总数 */
#define U8G2_NUM_COLORS (1 << U8G2_PORTING_BPP)

/*
 * 默认调色板（RGB565）。仅 U8G2_PORTING_BPP == 3 时默认值有意义，
 * 其他位深需要自行在编译时覆盖此宏，且条目数必须等于 U8G2_NUM_COLORS。
 */
#ifndef U8G2_PORTING_DEFAULT_PALETTE
#define U8G2_PORTING_DEFAULT_PALETTE                                           \
  {                                                                            \
    0x0000, /*  0 黑色  */                                                     \
        0xFFFF, /*  1 白色  */                                                 \
        0xF800, /*  2 红色  */                                                 \
        0x07E0, /*  3 绿色  */                                                 \
        0x001F, /*  4 蓝色  */                                                 \
        0xFFE0, /*  5 黄色  */                                                 \
        0x07FF, /*  6 青色  */                                                 \
        0xF81F  /*  7 洋红  */                                                 \
  }
#endif

/*
 * 底层刷新接口：由 src/bsp/bsp_lcd_hw.c 提供（LCD_WritePixels）。
 * 这里只做前置声明，不包含 bsp 头文件，方便在主机端做单元测试时替换。
 * x1<=x2, y1<=y2，pixels 为 RGB565 像素数组，count 必须等于
 * (x2-x1+1) * (y2-y1+1)，屏幕按行填充。
 */
void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                     const uint16_t *pixels, uint32_t count);

#endif /* __U8G2_PORTING_H__ */
