#include "CH58x_common.h"
#include "bsp_uart.h"
#include "bsp_io_ext.h"
#include "bsp_pin_defs.h"
#include "bsp_spi.h"
#include "bsp_lcd_hw.h"

#include "pikaScript.h"
/* 定义 PCA9539 的 I2C 地址（7 位） */
uint8_t g_ioext_addr = 0x74;

/* 简单的延时（毫秒） */
static void delay_ms(uint32_t ms) {
    while(ms--) mDelaymS(1);
}

int main() {
    /* 1. 系统时钟初始化（外部晶振 32MHz） */
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(SYSCLK_FREQ);


    /* 2. 调试串口初始化（UART0，PB4/PB7，115200） */
    BSP_UART_Init(115200);
    PRINT("\r\n========== CH585F BSP Test ==========\r\n");
    PRINT("System Clock: %ld Hz\r\n", GetSysClock());


    /* 3. 初始化 PCA9539（扩展 I/O） */
    PRINT("Initializing PCA9539... ");
    BSP_IO_EXT_Init(g_ioext_addr);
    BSP_IO_EXT_ConfigPort(g_ioext_addr, 0, 0x00);   // Port0 全部输出
    // 将未使用的引脚设为输入（省电）
    for (int i = 5; i < 8; i++) {
        BSP_IO_EXT_SetPinDirection(g_ioext_addr, 0, i, 0);
    }
    
    PRINT("OK\r\n");

    /* 4. 初始化 SPI（硬件 SPI0，PA13/PA14/PA15） */
    PRINT("Initializing SPI... ");
    bsp_spi_init();
    PRINT("OK\r\n");

    /* 5. 初始化 LCD（ST7789V，240x240） */
    PRINT("Initializing LCD... ");
    Lcd_Init();
    LCD_Clear(LCD_BLUE);
    PRINT("CON\r\n");
    LCD_ShowString(10, 10, "Hello CH585!", LCD_WHITE);
    LCD_ShowString(10, 30, "LCD Test OK", LCD_GREEN);

    PRINT("OK\r\n");

    // /* 9. 主循环：显示一些动态信息（例如时间或计数） */
    uint32_t counter = 0;
    char buf[32];

    BSP_PikaScript_Init();
    BSP_PikaScript_RunString("print('Hello from PikaPython!')");

    while (1) {
        // 在 LCD 右上角显示计数
        sprintf(buf, "Cnt: %lu", counter++);
        LCD_ShowString(180, 10, buf, LCD_YELLOW);
        delay_ms(2000);
    }
}