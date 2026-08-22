/*
 * u8g2.c — u8g2 风格显示框架的实现（对接 bsp_lcd_hw 的彩色屏幕）
 *
 * 帧缓冲：全缓冲，像素位深由 U8G2_PORTING_BPP 决定（默认 3bit/像素，
 * LSB-first 打包，每 3 字节 8 像素）。绘制时按当前裁剪窗口落笔，
 * SendBuffer 时逐行取出像素、经调色板映射为 RGB565 交给 LCD_WritePixels。
 *
 * 字体：u8g2 原版字体协议（23 字节头 + 跳转表 + MSB-first 位流 RLE），
 * 解码算法与 u8g2_font.c 一致；字形数据见 u8g2_fonts.c。
 */
#include "u8g2.h"

#include <string.h>

/* ==================== 全局显示对象 ==================== */
static uint8_t u8g2_pix_buf[U8G2_PIX_BUF_BYTES];
static const uint16_t u8g2_default_palette[U8G2_NUM_COLORS] =
    U8G2_PORTING_DEFAULT_PALETTE;

u8g2_t u8g2;

/* ==================== 帧缓冲位操作 ==================== */
/*
 * 像素 bit 位宽 BPP，LSB-first 组打包：
 *   位偏移 bit = (y * W + x) * BPP；字节 byte = bit >> 3；组内移位 sh = bit & 7
 * 一组的位可能跨字节：跨到第二字节时高位被 (8 - sh) 截断（第二字节只用到
 * 低位 sh + BPP - 8 位），与 u8g2 1bpp 的字节读取方式一致。
 */

/* 3bpp 常量定义 */
#define U8G2_3BPP_MASK     0x07 /* (1 << 3) - 1 */

static inline void u8g2_pix_write(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, uint8_t color) {
  // 1. 计算 bit 偏移与 Byte 索引 (x * 3 可以用 (x << 1) + x 优化，编译器通常会自动做)
  uint32_t bit = ((uint32_t)y * u->width + x) * 3;
  uint32_t byte = bit >> 3;
  uint8_t sh = bit & 7;

  // 2. 预裁剪颜色值 (取低 3 位)
  uint8_t c = color & U8G2_3BPP_MASK;

  // 3. 构建 16 位掩码与值（避免重复移位与类型转换）
  uint16_t mask = (uint16_t)(U8G2_3BPP_MASK << sh);
  uint16_t val  = (uint16_t)(c << sh);

  // 4. 写入首字节
  u->pix_buf[byte] = (u->pix_buf[byte] & ~(uint8_t)mask) | (uint8_t)val;

  // 5. 跨字节处理：当 sh 为 6 或 7 时（sh + 3 > 8），3bpp 才会跨入下一个字节
  // 使用显式常数判断代替加法，更容易被编译器优化为极简条件分支
  if (sh >= 6) {
    u->pix_buf[byte + 1] = (u->pix_buf[byte + 1] & ~(uint8_t)(mask >> 8)) | (uint8_t)(val >> 8);
  }
}

/* ==================== 裁剪与像素绘制 ==================== */
static uint8_t u8g2_is_inside_clip(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  return x >= u->clip_x0 && x <= u->clip_x1 && y >= u->clip_y0 &&
         y <= u->clip_y1;
}

/* 带色落笔：裁剪后按索引写入帧缓冲（所有图元的最终落笔点） */
static void u8g2_pix_set(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                         uint8_t color) {
  if (u8g2_is_inside_clip(u, x, y)) {
    u8g2_pix_write(u, x, y, color);
  }
}

/* ==================== 初始化 ==================== */
void u8g2_porting_init(u8g2_t *u) {
  u->width = U8G2_PORTING_SCREEN_W;
  u->height = U8G2_PORTING_SCREEN_H;
  u->pix_buf = u8g2_pix_buf;
  u->pix_buf_size = U8G2_PIX_BUF_BYTES;
  u->palette = u8g2_default_palette;
  u->font = NULL;
  u->font_mode = 1; /* 透明文本（u8g2 默认） */
  u->font_decode.is_transparent = 1;
  u->draw_color = 1;
  u8g2_SetMaxClipWindow(u);
  u8g2_ClearBuffer(u);
}

void u8g2_porting_set_palette(u8g2_t *u, const uint16_t *pal) {
  u->palette = (pal != NULL) ? pal : u8g2_default_palette;
}

uint8_t *u8g2_get_buffer(u8g2_t *u) { return u->pix_buf; }

/* ==================== 兼容空操作 ==================== */
void u8g2_InitDisplay(u8g2_t *u) { (void)u; }
void u8g2_SetPowerSave(u8g2_t *u, uint8_t is_enable) {
  (void)u;
  (void)is_enable;
}

/* ==================== 缓冲管理 ==================== */
void u8g2_ClearBuffer(u8g2_t *u) { memset(u->pix_buf, 0, u->pix_buf_size); }

/* 上次实际送显的帧内容（影子缓冲），用于脏帧检测 */
static uint8_t u8g2_last_buf[U8G2_PIX_BUF_BYTES];
static bool   u8g2_first_flush = true;

bool u8g2_BufferChanged(u8g2_t *u) {
  if (u == NULL || u->pix_buf == NULL) return true;
  if (u8g2_first_flush ||
      memcmp(u->pix_buf, u8g2_last_buf, u->pix_buf_size) != 0) {
    u8g2_first_flush = false;
    memcpy(u8g2_last_buf, u->pix_buf, u->pix_buf_size);
    return true;
  }
  return false;
}

/* 颜色索引 0 恒为背景色（黑色）：清空时填 0 即得背景。
 * 整屏索引缓冲一次发送：内部由 bsp 的 LCD_SendBuffer 逐像素解出索引、
 * 经调色板映射为 RGB565 批量 SPI 发送，无需逐行切换窗口。
 * 注意：这是一次阻塞的全屏 SPI 刷新（240x240≈115KB，约 3~5ms），
 * 期间 TMOS_SystemProcess 无法运行；静态画面请用 u8g2_BufferChanged
 * 判定后再调用，避免无谓阻塞 BLE。 */
void u8g2_SendBuffer(u8g2_t *u) {
  LCD_SendBuffer(u->pix_buf, u->palette, u->width, u->height,
                 U8G2_PORTING_BPP);
}

/* ==================== 颜色/字体设置 ==================== */
void u8g2_SetDrawColor(u8g2_t *u, uint8_t color) {
  u->draw_color = color & (U8G2_NUM_COLORS - 1);
}

/* is_transparent: 1=透明（只画前景，默认），0=实心（背景用 bg_color 填充） */
void u8g2_SetFontMode(u8g2_t *u, uint8_t is_transparent) {
  u->font_mode = is_transparent ? 1 : 0;
  u->font_decode.is_transparent = u->font_mode;
}

/* ==================== 裁剪窗口 ==================== */
void u8g2_SetClipWindow(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                        u8g2_uint_t x1, u8g2_uint_t y1) {
  if (x1 < x0 || y1 < y0) { /* 空窗口 */
    u->clip_x0 = 1;
    u->clip_y0 = 1;
    u->clip_x1 = 0;
    u->clip_y1 = 0;
    return;
  }
  u->clip_x0 = x0;
  u->clip_y0 = y0;
  u->clip_x1 = x1;
  u->clip_y1 = y1;
}

void u8g2_SetMaxClipWindow(u8g2_t *u) {
  u->clip_x0 = 0;
  u->clip_y0 = 0;
  u->clip_x1 = (u8g2_uint_t)(u->width - 1);
  u->clip_y1 = (u8g2_uint_t)(u->height - 1);
}

/* ==================== 基本图元 ==================== */
/*
 * 带色内部实现：所有图元算法的"落笔"都收敛到 u8g2_pix_set()。
 * 公开层分两套（见 u8g2.h）：
 *   - ui_draw_*  : 显式携带颜色索引（操作全局 u8g2 实例）
 *   - u8g2_Draw* : u8g2 兼容签名，转发到带色实现并传入 u->draw_color
 *                  （默认 1 = 白色，u8g2_SetDrawColor 可改）
 */
static void u8g2_draw_hline_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                  u8g2_uint_t len, uint8_t color) {
  u8g2_uint_t i;
  if (y < u->clip_y0 || y > u->clip_y1 || len == 0) {
    return;
  }
  for (i = 0; i < len; i++) {
    u8g2_pix_set(u, (u8g2_uint_t)(x + i), y, color);
  }
}

static void u8g2_draw_vline_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                  u8g2_uint_t len, uint8_t color) {
  u8g2_uint_t i;
  if (x < u->clip_x0 || x > u->clip_x1 || len == 0) {
    return;
  }
  for (i = 0; i < len; i++) {
    u8g2_pix_set(u, x, (u8g2_uint_t)(y + i), color);
  }
}

/* Bresenham 直线（u8g2 同款算法：不做坐标排序，按实际方向步进，
 * 保证 (x1,y1)->(x2,y2) 的方向/端点与 u8g2 一致） */
static void u8g2_draw_line_color(u8g2_t *u, u8g2_uint_t x1, u8g2_uint_t y1,
                                 u8g2_uint_t x2, u8g2_uint_t y2,
                                 uint8_t color) {
  int16_t dx, dy;
  int8_t sx, sy;
  int16_t err, e2;

  dx = (int16_t)(x2 > x1 ? x2 - x1 : x1 - x2);
  dy = (int16_t)(y2 > y1 ? y2 - y1 : y1 - y2);
  sx = (int8_t)((x1 < x2) ? 1 : -1);
  sy = (int8_t)((y1 < y2) ? 1 : -1);
  err = (int16_t)(dx - dy);
  for (;;) {
    u8g2_pix_set(u, x1, y1, color);
    if (x1 == x2 && y1 == y2) {
      break;
    }
    e2 = (int16_t)(2 * err);
    if (e2 > -dy) {
      err -= dy;
      x1 += (u8g2_uint_t)sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += (u8g2_uint_t)sy;
    }
  }
}

static void u8g2_draw_box_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                u8g2_uint_t w, u8g2_uint_t h,
                                uint8_t color) {
  u8g2_uint_t i, j;
  u8g2_uint_t x0 = x, y0 = y;
  u8g2_uint_t x1 = (u8g2_uint_t)(x + w - 1), y1 = (u8g2_uint_t)(y + h - 1);

  if (w == 0 || h == 0) return;
  /* 与裁剪窗口求交 */
  if (x0 < u->clip_x0) x0 = u->clip_x0;
  if (y0 < u->clip_y0) y0 = u->clip_y0;
  if (x1 > u->clip_x1) x1 = u->clip_x1;
  if (y1 > u->clip_y1) y1 = u->clip_y1;
  if (x0 > x1 || y0 > y1) return;

  for (j = y0; j <= y1; j++) {
    for (i = x0; i <= x1; i++) {
      u8g2_pix_write(u, i, j, color);
    }
  }
}

static void u8g2_draw_frame_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                  u8g2_uint_t w, u8g2_uint_t h,
                                  uint8_t color) {
  if (w < 2 || h < 2) {
    u8g2_draw_box_color(u, x, y, w, h, color);
    return;
  }
  u8g2_draw_hline_color(u, x, y, w, color);
  u8g2_draw_hline_color(u, x, (u8g2_uint_t)(y + h - 1), w, color);
  u8g2_draw_vline_color(u, x, y, h, color);
  u8g2_draw_vline_color(u, (u8g2_uint_t)(x + w - 1), y, h, color);
}

/* 圆角矩形（四个角画 1/4 圆，r==0 退化为直角矩形） */
static void u8g2_draw_corner_color(u8g2_t *u, u8g2_uint_t cx, u8g2_uint_t cy,
                                   int8_t sx, int8_t sy, u8g2_uint_t rad,
                                   uint8_t filled, uint8_t color) {
  u8g2_uint_t x, y;
  int16_t d;
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (filled ? (d <= (int16_t)(rad * rad)) : (d == (int16_t)(rad * rad))) {
        u8g2_pix_set(u, (u8g2_uint_t)(cx + (int16_t)x * sx),
                     (u8g2_uint_t)(cy + (int16_t)y * sy), color);
      }
    }
  }
}

static void u8g2_draw_rbox_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                 u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r,
                                 uint8_t color) {
  if (r == 0) {
    u8g2_draw_box_color(u, x, y, w, h, color);
    return;
  }
  if (w < 2 * r + 1 || h < 2 * r + 1) {
    u8g2_draw_box_color(u, x, y, w, h, color);
    return;
  }
  /* 中间矩形 */
  u8g2_draw_box_color(u, (u8g2_uint_t)(x + r), y, (u8g2_uint_t)(w - 2 * r), h,
                      color);
  u8g2_draw_box_color(u, x, (u8g2_uint_t)(y + r), r, (u8g2_uint_t)(h - 2 * r),
                      color);
  u8g2_draw_box_color(u, (u8g2_uint_t)(x + w - r), (u8g2_uint_t)(y + r), r,
                      (u8g2_uint_t)(h - 2 * r), color);
  /* 四个角 */
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + r), -1, -1,
                         r, 1, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + w - r - 1), (u8g2_uint_t)(y + r),
                         1, -1, r, 1, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - r - 1),
                         -1, 1, r, 1, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + w - r - 1),
                         (u8g2_uint_t)(y + h - r - 1), 1, 1, r, 1, color);
}

static void u8g2_draw_rframe_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                   u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r,
                                   uint8_t color) {
  if (r == 0) {
    u8g2_draw_frame_color(u, x, y, w, h, color);
    return;
  }
  if (w < 2 * r + 1 || h < 2 * r + 1) {
    u8g2_draw_frame_color(u, x, y, w, h, color);
    return;
  }
  u8g2_draw_hline_color(u, (u8g2_uint_t)(x + r), y, (u8g2_uint_t)(w - 2 * r),
                        color);
  u8g2_draw_hline_color(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - 1),
                        (u8g2_uint_t)(w - 2 * r), color);
  u8g2_draw_vline_color(u, x, (u8g2_uint_t)(y + r), (u8g2_uint_t)(h - 2 * r),
                        color);
  u8g2_draw_vline_color(u, (u8g2_uint_t)(x + w - 1), (u8g2_uint_t)(y + r),
                        (u8g2_uint_t)(h - 2 * r), color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + r), -1, -1,
                         r, 0, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + w - r - 1), (u8g2_uint_t)(y + r),
                         1, -1, r, 0, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - r - 1),
                         -1, 1, r, 0, color);
  u8g2_draw_corner_color(u, (u8g2_uint_t)(x + w - r - 1),
                         (u8g2_uint_t)(y + h - r - 1), 1, 1, r, 0, color);
}

static void u8g2_draw_circle_color(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                                   u8g2_uint_t rad, uint8_t color) {
  u8g2_uint_t x, y;
  int16_t d;
  if (rad == 0) {
    u8g2_pix_set(u, x0, y0, color);
    return;
  }
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (d == (int16_t)(rad * rad)) {
        u8g2_pix_set(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 - y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 + y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 - y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 + y), color);
      }
    }
  }
}

static void u8g2_draw_disc_color(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                                 u8g2_uint_t rad, uint8_t color) {
  u8g2_uint_t x, y;
  int16_t d;
  if (rad == 0) {
    u8g2_pix_set(u, x0, y0, color);
    return;
  }
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (d <= (int16_t)(rad * rad)) {
        u8g2_pix_set(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 - y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 + y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 - y), color);
        u8g2_pix_set(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 + y), color);
      }
    }
  }
}

static void u8g2_draw_triangle_color(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                                     u8g2_uint_t x1, u8g2_uint_t y1,
                                     u8g2_uint_t x2, u8g2_uint_t y2,
                                     uint8_t color) {
  u8g2_uint_t ymin = y0, ymax = y0;
  u8g2_uint_t y;
  u8g2_uint_t xs[2];

  if (y1 < ymin) ymin = y1;
  if (y2 < ymin) ymin = y2;
  if (y1 > ymax) ymax = y1;
  if (y2 > ymax) ymax = y2;

  for (y = ymin; y <= ymax; y++) {
    /* 求当前扫描线与三条边的交点（每条边与扫描线至多相交一次） */
    u8g2_uint_t cnt = 0;
    xs[0] = 0;
    xs[1] = 0;

    if ((y0 <= y && y < y1) || (y1 <= y && y < y0)) {
      /* 边 (x0,y0)-(x1,y1)，线性插值 */
      if (y0 != y1) {
        xs[cnt++] = (u8g2_uint_t)(x0 +
                                  (int32_t)(x1 - x0) * (int32_t)(y - y0) /
                                      (int32_t)(y1 - y0));
      }
    }
    if ((y1 <= y && y < y2) || (y2 <= y && y < y1)) {
      if (y1 != y2) {
        xs[cnt++] = (u8g2_uint_t)(x1 +
                                  (int32_t)(x2 - x1) * (int32_t)(y - y1) /
                                      (int32_t)(y2 - y1));
      }
    }
    if ((y2 <= y && y < y0) || (y0 <= y && y < y2)) {
      if (y2 != y0) {
        xs[cnt++] = (u8g2_uint_t)(x2 +
                                  (int32_t)(x0 - x2) * (int32_t)(y - y2) /
                                      (int32_t)(y0 - y2));
      }
    }
    if (cnt == 2) {
      u8g2_uint_t a = xs[0], b = xs[1];
      if (a > b) {
        u8g2_uint_t t = a; a = b; b = t;
      }
      u8g2_draw_hline_color(u, a, y, (u8g2_uint_t)(b - a + 1), color);
    }
  }
}

/* XBM 位图：每行像素 MSB 在前，行尾补齐到整字节；1 位画 color，0 位跳过 */
static void u8g2_draw_xbm_color(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                                u8g2_uint_t w, u8g2_uint_t h,
                                const uint8_t *bitmap, uint8_t color) {
  u8g2_uint_t i, j;
  u8g2_uint_t byte_per_row = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      uint8_t b = bitmap[j * byte_per_row + (i >> 3)];
      if (b & (0x80 >> (i & 7))) {
        u8g2_pix_set(u, (u8g2_uint_t)(x + i), (u8g2_uint_t)(y + j), color);
      }
    }
  }
}

/* ==================== ui_* 彩色 API（操作全局 u8g2 实例） ==================== */
void ui_clear(void) { u8g2_ClearBuffer(&u8g2); }

void ui_send_buffer(void) { u8g2_SendBuffer(&u8g2); }

void ui_draw_pixel(u8g2_uint_t x, u8g2_uint_t y, uint8_t color) {
  u8g2_pix_set(&u8g2, x, y, color);
}

void ui_draw_hline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len,
                   uint8_t color) {
  u8g2_draw_hline_color(&u8g2, x, y, len, color);
}

void ui_draw_vline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len,
                   uint8_t color) {
  u8g2_draw_vline_color(&u8g2, x, y, len, color);
}

void ui_draw_line(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                  u8g2_uint_t y2, uint8_t color) {
  u8g2_draw_line_color(&u8g2, x1, y1, x2, y2, color);
}

void ui_draw_box(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                 uint8_t color) {
  u8g2_draw_box_color(&u8g2, x, y, w, h, color);
}

void ui_draw_frame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                   uint8_t color) {
  u8g2_draw_frame_color(&u8g2, x, y, w, h, color);
}

void ui_draw_rbox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                  u8g2_uint_t r, uint8_t color) {
  u8g2_draw_rbox_color(&u8g2, x, y, w, h, r, color);
}

void ui_draw_rframe(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                    u8g2_uint_t h, u8g2_uint_t r, uint8_t color) {
  u8g2_draw_rframe_color(&u8g2, x, y, w, h, r, color);
}

void ui_draw_circle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad,
                    uint8_t color) {
  u8g2_draw_circle_color(&u8g2, x0, y0, rad, color);
}

void ui_draw_disc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad,
                  uint8_t color) {
  u8g2_draw_disc_color(&u8g2, x0, y0, rad, color);
}

void ui_draw_triangle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t x1,
                      u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2,
                      uint8_t color) {
  u8g2_draw_triangle_color(&u8g2, x0, y0, x1, y1, x2, y2, color);
}

void ui_draw_xbm(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
                 const uint8_t *bitmap, uint8_t color) {
  u8g2_draw_xbm_color(&u8g2, x, y, w, h, bitmap, color);
}

/* ==================== u8g2 兼容层（默认白字） ==================== */
/* u8g2_* 保持原签名，内部转发到带色实现并传入当前 draw_color（默认 1=白）。
 * 本框架为单显示实例，u 与全局 u8g2 共享同一状态。 */
void u8g2_DrawPixel(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  u8g2_pix_set(u, x, y, u->draw_color);
}

void u8g2_DrawHLine(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len) {
  u8g2_draw_hline_color(u, x, y, len, u->draw_color);
}

void u8g2_DrawVLine(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len) {
  u8g2_draw_vline_color(u, x, y, len, u->draw_color);
}

void u8g2_DrawLine(u8g2_t *u, u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                   u8g2_uint_t y2) {
  u8g2_draw_line_color(u, x1, y1, x2, y2, u->draw_color);
}

void u8g2_DrawBox(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h) {
  u8g2_draw_box_color(u, x, y, w, h, u->draw_color);
}

void u8g2_DrawFrame(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                    u8g2_uint_t h) {
  u8g2_draw_frame_color(u, x, y, w, h, u->draw_color);
}

void u8g2_DrawRBox(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, u8g2_uint_t r) {
  u8g2_draw_rbox_color(u, x, y, w, h, r, u->draw_color);
}

void u8g2_DrawRFrame(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                     u8g2_uint_t h, u8g2_uint_t r) {
  u8g2_draw_rframe_color(u, x, y, w, h, r, u->draw_color);
}

void u8g2_DrawCircle(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                     u8g2_uint_t rad) {
  u8g2_draw_circle_color(u, x0, y0, rad, u->draw_color);
}

void u8g2_DrawDisc(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                   u8g2_uint_t rad) {
  u8g2_draw_disc_color(u, x0, y0, rad, u->draw_color);
}

void u8g2_DrawTriangle(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                       u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                       u8g2_uint_t y2) {
  u8g2_draw_triangle_color(u, x0, y0, x1, y1, x2, y2, u->draw_color);
}

void u8g2_DrawXBM(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h, const uint8_t *bitmap) {
  u8g2_draw_xbm_color(u, x, y, w, h, bitmap, u->draw_color);
}

void u8g2_DrawXBMP(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, const uint8_t *bitmap) {
  u8g2_draw_xbm_color(u, x, y, w, h, bitmap, u->draw_color);
}

/* ==================== 字体解码 ==================== */
/* 字位读取：MSB-first，从解码指针处连续读取 cnt 位 */
static uint8_t u8g2_font_decode_get_unsigned_bits(u8g2_t *u, uint8_t cnt) {
  uint8_t val;
  uint8_t bit_pos = u->font_decode.decode_bit_pos;
  const uint8_t *ptr = u->font_decode.decode_ptr;

  val = (uint8_t)(ptr[0] >> bit_pos);
  if (bit_pos + cnt >= 8) {
    uint8_t s = (uint8_t)(8 - bit_pos);
    ptr++;
    val |= (uint8_t)((uint8_t)(ptr[0]) << s); /* uint8 左移自动截断，勿改 */
  }
  val &= (uint8_t)((1 << cnt) - 1);
  u->font_decode.decode_bit_pos =
      (uint8_t)((bit_pos + cnt) & 7);
  u->font_decode.decode_ptr = ptr;
  return val;
}

static int8_t u8g2_font_decode_get_signed_bits(u8g2_t *u, uint8_t cnt) {
  int8_t val = (int8_t)u8g2_font_decode_get_unsigned_bits(u, cnt);
  val -= (int8_t)(1 << (cnt - 1));
  return val;
}

/* 字体头解析（23 字节，全部字段在 u8g2.h 的 u8g2_font_info_t 中） */
static void u8g2_font_decode_init(u8g2_t *u) {
  const uint8_t *font = u->font;
  u8g2_font_info_t *fi = &u->font_info;

  fi->glyph_cnt = font[0];
  fi->bbx_mode = font[1];
  fi->bits_per_0 = font[2];
  fi->bits_per_1 = font[3];
  fi->bits_per_char_width = font[4];
  fi->bits_per_char_height = font[5];
  fi->bits_per_char_x = font[6];
  fi->bits_per_char_y = font[7];
  fi->bits_per_delta_x = font[8];
  fi->max_char_width = font[9];
  fi->max_char_height = font[10];
  fi->x_offset = (int8_t)font[11];
  fi->y_offset = (int8_t)font[12];
  fi->ascent_A = font[13];
  fi->descent_g = font[14];
  fi->ascent_para = font[15];
  fi->descent_para = font[16];
  fi->start_pos_upper_A = (uint16_t)(((uint16_t)font[17] << 8) | font[18]);
  fi->start_pos_lower_a = (uint16_t)(((uint16_t)font[19] << 8) | font[20]);
  fi->start_pos_unicode = (uint16_t)(((uint16_t)font[21] << 8) | font[22]);
}

/* 大端 16 位读取 */
static uint16_t u8g2_font_get_word(const uint8_t *p) {
  return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/*
 * 定位字形数据：base+23 之后是跳转表（与 u8g2_font.c 的
 * u8g2_font_get_glyph_data 一致）。
 *  ASCII 区条目 [encoding, size]，size 含 2 字节头，size==0 终止；
 *  Unicode 区：查找表 [offset BE, encoding BE] 定位块，块内条目
 *  [encoding BE, size]（size 含 3 字节头），e==0 终止。
 */
static const uint8_t *u8g2_font_get_glyph_data(u8g2_t *u, uint16_t encoding) {
  const uint8_t *font = u->font + 23;
  const u8g2_font_info_t *fi = &u->font_info;

  if (encoding <= 255) {
    if (encoding >= 'a') {
      font += fi->start_pos_lower_a;
    } else if (encoding >= 'A') {
      font += fi->start_pos_upper_A;
    }
    for (;;) {
      if (font[1] == 0) {
        break;
      }
      if (font[0] == encoding) {
        return font + 2; /* 跳过 [encoding, size] */
      }
      font += font[1];
    }
    return NULL;
  }
  /* Unicode 区 */
  {
    uint16_t e;
    const uint8_t *entry;
    uint16_t cnt; /* 上界保护：无 unicode 字形的字体（LUT 仅哨兵）
                   * 查询任意编码会落到字形数据区，参考实现同样无界跳走，
                   * 这里用 glyph_cnt 封顶避免死循环 */

    font += fi->start_pos_unicode;
    /* 查找表逐项推进，直到块内最大编码 >= 请求编码 */
    do {
      entry = u->font + 23 + u8g2_font_get_word(font);
      e = u8g2_font_get_word(font + 2);
      font += 4;
    } while (e < encoding);

    /* 块内扫描字形条目 */
    cnt = 0;
    for (;;) {
      e = u8g2_font_get_word(entry);
      if (e == 0) {
        break;
      }
      if (e == encoding) {
        return entry + 3; /* 跳过 [encoding BE, size] */
      }
      if (cnt++ >= fi->glyph_cnt) {
        break;
      }
      entry += entry[2]; /* size 含 3 字节头 */
    }
  }
  return NULL;
}

/* 绘制一段水平运行（长度 len），is_foreground 区分前景/背景。
 * 超出一行宽度时自动折行，逻辑与 u8g2_font.c 的 decode_len 一致。 */
static void u8g2_font_decode_len(u8g2_t *u, uint8_t len,
                                 uint8_t is_foreground) {
  u8g2_font_decode_t *decode = &u->font_decode;
  uint8_t cnt = len;
  uint8_t lx = decode->x;
  uint8_t ly = decode->y;

  for (;;) {
    uint8_t rem = (uint8_t)(decode->glyph_width - lx);
    uint8_t current = (cnt < rem) ? cnt : rem;
    u8g2_uint_t x = (u8g2_uint_t)(decode->target_x + lx);
    u8g2_uint_t y = (u8g2_uint_t)(decode->target_y + ly);
    u8g2_uint_t i;

    if (is_foreground) {
      for (i = 0; i < current; i++) {
        u8g2_pix_set(u, (u8g2_uint_t)(x + i), y, decode->fg_color);
      }
    } else if (decode->is_transparent == 0) {
      for (i = 0; i < current; i++) {
        u8g2_pix_set(u, (u8g2_uint_t)(x + i), y, decode->bg_color);
      }
    }

    if (cnt < rem) {
      break;
    }
    cnt -= rem;
    lx = 0;
    ly++;
  }
  lx += cnt;

  decode->x = lx;
  decode->y = ly;
}

/* 初始化字形解码：读几何（宽度/高度），设定前景/背景色
 * （bg 恒为 fg 取反：实心模式下背景不与前景同色） */
static void u8g2_font_setup_decode(u8g2_t *u, const uint8_t *glyph_data,
                                   uint8_t fg_color) {
  u8g2_font_decode_t *decode = &u->font_decode;

  decode->decode_ptr = glyph_data;
  decode->decode_bit_pos = 0;

  decode->glyph_width =
      u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_char_width);
  decode->glyph_height =
      u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_char_height);

  decode->fg_color = fg_color;
  decode->bg_color = (uint8_t)(fg_color == 0 ? 1 : 0);
}

/*
 * 解码并绘制一个字形，返回横向步进 delta_x（有符号）。
 * 字形定位：target 以基线为锚，target_y -= h + y_offset。
 * RLE 运行段：每行 [背景长, 前景长] 交替，行尾带 1bit 继续标志，
 * 重复读段直到标志为 0；外层循环直到绘制完 h 行。
 */
static int8_t u8g2_font_decode_glyph(u8g2_t *u, const uint8_t *glyph_data,
                                     uint8_t fg_color) {
  uint8_t a, b;
  int8_t x, y;
  int8_t d;
  int8_t h;
  u8g2_font_decode_t *decode = &u->font_decode;

  u8g2_font_setup_decode(u, glyph_data, fg_color);
  h = (int8_t)u->font_decode.glyph_height;

  x = u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_char_x);
  y = u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_char_y);
  d = u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_delta_x);

  if (decode->glyph_width > 0) {
    decode->target_x += (u8g2_uint_t)x;
    decode->target_y -= (u8g2_uint_t)(h + y);

    /* 重置字形局部坐标 */
    decode->x = 0;
    decode->y = 0;

    /* 解码 RLE 运行段 */
    for (;;) {
      a = u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_0);
      b = u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_1);
      do {
        u8g2_font_decode_len(u, a, 0);
        u8g2_font_decode_len(u, b, 1);
      } while (u8g2_font_decode_get_unsigned_bits(u, 1) != 0);

      if (decode->y >= h) {
        break;
      }
    }

  }
  return d;
}

/* ==================== 文本绘制 ==================== */
/* 带色字形绘制核心：fg 为前景索引，背景由 setup_decode 取反；
 * 返回横向步进（advance），调用方内部累加。 */
static u8g2_uint_t u8g2_draw_glyph_color(u8g2_t *u, u8g2_uint_t x,
                                         u8g2_uint_t y, uint16_t encoding,
                                         uint8_t fg) {
  const uint8_t *glyph_data;

  if (u->font == NULL) return 0;

  u->font_decode.target_x = x;
  u->font_decode.target_y = y;
  u->font_decode.glyph_width = 0;
  u->font_decode.glyph_height = 0;

  glyph_data = u8g2_font_get_glyph_data(u, encoding);
  if (glyph_data != NULL) {
    return (u8g2_uint_t)u8g2_font_decode_glyph(u, glyph_data, fg);
  }
  return 0;
}

u8g2_uint_t ui_draw_glyph(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding,
                          uint8_t color) {
  return u8g2_draw_glyph_color(&u8g2, x, y, encoding, color);
}

u8g2_uint_t u8g2_DrawGlyph(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                           uint16_t encoding) {
  return u8g2_draw_glyph_color(u, x, y, encoding, u->draw_color);
}

u8g2_uint_t u8g2_DrawStr(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                         const char *str) {
  while (*str != '\0') {
    x += u8g2_draw_glyph_color(u, x, y, (uint16_t)(uint8_t)(*str),
                               u->draw_color);
    str++;
  }
  return x;
}

/* 简易 UTF-8 解码（1~4 字节） */
static uint32_t u8g2_utf8_next(const char **sp) {
  const uint8_t *s = (const uint8_t *)*sp;
  uint32_t cp;

  if (s[0] < 0x80) {
    *sp = (const char *)(s + 1);
    return s[0];
  }
  if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
    cp = ((uint32_t)(s[0] & 0x1F) << 6) | (s[1] & 0x3F);
    *sp = (const char *)(s + 2);
    return cp;
  }
  if ((s[0] & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 &&
      (s[2] & 0xC0) == 0x80) {
    cp = ((uint32_t)(s[0] & 0x0F) << 12) | ((uint32_t)(s[1] & 0x3F) << 6) |
         (s[2] & 0x3F);
    *sp = (const char *)(s + 3);
    return cp;
  }
  if ((s[0] & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 &&
      (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
    cp = ((uint32_t)(s[0] & 0x07) << 18) | ((uint32_t)(s[1] & 0x3F) << 12) |
         ((uint32_t)(s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    *sp = (const char *)(s + 4);
    return cp;
  }
  /* 非法序列：跳过一个字节 */
  *sp = (const char *)(s + 1);
  return 0xFFFD;
}

u8g2_uint_t u8g2_DrawUTF8(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                          const char *str) {
  const char *p = str;
  while (*p != '\0') {
    x += u8g2_draw_glyph_color(u, x, y, (uint16_t)u8g2_utf8_next(&p),
                               u->draw_color);
  }
  return x;
}

u8g2_uint_t ui_draw_str(u8g2_uint_t x, u8g2_uint_t y, const char *str,
                        uint8_t color) {
  while (*str != '\0') {
    x += u8g2_draw_glyph_color(&u8g2, x, y, (uint16_t)(uint8_t)(*str), color);
    str++;
  }
  return x;
}

u8g2_uint_t ui_draw_utf8(u8g2_uint_t x, u8g2_uint_t y, const char *str,
                         uint8_t color) {
  const char *p = str;
  while (*p != '\0') {
    x += u8g2_draw_glyph_color(&u8g2, x, y, (uint16_t)u8g2_utf8_next(&p),
                               color);
  }
  return x;
}

/* ==================== 文本测量 ==================== */
/* 返回有符号步进 delta_x（与 u8g2 一致）；副带副作用：更新 font_decode 与 x 偏移 */
int8_t u8g2_GetGlyphWidth(u8g2_t *u, uint16_t encoding) {
  const uint8_t *glyph_data = u8g2_font_get_glyph_data(u, encoding);
  if (glyph_data == NULL) {
    return 0;
  }
  u8g2_font_setup_decode(u, glyph_data, 0); /* 测量只读几何，颜色无关 */
  u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_char_x);
  u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_char_y);
  /* 字形宽度在 u->font_decode.glyph_width 中 */
  return u8g2_font_decode_get_signed_bits(u, u->font_info.bits_per_delta_x);
}

u8g2_uint_t u8g2_GetStrWidth(u8g2_t *u, const char *s) {
  const char *p = s;
  u8g2_uint_t total = 0;

  if (u->font == NULL) return 0;
  while (*p != '\0') {
    total += (u8g2_uint_t)u8g2_GetGlyphWidth(u, (uint16_t)(uint8_t)(*p));
    p++;
  }
  return total;
}

u8g2_uint_t u8g2_GetUTF8Width(u8g2_t *u, const char *s) {
  const char *p = s;
  u8g2_uint_t total = 0;

  if (u->font == NULL) return 0;
  while (*p != '\0') {
    total += (u8g2_uint_t)u8g2_GetGlyphWidth(u, (uint16_t)u8g2_utf8_next(&p));
  }
  return total;
}

/* ==================== 字体度量 ==================== */
void u8g2_SetFont(u8g2_t *u, const uint8_t *font) {
  u->font = font;
  if (font != NULL) {
    u8g2_font_decode_init(u);
  }
}

void u8g2_SetFontPosBaseline(u8g2_t *u) { (void)u; }

uint8_t u8g2_GetFontAscent(u8g2_t *u) {
  if (u->font == NULL) return 0;
  return u->font_info.ascent_A;
}

uint8_t u8g2_GetFontDescent(u8g2_t *u) {
  if (u->font == NULL) return 0;
  return u->font_info.descent_g;
}

uint8_t u8g2_GetFontHeight(u8g2_t *u) {
  if (u->font == NULL) return 0;
  return (uint8_t)(u->font_info.ascent_A + u->font_info.descent_g);
}
