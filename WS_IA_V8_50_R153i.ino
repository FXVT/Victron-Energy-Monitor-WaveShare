// A tester. Mettre dans acquisition_BT les clé de chiffrage et tester.
// ========================================
// Fichier: WS_IA_8.50.ino
// Version 8.50.125 - Backlight PWM sur GPIO2
// V8.50.140 Fonctionne avec simulation de Alternateur et quai et récup BT pour le reste mais pas testé
// ========================================

// ⭐ IMPORTANT: Inclure AVANT toute autre bibliothèque NEW
// #include "ESP_Panel_Conf.h"  // ← Votre fichier de config NEW

#include <Arduino.h>

// Au tout début du sketch, AVANT les includes NEW
//#define ESP_PANEL_USE_BOARD_ESP32_S3_WAVESHARE_ESP32_S3_TOUCH_LCD_5B    //NEW
#include <esp_utils_conf.h>
#include <esp_io_expander.hpp>     //NEW
#include <esp_display_panel.hpp>
#include <lvgl.h>

// Vérification de version LVGL
#if LV_VERSION_CHECK(9, 0, 0)
  #error "ERREUR: LVGL 9.x détecté ! Ce projet nécessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
  #warning "Attention: Version LVGL différente de 8.4.0"
#endif

#include "display_gauges.h"
#include "display_compteurs.h"
#include "acquisition_BT.h"
#include "display_overlay.h"  // ← AJOUTER

using namespace esp_panel::board;
using namespace esp_panel::drivers;

Board *board = nullptr;

void setup()
{
    Serial.begin(115200);
    //Serial.flush();  // ← AJOUTER
    delay(500);

    // Afficher la version LVGL détectée
    Serial.printf("\n\n=== INFO VERSIONS ===\n");
    Serial.printf("LVGL Version: %d.%d.%d\n", 
                  LVGL_VERSION_MAJOR, 
                  LVGL_VERSION_MINOR, 
                  LVGL_VERSION_PATCH);
    Serial.printf("=====================\n\n");
    


    Serial.println("=== Démarrage WS_IA_8.50.125 ===");
    
    Serial.println("1. Initialize board");
    board = new Board();
    if (!board->begin()) {
        Serial.println("ERREUR: Impossible d'initialiser le board!");
        while(1) delay(1000);
    }
    Serial.println("1. ✅ Board OK");
    
    Serial.println("2. Initialisation Display Gauges...");
    initDisplayGauges(board);
    Serial.println("2. ✅ Display Gauges OK");
    
    Serial.println("3. Création Compteurs...");
    createCompteurs();
    Serial.println("3. ✅ Compteurs OK");
    
    Serial.println("4. Création Overlay...");
    createOverlay();
    Serial.println("4. ✅ Overlay OK");
    
    Serial.println("5. Initialisation Acquisition...");
    initAcquisition();
    Serial.println("5. ✅ Acquisition OK");
    
    Serial.println("=== Système initialisé ===");
}

void loop()
{
    Serial.println("DEBUT loop");
    // Gérer LVGL (OBLIGATOIRE!)
    lv_timer_handler();
    
    // Mise à jour de l'acquisition (calcul des nouvelles valeurs)
    updateAcquisition();
    
    // Mise à jour tactile pour la luminosité
    updateTouchBacklight(board);
    
    delay(5);
}
