// ========================================
// Fichier: acquisition_BT.cpp
// Version 8.50.R154O - Personnalisation + performances + fix BLE saturé
// CHANGEMENTS R154O:
// - Chapitre Personnalisation (Caractéristiques, MAC, Puissances, Noms, Clés)
// - Chapitre Variables de performance (perfSOL, perfALT, perfQUAI)
// - Chapitre Gestion des LEDs regroupé
// - Calcul des performances (%) dans chaque case du switch
// - Commentaires séparateurs dans switch/case pour lisibilité
// - Export des constantes pour usage dans display_gauges
// - FIX: Filtrage précoce vendorID (environnement BLE saturé)
// - FIX: Variables static dans onResult() (économie stack)
// - FIX: Suppression localOutput IP22 (économie 16 bytes stack)
// ========================================
#include "display_gauges.h"
#include "display_compteurs.h"

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <aes/esp_aes.h>

// ========================================
// CUSTOMIZATION
// ========================================

// Caractéristiques des appareils Victron
int CAPACITE_BATTERIE_AH = 280;  // Capacité batterie (Ah) pour calculs TTG
int PSOL = 350;   // Puissance max panneau solaire (Watt)
int PALT = 50;    // Puissance max alternateur (Ampère)
int PQUAI = 30;   // Puissance max chargeur de quai (Ampère)

// Adresses MAC des appareils Victron
#define MAC_SMARTSOLAR "f3:81:dc:56:9f:97"
#define MAC_BMV_712 "c5:1d:ac:ed:91:92"
#define MAC_ORION_XS "fb:c1:a3:08:4e:8c"
#define MAC_IP22 "fb:82:24:5d:bb:27"

// Noms des appareils
#define NAME_SMARTSOLAR "SmartSolar"
#define NAME_BMV_712 "BMV-712"
#define NAME_ORION_XS "Orion XS"
#define NAME_IP22 "IP22"

// Clés de décryptage
uint8_t key_SmartSolar[16] = {
  0x4B, 0x05, 0x18, 0xE7, 0x42, 0x76, 0x88, 0x3A, 0xAE, 0x6F, 0x1C, 0xC9, 0xB0, 0x84, 0x25, 0x06
};

uint8_t key_BMV[16] = {
  0xB4, 0x26, 0xA6, 0x45, 0x33, 0xFA, 0xD9, 0x63, 0x66, 0xAB, 0x72, 0xD3, 0x30, 0xAC, 0x13, 0x5A
};

uint8_t key_OrionXS[16] = {
  0xEC, 0x7E, 0x29, 0xE7, 0x60, 0x43, 0xB4, 0x91, 0x07, 0xB6, 0x01, 0x33, 0x06, 0xBB, 0xD0, 0x13
};

uint8_t key_IP22[16] = {
  0x64, 0xEB, 0xC5, 0x9D, 0xA0, 0xA1, 0x80, 0xB5, 0x9A, 0xE7, 0x44, 0xCF, 0x2B, 0xB4, 0xFF, 0x8B
};

// ========================================

enum DeviceID {
  DEVICE_UNKNOWN = 0,
  DEVICE_ORION_XS = 1,
  DEVICE_IP22 = 2,
  DEVICE_SMARTSOLAR = 3,
  DEVICE_BMV_712 = 4
};

int scanTime = 1;
int keyBits = 128;
BLEScan *pBLEScan = nullptr;

static int count_smartsolar_attempts = 0;
static int count_smartsolar_valid = 0;
static int count_bmv_attempts = 0;
static int count_bmv_valid = 0;
static int count_orion_attempts = 0;
static int count_orion_valid = 0;
static int count_ip22_attempts = 0;
static int count_ip22_valid = 0;

// ========================================
// Variables de performance
// ========================================
int perfSOL = 0;   // R154O: Performance panneau solaire (%)
int perfALT = 0;   // R154O: Performance alternateur (%)
int perfQUAI = 0;  // R154O: Performance chargeur de quai (%)
// ========================================

// ========================================
// Gestion des LEDs
// ========================================
#define IP22_TIMEOUT_MS 60000
#define LED_TIMEOUT_MS 15000

static unsigned long last_ip22_update = 0;
static unsigned long last_bmv_update = 0;  // R154N: Timeout TTG

unsigned long last_led_update_Solaire = 0;
unsigned long last_led_update_Alternateur = 0;
unsigned long last_led_update_Quai = 0;
unsigned long last_led_update_Batterie = 0;

bool data_received_Solaire = false;
bool data_received_Alternateur = false;
bool data_received_Quai = false;
bool data_received_Batterie = false;
// ========================================

float VALT = 0;
float VSOL = 0;
float VQUAI = 0;
float VBAT = 0;

float prodSolar = 0.0;
char statusBatt[32] = "---";
float voltageBatterie = 0.0;
float ampereBatterie = 0.0;
int heuresFull = 0;
int minutesFull = 0;
int joursTTG = -1;  // R154N: -1 au démarrage = "---" (pas encore reçu)
int heuresTTG = 0;
int minutesTTG = 0;
bool ttgInfini = true;

typedef struct {
  uint16_t vendorID;
  uint8_t beaconType;
  uint8_t unknownData1[3];
  uint8_t victronRecordType;
  uint16_t nonceDataCounter;
  uint8_t encryptKeyMatch;
  uint8_t victronEncryptedData[21];
  uint8_t nullPad;
} __attribute__((packed)) victronManufacturerData;

typedef struct {
  uint8_t deviceState;
  uint8_t errorCode;
  int16_t batteryVoltage;
  int16_t batteryCurrent;
  uint16_t todayYield;
  uint16_t inputPower;
  uint8_t outputCurrentLo;
  uint8_t outputCurrentHi;
  uint8_t unused[4];
} __attribute__((packed)) victronPanelData;

typedef struct {
  uint16_t bmvTTG;
  uint16_t bmvBat1Voltage;
  uint16_t bmvAlarmReason;
  uint16_t bmvBat2Voltage;
  uint8_t byte0;
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
  uint32_t bmvData2;
} __attribute__((packed)) victronBMVData;

typedef struct {
  uint8_t deviceState;
  uint8_t errorCode;
  uint16_t outputVoltage;
  uint16_t outputCurrent;
  uint16_t inputVoltage;
  uint16_t inputCurrent;
  uint32_t offReason;
  uint16_t unused;
} __attribute__((packed)) victronOrionXSData;

DeviceID identifyDevice(const String &macAddress) {
  if (macAddress == MAC_ORION_XS) return DEVICE_ORION_XS;
  if (macAddress == MAC_IP22) return DEVICE_IP22;
  if (macAddress == MAC_SMARTSOLAR) return DEVICE_SMARTSOLAR;
  if (macAddress == MAC_BMV_712) return DEVICE_BMV_712;
  return DEVICE_UNKNOWN;
}

void printTimestamp() {
  unsigned long ms = millis();
  int seconds = (ms / 1000) % 60;
  int minutes = (ms / 60000) % 60;
  int hours = (ms / 3600000) % 24;
  Serial.printf("[%02d:%02d:%02d] ", hours, minutes, seconds);
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    if (!advertisedDevice.haveManufacturerData()) return;

    String manData = advertisedDevice.getManufacturerData();
    int manDataSize = manData.length();

    // R154O: Filtrage précoce vendorID AVANT allocation (environnement BLE saturé)
    if (manDataSize < 10) return;
    uint16_t vendorID = (uint8_t)manData[1] << 8 | (uint8_t)manData[0];
    if (vendorID != 0x02e1) return;  // Victron uniquement

    uint8_t manCharBuf[32];
    memcpy(manCharBuf, manData.c_str(), manDataSize);

    victronManufacturerData *vicData = (victronManufacturerData *)manCharBuf;

    if (vicData->beaconType != 0x10) return;

    String macAddressStr = advertisedDevice.getAddress().toString();
    macAddressStr.toLowerCase();

// R154K: LOGGER TOUS LES PAQUETS IP22
  if (macAddressStr == MAC_IP22) {
    Serial.printf("[IP22-DEBUG] MAC=%s, beaconType=0x%02X, recordType=0x%02X, keyMatch=0x%02X\n",
                  macAddressStr.c_str(),
                  vicData->beaconType,
                  vicData->victronRecordType,
                  vicData->encryptKeyMatch);
  }

    DeviceID deviceID = identifyDevice(macAddressStr);

    // R154O: Variables static (hors stack) pour environnement BLE saturé
    static uint8_t inputData[16];
    static uint8_t outputData[16];
    static esp_aes_context ctx;
    static uint8_t nonce_counter[16];
    static uint8_t stream_block[16];
    size_t nonce_offset;
    int encrDataSize;
    int status;

    switch (deviceID) {

      // ========================================
      // Traitement de l'Orion XS (Alternateur)
      // ========================================
      case DEVICE_ORION_XS:
        {
          count_orion_attempts++;

          // R154M: Réutilisation variables communes
          memset(outputData, 0, 16);
          encrDataSize = manDataSize - 10;

          for (int i = 0; i < encrDataSize; i++) {
            inputData[i] = vicData->victronEncryptedData[i];
          }

          esp_aes_init(&ctx);
          status = esp_aes_setkey(&ctx, key_OrionXS, keyBits);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          uint8_t data_counter_lsb = (vicData->nonceDataCounter) & 0xff;
          uint8_t data_counter_msb = ((vicData->nonceDataCounter) >> 8) & 0xff;
          memset(nonce_counter, 0, 16);
          nonce_counter[0] = data_counter_lsb;
          nonce_counter[1] = data_counter_msb;
          memset(stream_block, 0, 16);
          nonce_offset = 0;

          status = esp_aes_crypt_ctr(&ctx, encrDataSize, &nonce_offset, nonce_counter,
                                     stream_block, inputData, outputData);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          esp_aes_free(&ctx);

          victronOrionXSData localData = *((victronOrionXSData *)outputData);
          float outputCurrent = float(localData.outputCurrent) * 0.1;

          VALT = outputCurrent;
          data_received_Alternateur = true;
          last_led_update_Alternateur = millis();
          count_orion_valid++;

          // R154O: Calcul performance alternateur
          perfALT = (int)((outputCurrent / (float)PALT) * 100.0);

          printTimestamp();
          Serial.printf("Orion: %.1fA (#%d/%d)\n", outputCurrent, count_orion_valid, count_orion_attempts);
          return;
        }
        break;

      // ========================================
      // Traitement de l'IP22 (Chargeur de quai)
      // ========================================
      case DEVICE_IP22:
        {
          count_ip22_attempts++;

          // R154M: Réutilisation variables communes
          memset(outputData, 0, 16);
          encrDataSize = manDataSize - 10;

          for (int i = 0; i < encrDataSize; i++) {
            inputData[i] = vicData->victronEncryptedData[i];
          }

          esp_aes_init(&ctx);
          status = esp_aes_setkey(&ctx, key_IP22, keyBits);

          if (status != 0) {
            Serial.printf("IP22: ERREUR setkey status=%d\n", status);
            esp_aes_free(&ctx);
            return;
          }

          uint8_t data_counter_lsb = (vicData->nonceDataCounter) & 0xff;
          uint8_t data_counter_msb = ((vicData->nonceDataCounter) >> 8) & 0xff;
          memset(nonce_counter, 0, 16);
          nonce_counter[0] = data_counter_lsb;
          nonce_counter[1] = data_counter_msb;
          memset(stream_block, 0, 16);
          nonce_offset = 0;

          status = esp_aes_crypt_ctr(&ctx, encrDataSize, &nonce_offset, nonce_counter,
                                     stream_block, inputData, outputData);

          if (status != 0) {
            Serial.printf("IP22: ERREUR decrypt status=%d\n", status);
            esp_aes_free(&ctx);
            return;
          }

          esp_aes_free(&ctx);

          if (vicData->victronRecordType != 0x08) {
            Serial.printf("IP22: RecordType incorrect 0x%02X (attendu 0x08)\n", vicData->victronRecordType);
            return;
          }

          if (vicData->encryptKeyMatch != key_IP22[0]) {
            Serial.printf("IP22: KeyMatch incorrect 0x%02X (attendu 0x%02X)\n",
                          vicData->encryptKeyMatch, key_IP22[0]);
            return;
          }

          // R154O: Utiliser directement outputData (économie 16 bytes stack)
          uint16_t currentRaw = ((((uint16_t)outputData[4] << 8) | outputData[3]) >> 5) & 0x7FF;
          float batteryCurrentIP22 = float(currentRaw) * 0.1;

          VQUAI = batteryCurrentIP22;
          last_ip22_update = millis();
          data_received_Quai = true;
          last_led_update_Quai = millis();
          count_ip22_valid++;

          // R154O: Calcul performance chargeur de quai
          perfQUAI = (int)((batteryCurrentIP22 / (float)PQUAI) * 100.0);

          printTimestamp();
          Serial.printf("IP22: %.1fA (#%d/%d)\n", batteryCurrentIP22, count_ip22_valid, count_ip22_attempts);
          return;
        }
        break;

      // ========================================
      // Traitement du SmartSolar (Regulateur Panneau solaire)
      // ========================================
      case DEVICE_SMARTSOLAR:
        {
          count_smartsolar_attempts++;

          // R154M: Réutilisation variables communes
          memset(outputData, 0, 16);
          encrDataSize = manDataSize - 10;

          for (int i = 0; i < encrDataSize; i++) {
            inputData[i] = vicData->victronEncryptedData[i];
          }

          esp_aes_init(&ctx);
          status = esp_aes_setkey(&ctx, key_SmartSolar, keyBits);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          uint8_t data_counter_lsb = (vicData->nonceDataCounter) & 0xff;
          uint8_t data_counter_msb = ((vicData->nonceDataCounter) >> 8) & 0xff;
          memset(nonce_counter, 0, 16);
          nonce_counter[0] = data_counter_lsb;
          nonce_counter[1] = data_counter_msb;
          memset(stream_block, 0, 16);
          nonce_offset = 0;

          status = esp_aes_crypt_ctr(&ctx, encrDataSize, &nonce_offset, nonce_counter,
                                     stream_block, inputData, outputData);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          esp_aes_free(&ctx);

          victronPanelData localData = *((victronPanelData *)outputData);

          float batteryVoltage = float(localData.batteryVoltage) * 0.01;
          uint16_t inputPower = localData.inputPower;
          uint16_t todayYield = localData.todayYield * 10;
          uint8_t deviceState = localData.deviceState;

          char tempStatusBatt[32];

          switch (deviceState) {
            case 0: strcpy(tempStatusBatt, "Off"); break;
            case 3: strcpy(tempStatusBatt, "Bulk"); break;
            case 4: strcpy(tempStatusBatt, "Absorption"); break;
            case 5: strcpy(tempStatusBatt, "Float"); break;
            case 6: strcpy(tempStatusBatt, "Storage"); break;
            default: strcpy(tempStatusBatt, "---"); break;
          }

          VSOL = inputPower;
          prodSolar = todayYield;
          voltageBatterie = batteryVoltage;
          strcpy(statusBatt, tempStatusBatt);
          data_received_Solaire = true;
          last_led_update_Solaire = millis();
          count_smartsolar_valid++;

          // R154O: Calcul performance panneau solaire
          perfSOL = (int)((inputPower / (float)PSOL) * 100.0);

          printTimestamp();
          Serial.printf("Solar: %dW (#%d/%d)\n", inputPower, count_smartsolar_valid, count_smartsolar_attempts);
          return;
        }
        break;

      // ========================================
      // Traitement du BMV-712 (Moniteur batterie)
      // ========================================
      case DEVICE_BMV_712:
        {
          count_bmv_attempts++;

          // R154M: Réutilisation variables communes
          memset(outputData, 0, 16);
          encrDataSize = manDataSize - 10;

          for (int i = 0; i < encrDataSize; i++) {
            inputData[i] = vicData->victronEncryptedData[i];
          }

          esp_aes_init(&ctx);
          status = esp_aes_setkey(&ctx, key_BMV, keyBits);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          uint8_t data_counter_lsb = (vicData->nonceDataCounter) & 0xff;
          uint8_t data_counter_msb = ((vicData->nonceDataCounter) >> 8) & 0xff;
          memset(nonce_counter, 0, 16);
          nonce_counter[0] = data_counter_lsb;
          nonce_counter[1] = data_counter_msb;
          memset(stream_block, 0, 16);
          nonce_offset = 0;

          status = esp_aes_crypt_ctr(&ctx, encrDataSize, &nonce_offset, nonce_counter,
                                     stream_block, inputData, outputData);

          if (status != 0) {
            esp_aes_free(&ctx);
            return;
          }

          esp_aes_free(&ctx);

          victronBMVData localData = *((victronBMVData *)outputData);

          float bmvBat1Voltage = float(localData.bmvBat1Voltage) * 0.01;
          uint8_t byte0 = localData.byte0;
          uint8_t byte1 = localData.byte1;
          uint8_t byte2 = localData.byte2;
          uint32_t bmvData2 = localData.bmvData2;

          byte0 = byte0 & 0xFC;
          uint32_t mot = (byte2 << 14) | (byte1 << 6) | (byte0 >> 2);
          int32_t Current = mot;
          if (mot & 0x00200000) {
            Current |= 0xFFC00000;
          }
          float batteryCurrentBMV = (float)Current / 1000.0;

          uint32_t extractedBits = (bmvData2 >> 12) & 0x3FF;
          float socValue = extractedBits / 10.0;

          float capaciteRestante = (socValue / 100.0) * CAPACITE_BATTERIE_AH;

          int tempHeuresFull = 0;
          int tempMinutesFull = 0;
          int tempJoursTTG = 0;
          int tempHeuresTTG = 0;
          int tempMinutesTTG = 0;
          bool tempTtgInfini = true;

          if (abs(batteryCurrentBMV) > 0.1) {

            if (batteryCurrentBMV > 0.1) {
              float tempsDeCharge = (CAPACITE_BATTERIE_AH - capaciteRestante) / batteryCurrentBMV;
              tempHeuresFull = int(tempsDeCharge);
              tempMinutesFull = int((tempsDeCharge - tempHeuresFull) * 60);
              tempTtgInfini = true;

            } else if (batteryCurrentBMV < -0.1) {
              float tempsTTG_heures = capaciteRestante / abs(batteryCurrentBMV);
              float tempsTTG_jours = tempsTTG_heures / 24.0;

              tempJoursTTG = int(tempsTTG_jours);
              float heuresRestantes = (tempsTTG_jours - tempJoursTTG) * 24.0;
              tempHeuresTTG = int(heuresRestantes);
              tempMinutesTTG = int((heuresRestantes - tempHeuresTTG) * 60);
              tempTtgInfini = false;
            }

          } else {
            tempTtgInfini = true;
          }

          VBAT = socValue;
          ampereBatterie = batteryCurrentBMV;
          voltageBatterie = bmvBat1Voltage;
          heuresFull = tempHeuresFull;
          minutesFull = tempMinutesFull;
          joursTTG = tempJoursTTG;
          heuresTTG = tempHeuresTTG;
          minutesTTG = tempMinutesTTG;
          ttgInfini = tempTtgInfini;
          last_bmv_update = millis();  // R154N: Timestamp BMV pour TTG
          data_received_Batterie = true;
          last_led_update_Batterie = millis();
          count_bmv_valid++;

          // Pas de calcul de performance pour le moniteur batterie

          printTimestamp();
          Serial.printf("BMV: %.1f%% (#%d/%d)\n", socValue, count_bmv_valid, count_bmv_attempts);
          return;
        }
        break;

      case DEVICE_UNKNOWN:
      default:
        return;
    }
  }
};

void initAcquisition() {
  Serial.println("========================================");
  Serial.println("[R154O] Initialisation Acquisition BT");
  Serial.println("[R154O] Mode SYNCHRONE (comme WT32)");
  Serial.println("[R154O] IP22: timeout 60s, LED timeout 15s/60s");
  Serial.println("[R154O] LEDs: Toggle + auto-extinction");
  Serial.println("[R154O] Stack BLE: 8192 bytes (anti-overflow)");
  Serial.printf("[R154O] Capacité batterie: %dAh\n", CAPACITE_BATTERIE_AH);  // R154O: Utilise la constante

  Serial.println("[R154O] Appareils:");
  Serial.printf("  1. %s: %s key:0x%02X\n", NAME_ORION_XS, MAC_ORION_XS, key_OrionXS[0]);
  Serial.printf("  2. %s: %s key:0x%02X\n", NAME_IP22, MAC_IP22, key_IP22[0]);
  Serial.printf("  3. %s: %s key:0x%02X\n", NAME_SMARTSOLAR, MAC_SMARTSOLAR, key_SmartSolar[0]);
  Serial.printf("  4. %s: %s key:0x%02X\n", NAME_BMV_712, MAC_BMV_712, key_BMV[0]);
  Serial.println();

  BLEDevice::init("");

  // R154M: Stack BLE augmentée 5120 → 8192 (anti-overflow)
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  bt_cfg.controller_task_stack_size = 8192;

  delay(1000);
  Serial.println("[R154O] >>> Délai 1000ms post-init BLE <<<");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  //pBLEScan->setInterval(100);
  //pBLEScan->setWindow(99);
  pBLEScan->setInterval(80);
  pBLEScan->setWindow(79);

  Serial.println("[R154O] BLE initialisé");
  Serial.println("[R154O] Acquisition prête");
  Serial.println("========================================");
}

void updateAcquisition() {

  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);

  setGaugeValue(JSOLAIRE, VSOL);
  setGaugeValue(JALTERNATEUR, VALT);
  setGaugeValue(JQUAI, VQUAI);
  setGaugeValue(JBATTERIE, VBAT);

  // R154N: Timeout TTG 15s - modifier les variables AVANT updateCounters
  if (last_bmv_update > 0 && (millis() - last_bmv_update > 15000)) {
    // Pas de réception depuis 15s → forcer TTG à "---"
    joursTTG = -1;
    heuresTTG = -1;
    minutesTTG = -1;
  }

  // R154N: Appel unique à updateCounters (avec variables à jour)
  updateCounters(prodSolar, statusBatt, voltageBatterie, ampereBatterie,
                 heuresFull, minutesFull,
                 joursTTG, heuresTTG, minutesTTG, ttgInfini);

  if (last_ip22_update > 0 && (millis() - last_ip22_update > IP22_TIMEOUT_MS)) {
    if (VQUAI != 0) {
      Serial.println("IP22: Timeout 60s -> reset à 0A");
      VQUAI = 0;
      setGaugeValue(JQUAI, 0);
    }
  }

  pBLEScan->clearResults();
}
