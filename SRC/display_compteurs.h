// ========================================
// Fichier: display_compteurs.h
// Version 8.50.R154E - Ajout compteur TTG
// CHANGEMENTS R154E:
// - Ajout paramètres TTG à updateCounters()
// ========================================
#ifndef DISPLAY_COMPTEURS_H
#define DISPLAY_COMPTEURS_H

#include <lvgl.h>

// Fonction de création des compteurs
void createCompteurs();

// Fonctions de mise à jour des compteurs
// R154E: Ajout des 4 paramètres TTG (jours, heures, minutes, infini)
void updateCounters(float prodSolar, const char* statusBatt, 
                    float voltageBatt, float ampereBatt, 
                    int heuresFull, int minutesFull,
                    int joursTTG, int heuresTTG, int minutesTTG, bool ttgInfini);

#endif
