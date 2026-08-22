// HAL
#include "CH58x_common.h"
#include "hal/include/CONFIG.h"
#include "hal/include/HAL.h"
//BSP
#include "bsp_board.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
// MiddleWare
#include "middleware_pikaScript.h"
// Compenonts
#include "page_stack.h"
// APP
#include "app_splash_screen.h"
#include "broadcaster.h"


/* PCA9539 扩展 IO 的 I2C 地址（7 位地址） */
uint8_t g_ioext_addr = 0x74;

/* 简单延时（毫秒级） */
static void delay_ms(uint32_t ms) {
    while(ms--) mDelaymS(1);
}

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] =
    {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
    }
}

// ===================== 串口按键处理 =====================
static void uart_btn_process(void) {
  char ch = BSP_UART_RecvByteNonBlock();
  if (ch != 0) {
    switch (ch) {
    case 'w':
      btn_fifo_push(BTN_UP);
      break;
    case 's':
      btn_fifo_push(BTN_DOWN);
      break;
    case 'a':
      btn_fifo_push(BTN_ENTER);
      break;
    case 'd':
      btn_fifo_push(BTN_BACK);
      break;
    case 'h':
      btn_fifo_push(BTN_LONG_PRESS);
      break;
    case 'l':
      btn_fifo_push(BTN_LEFT);
      break;
    case 'r':
      btn_fifo_push(BTN_RIGHT);
      break;
    default:
      break;
    }
    printf("rev:%c\r\n", ch);
  }
}

int main() {
  bsp_borad_init();
    CH58x_BLEInit();
    HAL_Init();
    GAPRole_BroadcasterInit();
    Broadcaster_Init();
  app_splash_screen_entry();
  while (1) {
    BSP_Timer_Tick(); 
    uart_btn_process();
    btn_type_t btn = btn_fifo_pop();
    page_update(&g_page_stack, btn);
    Main_Circulation();
  }
}
