#include "btn_fifo.h"

static btn_fifo_t g_btn_fifo;

static inline void btn_fifo_enter_critical(void)
{
    //关闭全局中断
    // __disable_irq();
}

static inline void btn_fifo_exit_critical(void)
{
    // __enable_irq();
}

void btn_fifo_init(void)
{
    btn_fifo_enter_critical();
    g_btn_fifo.head = 0;
    g_btn_fifo.tail = 0;
    g_btn_fifo.count = 0;
    btn_fifo_exit_critical();
}

bool btn_fifo_push(btn_type_t btn)
{
    // 校验按键类型有效性
    if (btn <= BTN_NONE || btn >= BTN_MAX) {
        return false;
    }

    btn_fifo_enter_critical();
    
    // 检查FIFO是否已满
    if (g_btn_fifo.count >= BTN_FIFO_SIZE) {
        btn_fifo_exit_critical();
        return false;
    }

    // 写入按键事件到FIFO
    g_btn_fifo.buf[g_btn_fifo.tail] = btn;
    g_btn_fifo.tail = (g_btn_fifo.tail + 1) % BTN_FIFO_SIZE;
    g_btn_fifo.count++;
    
    btn_fifo_exit_critical();
    return true;
}

btn_type_t btn_fifo_pop(void)
{
    btn_type_t btn = BTN_NONE;
    
    btn_fifo_enter_critical();
    
    // 检查FIFO是否为空
    if (g_btn_fifo.count == 0) {
        btn_fifo_exit_critical();
        return BTN_NONE;
    }

    // 从FIFO读取按键事件
    btn = g_btn_fifo.buf[g_btn_fifo.head];
    g_btn_fifo.head = (g_btn_fifo.head + 1) % BTN_FIFO_SIZE;
    g_btn_fifo.count--;
    
    btn_fifo_exit_critical();
    return btn;
}

bool btn_fifo_is_empty(void)
{
    bool ret;
    btn_fifo_enter_critical();
    ret = (g_btn_fifo.count == 0);
    btn_fifo_exit_critical();
    return ret;
}

bool btn_fifo_is_full(void)
{
    bool ret;
    btn_fifo_enter_critical();
    ret = (g_btn_fifo.count >= BTN_FIFO_SIZE);
    btn_fifo_exit_critical();
    return ret;
}

void btn_fifo_clear(void)
{
    btn_fifo_enter_critical();
    g_btn_fifo.head = 0;
    g_btn_fifo.tail = 0;
    g_btn_fifo.count = 0;
    btn_fifo_exit_critical();
}

uint8_t btn_fifo_get_count(void)
{
    uint8_t count;
    btn_fifo_enter_critical();
    count = g_btn_fifo.count;
    btn_fifo_exit_critical();
    return count;
}