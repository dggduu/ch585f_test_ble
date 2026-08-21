#include "bsp_lcd_hw.h"
#include "bsp_spi.h"
#include "bsp_pin_defs.h"          // 提供 SCREEN_CS_SET/CLR, DC, RST, BLC 宏
#include "bsp_lcd_font_chinese.h"
#include "CH58x_common.h"          // 提供 mDelaymS
#include "bsp_uart.h"

// 背景色全局变量
uint16_t BACK_COLOR = LCD_BLACK;

// SPI写字节（使用BSP SPI驱动）
static void LCD_SPI_WriteByte(uint8_t dat)
{
    bsp_spi_transfer_byte(dat);    // 发送并忽略接收
}

// 写命令
static void LCD_WR_REG(uint8_t dat)
{
    SCREEN_CS_CLR();
    SCREEN_DC_CLR();               // 命令模式
    LCD_SPI_WriteByte(dat);
    SCREEN_CS_SET();
}

// 写8位数据
static void LCD_WR_DATA8(uint8_t dat)
{
    SCREEN_CS_CLR();
    SCREEN_DC_SET();               // 数据模式
    LCD_SPI_WriteByte(dat);
    SCREEN_CS_SET();
}

// 写16位数据（高字节先发）
static void LCD_WR_DATA(uint16_t dat)
{
    SCREEN_CS_CLR();
    SCREEN_DC_SET();
    LCD_SPI_WriteByte(dat >> 8);
    LCD_SPI_WriteByte(dat);
    SCREEN_CS_SET();
}

static void LCD_WriteDataBulk(uint16_t color, uint32_t count)
{
    uint8_t ch = color >> 8;
    uint8_t cl = color & 0xFF;

    SCREEN_CS_CLR();      // 只拉低一次
    SCREEN_DC_SET();      // 只设置一次

    while (count--) {
        SPI0_MasterSendByte(ch);
        SPI0_MasterSendByte(cl);
    }

    SCREEN_CS_SET();      // 最后再拉高
}

// 设置窗口地址
static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (USE_HORIZONTAL == 1) {
        y1 += 40;
        y2 += 40;
    }
    LCD_WR_REG(0x2A);            // 列地址
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2B);            // 行地址
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2C);            // 开始写显存
}

// LCD初始化
void Lcd_Init(void)
{
    SCREEN_BLC_CLR();
    // 关闭片选、DC默认高
    SCREEN_CS_SET();
    SCREEN_DC_SET();

    // ---- 复位 LCD ----
    SCREEN_RST_SET();
    mDelaymS(10);
    SCREEN_RST_CLR();       // 拉低复位
    mDelaymS(20);
    SCREEN_RST_SET();       // 拉高，结束复位
    mDelaymS(100);          // 等待内部稳定

    //BOE154IPS ST7789V2初始化//			
    LCD_WR_REG(0x11);
    mDelaymS(120);//delay_ms 120ms
    //--------------------------------------Display Setting------------------------------------------//
    LCD_WR_REG(0x36);
    if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
    else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
    else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
    else LCD_WR_DATA8(0xA0);
    LCD_WR_REG(0x3a);
    LCD_WR_DATA8(0x05);
    LCD_WR_REG(0x21);
    LCD_WR_REG(0x2a);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xef);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xef);
    //--------------------------------ST7789V Frame rate setting----------------------------------//
    LCD_WR_REG(0xb2);
    LCD_WR_DATA8(0x0c);
    LCD_WR_DATA8(0x0c);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);
    LCD_WR_REG(0xb7);
    LCD_WR_DATA8(0x35);
    //---------------------------------ST7789V Power setting--------------------------------------//
    LCD_WR_REG(0xbb);
    LCD_WR_DATA8(0x1f);
    LCD_WR_REG(0xc0);
    LCD_WR_DATA8(0x2c);
    LCD_WR_REG(0xc2);
    LCD_WR_DATA8(0x01);
    LCD_WR_REG(0xc3);
    LCD_WR_DATA8(0x12);
    LCD_WR_REG(0xc4);
    LCD_WR_DATA8(0x20);
    LCD_WR_REG(0xc6);
    LCD_WR_DATA8(0x0f);
    LCD_WR_REG(0xd0);
    LCD_WR_DATA8(0xa4);
    LCD_WR_DATA8(0xa1);
    //--------------------------------ST7789V gamma setting--------------------------------------//
    LCD_WR_REG(0xe0);
    LCD_WR_DATA8(0xd0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0c);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x39);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x50);
    LCD_WR_DATA8(0x36);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x29);
    LCD_WR_DATA8(0x2d);
    LCD_WR_REG(0xe1);
    LCD_WR_DATA8(0xd0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x39);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x0b);
    LCD_WR_DATA8(0x16);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x2f);
    LCD_WR_DATA8(0x31);

    LCD_WR_REG(0x2A); //Column Address Set
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00); //0
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xEF); //239

    LCD_WR_REG(0x2B); //Row Address Set
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00); //0
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xEF); //239

    LCD_WR_REG(0x29);	//Display on	

    SCREEN_BLC_SET();             // 点亮背光
}

// 发送一块颜色数据（连续 count 个像素，每个像素 16 位）
// 该函数会先设置 CS=0, DC=1，发送完后恢复 CS=1
static void LCD_WriteDataBlock(uint16_t color, uint32_t count)
{
    if (count == 0) return;

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // 只切换一次 CS 和 DC
    SCREEN_CS_CLR();
    SCREEN_DC_SET();

    // 为了提高效率，可以将颜色字节对填充到一个缓冲区，然后一次发送
    // 这里使用一个较小的静态缓冲区，避免占用过多栈空间
    static uint8_t buf[64];  // 32 像素 * 2 字节 = 64 字节
    uint32_t i = 0;
    while (i < count) {
        uint32_t chunk = count - i;
        if (chunk > 32) chunk = 32;  // 每次发送 32 个像素

        // 填充缓冲区
        uint16_t idx = 0;
        for (uint32_t j = 0; j < chunk; j++) {
            buf[idx++] = hi;
            buf[idx++] = lo;
        }

        bsp_spi_send_bulk(buf, chunk * 2);
        i += chunk;
    }

    SCREEN_CS_SET();
}

// 优化后的快速清屏/填充函数
void LCD_Clear(uint16_t Color)
{
    uint32_t total_pixels = (uint32_t)LCD_W * LCD_H;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    LCD_WriteDataBlock(Color, total_pixels);
}

// 填充矩形
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t width = xend - xsta + 1;
    uint16_t height = yend - ysta + 1;
    uint32_t total = (uint32_t)width * height;
    LCD_Address_Set(xsta, ysta, xend, yend);
    LCD_WriteDataBlock(color, total);
}

// 批量写像素（供 u8g2 显示框架调用）：设置窗口后一次发送 RGB565 像素数组。
// 逐行刷新时窗口为 (0,y)-(W-1,y)，count 必须等于 (x2-x1+1)*(y2-y1+1)。
// 分块打包发送，480 字节静态缓冲可一次容纳整行 240 像素。
void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                     const uint16_t *pixels, uint32_t count)
{
    if (count == 0) return;

    LCD_Address_Set(x1, y1, x2, y2);

    static uint8_t buf[512];
    uint32_t i = 0;

    SCREEN_CS_CLR();
    SCREEN_DC_SET();

    while (i < count) {
        uint32_t chunk = count - i;
        uint32_t nbytes = chunk * 2;
        if (nbytes > sizeof(buf)) {
            chunk = sizeof(buf) / 2;
            nbytes = sizeof(buf);
        }

        uint16_t idx = 0;
        for (uint32_t j = 0; j < chunk; j++) {
            uint16_t c = pixels[i + j];
            buf[idx++] = (uint8_t)(c >> 8);
            buf[idx++] = (uint8_t)(c & 0xFF);
        }

        bsp_spi_send_bulk(buf, (uint16_t)nbytes);
        i += chunk;
    }

    SCREEN_CS_SET();
}

// 画点
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA(color);
}

// 画大点（3x3）
void LCD_DrawPoint_big(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Fill(x - 1, y - 1, x + 1, y + 1, color);
}

// 画线（Bresenham）
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0) incx = 1;
    else if (delta_x == 0) incx = 0;
    else { incx = -1; delta_x = -delta_x; }

    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0;
    else { incy = -1; delta_y = -delta_y; }

    if (delta_x > delta_y) distance = delta_x;
    else distance = delta_y;

    for (t = 0; t < distance + 1; t++) {
        LCD_DrawPoint(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

// 圆（中点画圆）
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a = 0, b = r;
    while (a <= b) {
        LCD_DrawPoint(x0 - b, y0 - a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        a++;
        if ((a * a + b * b) > (r * r)) b--;
    }
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color)
{
    uint8_t temp, pos, t;
    if (x > LCD_W - 16 || y > LCD_H - 16) return;

    num -= ' ';
    LCD_Address_Set(x, y, x + 8 - 1, y + 16 - 1);

    if (!mode) {   // 非叠加模式
        // 缓冲区：16 行 × 8 列 × 2 字节 = 256 字节
        static uint8_t charBuf[256];
        uint16_t idx = 0;

        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                uint16_t c = (temp & 0x01) ? color : BACK_COLOR;
                charBuf[idx++] = c >> 8;
                charBuf[idx++] = c & 0xFF;
                temp >>= 1;
            }
        }

        // 切换 CS/DC 一次，发送所有数据
        SCREEN_CS_CLR();
        SCREEN_DC_SET();
        bsp_spi_send_bulk(charBuf, 256);
        SCREEN_CS_SET();
    } else {      // 叠加模式：逐点画（使用原有方式，但也可优化）
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01) LCD_DrawPoint(x + t, y + pos, color);
                temp >>= 1;
            }
        }
    }
}

// 显示字符串
void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t color)
{
    while (*p != '\0') {
        if (x > LCD_W - 16) { x = 0; y += 16; }
        if (y > LCD_H - 16) { y = x = 0; LCD_Clear(LCD_RED); }
        LCD_ShowChar(x, y, *p, 0, color);
        x += 8;
        p++;
    }
}

// 计算10的幂
static uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

// 显示整数
void LCD_ShowNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + 8 * t, y, ' ', 0, color);
                continue;
            } else enshow = 1;
        }
        LCD_ShowChar(x + 8 * t, y, temp + '0', 0, color);
    }
}

// 显示小数（保留两位）
void LCD_ShowNum1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    uint16_t num1 = (uint16_t)(num * 100);
    for (t = 0; t < len; t++) {
        temp = (num1 / mypow(10, len - t - 1)) % 10;
        if (t == (len - 2)) {
            LCD_ShowChar(x + 8 * (len - 2), y, '.', 0, color);
            t++;
            len++;
        }
        LCD_ShowChar(x + 8 * t, y, temp + '0', 0, color);
    }
}


// 显示彩条
void LCD_DispBand(void)
{
    uint16_t i, j, k;
    uint16_t color[8] = {0x001F, 0x07E0, 0xF800, 0x07FF, 0xF81F, 0xFFE0, 0x0000, 0xFFFF};
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < LCD_H / 8; j++) {
            for (k = 0; k < LCD_W; k++) {
                LCD_WR_DATA(color[i]);
            }
        }
    }
    for (j = 0; j < LCD_H % 8; j++) {
        for (k = 0; k < LCD_W; k++) {
            LCD_WR_DATA(color[7]);
        }
    }
}

// 显示灰度水平条
void LCD_DispGrayHor16(void)
{
    uint16_t i, j, k;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for (i = 0; i < LCD_H; i++) {
        for (j = 0; j < LCD_W % 8; j++) {
            LCD_WR_DATA(0);
        }
        for (j = 0; j < 16; j++) {
            for (k = 0; k < LCD_W / 16; k++) {
                LCD_WR_DATA8(((j * 2) << 3) | ((j * 4) >> 3));
                LCD_WR_DATA8(((j * 4) << 5) | (j * 2));
            }
        }
    }
}

// 显示雪花（棋盘）
void LCD_DispSnow(void)
{
    uint16_t i, j;
    uint16_t dat = 0;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for (i = 0; i < LCD_H; i++) {
        for (j = 0; j < LCD_W / 2; j++) {
            LCD_WR_DATA(dat);
            LCD_WR_DATA(~dat);
        }
        dat = ~dat;
    }
}

// 显示方块
void LCD_DispBlock(void)
{
    uint16_t i, j, k;
    k = LCD_H / 4;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for (i = 0; i < k; i++) {
        for (j = 0; j < LCD_W; j++) LCD_WR_DATA(0x7BEF);
    }
    for (i = 0; i < k * 2; i++) {
        for (j = 0; j < LCD_W / 4; j++) LCD_WR_DATA(0x7BEF);
        for (j = 0; j < LCD_W / 2; j++) LCD_WR_DATA(0x0000);
        for (j = 0; j < LCD_W / 4; j++) LCD_WR_DATA(0x7BEF);
    }
    for (i = 0; i < k; i++) {
        for (j = 0; j < LCD_W; j++) LCD_WR_DATA(0x7BEF);
    }
}

// 整屏索引缓冲刷新（u8g2 框架主刷新路径）
// index_buf: 每像素 bpp 位的调色板索引，LSB-first 组打包（与 u8g2_porting 的
//             pix_buf 布局一致：像素 bit 偏移 = (y*width+x)*bpp）
// palette:   2^bpp 项 RGB565 调色板
// 说明: 一次设置窗口 + CS/DC 后连续发送，逐像素解出索引并经调色板映射为大端
//       RGB565，分块批量 SPI 发送，避免逐行窗口切换。
void LCD_SendBuffer(const uint8_t *index_buf, const uint16_t *palette, uint16_t width, uint16_t height, uint8_t bpp)
{
    uint32_t total_pixels = (uint32_t)width * height;
    uint8_t mask = (uint8_t)((1u << bpp) - 1);   // bpp=8 时 (1u<<8)-1=255 不溢出

    LCD_Address_Set(0, 0, width - 1, height - 1);
    SCREEN_CS_CLR();
    SCREEN_DC_SET();

    static uint8_t send_buf[512];
    uint16_t buf_idx = 0;

    for (uint32_t pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) {
        uint32_t bit_pos = pixel_idx * bpp;
        uint32_t byte_idx = bit_pos >> 3;
        uint8_t bit_offset = (uint8_t)(bit_pos & 0x07);
        uint8_t idx;
        if (bit_offset + bpp <= 8) {
            /* LSB-first 打包：索引低位在字节低位，右移 bit_offset 即得 */
            idx = (uint8_t)(index_buf[byte_idx] >> bit_offset) & mask;
        } else {
            /* 跨字节：低位在本字节 [bit_offset..7]，高位在下一字节开头 */
            idx = (uint8_t)((index_buf[byte_idx] >> bit_offset) |
                            (index_buf[byte_idx + 1] << (8 - bit_offset)));
            idx &= mask;
        }

        uint16_t rgb = palette[idx];
        send_buf[buf_idx++] = (uint8_t)(rgb >> 8);
        send_buf[buf_idx++] = (uint8_t)(rgb & 0xFF);

        if (buf_idx >= sizeof(send_buf)) {
            bsp_spi_send_bulk(send_buf, buf_idx);
            buf_idx = 0;
        }
    }

    if (buf_idx > 0) {
        bsp_spi_send_bulk(send_buf, buf_idx);
    }

    SCREEN_CS_SET();
}