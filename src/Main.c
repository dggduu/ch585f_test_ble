#include "CH58x_common.h"
#include "bsp_uart.h"
#include "bsp_io_ext.h"
#include "bsp_pin_defs.h"
#include "bsp_spi.h"
#include "bsp_lcd_hw.h"
#include "u8g2.h"
#include "bsp_pikaScript.h"

/* PCA9539 扩展 IO 的 I2C 地址（7 位地址） */
uint8_t g_ioext_addr = 0x74;

/* 简单延时（毫秒级） */
static void delay_ms(uint32_t ms) {
    while(ms--) mDelaymS(1);
}

int main() {
    /* 1. 系统时钟初始化（外部晶振 32MHz） */
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(SYSCLK_FREQ);

    /* 2. 调试串口初始化（UART0，PB4/PB7，115200） */
    BSP_UART_Init(115200);
    PRINT("\r\n========== CH585F u8g2 color porting ==========\r\n");
    PRINT("System Clock: %ld Hz\r\n", GetSysClock());

    /* 3. 初始化 PCA9539 扩展 IO（控制屏幕 CS/DC/RST/BLC） */
    PRINT("Initializing PCA9539... ");
    BSP_IO_EXT_Init(g_ioext_addr);
    BSP_IO_EXT_ConfigPort(g_ioext_addr, 0, 0x00);   // Port0 全为输出
    // 未使用的引脚设为输入（省电）
    for (int i = 5; i < 8; i++) {
        BSP_IO_EXT_SetPinDirection(g_ioext_addr, 0, i, 0);
    }
    PRINT("OK\r\n");

    /* 4. 初始化 SPI（硬件 SPI0，PA13/PA14/PA15） */
    PRINT("Initializing SPI... ");
    bsp_spi_init();
    PRINT("OK\r\n");

    /* 5. 初始化 LCD 与显示框架（u8g2 风格彩色移植层） */
    PRINT("Initializing LCD... ");
    Lcd_Init();
    u8g2_porting_init(&u8g2);
    PRINT("OK\r\n");

    /* 6. 混合绘制演示：
     *    - u8g2_* 系列：u8g2 兼容签名，默认白色文字
     *      （颜色由 u8g2_SetDrawColor 控制，默认索引 1 = 白）
     *    - ui_* 系列：显式携带颜色索引（ui_color_t 枚举，见 u8g2.h） */
    u8g2_SetFont(&u8g2, u8g2_font_8x13_tr);

    ui_clear();
    /* 默认白字：u8g2 兼容调用 */
    u8g2_DrawStr(&u8g2, 10, 14, "CH585F u8g2 porting");
    /* 显式带色：ui_* 调用 */
    ui_draw_str(10, 32, "ui_* colored text", UI_COLOR_GREEN);
    ui_draw_str(10, 50, "red green blue yellow", UI_COLOR_RED);

    /* 彩色图元 */
    ui_draw_frame(10, 62, 220, 40, UI_COLOR_CYAN);
    ui_draw_box(20, 72, 40, 20, UI_COLOR_RED);
    ui_draw_box(70, 72, 40, 20, UI_COLOR_GREEN);
    ui_draw_box(120, 72, 40, 20, UI_COLOR_BLUE);
    ui_draw_circle(200, 82, 12, UI_COLOR_YELLOW);

    ui_draw_line(10, 116, 230, 136, UI_COLOR_MAGENTA);
    ui_draw_disc(30, 150, 10, UI_COLOR_CYAN);
    ui_draw_triangle(100, 170, 140, 170, 120, 140, UI_COLOR_GREEN);

    /* 整屏索引缓冲一次刷新（内部 LCD_SendBuffer，无逐行窗口切换） */
    ui_send_buffer();
    delay_ms(2000);

    /* 7. PikaScript 初始化（PIKASCRIPT_ENABLE=0 时为空操作，不生成调用） */
#if PIKASCRIPT_ENABLE
    BSP_PikaScript_Init();
#endif

    /* 8. 主循环：白色标题 + 彩色方块滚动演示 */
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_uint_t bx = 0;
    while (1) {
        ui_clear();
        /* u8g2_* 默认白字 */
        u8g2_DrawStr(&u8g2, 20, 232, "CH585F u8g2 + ui color demo");
        /* ui_* 彩色 */
        ui_draw_box(bx, 100, 30, 30, UI_COLOR_RED);
        ui_draw_rframe(bx + 34, 100, 30, 30, 6, UI_COLOR_YELLOW);
        ui_draw_disc(bx + 68, 115, 15, UI_COLOR_GREEN);
        ui_send_buffer();

        bx += 4;
        if (bx > 200) bx = 0;
        delay_ms(30);
    }
}
