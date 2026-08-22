#include "bsp_board.h"
#include "CH58x_common.h"
#include "bsp_i2c.h"
#include "bsp_io_ext.h"
#include "bsp_lcd_hw.h"
#include "bsp_pin_defs.h"
#include "bsp_spi.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "u8g2_porting.h"
#include "u8g2.h"
#include "HAL.h"
/**
 * @brief bsp_borad_init
 * 
 * @return uint8_t 
 */
uint8_t bsp_borad_init(){
    /* 1. 系统时钟初始化（外部晶振 32MHz） */
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(SYSCLK_FREQ);

    
    // HAL_Init();

    /* 2. 调试串口初始化（UART0，PB4/PB7，115200） */
    BSP_UART_Init(115200);

    BSP_Timer_Init();
    // bsp_timer_id_t timer1 = BSP_Timer_Create(500, true, my_timer_cb, NULL);
    // BSP_Timer_Start(timer1);

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
    LCD_Clear(LCD_BLACK);
    u8g2_porting_init(&u8g2);
    PRINT("OK\r\n");

    return 0;
}