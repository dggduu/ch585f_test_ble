#include "hlist.h"
#include "u8g2.h"
#include <math.h>
#include <string.h>

void hlist_init(hlist_t *hl, uint32_t *tick_ptr) {
  if (hl == NULL || tick_ptr == NULL) {
    return;
  }

  memset(hl, 0, sizeof(hlist_t));
  hl->main_tick = tick_ptr;
  hl->from_index = 0;
  hl->to_index = 0;
  hl->start_tick = *tick_ptr;
  hl->alert.text = NULL;
}

void hlist_add_xbm_item(hlist_t *hl, const char *title, const uint8_t *icon_xbm,
                    const page_component_t *comp, void *ctx) {
  if (hl == NULL) {
    return;
  }
  if (hl->count < 8) {
    hl->items[hl->count].title = title;
    hl->items[hl->count].icon_type = ICON_TYPE_XBM;
    hl->items[hl->count].icon_data.xbm = icon_xbm;
    hl->items[hl->count].comp = comp;
    hl->items[hl->count].ctx = ctx;
    hl->items[hl->count].protect.guard_flag = true;
    hl->items[hl->count].protect.alert_text = NULL;
    hl->count++;
  }
}

void hlist_add_glyph_item(hlist_t *hl, const char *title, uint16_t glyph,
                    const page_component_t *comp, void *ctx) {
  if (hl == NULL) {
    return;
  }
  if (hl->count < 8) {
    hl->items[hl->count].title = title;
    hl->items[hl->count].icon_type = ICON_TYPE_GLYPH;
    hl->items[hl->count].icon_data.glyph = glyph;
    hl->items[hl->count].comp = comp;
    hl->items[hl->count].ctx = ctx;
    hl->items[hl->count].protect.guard_flag = true;
    hl->items[hl->count].protect.alert_text = NULL;
    hl->count++;
  }
}

void hlist_add_protected_xbm_item(hlist_t *hl, const char *title,
                    const uint8_t *icon_xbm, const page_component_t *comp,
                    void *ctx, bool guard_flag, char *alert_text) {
  if (hl == NULL || alert_text == NULL) {
    return;
  }
  if (hl->count < 8) {
    hl->items[hl->count].title = title;
    hl->items[hl->count].icon_type = ICON_TYPE_XBM;
    hl->items[hl->count].icon_data.xbm = icon_xbm;
    hl->items[hl->count].comp = comp;
    hl->items[hl->count].ctx = ctx;
    hl->items[hl->count].protect.guard_flag = guard_flag;
    hl->items[hl->count].protect.alert_text = alert_text;
    hl->count++;
  }
}

void hlist_add_protected_glyph_item(hlist_t *hl, const char *title,
                    uint16_t glyph, const page_component_t *comp,
                    void *ctx, bool guard_flag, char *alert_text) {
  if (hl == NULL || alert_text == NULL) {
    return;
  }
  if (hl->count < 8) {
    hl->items[hl->count].title = title;
    hl->items[hl->count].icon_type = ICON_TYPE_GLYPH;
    hl->items[hl->count].icon_data.glyph = glyph;
    hl->items[hl->count].comp = comp;
    hl->items[hl->count].ctx = ctx;
    hl->items[hl->count].protect.guard_flag = guard_flag;
    hl->items[hl->count].protect.alert_text = alert_text;
    hl->count++;
  }
}

void hlist_draw(u8g2_t *u8g2, void *ctx) {
    if (!u8g2 || !ctx) return;
    hlist_t *hl = (hlist_t *)ctx;
    if (hl->count == 0) return;

    const Screen_t *screen_cfg = &g_screen_cfg;
    
    u8g2_SetMaxClipWindow(u8g2);
    u8g2_SetDrawColor(u8g2, 1);

    uint32_t elapsed = *hl->main_tick - hl->start_tick;
    if (elapsed > HLIST_ANIM_TICKS * 2) elapsed = HLIST_ANIM_TICKS;
    float p = (float)elapsed / HLIST_ANIM_TICKS;
    p = fminf(fmaxf(p, 0.0f), 1.0f);
    float ease_idx = hl->from_index + (hl->to_index - hl->from_index) * HLIST_ICON_ANIM(p);

    int screen_mid = screen_cfg->width / 2;
    int icon_gap = ICON_GAP;

    for (int i = 0; i < hl->count; i++) {
        int x = screen_mid + (int)((i - ease_idx) * icon_gap) - (ICON_WIDTH / 2);
        if (x < -ICON_WIDTH || x > screen_cfg->width) continue;

        int icon_y = (screen_cfg->height - ICON_HEIGHT - screen_cfg->font_height - 4) / 2;
        if (hl->items[i].icon_type == ICON_TYPE_XBM) {
            u8g2_DrawXBM(u8g2, x, icon_y, ICON_WIDTH, ICON_HEIGHT, hl->items[i].icon_data.xbm);
        } else {
            u8g2_SetFont(u8g2, g_screen_cfg.icon_font);
            u8g2_DrawGlyph(u8g2, x + 2, icon_y + ICON_HEIGHT - 2, hl->items[i].icon_data.glyph);
        }
    }

    u8g2_SetFont(u8g2, HLIST_TEXT_FONT);
    const char *title = hl->items[hl->to_index].title;
    if (title) {
        int text_width = u8g2_GetStrWidth(u8g2, title);
        int text_x = (screen_cfg->width - text_width) / 2;
        int text_base_y = screen_cfg->height - 10;
        
        if (p < 1.0f) { 
            float text_p = HLIST_TEXT_ANIM(p);
            int y_off = (int)(10 * (1.0f - text_p)); 
            u8g2_DrawStr(u8g2, text_x, text_base_y + y_off, title);
        } else {
            u8g2_DrawStr(u8g2, text_x, text_base_y, title);
        }
    }
}

void hlist_input(int btn, void *ctx) {
  if (ctx == NULL) return;
  hlist_t *hl = (hlist_t *)ctx;
  if (hl->count == 0) return;
  hlist_item_t *it = &hl->items[hl->to_index];
  if ((btn == BTN_UP || btn == BTN_LEFT) && hl->to_index > 0) {
    hl->from_index = hl->to_index;
    hl->to_index--;
    hl->start_tick = *hl->main_tick;
  } 
  else if ((btn == BTN_DOWN || btn == BTN_RIGHT) && hl->to_index < hl->count - 1) {
    hl->from_index = hl->to_index;
    hl->to_index++;
    hl->start_tick = *hl->main_tick;
  } 
  else if (btn == BTN_ENTER) {
    if (it->comp != NULL) {
      if (it->protect.guard_flag) {
        page_stack_push(&g_page_stack, it->comp, it->ctx);
      } else {
        page_stack_portal_toggle(
            &g_page_stack, 
            &PORTAL_MESSAGE_BOX, 
            &(portal_ctx_message_box_t){
                .title = it->title ? it->title : "Security", 
                .msg = it->protect.alert_text ? it->protect.alert_text : "Rejected!"
            },
            sizeof(portal_ctx_message_box_t)
        );
      }
    }
  }
}

const page_component_t HLIST_COMP = {hlist_draw, hlist_input};