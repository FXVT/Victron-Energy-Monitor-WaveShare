# ALBA III - Monitoring Énergie VE.Connect

**Version actuelle:** v8.50.R154P

Système de monitoring complet pour installation électrique marine utilisant des équipements Victron Energy. Affichage en temps réel sur écran tactile ESP32-S3 des données acquises via Bluetooth Low Energy (BLE).

![Version](https://img.shields.io/badge/version-8.50.R154P-blue)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-green)
![LVGL](https://img.shields.io/badge/LVGL-8.4.0-orange)

---

## 📋 Table des matières

- [Vue d'ensemble](#-vue-densemble)
- [Caractéristiques](#-caractéristiques)
- [Architecture du projet](#-architecture-du-projet)
- [Fichiers du projet](#-fichiers-du-projet)
- [Personnalisation](#-personnalisation)
- [Matériel requis](#-matériel-requis)
- [Installation](#-installation)
- [Configuration BLE](#-configuration-ble)
- [Performances et optimisations](#-performances-et-optimisations)
- [Évolutions récentes](#-évolutions-récentes)

---

## 🎯 Vue d'ensemble

ALBA III est un système de monitoring énergétique conçu spécifiquement pour les installations marines équipées de produits Victron Energy. Il permet de visualiser en temps réel :

- **Production solaire** (SmartSolar MPPT)
- **Charge alternateur** (Orion XS)
- **Charge secteur** (IP22 Shore Power)
- **État batterie** (BMV-712)

Le système utilise une communication BLE cryptée avec les appareils Victron et affiche les données sur un écran tactile 1024×600 pixels avec interface graphique LVGL professionnelle.

---

## ✨ Caractéristiques

### Interface utilisateur
- **4 jauges circulaires** avec arcs de progression et LEDs de statut
- **6 compteurs textuels** (production, voltage, ampérage, TTG, etc.)
- **Indicateurs de performance** (% du max) pour Solar/Alternateur/Quai
- **Mode veille tactile** pour économie d'énergie
- **Double buffering PSRAM** pour affichage instantané sans scintillement
- **Bandeau supérieur** avec logo Victron et nom du bateau

### Acquisition de données
- **Scan BLE synchrone** optimisé (intervalle 80ms, fenêtre 79ms)
- **Déchiffrement AES-128** des paquets Victron
- **Gestion intelligente des timeouts** (15s général, 60s pour IP22)
- **Filtrage environnemental** (ignore les interférences Apple/autres BLE)
- **Taux de réception** >99% dans environnement saturé

### Performances
- **Boucle principale** ~200 Hz (5ms de cycle)
- **Mémoire optimisée** (variables static, économie stack)
- **LED avec toggle** automatique et extinction sur timeout
- **TTG (Time To Go)** calculé avec affichage dynamique

---

## 🏗️ Architecture du projet

```
┌─────────────────┐
│  WS_IA_V8.ino   │ ← Point d'entrée principal
└────────┬────────┘
         │
         ├──→ [Initialisation]
         │    │
         │    ├──→ display_gauges.cpp     (LVGL + Jauges)
         │    ├──→ display_compteurs.cpp  (Compteurs texte)
         │    ├──→ display_overlay.cpp    (Mode veille)
         │    └──→ acquisition_BT.cpp     (BLE Victron)
         │
         └──→ [Boucle loop()]
              │
              ├──→ lv_timer_handler()     (Gestion LVGL)
              ├──→ updateAcquisition()    (Scan BLE + déchiffrement)
              ├──→ updateLEDStatus()      (Clignotement LEDs)
              └──→ updateTouchBacklight() (Gestion tactile)
```

### Flux de données

```
Appareils Victron (BLE crypté)
         ↓
acquisition_BT.cpp (Scan + AES decrypt)
         ↓
Variables globales (VSOL, VALT, VQUAI, VBAT, perfSOL, perfALT, perfQUAI)
         ↓
    ┌────┴────┐
    ↓         ↓
display_gauges   display_compteurs
    ↓         ↓
  LVGL (affichage écran)
```

---

## 📁 Fichiers du projet

### Fichier principal

#### `WS_IA_V8_50_R154P.ino` (v8.50.R154P)
**Rôle:** Point d'entrée principal de l'application Arduino  
**Fonctions:**
- Configuration initiale (Serial, Bluetooth, Board)
- Affichage du splash screen (3 secondes)
- Initialisation de tous les modules (LVGL, jauges, compteurs, overlay, BLE)
- Boucle principale coordonnant tous les modules
- Mesure de performance (durée loop)

**Appelle:** 
- `display_gauges.cpp` (initDisplayGauges, createGauges, setGaugeValue, updateLEDStatus)
- `display_compteurs.cpp` (createCompteurs, updateCounters)
- `display_overlay.cpp` (createOverlay, updateTouchBacklight)
- `acquisition_BT.cpp` (initAcquisition, updateAcquisition)

**Configuration:** Variables `NOM_BATEAU` et `VERSION_APP` en début de fichier

---

### Fichiers de configuration

#### `esp_panel_board_supported_conf.h`
**Rôle:** Configuration matérielle du board Waveshare  
**Contenu:** Sélection du modèle ESP32-S3 Touch LCD (actuellement `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_5_B`)  
**À modifier:** Ligne avec `#define BOARD_WAVESHARE_...` pour changer de modèle d'écran

#### `esp_utils_conf.h`
**Rôle:** Configuration des utilitaires ESP (log, mémoire, plugins)  
**Paramètres importants:**
- `ESP_UTILS_CONF_LOG_LEVEL` : Niveau de verbosité des logs
- `ESP_UTILS_CONF_MEM_GEN_ALLOC_TYPE` : Type d'allocation mémoire

---

### Module d'acquisition BLE

#### `acquisition_BT.h` + `acquisition_BT.cpp` (v8.50.R154O)
**Rôle:** Gestion complète de l'acquisition des données Victron via BLE

**Fonctionnalités:**
- Scan BLE synchrone (paramètres optimisés pour stabilité)
- Déchiffrement AES-128 des paquets Victron
- Identification des 4 appareils par adresse MAC
- Calcul des performances (%) pour Solar/Alt/Quai
- Gestion des timeouts différenciés (15s/60s)
- Filtrage précoce des vendorID non-Victron (0x02e1)

**Variables exportées:**
- `VSOL`, `VALT`, `VQUAI`, `VBAT` (valeurs instantanées)
- `perfSOL`, `perfALT`, `perfQUAI` (pourcentages de performance)
- `data_received_*` (flags pour clignotement LED)
- `last_led_update_*` (timestamps pour timeouts)
- `CAPACITE_BATTERIE_AH`, `PSOL`, `PALT`, `PQUAI` (caractéristiques)

**Appelle:** 
- `display_gauges::setGaugeValue()` pour mettre à jour les jauges
- `display_compteurs::updateCounters()` pour mettre à jour les compteurs

**Appelé par:** `WS_IA_V8.ino::loop()` via `updateAcquisition()`

**⚙️ PERSONNALISATION (dans acquisition_BT.cpp):**

```cpp
// Caractéristiques des appareils
int CAPACITE_BATTERIE_AH = 280;  // Capacité batterie (Ah)
int PSOL = 350;   // Puissance max panneau solaire (Watt)
int PALT = 50;    // Puissance max alternateur (Ampère)
int PQUAI = 30;   // Puissance max chargeur de quai (Ampère)

// Adresses MAC des appareils Victron
#define MAC_SMARTSOLAR "f3:81:dc:56:9f:97"
#define MAC_BMV_712 "c5:1d:ac:ed:91:92"
#define MAC_ORION_XS "fb:c1:a3:08:4e:8c"
#define MAC_IP22 "fb:82:24:5d:bb:27"

// Clés de déchiffrement AES-128 (16 bytes)
uint8_t key_SmartSolar[16] = { ... };
uint8_t key_BMV[16] = { ... };
uint8_t key_OrionXS[16] = { ... };
uint8_t key_IP22[16] = { ... };
```

**Optimisations techniques:**
- Variables `static` dans `onResult()` pour économie de stack
- Réutilisation des buffers AES (inputData, outputData)
- Suppression des variables locales temporaires
- Filtrage vendorID avant allocation mémoire

---

### Module d'affichage des jauges

#### `display_gauges.h` + `display_gauges.cpp` (v8.50.R154P)
**Rôle:** Gestion des 4 jauges circulaires et de leurs LEDs de statut

**Contenu:**
- **Jauge SOLAIRE** (0-350W, jaune #FFDD00, LED jaune)
- **Jauge ALTERNATEUR** (0-50A, orange #FF8800, LED orange)
- **Jauge QUAI** (0-30A, vert #00DD00, LED verte)
- **Jauge BATTERIE** (0-100%, bleu #0066CC, LED bleue)
- **Bandeau supérieur** bleu (#005FBE) avec logo Victron et nom du bateau
- **Labels de performance** (valeur + "%" en Montserrat 20)

**Structure Gauge:**
```cpp
struct Gauge {
    lv_obj_t *arc;                 // Arc de progression
    lv_obj_t *value_label;         // Valeur numérique
    lv_obj_t *unit_label;          // Unité (W, A, %)
    lv_obj_t *name_label;          // Nom (SOLAIRE, etc.)
    lv_obj_t *perf_label;          // Performance (0-125)
    lv_obj_t *perf_percent_label;  // Symbole "%"
    int max_value;                 // Valeur maximale (PSOL, PALT, PQUAI)
    uint32_t color;                // Couleur de l'arc
};
```

**Structure GaugeLED:**
```cpp
struct GaugeLED {
    lv_obj_t *led_obj;      // Cercle LED (15×15px)
    lv_obj_t *border_obj;   // Bordure noire (25×25px)
    bool current_state;     // État ON/OFF pour toggle
};
```

**Fonctions publiques:**
- `initDisplayGauges(Board*, boat_name, version_app)` : Initialise LVGL + double buffering PSRAM
- `createGauges(boat_name, version_app)` : Crée les 4 jauges + bandeau + logo
- `setGaugeValue(gauge_id, value)` : Met à jour une jauge + performance
- `updateLEDStatus()` : Gère le clignotement et timeout des LEDs

**Utilise:**
- Variables de `acquisition_BT.h` (PSOL, PALT, PQUAI, perfSOL, perfALT, perfQUAI)
- Variables de `acquisition_BT.h` (data_received_*, last_led_update_*)

**Appelé par:** 
- `WS_IA_V8.ino::setup()` (init + create)
- `acquisition_BT.cpp::updateAcquisition()` (setGaugeValue)
- `WS_IA_V8.ino::loop()` (updateLEDStatus)

**Spécificités R154P:**
- Labels performance harmonisés (même police 20 pour valeur ET "%")
- Position optimisée (-62 au lieu de -38) pour éviter débordement
- Gap fixe +40 entre valeur et "%" pour tous les cas (0% à 125%)
- Couleur rouge si perf >100%, blanc sinon

---

### Module d'affichage des compteurs

#### `display_compteurs.h` + `display_compteurs.cpp` (v8.50.R154N)
**Rôle:** Gestion des 6 compteurs textuels en bas de l'écran

**Compteurs affichés:**
1. **Production solaire** (Wh) - Jaune, en bas à gauche
2. **Status batterie** (Bulk/Absorption/Float/Storage/Off) - Orange, en bas à gauche
3. **TTG (Time To Go)** (autonomie restante) - Bleu cyan, en bas à gauche
4. **Voltage batterie** (V) - Blanc, en bas à droite (48pt)
5. **Ampérage batterie** (A) - Blanc/Orange si négatif, en bas à droite (48pt)
6. **Full** (temps avant batterie pleine) - Bleu cyan, en bas à droite

**Fonction publique:**
```cpp
void updateCounters(float prodSolar, const char* statusBatt, 
                    float voltageBatt, float ampereBatt,
                    int heuresFull, int minutesFull,
                    int joursTTG, int heuresTTG, int minutesTTG, 
                    bool ttgInfini);
```

**Logique TTG:**
- Si `joursTTG == -1` → Affiche "---" (timeout 15s BMV)
- Si `ttgInfini == true` → Affiche "infini" (charge ou repos)
- Sinon → Affiche "Xj XXh XXm" ou "XXh XXm" selon joursTTG

**Appelé par:** `acquisition_BT.cpp::updateAcquisition()`

**Alignement R154M:** Labels et valeurs alignés sur même baseline (y+2 pour compenser différence de police)

---

### Module overlay de veille

#### `display_overlay.h` + `display_overlay.cpp` (v8.50.R133)
**Rôle:** Gestion du mode veille tactile pour économie d'énergie

**Fonctionnalités:**
- Overlay noir semi-transparent (90% opacité) plein écran
- Message "En veille / Toucher pour rétablir"
- Toggle ON/OFF par appui tactile
- Debounce 300ms pour entrée en veille (évite faux contacts)
- Sortie immédiate de veille (réactivité)

**Variables exportées:**
- `sleep_overlay` : Objet LVGL de l'overlay
- `sleep_mode` : État actuel (true = veille, false = normal)

**Fonction publique:**
```cpp
void updateTouchBacklight(Board *board);
```

**Appelé par:** `WS_IA_V8.ino::loop()` en continu

**Note:** L'overlay est créé EN DERNIER (après jauges et compteurs) pour être au-dessus de tout

---

### Ressources graphiques

#### Images intégrées (format .c)

##### `Logo_victron_blanc_48x30TC.c`
Logo Victron blanc 48×30 pixels, format True Color  
**Utilisé par:** `display_gauges.cpp` (bandeau supérieur)

##### `Logo_Victron120x120TC.c`
Logo Victron 120×120 pixels, format True Color  
**Utilisé par:** `WS_IA_V8.ino` (splash screen, coin inférieur droit)

##### `Splash_screen_vierge341x200TC.c`
Image de fond du splash screen 341×200 pixels (agrandi à 300%)  
**Utilisé par:** `WS_IA_V8.ino` (splash screen, centré)

**Format des images:** True Color LVGL (RGB565), converties avec LVGL Image Converter

---

## 🎨 Personnalisation

### Configuration du bateau (WS_IA_V8.ino)

```cpp
// Nom affiché sur splash screen et bandeau supérieur
const char *NOM_BATEAU = "ALBA III";

// Version affichée sur splash et en bas de l'écran
const char *VERSION_APP = "v8.50.R154P";
```

### Caractéristiques électriques (acquisition_BT.cpp)

```cpp
// Définit les échelles des jauges et calculs de performance
int CAPACITE_BATTERIE_AH = 280;  // Pour calcul TTG (Time To Go)
int PSOL = 350;   // Échelle jauge SOLAIRE (0-350W)
int PALT = 50;    // Échelle jauge ALTERNATEUR (0-50A)
int PQUAI = 30;   // Échelle jauge QUAI (0-30A)
```

### Appareils Victron (acquisition_BT.cpp)

**Adresses MAC:**
```cpp
#define MAC_SMARTSOLAR "f3:81:dc:56:9f:97"  // Régulateur solaire
#define MAC_BMV_712 "c5:1d:ac:ed:91:92"     // Moniteur batterie
#define MAC_ORION_XS "fb:c1:a3:08:4e:8c"    // Chargeur alternateur
#define MAC_IP22 "fb:82:24:5d:bb:27"        // Chargeur secteur
```

**Clés de déchiffrement:**  
Obtenues via l'app VictronConnect (menu Produit → Instant readout via Bluetooth)

```cpp
uint8_t key_SmartSolar[16] = { 0x4B, 0x05, ... };  // 16 bytes
uint8_t key_BMV[16] = { 0xB4, 0x26, ... };
uint8_t key_OrionXS[16] = { 0xEC, 0x7E, ... };
uint8_t key_IP22[16] = { 0x64, 0xEB, ... };
```

### Timeouts (acquisition_BT.cpp / display_gauges.cpp)

```cpp
#define LED_TIMEOUT_MS 15000     // Extinction LED après 15s (Solar/Alt/BMV)
#define IP22_TIMEOUT_MS 60000    // Extinction LED après 60s (IP22 transmet moins)
```

### Paramètres BLE (acquisition_BT.cpp - fonction initAcquisition)

```cpp
pBLEScan->setInterval(80);  // Intervalle de scan (ms)
pBLEScan->setWindow(79);    // Fenêtre de scan (ms)
// Ratio 79/80 = 98.75% du temps en écoute
```

---

## 🛠️ Matériel requis

### Matériel principal

| Composant | Modèle | Spécifications |
|-----------|--------|----------------|
| **Microcontrôleur** | Waveshare ESP32-S3 Touch LCD 5B | ESP32-S3, 8MB PSRAM, écran 1024×600 tactile capacitif |
| **Appareils Victron** | SmartSolar MPPT | Régulateur solaire avec BLE |
| | BMV-712 | Moniteur batterie avec BLE |
| | Orion XS | Chargeur DC-DC alternateur avec BLE |
| | IP22 | Chargeur secteur avec BLE |

### Caractéristiques ESP32-S3

- **Processeur:** Dual-core Xtensa LX7 @ 240 MHz
- **RAM:** 512 KB SRAM + 8 MB PSRAM
- **Flash:** 16 MB
- **Écran:** 1024×600 pixels, RGB888, tactile capacitif
- **Bluetooth:** BLE 5.0

---

## 💾 Installation

### Prérequis logiciels

1. **Arduino IDE 2.3.6** ou supérieur
2. **ESP32 Board Support Package** (via Boards Manager)
3. **Bibliothèques Arduino:**
   - `lvgl` (version **8.4.0** exactement - CRITIQUE)
   - `ESP32_Display_Panel` (pour Waveshare)
   - `ESP_IOExpander_Library`
   - `NimBLE-Arduino` (stack BLE)

### Installation des bibliothèques

```bash
# Via Arduino Library Manager
Sketch → Include Library → Manage Libraries

# Installer:
- LVGL version 8.4.0 (PAS 9.x)
- ESP32_Display_Panel
- ESP_IOExpander_Library
- NimBLE-Arduino
```

### Configuration Arduino IDE

1. **Sélectionner la carte:**
   - Tools → Board → esp32 → ESP32S3 Dev Module

2. **Configuration des paramètres:**
   ```
   USB CDC On Boot: Enabled
   Flash Size: 16MB
   Partition Scheme: Huge APP (3MB No OTA)
   PSRAM: OPI PSRAM
   Upload Speed: 921600
   USB Mode: Hardware CDC and JTAG
   ```

3. **Configuration LVGL (lv_conf.h):**
   ```c
   #define LV_COLOR_DEPTH 16        // RGB565
   #define LV_MEM_SIZE (64 * 1024)  // 64KB heap LVGL
   ```

### Compilation et upload

1. Vérifier la configuration BLE (MAC, clés) dans `acquisition_BT.cpp`
2. Personnaliser le nom du bateau dans `WS_IA_V8.ino`
3. Compiler (Ctrl+R) - vérifier absence d'erreurs
4. Uploader (Ctrl+U) - surveiller les logs Serial (115200 bauds)

---

## 🔐 Configuration BLE

### Obtention des clés de déchiffrement Victron

1. Installer l'app **VictronConnect** (iOS/Android)
2. Se connecter à chaque appareil via l'app
3. Menu **Produit** → **Instant readout via Bluetooth**
4. Activer → Affiche la clé de 32 caractères hexadécimaux
5. Convertir en tableau de 16 bytes pour le code

**Exemple:**
```
Clé affichée: 4B0518E74276883AAE6F1CC9B0842506
Conversion:
uint8_t key[16] = {
  0x4B, 0x05, 0x18, 0xE7, 0x42, 0x76, 0x88, 0x3A,
  0xAE, 0x6F, 0x1C, 0xC9, 0xB0, 0x84, 0x25, 0x06
};
```

### Obtention des adresses MAC

1. Scanner BLE avec l'app VictronConnect
2. Noter l'adresse MAC de chaque appareil (format: `xx:xx:xx:xx:xx:xx`)
3. Entrer en minuscules dans `acquisition_BT.cpp`

---

## ⚡ Performances et optimisations

### Métriques de performance

| Métrique | Valeur | Description |
|----------|--------|-------------|
| **Fréquence loop()** | ~200 Hz | 5ms cycle + overhead |
| **Durée scan BLE** | 1000 ms | 1 scan par seconde |
| **Taux réception BLE** | >99% | Dans environnement saturé |
| **Mémoire PSRAM utilisée** | 2.4 MB | Double buffering (2×1024×600×2) |
| **Stack BLE** | 8192 bytes | Anti-overflow |
| **Timeout LED Solar/Alt/BMV** | 15 s | Extinction automatique |
| **Timeout LED Quai** | 60 s | IP22 transmet moins souvent |
| **Timeout TTG** | 15 s | Affiche "---" si pas de BMV |

### Optimisations clés

#### R154O - Performances et personnalisation
- Variables de performance (`perfSOL`, `perfALT`, `perfQUAI`) calculées et affichées
- Utilisation de `PSOL`/`PALT`/`PQUAI` comme max_value des jauges
- Bandeau bleu Victron avec logo intégré
- Filtrage précoce vendorID avant allocation mémoire
- Variables `static` dans `onResult()` pour économie stack

#### R154P - Labels performance harmonisés
- Même police (Montserrat 20) pour valeur ET symbole "%"
- Position ajustée (-62) pour éviter débordement du "%"
- Gap fixe optimisé (+40) pour tous les cas (0-125%)
- Pas de superposition garantie, code simple et rapide

#### R154M - Optimisation LED et TTG
- `updateLEDStatus()` avec un seul `lv_refr_now()` (gain CPU)
- Réutilisation des buffers AES (inputData, outputData)
- Suppression variable temporaire `localOutput` IP22 (16 bytes stack)
- TTG initial "---" au lieu de "Inf" (plus intuitif)

#### R154L - Toggle LED et timeouts
- LED avec état `current_state` pour toggle ON/OFF
- Extinction automatique après timeout différencié
- Clignotement à chaque réception de paquet

#### R154K - Diagnostic IP22
- Logger tous les paquets IP22 pour debug
- Surveillance beaconType, recordType, keyMatch

### Architecture BLE synchrone

Le système utilise un scan BLE **synchrone** (inspiration WT32-Victron):

```cpp
BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
// Traitement immédiat dans onResult() - pas de mutex, pas de queue
```

**Avantages:**
- Simplicité du code (pas de gestion de threads complexes)
- Pas de mutex (pas de contention, pas d'overhead)
- Écriture directe dans les variables globales
- Taux de réception stable >99%
- Pas de problème de timing entre threads

**vs Architecture asynchrone abandonnée:**
- Complexité élevée (mutex, queues, FreeRTOS tasks)
- Taux de réception <5% (problèmes de contention)
- Loop() ralentie à 1.9 Hz (au lieu de 200 Hz)

---

## 📈 Évolutions récentes

### Version R154P (actuelle)
- Labels performance harmonisés (même police 20 pour valeur et "%")
- Position optimisée (-62) pour éviter débordement du symbole "%"
- Gap fixe +40 entre valeur et "%"

### Version R154O
- Chapitre Personnalisation (MAC, clés, puissances, noms)
- Variables de performance (`perfSOL`, `perfALT`, `perfQUAI`)
- Calcul performances (%) dans chaque case du switch
- Bandeau bleu pleine largeur avec logo Victron
- Export constantes vers display_gauges
- Filtrage précoce vendorID (environnement BLE saturé)

### Version R154N
- TTG timeout "---" après 15s sans BMV
- Timestamp BMV pour détection timeout

### Version R154M
- Optimisation `updateLEDStatus()` (un seul refresh)
- Réutilisation buffers AES
- TTG initial "---" au lieu de "Inf"
- Alignement labels/valeurs compteurs

### Version R154L
- Toggle LED avec current_state
- Extinction automatique LED après timeout
- Timeouts différenciés (15s/60s)

### Version R154K
- Logger diagnostic IP22
- Surveillance détaillée paquets BLE

---

## 🤝 Contribution

Ce projet est personnel mais les suggestions d'amélioration sont bienvenues via issues GitHub.

### Conventions de code

- **Langue:** Commentaires en français, variables en anglais/français mixte
- **Formatage:** Indentation 2 espaces
- **Documentation:** Commentaires détaillés pour chaque section
- **Versioning:** Format `vX.YY.RXXXZ` (ex: v8.50.R154P)

---

## 📝 Licence

Projet personnel - Tous droits réservés  
Usage éducatif et personnel autorisé

---

## 🙏 Remerciements

- **Victron Energy** pour le protocole BLE et la documentation
- **LVGL** pour la bibliothèque graphique
- **Espressif** pour ESP32 et ESP-IDF
- **Waveshare** pour la carte ESP32-S3 Touch LCD

---

## 📞 Contact

Projet développé pour le bateau **ALBA III**  
Questions techniques → Issues GitHub

---

**Dernière mise à jour:** Décembre 2024  
**Prochaine version:** v8.50.R154Q (en développement)
