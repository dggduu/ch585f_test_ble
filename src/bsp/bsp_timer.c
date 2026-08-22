#include "bsp_timer.h"
#include "CH58x_common.h"
#include <string.h>

/* 毫秒计数器，由 TMR2 中断更新 */
volatile uint32_t g_timer_millis = 0;

/* 软件定时器控制块 */
typedef struct {
    bool used;
    bool running;
    bool repeat;
    uint32_t period_ms;
    uint32_t last_tick;
    bsp_timer_cb_t callback;
    void *arg;
} bsp_timer_item_t;

static bsp_timer_item_t s_timers[BSP_TIMER_MAX_NUM];

/* ================== TMR2 中断服务函数 ================== */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if (TMR2_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        g_timer_millis++;
    }
}

/* ================== 初始化 ================== */
void BSP_Timer_Init(void)
{
    memset(s_timers, 0, sizeof(s_timers));
    g_timer_millis = 0;

    /* 配置 TMR2 产生 1ms 中断
     * CH58x 官方 TMR2_TimerInit 内部直接将参数赋给 R32_TMR2_CNT_END
     * 计数值 62400 即为精准的 1ms，无需 -1 
     */
    TMR2_TimerInit(FREQ_SYS / 1000);
    TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR2_IRQn);
}

/* ================== 毫秒获取 ================== */
uint32_t BSP_Timer_GetMillis(void)
{
    /* 32 位 RISC-V 内核读取 32 位 volatile 变量属于单周期指令，绝对安全，无需开关中断 */
    return g_timer_millis;
}

/* ================== 软件定时器管理 ================== */
bsp_timer_id_t BSP_Timer_Create(uint32_t period_ms, bool repeat, bsp_timer_cb_t cb, void *arg)
{
    if (period_ms == 0 || cb == NULL) {
        return -1;
    }

    for (int i = 0; i < BSP_TIMER_MAX_NUM; i++) {
        if (!s_timers[i].used) {
            s_timers[i].used = true;
            s_timers[i].running = false;
            s_timers[i].repeat = repeat;
            s_timers[i].period_ms = period_ms;
            s_timers[i].last_tick = 0;
            s_timers[i].callback = cb;
            s_timers[i].arg = arg;
            return (bsp_timer_id_t)i;
        }
    }
    return -1;
}

bool BSP_Timer_Start(bsp_timer_id_t id)
{
    if (id < 0 || id >= BSP_TIMER_MAX_NUM || !s_timers[id].used) {
        return false;
    }
    s_timers[id].last_tick = BSP_Timer_GetMillis();
    s_timers[id].running = true;
    return true;
}

bool BSP_Timer_Stop(bsp_timer_id_t id)
{
    if (id < 0 || id >= BSP_TIMER_MAX_NUM || !s_timers[id].used) {
        return false;
    }
    s_timers[id].running = false;
    return true;
}

bool BSP_Timer_Delete(bsp_timer_id_t id)
{
    if (id < 0 || id >= BSP_TIMER_MAX_NUM || !s_timers[id].used) {
        return false;
    }
    s_timers[id].running = false;
    s_timers[id].used = false;
    return true;
}

/* ================== 主循环轮询处理 ================== */
void BSP_Timer_Tick(void)
{
    uint32_t now = BSP_Timer_GetMillis();

    for (int i = 0; i < BSP_TIMER_MAX_NUM; i++) {
        if (!s_timers[i].used || !s_timers[i].running) {
            continue;
        }

        /* 无符号数减法自动兼容 uint32_t 溢出翻转（约 49.7 天溢出一次） */
        if ((now - s_timers[i].last_tick) >= s_timers[i].period_ms) {
            
            if (!s_timers[i].repeat) {
                // 单次定时器：触发前将 running 清零，允许用户在回调函数内部重新 Start
                s_timers[i].running = false;
            } else {
                // 循环定时器：采用增量追赶法，防止 UI/长延时阻塞导致的累积相位漂移
                s_timers[i].last_tick += s_timers[i].period_ms;
            }

            // 执行回调
            if (s_timers[i].callback != NULL) {
                s_timers[i].callback(s_timers[i].arg);
            }
        }
    }
}