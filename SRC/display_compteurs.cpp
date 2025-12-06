// ========================================
// Fichier: display_compteurs.cpp
// Version 8.50.R154N - TTG timeout "---"
// CHANGEMENTS R154N:
// - TTG: Si joursTTG = -1 → afficher "---" (timeout 15s BMV)
// CHANGEMENTS R154M:
// - TTG: Valeur initiale "---" au lieu de "Inf" (pas encore reçu d'état)
// - Production: Libellé aligné sur même baseline que valeur (y+2)
// - Status: Libellé aligné sur même baseline que valeur (y+2)
// - TTG: Libellé aligné sur même baseline que valeur (y+2)
// ========================================
#include <Arduino.h>
#include "display_compteurs.h"

// Labels pour les compteurs
lv_obj_t *label_prodSolar = nullptr;
lv_obj_t *label_statusBatt = nullptr;
lv_obj_t *label_voltageBatterie = nullptr;
lv_obj_t *label_ampereBatterie = nullptr;
lv_obj_t *label_dureeFull = nullptr;
lv_obj_t *label_ttg = nullptr;

void createCompteurs()
{
    lv_obj_t *screen = lv_scr_act();
    
    // COMPTEUR: Production solaire (en bas à gauche)
    {
        int x = 170, y = 430;
        
        lv_obj_t *prod_label = lv_label_create(screen);
        lv_label_set_text(prod_label, "Production:");
        lv_obj_set_style_text_font(prod_label, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(prod_label, lv_color_white(), 0);
        // R154M: Libellé aligné avec valeur (y+2 pour compenser différence de police)
        lv_obj_align(prod_label, LV_ALIGN_TOP_LEFT, x - 105, y + 2);
        
        label_prodSolar = lv_label_create(screen);
        lv_label_set_text(label_prodSolar, "150 Wh");
        lv_obj_set_style_text_font(label_prodSolar, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(label_prodSolar, lv_color_hex(0xFFFF00), 0);
        // R154M: Valeur reste à position horizontale originale
        lv_obj_align(label_prodSolar, LV_ALIGN_TOP_LEFT, x + 65, y);
    }
    
    // COMPTEUR: Status batterie (en bas à gauche)
    {
        int x = 170, y = 470;
        
        lv_obj_t *status_label = lv_label_create(screen);
        lv_label_set_text(status_label, "Status:");
        lv_obj_set_style_text_font(status_label, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(status_label, lv_color_white(), 0);
        // R154M: Libellé aligné avec valeur (y+2 pour compenser différence de police)
        lv_obj_align(status_label, LV_ALIGN_TOP_LEFT, x - 105, y + 2);
        
        label_statusBatt = lv_label_create(screen);
        lv_label_set_text(label_statusBatt, "---");
        lv_obj_set_style_text_font(label_statusBatt, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(label_statusBatt, lv_color_hex(0xFFAA00), 0);
        // R154M: Valeur reste à position horizontale originale
        lv_obj_align(label_statusBatt, LV_ALIGN_TOP_LEFT, x - 5, y);
    }
    
    // COMPTEUR TTG (Time To Go) - en bas à gauche sous "Status"
    {
        int x = 170, y = 510;
        
        lv_obj_t *ttg_label = lv_label_create(screen);
        lv_label_set_text(ttg_label, "TTG:");
        lv_obj_set_style_text_font(ttg_label, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(ttg_label, lv_color_white(), 0);
        // R154M: Libellé aligné avec valeur (y+2 pour compenser différence de police)
        lv_obj_align(ttg_label, LV_ALIGN_TOP_LEFT, x - 105, y + 2);
        
        label_ttg = lv_label_create(screen);
        // R154M: Valeur initiale "---" (pas encore reçu d'état)
        lv_label_set_text(label_ttg, "---");
        lv_obj_set_style_text_font(label_ttg, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(label_ttg, lv_color_hex(0x00AAFF), 0);
        // R154M: Valeur reste à position horizontale originale
        lv_obj_align(label_ttg, LV_ALIGN_TOP_LEFT, x - 35, y);
    }
    
    // COMPTEUR: Voltage batterie (en bas à droite)
    {
        int x = 854, y = 400;
        
        label_voltageBatterie = lv_label_create(screen);
        lv_label_set_text(label_voltageBatterie, "13.3 V");
        lv_obj_set_style_text_font(label_voltageBatterie, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(label_voltageBatterie, lv_color_white(), 0);
        lv_obj_align(label_voltageBatterie, LV_ALIGN_TOP_LEFT, x - 55, y);
    }
    
    // COMPTEUR: Ampérage batterie (en bas à droite)
    {
        int x = 854, y = 460;
        
        label_ampereBatterie = lv_label_create(screen);
        lv_label_set_text(label_ampereBatterie, "0.3 A");
        lv_obj_set_style_text_font(label_ampereBatterie, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(label_ampereBatterie, lv_color_white(), 0);
        lv_obj_align(label_ampereBatterie, LV_ALIGN_TOP_LEFT, x - 45, y);
    }
    
    // COMPTEUR: Durée avant batterie pleine (en bas à droite)
    {
        int x = 854, y = 520;
        
        lv_obj_t *full_label = lv_label_create(screen);
        lv_label_set_text(full_label, "Full:");
        lv_obj_set_style_text_font(full_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(full_label, lv_color_white(), 0);
        lv_obj_align(full_label, LV_ALIGN_TOP_LEFT, x - 95, y);
        
        label_dureeFull = lv_label_create(screen);
        lv_label_set_text(label_dureeFull, "00h00");
        lv_obj_set_style_text_font(label_dureeFull, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(label_dureeFull, lv_color_hex(0x00AAFF), 0);
        lv_obj_align(label_dureeFull, LV_ALIGN_TOP_LEFT, x - 5, y);
    }
    
    Serial.println("[R154N] 6 compteurs créés (TTG initial '---', timeout 15s)");
}

void updateCounters(float prodSolar, const char* statusBatt, float voltageBatt, float ampereBatt, 
                    int heuresFull, int minutesFull,
                    int joursTTG, int heuresTTG, int minutesTTG, bool ttgInfini)
{
    if (label_prodSolar == nullptr || label_statusBatt == nullptr || 
        label_voltageBatterie == nullptr || label_ampereBatterie == nullptr || 
        label_dureeFull == nullptr || label_ttg == nullptr) {
        return;
    }
    
    char buf[32];
    
    // Production solaire
    snprintf(buf, sizeof(buf), "%.0f Wh", prodSolar);
    lv_label_set_text(label_prodSolar, buf);
    
    // Status batterie
    lv_label_set_text(label_statusBatt, statusBatt);
    
    // Voltage batterie
    snprintf(buf, sizeof(buf), "%.1f V", voltageBatt);
    lv_label_set_text(label_voltageBatterie, buf);
    
    // Ampérage batterie (avec changement de couleur si négatif)
    snprintf(buf, sizeof(buf), "%.1f A", ampereBatt);
    lv_label_set_text(label_ampereBatterie, buf);
    if (ampereBatt < 0) {
        lv_obj_set_style_text_color(label_ampereBatterie, lv_color_hex(0xFF8800), 0);
    } else {
        lv_obj_set_style_text_color(label_ampereBatterie, lv_color_white(), 0);
    }
    
    // Durée avant Full
    snprintf(buf, sizeof(buf), "%02dh%02d", heuresFull, minutesFull);
    lv_label_set_text(label_dureeFull, buf);
    
    // Mise à jour compteur TTG
    // R154N: Si joursTTG = -1 → mode timeout (pas de réception BMV depuis 15s)
    if (joursTTG == -1) {
        lv_label_set_text(label_ttg, "---");
    } else if (ttgInfini) {
        // Si charge ou repos → afficher "infini" (minuscules comme demandé en R154J)
        lv_label_set_text(label_ttg, "infini");
    } else {
        // Si décharge > 100mA → afficher temps restant
        if (joursTTG > 0) {
            // Afficher jours uniquement si > 0
            snprintf(buf, sizeof(buf), "%dj %02dh %02dm", joursTTG, heuresTTG, minutesTTG);
        } else {
            // Pas de jours, afficher seulement heures et minutes
            snprintf(buf, sizeof(buf), "%02dh %02dm", heuresTTG, minutesTTG);
        }
        lv_label_set_text(label_ttg, buf);
    }
}
