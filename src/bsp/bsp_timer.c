#include "bsp_timer.h"
#include <string.h>   // 添加 string.h，用于 memset

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

/* ================== TMR2 中断服务 ================== */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if (TMR2_GetITFlag(TMR0_3_IT_CYC_END)) {   // 使用通用周期结束标志
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        g_timer_millis++;
    }
}

/* ================== 初始化 ================== */
void BSP_Timer_Init(void)
{
    /* 清空软件定时器表 */
    memset(s_timers, 0, sizeof(s_timers));
    g_timer_millis = 0;

    /* 配置 TMR2 产生 1ms 中断 */
    TMR2_TimerInit(FREQ_SYS / 1000);          // 计数周期 = 1ms
    TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);    // 使能周期结束中断
    PFIC_EnableIRQ(TMR2_IRQn);                // 使能 TMR2 中断
}

/* ================== 毫秒获取 ================== */
uint32_t BSP_Timer_GetMillis(void)
{
    uint32_t val;
    /* 使用关中断保证 32 位读取一致性（可选） */
    PFIC_DisableIRQ(TMR2_IRQn);
    val = g_timer_millis;
    PFIC_EnableIRQ(TMR2_IRQn);
    return val;
}

/* ================== 软件定时器管理 ================== */
bsp_timer_id_t BSP_Timer_Create(uint32_t period_ms, bool repeat, bsp_timer_cb_t cb, void *arg)
{
    if (period_ms == 0) return -1;

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
    s_timers[id].running = true;
    s_timers[id].last_tick = BSP_Timer_GetMillis();
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
    s_timers[id].used = false;
    s_timers[id].running = false;
    return true;
}

/* ================== 主循环处理 ================== */
void BSP_Timer_Tick(void)
{
    uint32_t now = BSP_Timer_GetMillis();

    for (int i = 0; i < BSP_TIMER_MAX_NUM; i++) {
        if (!s_timers[i].used || !s_timers[i].running) {
            continue;
        }
        if ((now - s_timers[i].last_tick) >= s_timers[i].period_ms) {
            s_timers[i].last_tick = now;

            if (s_timers[i].callback != NULL) {
                s_timers[i].callback(s_timers[i].arg);
            }

            if (!s_timers[i].repeat) {
                s_timers[i].running = false;
            }
        }
    }
}