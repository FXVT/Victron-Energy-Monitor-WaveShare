// ========================================
// Fichier: WS_IA_V8_50_R154O.ino
// Version 8.50.R154O - Personnalisation + performances
// CHANGEMENTS R154O:
// - Ajout chapitre Personnalisation (nom bateau)
// - Affichage % performances sur jauges (SOLAIRE, ALTERNATEUR, QUAI)
// - Bandeau bleu pleine largeur pour titre
// - Refactoring: PSOL/PALT/PQUAI utilisés pour max_value des jauges
// ========================================

#include <Arduino.h>
#include <ESP_IOExpander_Library.h>
#include <esp_bt.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x détecté ! Ce projet nécessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
#warning "Attention: Version LVGL différente de 8.4.0"
#endif

#include "display_gauges.h"
#include "display_compteurs.h"
#include "acquisition_BT.h"
#include "display_overlay.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

LV_IMG_DECLARE(Splash_screen_vierge341x200TC);
LV_IMG_DECLARE(Logo_Victron120x120TC);

// R154O: Variable version centralisée
const char *VERSION_APP = "v8.50.R154O";

// ========================================
// Personnalisation
// ========================================
const char *NOM_BATEAU = "ALBA III";  // R154O: Nom du bateau affiché sur splash et titre
// ========================================

Board *board = nullptr;

void displaySplashScreen() {
  Serial.println(">>> [R154O] Affichage splash screen <<<");

  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

  lv_obj_t *splash_img = lv_img_create(screen);
  lv_img_set_src(splash_img, &Splash_screen_vierge341x200TC);
  lv_img_set_zoom(splash_img, 768);
  lv_obj_center(splash_img);

  lv_obj_t *logo_img = lv_img_create(screen);
  lv_img_set_src(logo_img, &Logo_Victron120x120TC);
  lv_obj_align(logo_img, LV_ALIGN_BOTTOM_RIGHT, -30, -30);

  lv_obj_t *text1 = lv_label_create(screen);
  lv_label_set_text(text1, "Monitoring Energie VE.Connect");
  lv_obj_set_style_text_font(text1, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(text1, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text1, LV_ALIGN_CENTER, 0, -80);

  // R154O: Utilisation du nom du bateau depuis la personnalisation
  lv_obj_t *text2 = lv_label_create(screen);
  lv_label_set_text(text2, NOM_BATEAU);
  lv_obj_set_style_text_font(text2, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(text2, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text2, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *text_ver = lv_label_create(screen);
  lv_label_set_text(text_ver, VERSION_APP);
  lv_obj_set_style_text_font(text_ver, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_ver, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text_ver, LV_ALIGN_BOTTOM_LEFT, 20, -20);

  lv_refr_now(NULL);

  for (int i = 0; i < 60; i++) {
    lv_timer_handler();
    delay(50);
  }

  lv_obj_del(splash_img);
  lv_obj_del(logo_img);
  lv_obj_del(text1);
  lv_obj_del(text2);
  lv_obj_del(text_ver);

  Serial.println(">>> [R154O] Splash terminé <<<");
}

void setup() {
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  Serial.setRxBufferSize(2048);
  Serial.setTxBufferSize(2048);
  Serial.begin(115200);

  Serial.println(">>> SETUP START <<<");
  Serial.flush();

  Serial.printf("\n\n=== INFO VERSIONS ===\n");
  Serial.printf("LVGL Version: %d.%d.%d\n",
                LVGL_VERSION_MAJOR,
                LVGL_VERSION_MINOR,
                LVGL_VERSION_PATCH);
  Serial.printf("=====================\n\n");

  Serial.println("=== Démarrage WS_IA_8.50.R154O ===");

  Serial.println("1. Initialize board");
  board = new Board();
  if (!board->begin()) {
    Serial.println("ERREUR: Impossible d'initialiser le board!");
    while (1) delay(1000);
  }
  Serial.println("1. ✓ Board OK");

  Serial.println("2. Initialisation LVGL...");
  initDisplayGauges(board, NOM_BATEAU, VERSION_APP);  // R154O: Passage nom bateau
  Serial.println("2. ✓ LVGL initialisé");

  Serial.println("3. Affichage Splash Screen...");
  displaySplashScreen();
  Serial.println("3. ✓ Splash Screen terminé");

  Serial.println("4. Création Gauges...");
  createGauges(NOM_BATEAU, VERSION_APP);  // R154O: Passage nom bateau
  Serial.println("4. ✓ Gauges créées");

  Serial.println("5. Création Compteurs...");
  createCompteurs();
  Serial.println("5. ✓ Compteurs OK");

  Serial.println("6. Création Overlay...");
  createOverlay();
  Serial.println("6. ✓ Overlay OK");

  Serial.println("7. Initialisation Acquisition...");
  initAcquisition();
  Serial.println("7. ✓ Acquisition OK");

  Serial.println("=== Système initialisé ===");
}

void loop() {
  unsigned long loop_start = millis();
  lv_timer_handler();
  updateAcquisition();
  // R154L/M: Mise à jour LED avec toggle + timeout
  updateLEDStatus();
  updateTouchBacklight(board);
  delay(5);
  Serial.printf("Loop: %lu ms\n", millis() - loop_start);
}
