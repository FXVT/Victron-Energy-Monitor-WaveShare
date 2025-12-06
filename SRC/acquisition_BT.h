// ========================================
// Fichier: acquisition_BT.h
// Version 8.50.R154O - Export performances et caractéristiques
// CHANGEMENTS R154O:
// - Export perfSOL, perfALT, perfQUAI (performances en %)
// - Export CAPACITE_BATTERIE_AH, PSOL, PALT, PQUAI (caractéristiques)
// ========================================

// ========================================
// PROTECTION CONTRE L'INCLUSION MULTIPLE
// Empêche le fichier d'être inclus plusieurs fois dans le même projet
// ========================================
#ifndef ACQUISITION_BT_H
#define ACQUISITION_BT_H

// ========================================
// FONCTIONS PUBLIQUES D'ACQUISITION BLE
// Ces fonctions sont appelées depuis le fichier principal .ino
// ========================================

// Fonction: initAcquisition()
// Initialise le système Bluetooth BLE:
// - Libère la mémoire Bluetooth Classic (récupération ~40KB RAM)
// - Configure la stack BLE (8192 bytes anti-overflow)
// - Initialise le scanner BLE avec paramètres optimisés (intervalle 80ms, fenêtre 79ms)
// - Enregistre les callbacks pour traiter les paquets reçus
// - Affiche les informations de configuration (MAC, clés, capacités)
// APPELÉE PAR: setup() une seule fois au démarrage
void initAcquisition();

// Fonction: updateAcquisition()
// Met à jour l'acquisition des données BLE:
// - Lance un scan BLE de 1 seconde (défini par scanTime)
// - Détecte les appareils Victron (SmartSolar, BMV-712, Orion XS, IP22)
// - Déchiffre les données reçues avec les clés AES-128
// - Met à jour les jauges avec les nouvelles valeurs
// - Met à jour les compteurs (production, status, TTG, voltage, ampérage)
// - Gère les timeouts (15s pour Solar/Orion/BMV, 60s pour IP22)
// - Nettoie les résultats du scan
// APPELÉE PAR: loop() en continu (~200 fois par seconde)
void updateAcquisition();

// ========================================
// TIMESTAMPS DE RÉCEPTION DES DONNÉES
// R154L: Ces timestamps sont utilisés pour gérer le timeout des LEDs de statut
// Permettent de détecter l'absence de communication avec un appareil
// ========================================

// Timestamp (en millisecondes) de la dernière réception de données du SmartSolar
// Utilisé pour éteindre la LED après 15 secondes sans réception
extern unsigned long last_led_update_Solaire;

// Timestamp (en millisecondes) de la dernière réception de données de l'Orion XS
// Utilisé pour éteindre la LED après 15 secondes sans réception
extern unsigned long last_led_update_Alternateur;

// Timestamp (en millisecondes) de la dernière réception de données de l'IP22
// Utilisé pour éteindre la LED après 60 secondes sans réception (timeout plus long)
extern unsigned long last_led_update_Quai;

// Timestamp (en millisecondes) de la dernière réception de données du BMV-712
// Utilisé pour éteindre la LED après 15 secondes sans réception
extern unsigned long last_led_update_Batterie;

// ========================================
// FLAGS DE RÉCEPTION DE DONNÉES
// R154L: Ces booléens indiquent si de nouvelles données ont été reçues
// Utilisés pour faire clignoter les LEDs (toggle ON/OFF à chaque réception)
// Remis à false après traitement dans updateLEDStatus()
// ========================================

// Flag: Données SmartSolar reçues (true = données fraîches disponibles)
// Provoque le clignotement de la LED jaune de la jauge SOLAIRE
extern bool data_received_Solaire;

// Flag: Données Orion XS reçues (true = données fraîches disponibles)
// Provoque le clignotement de la LED orange de la jauge ALTERNATEUR
extern bool data_received_Alternateur;

// Flag: Données IP22 reçues (true = données fraîches disponibles)
// Provoque le clignotement de la LED verte de la jauge QUAI
extern bool data_received_Quai;

// Flag: Données BMV-712 reçues (true = données fraîches disponibles)
// Provoque le clignotement de la LED bleue de la jauge BATTERIE
extern bool data_received_Batterie;

// ========================================
// CARACTÉRISTIQUES DES APPAREILS (PERSONNALISATION)
// R154O: Ces valeurs sont modifiables dans acquisition_BT.cpp
// Elles définissent les capacités maximales des équipements installés
// Utilisées pour:
// - Définir les échelles des jauges (max_value)
// - Calculer les performances (%) affichées sur les jauges
// - Calculer le Time To Go (TTG) de la batterie
// ========================================

// Capacité de la batterie en Ampères-heures (Ah)
// Valeur par défaut: 280 Ah
// Utilisée pour:
// - Calculer le Time To Go (autonomie restante en jours/heures/minutes)
// - Calculer le temps de charge complet (Full)
// - Calculer la capacité restante en fonction du SOC (State Of Charge)
// MODIFIABLE dans le chapitre Personnalisation de acquisition_BT.cpp
extern int CAPACITE_BATTERIE_AH;

// Puissance maximale du panneau solaire en Watts (W)
// Valeur par défaut: 350 W
// Utilisée pour:
// - Définir l'échelle de la jauge SOLAIRE (0-350W)
// - Calculer la performance solaire: perfSOL = (puissance_actuelle / PSOL) × 100%
// MODIFIABLE dans le chapitre Personnalisation de acquisition_BT.cpp
extern int PSOL;

// Puissance maximale de l'alternateur en Ampères (A)
// Valeur par défaut: 50 A
// Utilisée pour:
// - Définir l'échelle de la jauge ALTERNATEUR (0-50A)
// - Calculer la performance alternateur: perfALT = (courant_actuel / PALT) × 100%
// MODIFIABLE dans le chapitre Personnalisation de acquisition_BT.cpp
extern int PALT;

// Puissance maximale du chargeur de quai en Ampères (A)
// Valeur par défaut: 30 A
// Utilisée pour:
// - Définir l'échelle de la jauge QUAI (0-30A)
// - Calculer la performance chargeur: perfQUAI = (courant_actuel / PQUAI) × 100%
// MODIFIABLE dans le chapitre Personnalisation de acquisition_BT.cpp
extern int PQUAI;

// ========================================
// VARIABLES DE PERFORMANCE (%)
// R154O: Performances calculées en temps réel pour chaque appareil
// Affichées sur les jauges SOLAIRE, ALTERNATEUR et QUAI
// Formule: performance = (valeur_actuelle / valeur_maximale) × 100
// Mise à jour à chaque réception de données BLE
// ========================================

// Performance du panneau solaire en pourcentage (0-100%)
// Calcul: perfSOL = (puissance_actuelle_watts / PSOL) × 100
// Exemple: Si SmartSolar produit 175W et PSOL=350W → perfSOL = 50%
// Affichée sous forme de texte sur la jauge SOLAIRE
extern int perfSOL;

// Performance de l'alternateur en pourcentage (0-100%)
// Calcul: perfALT = (courant_actuel_amperes / PALT) × 100
// Exemple: Si Orion XS débite 25A et PALT=50A → perfALT = 50%
// Affichée sous forme de texte sur la jauge ALTERNATEUR
extern int perfALT;

// Performance du chargeur de quai en pourcentage (0-100%)
// Calcul: perfQUAI = (courant_actuel_amperes / PQUAI) × 100
// Exemple: Si IP22 débite 15A et PQUAI=30A → perfQUAI = 50%
// Affichée sous forme de texte sur la jauge QUAI
extern int perfQUAI;

// ========================================
// FIN DU FICHIER HEADER
// ========================================
#endif  // ACQUISITION_BT_H
