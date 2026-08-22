#ifndef __BTN_FIFO__
#define __BTN_FIFO__

#include <stdbool.h>
#include <stdint.h>

// 队列栈深度
#define BTN_FIFO_SIZE 16

typedef enum {
  BTN_NONE = 0, // 无按键
  BTN_UP,       // 上键
  BTN_DOWN,     // 下键
  BTN_LEFT,     // 左键
  BTN_RIGHT,    // 右键
  BTN_ENTER,    // 确认键
  BTN_BACK,     // 返回键
  BTN_CANCEL,   // 取消键
  BTN_LONG_PRESS,
  BTN_MAX // 按键类型最大值（用于FIFO边界检查）
} btn_type_t;

typedef struct {
  btn_type_t buf[BTN_FIFO_SIZE]; // FIFO缓冲区
  uint8_t head;                  // 读指针
  uint8_t tail;                  // 写指针
  uint8_t count;                 // 当前队列元素数
} btn_fifo_t;

// ========== 核心接口 ==========
/**
 * @brief 初始化按键FIFO
 */
void btn_fifo_init(void);

/**
 * @brief 软件触发按键事件
 * @param btn 要触发的按键类型
 * @return true-添加成功，false-FIFO已满
 */
bool btn_fifo_push(btn_type_t btn);

/**
 * @brief 读取FIFO中的按键事件
 * @return 按键类型，BTN_NONE表示队列为空
 */
btn_type_t btn_fifo_pop(void);

/**
 * @brief 检查FIFO是否为空
 * @return true-空，false-非空
 */
bool btn_fifo_is_empty(void);

/**
 * @brief 检查FIFO是否已满
 * @return true-满，false-未满
 */
bool btn_fifo_is_full(void);

/**
 * @brief 清空按键FIFO
 */
void btn_fifo_clear(void);

/**
 * @brief 获取FIFO中当前按键事件数量
 * @return 事件数量
 */
uint8_t btn_fifo_get_count(void);

#endif