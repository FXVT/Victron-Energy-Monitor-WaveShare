# Configuration des bibliothèques ESP32 Display

For the English version, please read the file ReadMe_EN.md

Ce répertoire contient les fichiers de configuration pré-paramétrés pour les bibliothèques nécessaires au développement d'interfaces graphiques sur ESP32 avec LVGL.

## 📁 Structure des fichiers

### 1. **ESP32_Display_Panel**
Bibliothèque de gestion des écrans et écrans tactiles pour ESP32

#### Fichiers de configuration :
- **`esp_panel_drivers_conf.h`**  
  Définit les pilotes matériels utilisés :
  - Pilote d'écran (display driver)
  - Pilote tactile (touch driver)
  - Autres périphériques d'affichage

- **`esp_panel_board_supported_conf.h`**  
  Spécifie la carte matérielle utilisée :  
  ✅ **WaveShare ESP32 S3 Touch 5B**

#### 📍 Emplacement :
```
..\Arduino\libraries\ESP32_Display_Panel\
```

### 2. **LVGL (Light and Versatile Graphics Library)**
Bibliothèque graphique embarquée pour créer des interfaces utilisateur

#### Fichier de configuration :
- **`lv_conf.h`**  
  Contient tous les paramètres de configuration LVGL :
  - Sélection des polices
  - Gestion des dégradés
  - Configuration de la mémoire
  - Paramètres de performance
  - Fonctionnalités activées/désactivées

#### 📍 Emplacement :
```
..\Arduino\libraries\lvgl\
```

## 🛠️ Configuration matérielle

### Carte supportée
- **WaveShare ESP32 S3 Touch 5B**
- Écran tactile intégré
- Processeur ESP32-S3

## 🔧 Installation

1. Assurez-vous que les bibliothèques sont installées via le gestionnaire de bibliothèques Arduino :
   - ESP32_Display_Panel
   - lvgl v8.4

2. Copiez les fichiers de configuration fournis dans ce répertoire vers leurs emplacements respectifs

3. Redémarrez l'IDE Arduino si nécessaire

## ⚙️ Personnalisation

### Pour modifier les pilotes :
Éditez `esp_panel_drivers_conf.h` pour :
- Changer le pilote d'écran
- Modifier la configuration tactile
- Ajouter/supprimer des périphériques

### Pour changer de carte matérielle :
Modifiez `esp_panel_board_supported_conf.h` et sélectionnez une nouvelle carte parmi les options supportées

### Pour ajuster LVGL :
Adaptez `lv_conf.h` selon vos besoins :
- Activer/désactiver les dégradés
- Changer les polices par défaut
- Ajuster les paramètres mémoire
- Modifier les fonctionnalités graphiques

## 📋 Notes importantes

- Ces configurations sont optimisées pour la **WaveShare ESP32 S3 Touch 5B**
- Les chemins d'accès sont relatifs à votre installation Arduino
- Sauvegardez vos modifications avant de mettre à jour les bibliothèques
- Consultez la documentation officielle pour des configurations avancées


## 📄 Licence

Les fichiers de configuration sont fournis sous licence MIT.  

Reportez-vous aux licences respectives des bibliothèques pour plus d'informations.

