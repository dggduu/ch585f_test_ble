#include "bsp_ec11.h"
#include "CH58x_common.h"
#include "btn_fifo.h"
#include "bsp_timer.h"    // 使用软件定时器接口

// ==================== 硬件引脚定义 ====================
#define EC11_A_PORT     GPIOB
#define EC11_A_PIN      GPIO_Pin_8
#define EC11_B_PORT     GPIOB
#define EC11_B_PIN      GPIO_Pin_17
#define EC11_KEY_PORT   GPIOB
#define EC11_KEY_PIN    GPIO_Pin_9      // 低电平按下

// ==================== 状态变量 ====================
static uint8_t s_last_a = 0;
static uint8_t s_key_debounce = 0;
static bool    s_key_pressed = false;
static uint8_t s_step = 1;

// 软件定时器 ID
static bsp_timer_id_t s_scan_timer_id = -1;



// ==================== EC11 扫描函数 ====================
void BSP_EC11_Scan(void)
{
    // ---------- 1. 旋转检测 ----------
    uint8_t a = (GPIOB_ReadPortPin(EC11_A_PIN) != 0) ? 1 : 0;
    uint8_t b = (GPIOB_ReadPortPin(EC11_B_PIN) != 0) ? 1 : 0;

    if (a && !s_last_a) {
        if (b) {
            for (uint8_t i = 0; i < s_step; i++) btn_fifo_push(BTN_UP);
        } else {
            for (uint8_t i = 0; i < s_step; i++) btn_fifo_push(BTN_DOWN);
        }
    }
    s_last_a = a;

    // ---------- 2. 按键检测 ----------
    uint8_t key_level = (GPIOB_ReadPortPin(EC11_KEY_PIN) != 0) ? 1 : 0;
    if (key_level == 0) {
        if (s_key_debounce < 0xFF) s_key_debounce++;
        if (s_key_debounce >= 10 && !s_key_pressed) {
            s_key_pressed = true;
            btn_fifo_push(BTN_ENTER);
        }
    } else {
        s_key_debounce = 0;
        s_key_pressed = false;
    }
}

// 定时器回调函数（由 BSP_Timer 调用）
static void ec11_timer_callback(void *arg)
{
    (void)arg;
    BSP_EC11_Scan();   // 执行实际扫描
}

// ==================== 初始化 ====================
void BSP_EC11_Init(void)
{
    // 配置引脚为输入上拉
    GPIOB_ModeCfg(EC11_A_PIN | EC11_B_PIN | EC11_KEY_PIN, GPIO_ModeIN_PU);
    s_last_a = (GPIOB_ReadPortPin(EC11_A_PIN) != 0) ? 1 : 0;

    // 创建一个 2ms 周期的循环定时器
    s_scan_timer_id = BSP_Timer_Create(2, true, ec11_timer_callback, NULL);
    if (s_scan_timer_id >= 0) {
        BSP_Timer_Start(s_scan_timer_id);
    } else {
        // 可打印错误或死循环
        while (1);
    }
}

// ==================== 控制接口 ====================
void BSP_EC11_Scan_Start(void)
{
    if (s_scan_timer_id >= 0) {
        BSP_Timer_Start(s_scan_timer_id);
    }
}

void BSP_EC11_Scan_Stop(void)
{
    if (s_scan_timer_id >= 0) {
        BSP_Timer_Stop(s_scan_timer_id);
    }
}

// 可选：设置旋转灵敏度
void BSP_EC11_SetStep(uint8_t step)
{
    if (step > 0) s_step = step;
}