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
// Task
#include "task/Inc/task_ui.h"
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
    /* 单次调度：循环由 main() 的 while(1) 驱动，
     * 这样主循环里还能穿插执行 uart_btn_process() 等非阻塞轮询 */
    TMOS_SystemProcess();
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
  bsp_borad_init();      // 时钟 / UART / SPI / LCD（不涉及 TMOS，可最先执行）

  /* 重要：初始化顺序必须与官方 CH585EVT 一致 —— CH58x_BLEInit() 先于 HAL_Init()。
   * CH58x_BLEInit() → BLE_LibInit() → TMOS_Init() 会清空整个 TMOS 状态
   * （任务表 / 定时器链表 / 时钟回调 fnGetClockCBs 全部清零）。
   * 若先执行 HAL_Init()，其中注册的 TMOS 时钟（HAL_TimeInit→TMOS_TimerInit）
   * 会被 TMOS_Init 清掉，导致 tmos_start_task() 静默失败、
   * TMOS_SystemProcess() 永不推进定时器，UI_ProcessEvent 永远不被调用。 */
  CH58x_BLEInit();
  HAL_Init();

  GAPRole_BroadcasterInit();
  Broadcaster_Init();
  UI_Task_Init();
  while (1) {
    uart_btn_process();  // 非阻塞串口按键扫描
    Main_Circulation();  // TMOS 系统调度
  }
}
