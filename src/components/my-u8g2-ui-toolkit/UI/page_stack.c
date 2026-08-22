#include "page_stack.h"
#include "screen.h"
#include "bsp_timer.h"

// 全局页面栈实例
page_stack_t g_page_stack;

// 纯整数缓动函数
static inline int32_t QuadraticEaseOut_Int(int32_t t) {
    int32_t diff = 256 - t;
    return 256 - ((diff * diff) >> 8);
}

static inline int32_t QuadraticEaseIn_Int(int32_t t) {
    return (t * t) >> 8;
}

void page_stack_init(page_stack_t *ps, u8g2_t *u8g2) {
    memset(ps, 0, sizeof(page_stack_t));
    ps->u8g2 = u8g2;
    ps->global_btn_handler = NULL;
}

int page_stack_push(page_stack_t *ps, const page_component_t *comp, void *ctx) {
    if (ps->top >= PAGE_STACK_MAX_DEPTH || comp == NULL) return -1;
    ps->stack[ps->top].comp = comp;
    ps->stack[ps->top].ctx = ctx;
    ps->top++;
    return 0;
}

int page_stack_pop(page_stack_t *ps) {
    if (ps->top > 1) { 
        ps->top--; 
        return 0; 
    }
    return -1;
}

page_t* page_stack_current(page_stack_t *ps) {
    return (ps->top > 0) ? &ps->stack[ps->top - 1] : NULL;
}

void page_stack_register_global_btn_cb(page_stack_t *ps, global_btn_cb_t cb) {
    if (ps != NULL) {
        ps->global_btn_handler = cb;
    }
}

void page_stack_portal_toggle(page_stack_t *ps, const portal_component_t *comp, void *ctx, size_t ctx_size) {
    if (!ps) return;

    if (ps->is_portal_running) {
        ps->is_exiting = true;
    } else {
        if (comp && ctx) {
            ps->active_portal = comp;
            
            if (ctx_size > PORTAL_CTX_BUFFER_SIZE) ctx_size = PORTAL_CTX_BUFFER_SIZE;
            memcpy(ps->portal_ctx_buffer, ctx, ctx_size);
            ps->portal_ctx = ps->portal_ctx_buffer;
            
            ps->is_portal_running = true;
            ps->is_exiting = false;
            ps->ani_progress = 0; // 整数化：范围 0 ~ 256
        }
    }
}

void page_update(page_stack_t *ps, btn_type_t btn) {
    if (!ps) return;
    
    ps->main_tick = BSP_Timer_GetMillis();

    // 1. 输入拦截与分发
    if (btn != BTN_NONE) {
        if (ps->global_btn_handler) ps->global_btn_handler(btn);
        
        if (ps->is_portal_running && !ps->is_exiting) {
            if (ps->active_portal && ps->active_portal->input)
                ps->active_portal->input(btn, ps->portal_ctx);
            btn = BTN_NONE; // 拦截
        } else {
            page_t *p = page_stack_current(ps);
            if (p && p->comp && p->comp->input) p->comp->input(btn, p->ctx);
        }
    }

    // 2. 纯整数动画步进 (Portal)
    if (ps->is_portal_running) {
        // 防止除以 0 导致硬件异常
        uint16_t duration = g_screen_cfg.animation_duration ? g_screen_cfg.animation_duration : 1;
        
        // 放大 256 倍算 Step，极速整型计算
        int32_t step = 256 / duration;
        if (step < 1) step = 1; // 保证至少步进 1

        if (ps->is_exiting) {
            ps->ani_progress -= step;
            if (ps->ani_progress <= 0) {
                ps->ani_progress = 0;
                ps->is_portal_running = false;
                ps->active_portal = NULL;
            }
        } else {
            ps->ani_progress += step;
            if (ps->ani_progress >= 256) {
                ps->ani_progress = 256;
            }
        }
    }

    u8g2_ClearBuffer(ps->u8g2);

    // 3. 底层页面绘制
    page_t *p_curr = page_stack_current(ps);
    if (p_curr && p_curr->comp && p_curr->comp->draw) {
        u8g2_SetMaxClipWindow(ps->u8g2);
        u8g2_SetDrawColor(ps->u8g2, 1);
        p_curr->comp->draw(ps->u8g2, p_curr->ctx);
    }

    // 4. Portal 顶层绘制 (纯整数算坐标)
    if (ps->is_portal_running && ps->active_portal) {
        u8g2_SetMaxClipWindow(ps->u8g2);
        
        // 获取缓动值 (范围 0 ~ 256)
        int32_t eased = ps->is_exiting ? 
                        QuadraticEaseIn_Int(ps->ani_progress) : 
                        QuadraticEaseOut_Int(ps->ani_progress);

        // 目标移动距离
        int32_t target_dist = (g_screen_cfg.height - ps->active_portal->h) / 2 + ps->active_portal->h;
        
        // 纯整数算 current_y，利用 >> 8 取代浮点除法
        int current_y = -ps->active_portal->h + (int)((target_dist * eased) >> 8);

        ps->active_portal->draw(ps->u8g2, 
                                (g_screen_cfg.width - ps->active_portal->w) / 2, 
                                current_y, 
                                ps->active_portal->w, 
                                ps->active_portal->h, 
                                ps->portal_ctx);
    }

    u8g2_SendBuffer(ps->u8g2);
}