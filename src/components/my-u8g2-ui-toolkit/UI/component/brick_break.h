#ifndef __BRICK_BREAK_H__
#define __BRICK_BREAK_H__

#include "btn_fifo.h"
#include "page_stack.h"
#include "screen.h"
#include "u8g2.h"
#include <stdint.h>

// ========== 游戏配置项 ==========
#define BRICK_ROWS 4                // 砖块行数
#define BRICK_COLS 8                // 砖块列数
#define BRICK_WIDTH 15              // 砖块宽度
#define BRICK_HEIGHT 6              // 砖块高度
#define BRICK_GAP 2                 // 砖块间距
#define PADDLE_WIDTH 20             // 挡板宽度
#define PADDLE_HEIGHT 3             // 挡板高度
#define BALL_SIZE 2                 // 小球尺寸
#define PADDLE_SPEED 2              // 挡板移动速度（调低更易操作）
#define BALL_SPEED_DIVISOR 3        // 小球速度除数（值越大越慢）
#define BALL_SPEED_X 1              // 小球X轴基础速度
#define BALL_SPEED_Y 1              // 小球Y轴基础速度

// 游戏状态枚举
typedef enum {
    GAME_STATE_READY,       // 准备开始
    GAME_STATE_PLAYING,     // 游戏中
    GAME_STATE_GAME_OVER,   // 游戏结束
    GAME_STATE_VICTORY      // 游戏胜利
} game_state_t;

// 砖块结构体
typedef struct {
    uint8_t x;              // X坐标
    uint8_t y;              // Y坐标
    bool active;            // 是否存在（未被击碎）
} brick_t;

// 游戏上下文（增加tick字段）
typedef struct {
    // 挡板属性
    uint8_t paddle_x;
    uint8_t paddle_y;
    
    // 小球属性
    uint8_t ball_x;
    uint8_t ball_y;
    int8_t ball_dir_x;      // X方向：-1(左) / 1(右)
    int8_t ball_dir_y;      // Y方向：-1(上) / 1(下)
    uint32_t ball_tick;     // 小球移动计时tick
    
    // 砖块数组
    brick_t bricks[BRICK_ROWS][BRICK_COLS];
    
    // 游戏状态
    game_state_t state;
    uint32_t *main_tick;    // 全局tick指针
    const Screen_t *screen; // 屏幕配置
} brick_break_ctx_t;

// 游戏组件实例
extern const page_component_t BRICK_BREAK_COMP;

// 初始化游戏上下文
void brick_break_init(brick_break_ctx_t *ctx, uint32_t *main_tick, const Screen_t *screen);

#endif // __BRICK_BREAK_H__