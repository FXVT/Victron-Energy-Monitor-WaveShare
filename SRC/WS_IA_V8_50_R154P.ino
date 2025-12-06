// ========================================
// Fichier: WS_IA_V8_50_R154O.ino
// Version 8.50.R154O - Personnalisation + performances
// CHANGEMENTS R154O:
// - Ajout chapitre Personnalisation (nom bateau)
// - Affichage % performances sur jauges (SOLAIRE, ALTERNATEUR, QUAI)
// - Bandeau bleu pleine largeur pour titre
// - Refactoring: PSOL/PALT/PQUAI utilisés pour max_value des jauges
// ========================================

// ========================================
// INCLUSIONS DES BIBLIOTHÈQUES
// ========================================

#include <Arduino.h>                      // Bibliothèque de base Arduino pour les fonctions standard
#include <ESP_IOExpander_Library.h>       // Bibliothèque pour gérer les expansions GPIO de l'ESP32
#include <esp_bt.h>                       // Bibliothèque Bluetooth ESP32 pour la communication BLE
#include <esp_display_panel.hpp>          // Bibliothèque pour gérer l'écran LCD de la carte Waveshare
#include <lvgl.h>                         // Bibliothèque graphique LVGL 8.4.0 pour l'interface utilisateur

// ========================================
// VÉRIFICATION DE LA VERSION LVGL
// Ces contrôles s'assurent que la version correcte de LVGL est utilisée
// ========================================

#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x détecté ! Ce projet nécessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
#warning "Attention: Version LVGL différente de 8.4.0"
#endif

// ========================================
// INCLUSIONS DES MODULES PERSONNALISÉS
// ========================================

#include "display_gauges.h"               // Module de gestion des jauges circulaires (SOLAIRE, ALTERNATEUR, QUAI, BATTERIE)
#include "display_compteurs.h"            // Module de gestion des compteurs textuels (Production, Status, TTG, Voltage, etc.)
#include "acquisition_BT.h"               // Module d'acquisition des données Victron via Bluetooth BLE
#include "display_overlay.h"              // Module de gestion de l'overlay de veille (mode économie d'énergie)

// ========================================
// NAMESPACES
// Simplifie l'utilisation des classes du panneau ESP
// ========================================

using namespace esp_panel::board;        // Namespace pour les fonctions du board
using namespace esp_panel::drivers;      // Namespace pour les drivers LCD et tactile

// ========================================
// DÉCLARATION DES IMAGES INTÉGRÉES
// Ces images sont stockées en mémoire Flash et utilisées pour le splash screen
// ========================================

LV_IMG_DECLARE(Splash_screen_vierge341x200TC);  // Image de fond du splash screen (341x200 pixels)
LV_IMG_DECLARE(Logo_Victron120x120TC);          // Logo Victron (120x120 pixels) affiché en bas à droite

// ========================================
// VARIABLES GLOBALES DE CONFIGURATION
// ========================================

// R154O: Variable version centralisée - affichée sur le splash et en bas de l'écran principal
const char *VERSION_APP = "v8.50.R154O";

// ========================================
// CUSTOMIZATION
// Modifiez ces paramètres pour adapter l'application à votre installation
// ========================================

const char *NOM_BATEAU = "ALBA III";  // R154O: Nom du bateau affiché sur le splash screen et dans le titre principal
                                       // Peut être modifié pour tout autre nom (bateau, camping-car, installation fixe, etc.)

// ========================================
// POINTEUR GLOBAL VERS LE BOARD
// Permet d'accéder au matériel (écran, tactile, backlight) depuis tout le programme
// ========================================

Board *board = nullptr;  // Initialisé à nullptr pour détecter les erreurs d'initialisation

// ========================================
// FONCTION: displaySplashScreen()
// DESCRIPTION: Affiche l'écran de démarrage pendant 3 secondes
// APPELÉE PAR: setup() une seule fois au démarrage
// ========================================
// Cette fonction crée un écran d'accueil professionnel qui s'affiche pendant
// le chargement du système. Il comprend:
// - Une image de fond
// - Le logo Victron
// - Le titre "Monitoring Energie VE.Connect"
// - Le nom du bateau (personnalisable)
// - Le numéro de version
// ========================================

void displaySplashScreen() {
  Serial.println(">>> [R154O] Affichage splash screen <<<");

  // Récupération de l'écran actif LVGL
  lv_obj_t *screen = lv_scr_act();
  
  // Fond noir pour l'écran de démarrage
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

  // ========================================
  // CRÉATION IMAGE DE FOND (341x200 pixels agrandie à 75%)
  // ========================================
  lv_obj_t *splash_img = lv_img_create(screen);
  lv_img_set_src(splash_img, &Splash_screen_vierge341x200TC);
  lv_img_set_zoom(splash_img, 768);  // Zoom à 768/256 = 3x (agrandissement à 300%)
  lv_obj_center(splash_img);         // Centré sur l'écran

  // ========================================
  // CRÉATION LOGO VICTRON (120x120 pixels en bas à droite)
  // ========================================
  lv_obj_t *logo_img = lv_img_create(screen);
  lv_img_set_src(logo_img, &Logo_Victron120x120TC);
  lv_obj_align(logo_img, LV_ALIGN_BOTTOM_RIGHT, -30, -30);  // Marge de 30px depuis le coin

  // ========================================
  // TEXTE 1: "Monitoring Energie VE.Connect"
  // Police Montserrat 48px, blanc, centré horizontalement, décalé vers le haut
  // ========================================
  lv_obj_t *text1 = lv_label_create(screen);
  lv_label_set_text(text1, "Monitoring Energie VE.Connect");
  lv_obj_set_style_text_font(text1, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(text1, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text1, LV_ALIGN_CENTER, 0, -80);  // Décalage de -80px vers le haut

  // ========================================
  // TEXTE 2: Nom du bateau (variable personnalisable)
  // R154O: Utilisation du nom du bateau depuis la personnalisation
  // Police Montserrat 48px, blanc, centré
  // ========================================
  lv_obj_t *text2 = lv_label_create(screen);
  lv_label_set_text(text2, NOM_BATEAU);  // Utilise la variable NOM_BATEAU définie en haut du fichier
  lv_obj_set_style_text_font(text2, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(text2, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text2, LV_ALIGN_CENTER, 0, 0);  // Centré parfaitement

  // ========================================
  // TEXTE VERSION: Numéro de version en bas à gauche
  // Police Montserrat 20px (plus petite), blanc
  // ========================================
  lv_obj_t *text_ver = lv_label_create(screen);
  lv_label_set_text(text_ver, VERSION_APP);  // Utilise la variable VERSION_APP
  lv_obj_set_style_text_font(text_ver, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_ver, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(text_ver, LV_ALIGN_BOTTOM_LEFT, 20, -20);  // Marge de 20px depuis le coin

  // ========================================
  // RAFRAÎCHISSEMENT IMMÉDIAT DE L'ÉCRAN
  // Force l'affichage de tous les éléments créés
  // ========================================
  lv_refr_now(NULL);

  // ========================================
  // BOUCLE D'ATTENTE DE 3 SECONDES
  // 60 itérations × 50ms = 3000ms = 3 secondes
  // lv_timer_handler() gère les animations LVGL pendant l'attente
  // ========================================
  for (int i = 0; i < 60; i++) {
    lv_timer_handler();  // Traitement des tâches LVGL (animations, etc.)
    delay(50);           // Pause de 50 millisecondes
  }

  // ========================================
  // NETTOYAGE: Suppression de tous les objets du splash screen
  // Libère la mémoire avant de passer à l'écran principal
  // ========================================
  lv_obj_del(splash_img);   // Suppression de l'image de fond
  lv_obj_del(logo_img);     // Suppression du logo Victron
  lv_obj_del(text1);        // Suppression du titre principal
  lv_obj_del(text2);        // Suppression du nom du bateau
  lv_obj_del(text_ver);     // Suppression du numéro de version

  Serial.println(">>> [R154O] Splash terminé <<<");
}
// ========================================
// FIN DE LA FONCTION displaySplashScreen()
// ========================================

// ========================================
// FONCTION: setup()
// DESCRIPTION: Fonction d'initialisation Arduino, exécutée une seule fois au démarrage
// SÉQUENCE D'INITIALISATION:
// 1. Configuration Bluetooth (libération mémoire Classic BT)
// 2. Configuration Serial (communication USB pour debug)
// 3. Initialisation du board (écran + tactile)
// 4. Initialisation LVGL (bibliothèque graphique)
// 5. Affichage du splash screen (3 secondes)
// 6. Création des jauges (4 jauges circulaires)
// 7. Création des compteurs (6 compteurs textuels)
// 8. Création de l'overlay (écran de veille)
// 9. Initialisation acquisition BLE (scan Victron)
// ========================================

void setup() {
  // ========================================
  // ÉTAPE PRÉLIMINAIRE: Libération mémoire Bluetooth Classic
  // Le mode Classic BT n'est pas utilisé (on utilise BLE uniquement)
  // Cette libération récupère ~40KB de RAM pour le système
  // ========================================
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  // ========================================
  // CONFIGURATION SERIAL (Communication USB pour debug)
  // Buffers augmentés à 2048 octets pour gérer le volume de logs BLE
  // Vitesse: 115200 bauds (standard pour ESP32)
  // ========================================
  Serial.setRxBufferSize(2048);  // Buffer de réception augmenté
  Serial.setTxBufferSize(2048);  // Buffer de transmission augmenté
  Serial.begin(115200);          // Démarrage du port série à 115200 bauds

  // ========================================
  // LOGS DE DÉMARRAGE
  // ========================================
  Serial.println(">>> SETUP START <<<");
  Serial.flush();  // Force l'envoi immédiat sur le port série

  // ========================================
  // AFFICHAGE DES INFORMATIONS DE VERSION
  // Utile pour vérifier que LVGL 8.4.0 est bien utilisé
  // ========================================
  Serial.printf("\n\n=== INFO VERSIONS ===\n");
  Serial.printf("LVGL Version: %d.%d.%d\n",
                LVGL_VERSION_MAJOR,   // Version majeure (doit être 8)
                LVGL_VERSION_MINOR,   // Version mineure (doit être 4)
                LVGL_VERSION_PATCH);  // Version patch (doit être 0)
  Serial.printf("=====================\n\n");

  Serial.println("=== Démarrage WS_IA_8.50.R154O ===");

  // ========================================
  // ÉTAPE 1: INITIALISATION DU BOARD
  // Initialise l'écran LCD, le contrôleur tactile et le backlight
  // Si l'initialisation échoue, le système reste bloqué (boucle infinie)
  // ========================================
  Serial.println("1. Initialize board");
  board = new Board();  // Création de l'objet Board (allocation mémoire)
  
  if (!board->begin()) {
    // Erreur fatale: impossible de communiquer avec l'écran
    Serial.println("ERREUR: Impossible d'initialiser le board!");
    while (1) delay(1000);  // Boucle infinie avec message d'erreur répété toutes les secondes
  }
  Serial.println("1. ✓ Board OK");

  // ========================================
  // ÉTAPE 2: INITIALISATION LVGL
  // Configure la bibliothèque graphique LVGL avec:
  // - Double buffering PSRAM (2 buffers de 1024×600 pixels)
  // - Driver d'affichage pour la Waveshare
  // - Activation du backlight
  // ========================================
  Serial.println("2. Initialisation LVGL...");
  initDisplayGauges(board, NOM_BATEAU, VERSION_APP);  // R154O: Passage du nom du bateau et de la version
  Serial.println("2. ✓ LVGL initialisé");

  // ========================================
  // ÉTAPE 3: AFFICHAGE DU SPLASH SCREEN
  // Écran d'accueil pendant 3 secondes
  // Donne le temps au système BLE de se stabiliser
  // ========================================
  Serial.println("3. Affichage Splash Screen...");
  displaySplashScreen();  // Appel de la fonction définie plus haut
  Serial.println("3. ✓ Splash Screen terminé");

  // ========================================
  // ÉTAPE 4: CRÉATION DES JAUGES
  // Crée les 4 jauges circulaires avec leurs LEDs de statut:
  // - Jauge SOLAIRE (0-350W, couleur jaune)
  // - Jauge ALTERNATEUR (0-50A, couleur orange)
  // - Jauge QUAI (0-30A, couleur verte)
  // - Jauge BATTERIE (0-100%, couleur bleue)
  // ========================================
  Serial.println("4. Création Gauges...");
  createGauges(NOM_BATEAU, VERSION_APP);  // R154O: Passage du nom du bateau et de la version
  Serial.println("4. ✓ Gauges créées");

  // ========================================
  // ÉTAPE 5: CRÉATION DES COMPTEURS
  // Crée les 6 compteurs textuels:
  // - Production solaire (Wh)
  // - Status batterie (Bulk/Absorption/Float/Storage)
  // - TTG (Time To Go - autonomie restante)
  // - Voltage batterie (V)
  // - Ampérage batterie (A)
  // - Durée avant batterie pleine (Full)
  // ========================================
  Serial.println("5. Création Compteurs...");
  createCompteurs();  // Appel du module display_compteurs.cpp
  Serial.println("5. ✓ Compteurs OK");

  // ========================================
  // ÉTAPE 6: CRÉATION DE L'OVERLAY DE VEILLE
  // Crée un overlay noir semi-transparent qui couvre tout l'écran
  // Affiché/masqué par un appui tactile pour économiser l'énergie
  // ========================================
  Serial.println("6. Création Overlay...");
  createOverlay();  // Appel du module display_overlay.cpp
  Serial.println("6. ✓ Overlay OK");

  // ========================================
  // ÉTAPE 7: INITIALISATION DE L'ACQUISITION BLE
  // Configure le scan BLE pour détecter les appareils Victron:
  // - SmartSolar MPPT
  // - BMV-712 (moniteur batterie)
  // - Orion XS (chargeur alternateur)
  // - IP22 (chargeur secteur)
  // Paramètres de scan: intervalle 80ms, fenêtre 79ms (scan quasi-continu)
  // Stack BLE: 8192 octets (anti-overflow)
  // ========================================
  Serial.println("7. Initialisation Acquisition...");
  initAcquisition();  // Appel du module acquisition_BT.cpp
  Serial.println("7. ✓ Acquisition OK");

  Serial.println("=== Système initialisé ===");
}
// ========================================
// FIN DE LA FONCTION setup()
// Le système est maintenant prêt, la boucle loop() va démarrer
// ========================================

// ========================================
// FONCTION: loop()
// DESCRIPTION: Boucle principale Arduino, exécutée en continu après setup()
// FRÉQUENCE: Environ 200 fois par seconde (délai de 5ms + temps d'exécution)
// TÂCHES EFFECTUÉES:
// 1. Gestion LVGL (animations, rafraîchissement écran)
// 2. Mise à jour acquisition BLE (scan des appareils Victron)
// 3. Mise à jour des LEDs de statut (clignotement + timeout)
// 4. Gestion du tactile et de l'overlay de veille
// 5. Pause de 5ms (évite la saturation du CPU)
// 6. Log de la durée d'exécution (debug performances)
// ========================================

void loop() {
  // ========================================
  // MESURE DE PERFORMANCE: Début du chronomètre
  // Permet de surveiller la durée d'exécution de la boucle
  // Objectif: < 100ms pour une UI fluide
  // ========================================
  unsigned long loop_start = millis();  // Timestamp de début en millisecondes

  // ========================================
  // TÂCHE 1: GESTIONNAIRE LVGL
  // Traite tous les événements LVGL en attente:
  // - Animations
  // - Rafraîchissement des zones modifiées
  // - Gestion des timers LVGL
  // DOIT être appelé régulièrement (au moins toutes les 5-10ms)
  // ========================================
  lv_timer_handler();

  // ========================================
  // TÂCHE 2: ACQUISITION DES DONNÉES BLE
  // Lance un scan BLE de 1 seconde (défini dans acquisition_BT.cpp)
  // Déchiffre les paquets Victron reçus
  // Met à jour les variables globales (VSOL, VALT, VQUAI, VBAT, etc.)
  // Met à jour l'affichage des jauges et compteurs
  // Gère les timeouts (15s pour Orion/Solar/BMV, 60s pour IP22)
  // ========================================
  updateAcquisition();

  // ========================================
  // TÂCHE 3: MISE À JOUR DES LEDs DE STATUT
  // R154L/M: Toggle des LEDs à chaque réception de données
  // Gère l'extinction automatique après timeout:
  // - 15 secondes pour SOLAIRE, ALTERNATEUR, BATTERIE
  // - 60 secondes pour QUAI (IP22 transmet moins souvent)
  // Optimisation R154M: Un seul lv_refr_now() pour économiser du CPU
  // ========================================
  updateLEDStatus();

  // ========================================
  // TÂCHE 4: GESTION DU TACTILE ET VEILLE
  // Détecte les appuis sur l'écran tactile
  // Bascule entre mode normal et mode veille
  // Entrée en veille: avec debounce (300ms)
  // Sortie de veille: immédiate (pour une meilleure réactivité)
  // ========================================
  updateTouchBacklight(board);

  // ========================================
  // PAUSE DE 5ms
  // Évite la saturation du CPU (watchdog ESP32)
  // Laisse du temps aux tâches système (WiFi, BLE, etc.)
  // ========================================
  delay(5);

  // ========================================
  // LOG DE PERFORMANCE
  // Affiche la durée d'exécution de la boucle
  // Utile pour détecter les ralentissements
  // Durée normale: 1000-1050ms (scan BLE de 1s + 5ms de pause + overhead)
  // ========================================
  Serial.printf("Loop: %lu ms\n", millis() - loop_start);
}
// ========================================
// FIN DE LA FONCTION loop()
// Cette boucle se répète indéfiniment jusqu'à l'arrêt du système
// ========================================
