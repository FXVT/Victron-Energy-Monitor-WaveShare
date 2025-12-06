// ========================================
// Fichier: display_gauges.h
// Version 8.50.R154P - Labels performance harmonisés
// CHANGEMENTS R154P:
// - Labels performance: même police (Montserrat 20) pour valeur ET symbole %
// CHANGEMENTS R154O:
// - Ajout champ perf_label et perf_percent_label dans struct Gauge
// - Signature createGauges() avec boat_name et version_app
// - Signature initDisplayGauges() avec boat_name et version_app
// ========================================
#ifndef DISPLAY_GAUGES_H
#define DISPLAY_GAUGES_H

#include <lvgl.h>
#include <esp_display_panel.hpp>

using namespace esp_panel::board;
using namespace esp_panel::drivers;

// IDs des jauges
#define JSOLAIRE      0
#define JALTERNATEUR  1
#define JQUAI         2
#define JBATTERIE     3

// R154L: Palette de couleurs pour les LEDs de statut
#define LED_ON_TOP     0xFF0000  // LED allumée - Rouge vif clair
#define LED_ON_BOTTOM  0xCC0000  // LED allumée - Rouge vif foncé
#define LED_OFF_TOP    0x330000  // LED éteinte - Rouge très sombre clair
#define LED_OFF_BOTTOM 0x1A0000  // LED éteinte - Rouge très sombre foncé
#define LED_BORDER     0x000000  // Bordure noire

// Structure d'une jauge avec couleur unie
struct Gauge {
    lv_obj_t *arc;
    lv_obj_t *value_label;
    lv_obj_t *unit_label;
    lv_obj_t *name_label;
    lv_obj_t *perf_label;         // R154O: Label valeur % rendement (nullptr pour BATTERIE)
    lv_obj_t *perf_percent_label; // R154O: Label symbole "%" (nullptr pour BATTERIE)
    int max_value;
    uint32_t color;  // Couleur unie
};

// R154L: Structure pour LED de statut avec current_state
struct GaugeLED {
    lv_obj_t *led_obj;      // Cercle principal (LED)
    lv_obj_t *border_obj;   // Cercle de bordure noire
    bool current_state;     // R154L: État actuel (ON/OFF) pour toggle
};

// Fonctions principales
void initDisplayGauges(Board *board, const char* boat_name, const char* version_app);  // R154O: + boat_name
void createGauges(const char* boat_name, const char* version_app);  // R154O: + boat_name
void setGaugeValue(int gauge_id, float value);

// R154L: Fonction de mise à jour des LEDs avec timeout
void updateLEDStatus();

#endif
