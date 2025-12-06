// ========================================
// Fichier: display_overlay.cpp
// Version 8.50.R133 - Module overlay veille rÃ©utilisable
// ========================================
#include <Arduino.h>
#include "display_overlay.h"

// Overlay pour mode veille
lv_obj_t *sleep_overlay = nullptr;
bool sleep_mode = false;

// Variables pour gestion tactile
static bool touch_detected = false;
static unsigned long last_touch_change = 0;
static const unsigned long DEBOUNCE_DELAY = 300;

void createOverlay()
{
    lv_obj_t *screen = lv_scr_act();
    
    // OVERLAY MODE VEILLE (plein Ã©cran transparent)
    sleep_overlay = lv_obj_create(screen);
    lv_obj_set_size(sleep_overlay, 1024, 600);
    lv_obj_set_pos(sleep_overlay, 0, 0);
    lv_obj_set_style_bg_color(sleep_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sleep_overlay, LV_OPA_90, 0);  // 90% opaque
    lv_obj_set_style_border_width(sleep_overlay, 0, 0);
    lv_obj_set_style_radius(sleep_overlay, 0, 0);
    lv_obj_set_style_anim_time(sleep_overlay, 0, 0);
    
    // Label de veille
    lv_obj_t *sleep_label = lv_label_create(sleep_overlay);
    lv_label_set_text(sleep_label, "En veille\nToucher pour retablir");
    lv_obj_set_style_text_font(sleep_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(sleep_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(sleep_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(sleep_label);
    
    // Cacher l'overlay par dÃ©faut
    lv_obj_add_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("[R133] Overlay veille crÃ©Ã© (module sÃ©parÃ©)");
}

void updateTouchBacklight(Board *board)
{
    auto touch = board->getTouch();
    
    if (touch == nullptr || sleep_overlay == nullptr) {
        return;
    }
    
    // Lire les points tactiles
    std::vector<TouchPoint> points;
    touch->readRawData(-1, -1, 10);
    touch->getPoints(points);
    
    bool currently_touching = !points.empty();
    unsigned long now = millis();
    
    // SORTIE DE VEILLE : rÃ©action IMMEDIATE (pas de debounce)
    if (currently_touching && sleep_mode && !touch_detected) {
        touch_detected = true;
        last_touch_change = now;
        sleep_mode = false;
        Serial.println("[R133] Touch â†’ Mode NORMAL (immediat)");
        lv_obj_add_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(sleep_overlay);
        lv_refr_now(NULL);
        return;
    }
    
    // ENTREE EN VEILLE : avec debounce pour Ã©viter les faux contacts
    if (currently_touching && !sleep_mode && !touch_detected) {
        if (now - last_touch_change >= DEBOUNCE_DELAY) {
            touch_detected = true;
            last_touch_change = now;
            sleep_mode = true;
            Serial.println("[R133] Touch â†’ Mode VEILLE");
            lv_obj_clear_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
            lv_obj_invalidate(sleep_overlay);
            lv_refr_now(NULL);
        }
        return;
    }
    
    // Release : reset touch_detected
    if (!currently_touching && touch_detected) {
        touch_detected = false;
        // Pas de debounce sur le release
    }
}