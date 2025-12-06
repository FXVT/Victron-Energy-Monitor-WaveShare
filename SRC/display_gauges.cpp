// ========================================
// Fichier: display_gauges.cpp
// Version 8.50.R154P - Labels performance harmonisés
// CHANGEMENTS R154P:
// - Labels performance: même police (Montserrat 20) pour valeur ET symbole %
// - Position ajustée (-62 au lieu de -38) pour éviter débordement du symbole %
// - Gap fixe optimisé (+40 au lieu de +18) pour tous les cas (0% à 125%)
// - Pas de superposition garantie, code simple et rapide
// CHANGEMENTS R154O:
// - Bandeau bleu pleine largeur (#005FBE) pour titre
// - Logo Victron blanc (48x30px) en haut à droite (10px du bord)
// - Titre avec nom du bateau dynamique (blanc, y=8)
// - Initialisation perf_label et perf_percent_label à nullptr
// - Création 2 labels perf pour SOLAIRE, ALTERNATEUR, QUAI (valeur + %)
// - Utilisation PSOL/PALT/PQUAI pour max_value des jauges
// - Mise à jour performances dans setGaugeValue()
// - Couleur rouge si perf >100%, blanc sinon (pour valeur ET %)
// - Jauges hautes descendues à y=170 (écart 5px avec bandeau)
// - Batterie descendue de 5px (y=455), cadre réduit à 300px
// ========================================
#include <Arduino.h>
#include "display_gauges.h"
#include "acquisition_BT.h"

using namespace esp_panel::drivers;

// R154O: Déclaration image logo Victron pour bandeau supérieur
LV_IMG_DECLARE(Logo_victron_blanc_48x30TC);

Gauge gauges[4];
GaugeLED leds[4];  // R154L: Avec current_state

static LCD *g_lcd = nullptr;

// R154L: Timeouts LED (15s général, 60s pour IP22)
#define LED_TIMEOUT_MS 15000
#define IP22_TIMEOUT_MS 60000

void createLED(int gauge_id, int base_x, int base_y, int size, int padding) {
    int led_x = base_x - (size + padding*2)/2 + 5;
    int led_y = base_y - size/2 - padding + 5;
    
    lv_obj_t *screen = lv_scr_act();
    
    leds[gauge_id].border_obj = lv_obj_create(screen);
    lv_obj_set_size(leds[gauge_id].border_obj, 25, 25);
    lv_obj_set_pos(leds[gauge_id].border_obj, led_x, led_y);
    lv_obj_set_style_bg_color(leds[gauge_id].border_obj, lv_color_hex(LED_BORDER), 0);
    lv_obj_set_style_border_width(leds[gauge_id].border_obj, 0, 0);
    lv_obj_set_style_radius(leds[gauge_id].border_obj, 13, 0);
    lv_obj_set_style_pad_all(leds[gauge_id].border_obj, 0, 0);
    
    leds[gauge_id].led_obj = lv_obj_create(screen);
    lv_obj_set_size(leds[gauge_id].led_obj, 15, 15);
    lv_obj_set_pos(leds[gauge_id].led_obj, led_x + 5, led_y + 5);
    lv_obj_set_style_bg_color(leds[gauge_id].led_obj, lv_color_hex(LED_OFF_TOP), 0);
    lv_obj_set_style_bg_grad_color(leds[gauge_id].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(leds[gauge_id].led_obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_dither_mode(leds[gauge_id].led_obj, LV_DITHER_ORDERED, 0);
    lv_obj_set_style_border_width(leds[gauge_id].led_obj, 0, 0);
    lv_obj_set_style_radius(leds[gauge_id].led_obj, 8, 0);
    lv_obj_set_style_pad_all(leds[gauge_id].led_obj, 0, 0);
    
    // R154L: État initial OFF
    leds[gauge_id].current_state = false;
}

void createGauges(const char* boat_name, const char* version_app)
{
    lv_obj_t *screen = lv_scr_act();
    
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_dither_mode(screen, LV_DITHER_ORDERED, 0);

    // R154O: Bandeau bleu pleine largeur pour titre
    lv_obj_t *title_banner = lv_obj_create(screen);
    lv_obj_set_size(title_banner, 1024, 45);
    lv_obj_set_pos(title_banner, 0, 0);
    lv_obj_set_style_bg_color(title_banner, lv_color_hex(0x005FBE), 0);
    lv_obj_set_style_border_width(title_banner, 0, 0);
    lv_obj_set_style_radius(title_banner, 0, 0);
    lv_obj_set_style_pad_all(title_banner, 0, 0);

    // R154O: Logo Victron blanc en haut à droite du bandeau (48x30px, 10px du bord)
    lv_obj_t *logo_victron = lv_img_create(screen);
    lv_img_set_src(logo_victron, &Logo_victron_blanc_48x30TC);
    lv_obj_set_pos(logo_victron, 966, 8);  // x=1024-48-10=966, y=8 (centré verticalement)

    // R154O: Titre avec nom du bateau
    char title_text[100];
    snprintf(title_text, sizeof(title_text), "Monitoring Energie VE.Connect %s", boat_name);
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);  // R154O: Blanc
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);  // R154O: 2 pixels plus haut (10→8)

    // R154N: Utiliser version_app au lieu de chaîne fixe
    lv_obj_t *version_label = lv_label_create(screen);
    lv_label_set_text(version_label, version_app);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(version_label, lv_color_hex(0x666666), 0);
    lv_obj_align(version_label, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    // R154O: Initialiser tous les perf_label et perf_percent_label à nullptr
    for (int i = 0; i < 4; i++) {
        gauges[i].perf_label = nullptr;
        gauges[i].perf_percent_label = nullptr;
    }

    // JAUGE SOLAIRE
    {
        int gauge_id = JSOLAIRE;
        int x = 170, y = 170, size = 200;  // R154O: y=170 pour 5px d'écart avec bandeau
        const char* name = "SOLAIRE";
        const char* unit = "W";
        int max_val = PSOL;
        uint32_t color = 0xFFDD00;
        
        gauges[gauge_id].max_value = max_val;
        gauges[gauge_id].color = color;
        
        int padding = 20;
        lv_obj_t *gauge_bg = lv_obj_create(screen);
        lv_obj_set_size(gauge_bg, size + padding*2, size + padding*2 + 5);
        lv_obj_set_pos(gauge_bg, x - (size + padding*2)/2, y - size/2 - padding);
        lv_obj_set_style_bg_color(gauge_bg, lv_color_hex(0x1a1a1a), 0);
        lv_obj_set_style_border_width(gauge_bg, 0, 0);
        lv_obj_set_style_radius(gauge_bg, 20, 0);
        lv_obj_set_style_pad_all(gauge_bg, 0, 0);
        lv_obj_set_style_shadow_width(gauge_bg, 20, 0);
        lv_obj_set_style_shadow_color(gauge_bg, lv_color_black(), 0);
        lv_obj_set_style_shadow_ofs_x(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_ofs_y(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_opa(gauge_bg, LV_OPA_40, 0);
        
        createLED(gauge_id, x, y, size, padding);
        
        gauges[gauge_id].arc = lv_arc_create(screen);
        lv_obj_set_size(gauges[gauge_id].arc, size, size);
        lv_obj_set_pos(gauges[gauge_id].arc, x - size/2, y - size/2);
        lv_arc_set_rotation(gauges[gauge_id].arc, 135);
        lv_arc_set_bg_angles(gauges[gauge_id].arc, 0, 270);
        lv_arc_set_range(gauges[gauge_id].arc, 0, max_val);
        lv_arc_set_value(gauges[gauge_id].arc, 0);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_MAIN);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(color), LV_PART_INDICATOR);
        lv_obj_clear_flag(gauges[gauge_id].arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 0, LV_PART_KNOB);
        
        gauges[gauge_id].value_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].value_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].value_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].value_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].value_label, LV_ALIGN_CENTER, x - 512, y - 300 - 10);
        
        gauges[gauge_id].unit_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].unit_label, unit);
        lv_obj_set_style_text_font(gauges[gauge_id].unit_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].unit_label, lv_color_hex(0xaaaaaa), 0);
        lv_obj_align(gauges[gauge_id].unit_label, LV_ALIGN_CENTER, x - 512, y - 300 + 30);
        
        gauges[gauge_id].name_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].name_label, name);
        lv_obj_set_style_text_font(gauges[gauge_id].name_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].name_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].name_label, LV_ALIGN_CENTER, x - 512, y - 300 + size/2);
        
        // R154P: Labels performance - même police (20) pour valeur ET symbole %
        gauges[gauge_id].perf_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_label, &lv_font_montserrat_20, 0);  // R154P: Police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_label, lv_color_hex(0xFFFFFF), 0);
        int perf_x = x + (size + padding*2)/2 - 62;  // R154P: Plus à gauche (-62) pour éviter débordement
        int perf_y = y - size/2 - padding + 12;
        lv_obj_set_pos(gauges[gauge_id].perf_label, perf_x, perf_y);
        
        gauges[gauge_id].perf_percent_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_percent_label, "%");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_percent_label, &lv_font_montserrat_20, 0);  // R154P: Même police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_percent_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(gauges[gauge_id].perf_percent_label, perf_x + 40, perf_y + 2);  // R154P: Gap +40 pour police 20
    }

    // JAUGE ALTERNATEUR
    {
        int gauge_id = JALTERNATEUR;
        int x = 512, y = 170, size = 200;  // R154O: y=170 pour 5px d'écart avec bandeau
        const char* name = "ALTERNATEUR";
        const char* unit = "A";
        int max_val = PALT;
        uint32_t color = 0xFF8800;
        
        gauges[gauge_id].max_value = max_val;
        gauges[gauge_id].color = color;
        
        int padding = 20;
        lv_obj_t *gauge_bg = lv_obj_create(screen);
        lv_obj_set_size(gauge_bg, size + padding*2, size + padding*2 + 5);
        lv_obj_set_pos(gauge_bg, x - (size + padding*2)/2, y - size/2 - padding);
        lv_obj_set_style_bg_color(gauge_bg, lv_color_hex(0x1a1a1a), 0);
        lv_obj_set_style_border_width(gauge_bg, 0, 0);
        lv_obj_set_style_radius(gauge_bg, 20, 0);
        lv_obj_set_style_pad_all(gauge_bg, 0, 0);
        lv_obj_set_style_shadow_width(gauge_bg, 20, 0);
        lv_obj_set_style_shadow_color(gauge_bg, lv_color_black(), 0);
        lv_obj_set_style_shadow_ofs_x(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_ofs_y(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_opa(gauge_bg, LV_OPA_40, 0);
        
        createLED(gauge_id, x, y, size, padding);
        
        gauges[gauge_id].arc = lv_arc_create(screen);
        lv_obj_set_size(gauges[gauge_id].arc, size, size);
        lv_obj_set_pos(gauges[gauge_id].arc, x - size/2, y - size/2);
        lv_arc_set_rotation(gauges[gauge_id].arc, 135);
        lv_arc_set_bg_angles(gauges[gauge_id].arc, 0, 270);
        lv_arc_set_range(gauges[gauge_id].arc, 0, max_val);
        lv_arc_set_value(gauges[gauge_id].arc, 0);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_MAIN);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(color), LV_PART_INDICATOR);
        lv_obj_clear_flag(gauges[gauge_id].arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 0, LV_PART_KNOB);
        
        gauges[gauge_id].value_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].value_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].value_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].value_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].value_label, LV_ALIGN_CENTER, x - 512, y - 300 - 10);
        
        gauges[gauge_id].unit_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].unit_label, unit);
        lv_obj_set_style_text_font(gauges[gauge_id].unit_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].unit_label, lv_color_hex(0xaaaaaa), 0);
        lv_obj_align(gauges[gauge_id].unit_label, LV_ALIGN_CENTER, x - 512, y - 300 + 30);
        
        gauges[gauge_id].name_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].name_label, name);
        lv_obj_set_style_text_font(gauges[gauge_id].name_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].name_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].name_label, LV_ALIGN_CENTER, x - 512, y - 300 + size/2);
        
        // R154P: Labels performance - même police (20) pour valeur ET symbole %
        gauges[gauge_id].perf_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_label, &lv_font_montserrat_20, 0);  // R154P: Police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_label, lv_color_hex(0xFFFFFF), 0);
        int perf_x = x + (size + padding*2)/2 - 62;  // R154P: Plus à gauche (-62) pour éviter débordement
        int perf_y = y - size/2 - padding + 12;
        lv_obj_set_pos(gauges[gauge_id].perf_label, perf_x, perf_y);
        
        gauges[gauge_id].perf_percent_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_percent_label, "%");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_percent_label, &lv_font_montserrat_20, 0);  // R154P: Même police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_percent_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(gauges[gauge_id].perf_percent_label, perf_x + 40, perf_y + 2);  // R154P: Gap +40 pour police 20
    }

    // JAUGE QUAI
    {
        int gauge_id = JQUAI;
        int x = 854, y = 170, size = 200;  // R154O: y=170 pour 5px d'écart avec bandeau
        const char* name = "QUAI";
        const char* unit = "A";
        int max_val = PQUAI;
        uint32_t color = 0x00DD00;
        
        gauges[gauge_id].max_value = max_val;
        gauges[gauge_id].color = color;
        
        int padding = 20;
        lv_obj_t *gauge_bg = lv_obj_create(screen);
        lv_obj_set_size(gauge_bg, size + padding*2, size + padding*2 + 5);
        lv_obj_set_pos(gauge_bg, x - (size + padding*2)/2, y - size/2 - padding);
        lv_obj_set_style_bg_color(gauge_bg, lv_color_hex(0x1a1a1a), 0);
        lv_obj_set_style_border_width(gauge_bg, 0, 0);
        lv_obj_set_style_radius(gauge_bg, 20, 0);
        lv_obj_set_style_pad_all(gauge_bg, 0, 0);
        lv_obj_set_style_shadow_width(gauge_bg, 20, 0);
        lv_obj_set_style_shadow_color(gauge_bg, lv_color_black(), 0);
        lv_obj_set_style_shadow_ofs_x(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_ofs_y(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_opa(gauge_bg, LV_OPA_40, 0);
        
        createLED(gauge_id, x, y, size, padding);
        
        gauges[gauge_id].arc = lv_arc_create(screen);
        lv_obj_set_size(gauges[gauge_id].arc, size, size);
        lv_obj_set_pos(gauges[gauge_id].arc, x - size/2, y - size/2);
        lv_arc_set_rotation(gauges[gauge_id].arc, 135);
        lv_arc_set_bg_angles(gauges[gauge_id].arc, 0, 270);
        lv_arc_set_range(gauges[gauge_id].arc, 0, max_val);
        lv_arc_set_value(gauges[gauge_id].arc, 0);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_MAIN);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 20, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(color), LV_PART_INDICATOR);
        lv_obj_clear_flag(gauges[gauge_id].arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 0, LV_PART_KNOB);
        
        gauges[gauge_id].value_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].value_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].value_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].value_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].value_label, LV_ALIGN_CENTER, x - 512, y - 300 - 10);
        
        gauges[gauge_id].unit_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].unit_label, unit);
        lv_obj_set_style_text_font(gauges[gauge_id].unit_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].unit_label, lv_color_hex(0xaaaaaa), 0);
        lv_obj_align(gauges[gauge_id].unit_label, LV_ALIGN_CENTER, x - 512, y - 300 + 30);
        
        gauges[gauge_id].name_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].name_label, name);
        lv_obj_set_style_text_font(gauges[gauge_id].name_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].name_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].name_label, LV_ALIGN_CENTER, x - 512, y - 300 + size/2);
        
        // R154P: Labels performance - même police (20) pour valeur ET symbole %
        gauges[gauge_id].perf_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_label, &lv_font_montserrat_20, 0);  // R154P: Police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_label, lv_color_hex(0xFFFFFF), 0);
        int perf_x = x + (size + padding*2)/2 - 62;  // R154P: Plus à gauche (-62) pour éviter débordement
        int perf_y = y - size/2 - padding + 12;
        lv_obj_set_pos(gauges[gauge_id].perf_label, perf_x, perf_y);
        
        gauges[gauge_id].perf_percent_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].perf_percent_label, "%");
        lv_obj_set_style_text_font(gauges[gauge_id].perf_percent_label, &lv_font_montserrat_20, 0);  // R154P: Même police 20
        lv_obj_set_style_text_color(gauges[gauge_id].perf_percent_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(gauges[gauge_id].perf_percent_label, perf_x + 40, perf_y + 2);  // R154P: Gap +40 pour police 20
    }

    // JAUGE BATTERIE
    {
        int gauge_id = JBATTERIE;
        int x = 512, y = 455, size = 270;
        const char* name = "BATTERIE";
        const char* unit = "%";
        int max_val = 100;
        uint32_t color = 0x0066CC;
        
        gauges[gauge_id].max_value = max_val;
        gauges[gauge_id].color = color;
        
        int padding = 20;
        lv_obj_t *gauge_bg = lv_obj_create(screen);
        lv_obj_set_size(gauge_bg, size + padding*2, 300);  // R154O: Hauteur 300 au lieu de 305
        lv_obj_set_pos(gauge_bg, x - (size + padding*2)/2, y - size/2 - padding);
        lv_obj_set_style_bg_color(gauge_bg, lv_color_hex(0x1a1a1a), 0);
        lv_obj_set_style_border_width(gauge_bg, 0, 0);
        lv_obj_set_style_radius(gauge_bg, 20, 0);
        lv_obj_set_style_pad_all(gauge_bg, 0, 0);
        lv_obj_set_style_shadow_width(gauge_bg, 20, 0);
        lv_obj_set_style_shadow_color(gauge_bg, lv_color_black(), 0);
        lv_obj_set_style_shadow_ofs_x(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_ofs_y(gauge_bg, 8, 0);
        lv_obj_set_style_shadow_opa(gauge_bg, LV_OPA_40, 0);
        
        createLED(gauge_id, x, y, size, padding);
        
        gauges[gauge_id].arc = lv_arc_create(screen);
        lv_obj_set_size(gauges[gauge_id].arc, size, size);
        lv_obj_set_pos(gauges[gauge_id].arc, x - size/2, y - size/2);
        lv_arc_set_rotation(gauges[gauge_id].arc, 135);
        lv_arc_set_bg_angles(gauges[gauge_id].arc, 0, 270);
        lv_arc_set_range(gauges[gauge_id].arc, 0, max_val);
        lv_arc_set_value(gauges[gauge_id].arc, 0);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 25, LV_PART_MAIN);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 25, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_arc_color(gauges[gauge_id].arc, lv_color_hex(color), LV_PART_INDICATOR);
        lv_obj_clear_flag(gauges[gauge_id].arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_width(gauges[gauge_id].arc, 0, LV_PART_KNOB);
        
        gauges[gauge_id].value_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].value_label, "0");
        lv_obj_set_style_text_font(gauges[gauge_id].value_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].value_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].value_label, LV_ALIGN_CENTER, x - 512, y - 300 - 10);
        
        gauges[gauge_id].unit_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].unit_label, unit);
        lv_obj_set_style_text_font(gauges[gauge_id].unit_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].unit_label, lv_color_hex(0xaaaaaa), 0);
        lv_obj_align(gauges[gauge_id].unit_label, LV_ALIGN_CENTER, x - 512, y - 300 + 30);
        
        gauges[gauge_id].name_label = lv_label_create(screen);
        lv_label_set_text(gauges[gauge_id].name_label, name);
        lv_obj_set_style_text_font(gauges[gauge_id].name_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(gauges[gauge_id].name_label, lv_color_white(), 0);
        lv_obj_align(gauges[gauge_id].name_label, LV_ALIGN_CENTER, x - 512, y - 300 + size/2 - 10);
    }

    Serial.println("[R154O] 4 jauges + 4 LEDs créées + 3 labels perf (état OFF)");
}

void initDisplayGauges(Board *board, const char* boat_name, const char* version_app)
{
    Serial.println("[R154O] Initialisation LVGL + Jauges");
    
    g_lcd = board->getLCD();
    if (g_lcd == nullptr) {
        Serial.println("ERREUR: LCD non disponible!");
        return;
    }
    
    if (!lv_is_initialized()) {
        Serial.println("Initialisation de LVGL...");
        lv_init();
        
        static lv_disp_draw_buf_t disp_buf;
        
        const size_t buf_size = 1024 * 600;
        static lv_color_t *buf1 = (lv_color_t*)heap_caps_malloc(
            buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
        static lv_color_t *buf2 = (lv_color_t*)heap_caps_malloc(
            buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
        
        if (buf1 == nullptr || buf2 == nullptr) {
            Serial.println("ERREUR: Impossible d'allouer les buffers en PSRAM!");
            Serial.printf("PSRAM disponible: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
            return;
        }
        
        Serial.printf("Buffer 1 alloué @ %p (%d KB)\n", buf1, (buf_size * sizeof(lv_color_t)) / 1024);
        Serial.printf("Buffer 2 alloué @ %p (%d KB)\n", buf2, (buf_size * sizeof(lv_color_t)) / 1024);
        Serial.printf("PSRAM restante: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
        
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, buf_size);
        
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.draw_buf = &disp_buf;
        disp_drv.hor_res = 1024;
        disp_drv.ver_res = 600;
        
        disp_drv.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
            if (g_lcd != nullptr) {
                uint32_t w = area->x2 - area->x1 + 1;
                uint32_t h = area->y2 - area->y1 + 1;
                g_lcd->drawBitmap(area->x1, area->y1, w, h, (uint8_t*)color_p);
            }
            lv_disp_flush_ready(drv);
        };
        
        lv_disp_drv_register(&disp_drv);
        Serial.println("LVGL initialisé");
    } else {
        Serial.println("LVGL déjà initialisé");
    }
    
    auto backlight = board->getBacklight();
    if (backlight != nullptr) {
        backlight->on();
        Serial.println("Backlight activé");
    }
    
    createGauges(boat_name, version_app);  // R154O: Passer boat_name et version_app
}

void setGaugeValue(int gauge_id, float value)
{
    if (gauge_id < 0 || gauge_id > 3) return;
    
    Gauge *g = &gauges[gauge_id];
    
    if (value < 0) value = 0;
    if (value > g->max_value) value = g->max_value;

    lv_arc_set_value(g->arc, (int)value);

    char buf[16];
    if (gauge_id == JBATTERIE || gauge_id == JSOLAIRE) {
        snprintf(buf, sizeof(buf), "%.0f", value);
    } else {
        snprintf(buf, sizeof(buf), "%.1f", value);
    }
    lv_label_set_text(g->value_label, buf);
    
    // R154O: Mise à jour performance si label existe
    if (gauges[gauge_id].perf_label != nullptr) {
        int perf = 0;
        if (gauge_id == JSOLAIRE) perf = perfSOL;
        else if (gauge_id == JALTERNATEUR) perf = perfALT;
        else if (gauge_id == JQUAI) perf = perfQUAI;
        
        char buf_perf[8];
        snprintf(buf_perf, sizeof(buf_perf), "%d", perf);  // R154O: Valeur seule (sans %)
        lv_label_set_text(gauges[gauge_id].perf_label, buf_perf);
        
        // R154O: Couleur rouge si >100%, blanc sinon (pour les DEUX labels)
        uint32_t color = (perf > 100) ? 0xFF0000 : 0xFFFFFF;
        lv_obj_set_style_text_color(gauges[gauge_id].perf_label, lv_color_hex(color), 0);
        if (gauges[gauge_id].perf_percent_label != nullptr) {
            lv_obj_set_style_text_color(gauges[gauge_id].perf_percent_label, lv_color_hex(color), 0);
        }
    }
    
    lv_refr_now(NULL);
}


// R154M: Fonction updateLEDStatus optimisÃ©e (un seul lv_refr_now)
void updateLEDStatus()
{
    unsigned long now = millis();
    bool any_change = false;  // R154M: Tracker changements
    
    // LED SOLAIRE (timeout 15s)
    if (data_received_Solaire) {
        leds[JSOLAIRE].current_state = !leds[JSOLAIRE].current_state;
        
        if (leds[JSOLAIRE].current_state) {
            lv_obj_set_style_bg_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_ON_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_ON_BOTTOM), 0);
        } else {
            lv_obj_set_style_bg_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
        }
        lv_obj_invalidate(leds[JSOLAIRE].led_obj);
        data_received_Solaire = false;
        any_change = true;
        
    } else if (now - last_led_update_Solaire > LED_TIMEOUT_MS) {
        if (leds[JSOLAIRE].current_state) {
            leds[JSOLAIRE].current_state = false;
            lv_obj_set_style_bg_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JSOLAIRE].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
            lv_obj_invalidate(leds[JSOLAIRE].led_obj);
            any_change = true;
        }
    }
    
    // LED ALTERNATEUR (timeout 15s)
    if (data_received_Alternateur) {
        leds[JALTERNATEUR].current_state = !leds[JALTERNATEUR].current_state;
        
        if (leds[JALTERNATEUR].current_state) {
            lv_obj_set_style_bg_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_ON_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_ON_BOTTOM), 0);
        } else {
            lv_obj_set_style_bg_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
        }
        lv_obj_invalidate(leds[JALTERNATEUR].led_obj);
        data_received_Alternateur = false;
        any_change = true;
        
    } else if (now - last_led_update_Alternateur > LED_TIMEOUT_MS) {
        if (leds[JALTERNATEUR].current_state) {
            leds[JALTERNATEUR].current_state = false;
            lv_obj_set_style_bg_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JALTERNATEUR].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
            lv_obj_invalidate(leds[JALTERNATEUR].led_obj);
            any_change = true;
        }
    }
    
    // LED QUAI (timeout 60s pour IP22)
    if (data_received_Quai) {
        leds[JQUAI].current_state = !leds[JQUAI].current_state;
        
        if (leds[JQUAI].current_state) {
            lv_obj_set_style_bg_color(leds[JQUAI].led_obj, lv_color_hex(LED_ON_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JQUAI].led_obj, lv_color_hex(LED_ON_BOTTOM), 0);
        } else {
            lv_obj_set_style_bg_color(leds[JQUAI].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JQUAI].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
        }
        lv_obj_invalidate(leds[JQUAI].led_obj);
        data_received_Quai = false;
        any_change = true;
        
    } else if (now - last_led_update_Quai > IP22_TIMEOUT_MS) {
        if (leds[JQUAI].current_state) {
            leds[JQUAI].current_state = false;
            lv_obj_set_style_bg_color(leds[JQUAI].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JQUAI].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
            lv_obj_invalidate(leds[JQUAI].led_obj);
            any_change = true;
        }
    }
    
    // LED BATTERIE (timeout 15s)
    if (data_received_Batterie) {
        leds[JBATTERIE].current_state = !leds[JBATTERIE].current_state;
        
        if (leds[JBATTERIE].current_state) {
            lv_obj_set_style_bg_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_ON_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_ON_BOTTOM), 0);
        } else {
            lv_obj_set_style_bg_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
        }
        lv_obj_invalidate(leds[JBATTERIE].led_obj);
        data_received_Batterie = false;
        any_change = true;
        
    } else if (now - last_led_update_Batterie > LED_TIMEOUT_MS) {
        if (leds[JBATTERIE].current_state) {
            leds[JBATTERIE].current_state = false;
            lv_obj_set_style_bg_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_OFF_TOP), 0);
            lv_obj_set_style_bg_grad_color(leds[JBATTERIE].led_obj, lv_color_hex(LED_OFF_BOTTOM), 0);
            lv_obj_invalidate(leds[JBATTERIE].led_obj);
            any_change = true;
        }
    }
    
    // R154M: RafraÃ®chir UNE FOIS si changement (optimisation)
    if (any_change) {
        lv_refr_now(NULL);
    }
}
