// ========================================
// Fichier: display_overlay.h
// Version 8.50.R133 - Module overlay veille rÃ©utilisable
// ========================================
#ifndef DISPLAY_OVERLAY_H
#define DISPLAY_OVERLAY_H

#include <lvgl.h>
#include <esp_display_panel.hpp>

using namespace esp_panel::board;
using namespace esp_panel::drivers;

// Variables globales pour l'overlay
extern lv_obj_t *sleep_overlay;
extern bool sleep_mode;

// Fonctions du module overlay
void createOverlay();                    // CrÃ©er l'overlay (Ã  appeler EN DERNIER)
void updateTouchBacklight(Board *board); // Gestion tactile et toggle veille

#endif