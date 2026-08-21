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

static uint8_t u8g2_pix_read(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  uint32_t bit = ((uint32_t)y * u->width + x) * U8G2_PORTING_BPP;
  uint32_t byte = bit >> 3;
  uint8_t sh = (uint8_t)(bit & 7);
  uint16_t val;

  val = u->pix_buf[byte] >> sh;
  if (sh + U8G2_PORTING_BPP > 8) {
    val |= (uint16_t)u->pix_buf[byte + 1] << (8 - sh); /* uint8 << 截断语义 */
  }
  return (uint8_t)(val & (U8G2_NUM_COLORS - 1));
}


static void u8g2_pix_write(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                           uint8_t color) {
  uint32_t bit = ((uint32_t)y * u->width + x) * U8G2_PORTING_BPP;
  uint32_t byte = bit >> 3;
  uint8_t sh = (uint8_t)(bit & 7);
  /* 颜色值先提升为 uint32 再左移，避免 uint8 截断丢失跨字节的高位 */
  uint32_t group = (uint32_t)(color & (U8G2_NUM_COLORS - 1)) << sh;

  u->pix_buf[byte] =
      (uint8_t)((u->pix_buf[byte] &
                 (uint8_t) ~((uint8_t)((U8G2_NUM_COLORS - 1) << sh))) |
                (uint8_t)(group & 0xFF));
  if (sh + U8G2_PORTING_BPP > 8) {
    /* 跨字节：剩余高位进入第二字节的低位 */
    u->pix_buf[byte + 1] =
        (uint8_t)((u->pix_buf[byte + 1] &
                   (uint8_t) ~((uint8_t)((U8G2_NUM_COLORS - 1) >> (8 - sh)))) |
                  (uint8_t)(group >> 8));
  }
}

/* ==================== 裁剪与像素绘制 ==================== */
static uint8_t u8g2_is_inside_clip(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  return x >= u->clip_x0 && x <= u->clip_x1 && y >= u->clip_y0 &&
         y <= u->clip_y1;
}

static void u8g2_draw_pixel(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  if (u8g2_is_inside_clip(u, x, y)) {
    u8g2_pix_write(u, x, y, u->draw_color);
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

/* 颜色索引 0 恒为背景色（黑色）：清空时填 0 即得背景 */
void u8g2_SendBuffer(u8g2_t *u) {
  uint16_t line[U8G2_PORTING_SCREEN_W];
  u8g2_uint_t y, x;

  for (y = 0; y < u->height; y++) {
    for (x = 0; x < u->width; x++) {
      line[x] = u->palette[u8g2_pix_read(u, x, y)];
    }
    LCD_WritePixels(0, y, u->width - 1, y, line, u->width);
  }
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
void u8g2_DrawPixel(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y) {
  u8g2_draw_pixel(u, x, y);
}

void u8g2_DrawHLine(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len) {
  u8g2_uint_t i;
  if (y < u->clip_y0 || y > u->clip_y1 || len == 0) {
    return;
  }
  for (i = 0; i < len; i++) {
    u8g2_draw_pixel(u, (u8g2_uint_t)(x + i), y);
  }
}

void u8g2_DrawVLine(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len) {
  u8g2_uint_t i;
  if (x < u->clip_x0 || x > u->clip_x1 || len == 0) {
    return;
  }
  for (i = 0; i < len; i++) {
    u8g2_draw_pixel(u, x, (u8g2_uint_t)(y + i));
  }
}

/* Bresenham 直线（u8g2 同款算法） */
void u8g2_DrawLine(u8g2_t *u, u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                   u8g2_uint_t y2) {
  u8g2_uint_t tmp;
  u8g2_uint_t x, y;
  u8g2_uint_t dx, dy;
  int8_t sx, sy;
  int16_t err, e2;

  if (x1 > x2) {
    tmp = x1; x1 = x2; x2 = tmp;
  }
  if (y1 > y2) {
    tmp = y1; y1 = y2; y2 = tmp;
  }
  dx = (u8g2_uint_t)(x2 - x1);
  dy = (u8g2_uint_t)(y2 - y1);
  if (dx == 0 && dy == 0) {
    u8g2_draw_pixel(u, x1, y1);
    return;
  }
  x = x1;
  y = y1;
  sx = (dx > 0) ? 1 : -1; /* 已排序保证 dx>=0，sx 恒为 1 */
  sy = (dy > 0) ? 1 : -1; /* 已排序保证 dy>=0，sy 恒为 1 */
  dx = (u8g2_uint_t)(dx * 2);
  dy = (u8g2_uint_t)(dy * 2);
  err = (int16_t)(dx - dy);
  for (;;) {
    u8g2_draw_pixel(u, x, y);
    if (x == x2 && y == y2) {
      break;
    }
    e2 = (int16_t)(2 * err);
    if (e2 > (int16_t)(-dy)) {
      err -= (int16_t)dy;
      x += (u8g2_uint_t)sx;
    }
    if (e2 < (int16_t)dx) {
      err += (int16_t)dx;
      y += (u8g2_uint_t)sy;
    }
  }
}

void u8g2_DrawBox(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h) {
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
      u8g2_pix_write(u, i, j, u->draw_color);
    }
  }
}

void u8g2_DrawFrame(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                    u8g2_uint_t h) {
  if (w < 2 || h < 2) {
    u8g2_DrawBox(u, x, y, w, h);
    return;
  }
  u8g2_DrawHLine(u, x, y, w);
  u8g2_DrawHLine(u, x, (u8g2_uint_t)(y + h - 1), w);
  u8g2_DrawVLine(u, x, y, h);
  u8g2_DrawVLine(u, (u8g2_uint_t)(x + w - 1), y, h);
}

/* 圆角矩形（四个角画 1/4 圆，r==0 退化为直角矩形） */
static void u8g2_draw_corner(u8g2_t *u, u8g2_uint_t cx, u8g2_uint_t cy,
                             int8_t sx, int8_t sy, u8g2_uint_t rad,
                             uint8_t filled) {
  u8g2_uint_t x, y;
  int16_t d;
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (filled ? (d <= (int16_t)(rad * rad)) : (d == (int16_t)(rad * rad))) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(cx + (int16_t)x * sx),
                        (u8g2_uint_t)(cy + (int16_t)y * sy));
      }
    }
  }
}

void u8g2_DrawRBox(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, u8g2_uint_t r) {
  if (r == 0) {
    u8g2_DrawBox(u, x, y, w, h);
    return;
  }
  if (w < 2 * r + 1 || h < 2 * r + 1) {
    u8g2_DrawBox(u, x, y, w, h);
    return;
  }
  /* 中间矩形 */
  u8g2_DrawBox(u, (u8g2_uint_t)(x + r), y, (u8g2_uint_t)(w - 2 * r), h);
  u8g2_DrawBox(u, x, (u8g2_uint_t)(y + r), r, (u8g2_uint_t)(h - 2 * r));
  u8g2_DrawBox(u, (u8g2_uint_t)(x + w - r), (u8g2_uint_t)(y + r), r,
               (u8g2_uint_t)(h - 2 * r));
  /* 四个角 */
  u8g2_draw_corner(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + r), -1, -1, r, 1);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + w - r - 1), (u8g2_uint_t)(y + r), 1, -1,
                   r, 1);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - r - 1), -1, 1,
                   r, 1);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + w - r - 1),
                   (u8g2_uint_t)(y + h - r - 1), 1, 1, r, 1);
}

void u8g2_DrawRFrame(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                     u8g2_uint_t h, u8g2_uint_t r) {
  if (r == 0) {
    u8g2_DrawFrame(u, x, y, w, h);
    return;
  }
  if (w < 2 * r + 1 || h < 2 * r + 1) {
    u8g2_DrawFrame(u, x, y, w, h);
    return;
  }
  u8g2_DrawHLine(u, (u8g2_uint_t)(x + r), y, (u8g2_uint_t)(w - 2 * r));
  u8g2_DrawHLine(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - 1),
                 (u8g2_uint_t)(w - 2 * r));
  u8g2_DrawVLine(u, x, (u8g2_uint_t)(y + r), (u8g2_uint_t)(h - 2 * r));
  u8g2_DrawVLine(u, (u8g2_uint_t)(x + w - 1), (u8g2_uint_t)(y + r),
                 (u8g2_uint_t)(h - 2 * r));
  u8g2_draw_corner(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + r), -1, -1, r, 0);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + w - r - 1), (u8g2_uint_t)(y + r), 1, -1,
                   r, 0);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + r), (u8g2_uint_t)(y + h - r - 1), -1, 1,
                   r, 0);
  u8g2_draw_corner(u, (u8g2_uint_t)(x + w - r - 1),
                   (u8g2_uint_t)(y + h - r - 1), 1, 1, r, 0);
}

void u8g2_DrawCircle(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                     u8g2_uint_t rad) {
  u8g2_uint_t x, y;
  int16_t d;
  if (rad == 0) {
    u8g2_draw_pixel(u, x0, y0);
    return;
  }
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (d == (int16_t)(rad * rad)) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 - y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 + y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 - y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 + y));
      }
    }
  }
}

void u8g2_DrawDisc(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                   u8g2_uint_t rad) {
  u8g2_uint_t x, y;
  int16_t d;
  if (rad == 0) {
    u8g2_draw_pixel(u, x0, y0);
    return;
  }
  for (y = 0; y <= rad; y++) {
    for (x = 0; x <= rad; x++) {
      d = (int16_t)((int16_t)x * x + (int16_t)y * y);
      if (d <= (int16_t)(rad * rad)) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 - y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 + x), (u8g2_uint_t)(y0 + y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 - y));
        u8g2_draw_pixel(u, (u8g2_uint_t)(x0 - x), (u8g2_uint_t)(y0 + y));
      }
    }
  }
}

void u8g2_DrawTriangle(u8g2_t *u, u8g2_uint_t x0, u8g2_uint_t y0,
                       u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2,
                       u8g2_uint_t y2) {
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
      u8g2_DrawHLine(u, a, y, (u8g2_uint_t)(b - a + 1));
    }
  }
}

/* ==================== XBM 位图 ==================== */
void u8g2_DrawXBM(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                  u8g2_uint_t h, const uint8_t *bitmap) {
  u8g2_uint_t i, j;
  u8g2_uint_t byte_per_row = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      uint8_t b = bitmap[j * byte_per_row + (i >> 3)];
      if (b & (0x80 >> (i & 7))) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(x + i), (u8g2_uint_t)(y + j));
      }
    }
  }
}

void u8g2_DrawXBMP(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                   u8g2_uint_t h, const uint8_t *bitmap) {
  u8g2_DrawXBM(u, x, y, w, h, bitmap);
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
      u->draw_color = decode->fg_color; /* 末尾统一恢复 */
      for (i = 0; i < current; i++) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(x + i), y);
      }
    } else if (decode->is_transparent == 0) {
      u->draw_color = decode->bg_color;
      for (i = 0; i < current; i++) {
        u8g2_draw_pixel(u, (u8g2_uint_t)(x + i), y);
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

/* 初始化字形解码：读几何（宽度/高度），设定前景/背景色 */
static void u8g2_font_setup_decode(u8g2_t *u, const uint8_t *glyph_data) {
  u8g2_font_decode_t *decode = &u->font_decode;

  decode->decode_ptr = glyph_data;
  decode->decode_bit_pos = 0;

  decode->glyph_width =
      u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_char_width);
  decode->glyph_height =
      u8g2_font_decode_get_unsigned_bits(u, u->font_info.bits_per_char_height);

  decode->fg_color = u->draw_color;
  decode->bg_color = (uint8_t)(decode->fg_color == 0 ? 1 : 0);
}

/*
 * 解码并绘制一个字形，返回横向步进 delta_x（有符号）。
 * 字形定位：target 以基线为锚，target_y -= h + y_offset。
 * RLE 运行段：每行 [背景长, 前景长] 交替，行尾带 1bit 继续标志，
 * 重复读段直到标志为 0；外层循环直到绘制完 h 行。
 */
static int8_t u8g2_font_decode_glyph(u8g2_t *u, const uint8_t *glyph_data) {
  uint8_t a, b;
  int8_t x, y;
  int8_t d;
  int8_t h;
  u8g2_font_decode_t *decode = &u->font_decode;

  u8g2_font_setup_decode(u, glyph_data);
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

    /* 解码过程会改写 draw_color，这里恢复 */
    u->draw_color = decode->fg_color;
  }
  return d;
}

/* ==================== 文本绘制 ==================== */
u8g2_uint_t u8g2_DrawGlyph(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                           uint16_t encoding) {
  u8g2_uint_t dx = 0;
  const uint8_t *glyph_data;

  if (u->font == NULL) return 0;

  u->font_decode.target_x = x;
  u->font_decode.target_y = y;
  u->font_decode.glyph_width = 0;
  u->font_decode.glyph_height = 0;

  glyph_data = u8g2_font_get_glyph_data(u, encoding);
  if (glyph_data != NULL) {
    dx = (u8g2_uint_t)u8g2_font_decode_glyph(u, glyph_data);
  }
  return dx; /* 返回步进（advance），u8g2_DrawStr 内部累加 */
}

u8g2_uint_t u8g2_DrawStr(u8g2_t *u, u8g2_uint_t x, u8g2_uint_t y,
                         const char *str) {
  while (*str != '\0') {
    x += u8g2_DrawGlyph(u, x, y, (uint16_t)(uint8_t)(*str));
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
    x += u8g2_DrawGlyph(u, x, y, (uint16_t)u8g2_utf8_next(&p));
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
  u8g2_font_setup_decode(u, glyph_data);
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
