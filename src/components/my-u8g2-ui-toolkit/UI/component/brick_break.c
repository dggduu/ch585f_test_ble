#include "brick_break.h"
#include "u8g2.h"
#include <string.h>
#include <stdint.h>

/**
 * @brief 初始化砖块布局
 */
static void init_bricks(brick_break_ctx_t *ctx) {
    if (ctx == NULL) return;
    
    uint16_t total_brick_width = (BRICK_WIDTH + BRICK_GAP) * BRICK_COLS - BRICK_GAP;
    uint8_t start_x = (ctx->screen->width - total_brick_width) / 2;
    uint8_t start_y = 10; 
    
    for (uint8_t row = 0; row < BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BRICK_COLS; col++) {
            ctx->bricks[row][col].x = start_x + col * (BRICK_WIDTH + BRICK_GAP);
            ctx->bricks[row][col].y = start_y + row * (BRICK_HEIGHT + BRICK_GAP);
            ctx->bricks[row][col].active = true;
        }
    }
}

/**
 * @brief 重置游戏状态
 */
static void reset_game(brick_break_ctx_t *ctx) {
    if (ctx == NULL) return;
    
    // 挡板居中
    ctx->paddle_x = (ctx->screen->width - PADDLE_WIDTH) / 2;
    ctx->paddle_y = ctx->screen->height - 10;
    
    // 小球初始位置在挡板上方
    ctx->ball_x = ctx->paddle_x + PADDLE_WIDTH / 2 - BALL_SIZE / 2;
    ctx->ball_y = ctx->paddle_y - BALL_SIZE - 1;
    ctx->ball_dir_x = BALL_SPEED_X;
    ctx->ball_dir_y = -BALL_SPEED_Y; 
    ctx->ball_tick = 0;
    
    init_bricks(ctx);
    ctx->state = GAME_STATE_READY;
}

/**
 * @brief 碰撞检测：小球与砖块
 */
static void check_brick_collision(brick_break_ctx_t *ctx) {
    if (ctx == NULL || ctx->state != GAME_STATE_PLAYING) return;
    
    for (uint8_t row = 0; row < BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BRICK_COLS; col++) {
            brick_t *brick = &ctx->bricks[row][col];
            if (!brick->active) continue;
            
            // 精确碰撞检测
            if (ctx->ball_x + BALL_SIZE > brick->x &&
                ctx->ball_x < brick->x + BRICK_WIDTH &&
                ctx->ball_y + BALL_SIZE > brick->y &&
                ctx->ball_y < brick->y + BRICK_HEIGHT) {
                
                brick->active = false;
                ctx->ball_dir_y = -ctx->ball_dir_y; // 垂直反弹
                
                // 检查胜利条件
                bool all_broken = true;
                for (uint8_t r = 0; r < BRICK_ROWS && all_broken; r++) {
                    for (uint8_t c = 0; c < BRICK_COLS && all_broken; c++) {
                        if (ctx->bricks[r][c].active) all_broken = false;
                    }
                }
                if (all_broken) ctx->state = GAME_STATE_VICTORY;
                return;
            }
        }
    }
}

/**
 * @brief 碰撞检测：小球与边界/挡板
 */
static void check_boundary_collision(brick_break_ctx_t *ctx) {
    if (ctx == NULL || ctx->state != GAME_STATE_PLAYING) return;
    
    // 左右边界反弹
    if (ctx->ball_x <= 0) {
        ctx->ball_x = 0;
        ctx->ball_dir_x = BALL_SPEED_X;
    } else if (ctx->ball_x + BALL_SIZE >= ctx->screen->width) {
        ctx->ball_x = ctx->screen->width - BALL_SIZE;
        ctx->ball_dir_x = -BALL_SPEED_X;
    }
    
    // 上边界反弹
    if (ctx->ball_y <= 0) {
        ctx->ball_y = 0;
        ctx->ball_dir_y = BALL_SPEED_Y;
    }
    
    // 下边界：游戏结束
    if (ctx->ball_y + BALL_SIZE >= ctx->screen->height) {
        ctx->state = GAME_STATE_GAME_OVER;
        return;
    }
    
    // 挡板碰撞（带角度优化）
    if (ctx->ball_y + BALL_SIZE >= ctx->paddle_y &&
        ctx->ball_x + BALL_SIZE > ctx->paddle_x &&
        ctx->ball_x < ctx->paddle_x + PADDLE_WIDTH) {
        ctx->ball_y = ctx->paddle_y - BALL_SIZE;
        ctx->ball_dir_y = -BALL_SPEED_Y;
        
        // 碰撞位置影响X方向
        uint8_t hit_center = ctx->paddle_x + PADDLE_WIDTH / 2;
        if (ctx->ball_x + BALL_SIZE/2 < hit_center - 5) {
            ctx->ball_dir_x = -BALL_SPEED_X;
        } else if (ctx->ball_x + BALL_SIZE/2 > hit_center + 5) {
            ctx->ball_dir_x = BALL_SPEED_X;
        }
    }
}

/**
 * @brief 游戏逻辑更新（每帧调用）
 */
static void update_game_logic(brick_break_ctx_t *ctx) {
    if (ctx == NULL || ctx->state != GAME_STATE_PLAYING) return;
    
    // 用tick分频控制小球速度，避免移动过快
    ctx->ball_tick++;
    if (ctx->ball_tick >= BALL_SPEED_DIVISOR) {
        ctx->ball_x += ctx->ball_dir_x;
        ctx->ball_y += ctx->ball_dir_y;
        ctx->ball_tick = 0;
        
        // 碰撞检测
        check_boundary_collision(ctx);
        check_brick_collision(ctx);
    }
}

/**
 * @brief 绘制游戏界面（整合逻辑更新）
 */
static void brick_break_draw(u8g2_t *u8g2, void *ctx) {
    if (u8g2 == NULL || ctx == NULL) return;
    
    brick_break_ctx_t *game_ctx = (brick_break_ctx_t *)ctx;
    
    // ========== 每帧更新游戏逻辑 ==========
    update_game_logic(game_ctx);

    u8g2_ClearBuffer(u8g2);
    const Screen_t *screen = game_ctx->screen;
    
    // ========== 绘制挡板 ==========
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawBox(u8g2, game_ctx->paddle_x, game_ctx->paddle_y, 
                 PADDLE_WIDTH, PADDLE_HEIGHT);
    
    // ========== 绘制小球 ==========
    u8g2_DrawBox(u8g2, game_ctx->ball_x, game_ctx->ball_y, 
                 BALL_SIZE, BALL_SIZE);
    
    // ========== 绘制砖块 ==========
    for (uint8_t row = 0; row < BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BRICK_COLS; col++) {
            brick_t *brick = &game_ctx->bricks[row][col];
            if (brick->active) {
                u8g2_DrawBox(u8g2, brick->x, brick->y, 
                             BRICK_WIDTH, BRICK_HEIGHT);
            }
        }
    }
    
    // ========== 绘制游戏状态文本 ==========
    u8g2_SetFont(u8g2, screen->font);
    const char *status_text = NULL;
    switch (game_ctx->state) {
        case GAME_STATE_READY:
            status_text = "ENTER to Start";
            break;
        case GAME_STATE_GAME_OVER:
            status_text = "Game Over! ENTER=Retry";
            break;
        case GAME_STATE_VICTORY:
            status_text = "Victory! ENTER=Retry";
            break;
        default:
            status_text = NULL;
            break;
    }
    
    if (status_text != NULL) {
        uint8_t text_width = u8g2_GetStrWidth(u8g2, status_text);
        u8g2_DrawStr(u8g2, (screen->width - text_width) / 2, 
                     screen->height / 2, status_text);
    }
    
    // ========== 绘制操作提示 ==========
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    const char *hint_text = "LEFT/RIGHT:Move  BACK:Exit";
    uint8_t hint_width = u8g2_GetStrWidth(u8g2, hint_text);
    u8g2_DrawStr(u8g2, (screen->width - hint_width) / 2, 
                 screen->height - 5, hint_text);
    
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 修复按键处理：支持连续移动
 */
static void brick_break_input(int btn, void *ctx) {
    if (ctx == NULL) return;
    
    brick_break_ctx_t *game_ctx = (brick_break_ctx_t *)ctx;
    
    // 退出游戏
    if (btn == BTN_BACK) {
        page_stack_pop(&g_page_stack);
        return;
    }

    // 状态控制
    switch (game_ctx->state) {
        case GAME_STATE_READY:
        case GAME_STATE_GAME_OVER:
        case GAME_STATE_VICTORY:
            if (btn == BTN_ENTER) {
                reset_game(game_ctx);
                game_ctx->state = GAME_STATE_PLAYING;
            }
            break;
            
        case GAME_STATE_PLAYING:
            // 挡板连续移动：直接修改坐标，每一帧都会刷新
            if (btn == BTN_UP && game_ctx->paddle_x > 0) {
                game_ctx->paddle_x -= PADDLE_SPEED;
                // 修复：使用game_ctx而非ctx，且修正溢出判断逻辑
                if (game_ctx->paddle_x > game_ctx->screen->width) { 
                    game_ctx->paddle_x = 0;
                }
            } else if (btn == BTN_DOWN && 
                       (game_ctx->paddle_x + PADDLE_WIDTH) < game_ctx->screen->width) {
                game_ctx->paddle_x += PADDLE_SPEED;
                if (game_ctx->paddle_x + PADDLE_WIDTH > game_ctx->screen->width) {
                    game_ctx->paddle_x = game_ctx->screen->width - PADDLE_WIDTH;
                }
            }
            break;
    }
}

/**
 * @brief 初始化游戏上下文
 */
void brick_break_init(brick_break_ctx_t *ctx, uint32_t *main_tick, const Screen_t *screen) {
    if (ctx == NULL || main_tick == NULL || screen == NULL) return;
    
    memset(ctx, 0, sizeof(brick_break_ctx_t));
    ctx->main_tick = main_tick;
    ctx->screen = screen;
    reset_game(ctx);
}

// 注册游戏组件
const page_component_t BRICK_BREAK_COMP = {
    .draw = brick_break_draw,
    .input = brick_break_input
};