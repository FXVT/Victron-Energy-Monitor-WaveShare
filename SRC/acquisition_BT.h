// ========================================
// Fichier: acquisition_BT.h
// Version 8.50.R154O - Export performances et caractéristiques
// CHANGEMENTS R154O:
// - Export perfSOL, perfALT, perfQUAI (performances en %)
// - Export CAPACITE_BATTERIE_AH, PSOL, PALT, PQUAI (caractéristiques)
// ========================================
#ifndef ACQUISITION_BT_H
#define ACQUISITION_BT_H

// Fonctions publiques
void initAcquisition();
void updateAcquisition();

// R154L: Timestamps dernière réception pour timeout LED
extern unsigned long last_led_update_Solaire;
extern unsigned long last_led_update_Alternateur;
extern unsigned long last_led_update_Quai;
extern unsigned long last_led_update_Batterie;

// R154L: Flags réception données
extern bool data_received_Solaire;
extern bool data_received_Alternateur;
extern bool data_received_Quai;
extern bool data_received_Batterie;

// R154O: Caractéristiques des appareils (pour jauges et calculs)
extern int CAPACITE_BATTERIE_AH;
extern int PSOL;
extern int PALT;
extern int PQUAI;

// R154O: Variables de performance (%)
extern int perfSOL;
extern int perfALT;
extern int perfQUAI;

#endif
