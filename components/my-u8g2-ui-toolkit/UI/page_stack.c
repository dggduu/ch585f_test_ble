#include "page_stack.h"
#include "screen.h"  // 导入g_screen_cfg

// 全局页面栈实例
page_stack_t g_page_stack;

#define GET_ANI_STEP() (1.0f / (float)g_screen_cfg.animation_duration)

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

// 注册全局按键回调
void page_stack_register_global_btn_cb(page_stack_t *ps, global_btn_cb_t cb) {
    if (ps != NULL) {
        ps->global_btn_handler = cb;
    }
}

void page_stack_portal_toggle(page_stack_t *ps, const portal_component_t *comp, void *ctx, size_t ctx_size) {
    if (!ps) return;

    if (ps->is_portal_running) {
        // 如果正在运行，启动退出动画
        ps->is_exiting = true;
    } else {
        // 启动进入动画
        if (comp && ctx) {
            ps->active_portal = comp;
            
            // 将匿名结构体内容存入 buffer
            if (ctx_size > PORTAL_CTX_BUFFER_SIZE) ctx_size = PORTAL_CTX_BUFFER_SIZE;
            memcpy(ps->portal_ctx_buffer, ctx, ctx_size);
            ps->portal_ctx = ps->portal_ctx_buffer;
            
            ps->is_portal_running = true;
            ps->is_exiting = false;
            ps->ani_progress = 0.0f;
        }
    }
}

void page_update(page_stack_t *ps, btn_type_t btn) {
    if (!ps) return;
	ps->main_tick++;
    // 输入拦截与分发
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

    // 动画步进 (Portal)
    if (ps->is_portal_running) {
        float step = 1.0f / g_screen_cfg.animation_duration;
        ps->ani_progress += ps->is_exiting ? -step : step;
        if (ps->ani_progress >= 1.0f) ps->ani_progress = 1.0f;
        if (ps->ani_progress <= 0.0f) {
            ps->is_portal_running = false;
            ps->active_portal = NULL;
        }
    }

    u8g2_ClearBuffer(ps->u8g2);

    // 1. 底层页面绘制
    page_t *p_curr = page_stack_current(ps);
    if (p_curr && p_curr->comp && p_curr->comp->draw) {
        // 关键：确保底层页面绘制前状态干净
        u8g2_SetMaxClipWindow(ps->u8g2);
        u8g2_SetDrawColor(ps->u8g2, 1);
        p_curr->comp->draw(ps->u8g2, p_curr->ctx);
    }

    // 2. Portal 顶层绘制
    if (ps->is_portal_running && ps->active_portal) {
        u8g2_SetMaxClipWindow(ps->u8g2); // 防止被底层裁切污染
        float eased = ps->is_exiting ? QuadraticEaseIn(ps->ani_progress) : QuadraticEaseOut(ps->ani_progress);
        int current_y = -ps->active_portal->h + (int)(( (g_screen_cfg.height - ps->active_portal->h)/2 + ps->active_portal->h ) * eased);
        ps->active_portal->draw(ps->u8g2, (g_screen_cfg.width - ps->active_portal->w)/2, current_y, 
                                ps->active_portal->w, ps->active_portal->h, ps->portal_ctx);
    }

    u8g2_SendBuffer(ps->u8g2);
}