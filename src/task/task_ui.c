#include "task_ui.h"
#include "page_stack.h"
#include "btn_fifo.h"
#include "bsp_timer.h"          // 引入 BSP_Timer_Tick
#include "app_splash_screen.h"

tmosTaskID uiTaskID = 0xFF;

/* UI 周期刷新间隔（ms）。
 * 注意：一次全屏刷新（u8g2_SendBuffer → SPI 115KB）会阻塞主循环约 3~5ms，
 * 而 BLE 协议栈的 GAP/连接事件处理都在 TMOS_SystemProcess() 里进行。
 * 周期越短，BLE 被饿死的时间占比越大；40ms(25fps) 是显示流畅度与
 * BLE 存活之间的折中。若接连接类角色，可进一步调大。 */
#define UI_UPDATE_PERIOD_MS   40

/* 执行一次完整的 UI 更新（定时器+页面刷新） */
static void ui_update_full(void)
{
    // 1. 驱动软件定时器（执行到期的回调）
    BSP_Timer_Tick();

    // 2. 处理按键（从队列取出一个按键，若无按键则传递 BTN_NONE）
    btn_type_t btn = btn_fifo_pop();
    // 假设 btn_fifo_pop() 在无按键时返回 BTN_NONE（需定义）
    page_update(&g_page_stack, btn);
}

/* 仅处理按键（用于按键触发，不处理定时器） */
static void ui_update_key(void)
{
    btn_type_t btn = btn_fifo_pop();
    page_update(&g_page_stack, btn);
}

uint16_t UI_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    static uint32_t cnt = 0;
    cnt++;
    if (cnt % 50 == 0) PRINT("page_update called\n");
    if (events & UI_UPDATE_EVENT)
    {
        ui_update_full();
        // 重新启动定时事件（周期见 UI_UPDATE_PERIOD_MS）
        tmos_start_task(uiTaskID, UI_UPDATE_EVENT,
                        MS1_TO_SYSTEM_TIME(UI_UPDATE_PERIOD_MS));
        return events & ~UI_UPDATE_EVENT;
    }

    if (events & UI_KEY_EVENT)
    {
        ui_update_key();
        return events & ~UI_KEY_EVENT;
    }

    return 0;
}

void UI_Task_Init(void)
{
    // 注册 TMOS 任务（必须在 CH58x_BLEInit() 之后调用，
    // 否则 TMOS 任务表尚未初始化，注册必然返回 0xFF）
    uiTaskID = TMOS_ProcessEventRegister(UI_ProcessEvent);
    if (uiTaskID == 0xFF)
    {
        PRINT("UI task register failed (BLE lib not initialized?)\n");
        return; // 不阻塞系统，便于串口排查
    }

    // 初始化 UI 页面栈及菜单（包括 Splash）
    app_splash_screen_entry();

    // 启动周期性刷新，首次 40ms 后触发。
    // 本库 tmos_start_task 返回 1 表示成功、0 表示失败；
    // 失败最常见原因是 HAL_TimeInit()（TMOS_TimerInit）未被调用/被覆盖
    // —— 即初始化顺序错误（HAL_Init 必须晚于 CH58x_BLEInit）。
    if (tmos_start_task(uiTaskID, UI_UPDATE_EVENT,
                        MS1_TO_SYSTEM_TIME(UI_UPDATE_PERIOD_MS)) != 1)
    {
        PRINT("UI timer start failed: TMOS clock not registered?\n");
    }
}
