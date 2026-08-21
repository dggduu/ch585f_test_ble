#include "CH58x_common.h"
#include "bsp_uart.h"
#include "bsp_io_ext.h"
#include "bsp_pin_defs.h"
#include "bsp_spi.h"
#include "lcd_driver.h"
#include "lcd_wegui_driver.h"
#include "wegui_menu_demo.h"

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

    /* 5. 初始化 WeGui RGB 图形库（ST7789V，240x240） */
    PRINT("Initializing WeGui RGB... ");
    lcd_driver_init();          // 屏幕驱动初始化（SPI+复位+背光+IC）
    lcd_wegui_init();           // wegui 图形库初始化
    wegui.menu = &m_main;       // 开机初始菜单
    PRINT("OK\r\n");


    /* 6. TMR0 1ms 中断：驱动 wegui 软件计时 */
    TMR0_TimerInit(FREQ_SYS / 1000);   // 1ms 中断周期
    TMR0_ITCfg(ENABLE, RB_TMR_IE_CYC_END);
    PFIC_EnableIRQ(TMR0_IRQn);

    while (1) {
        // wegui 主循环：菜单绘制/动画/动态刷新
        wegui_loop_func();
    }
}

/*--------------------------------------------------------------
 * TMR0 1ms 中断：驱动 wegui 软件计时
----------------------------------------------------------------*/
void TMR0_IRQHandler(void)
{
    TMR0_ClearITFlag(RB_TMR_IE_CYC_END);
    wegui_1ms_stick();
}
