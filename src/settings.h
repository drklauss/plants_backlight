#ifndef SETTINGS_H
#define SETTINGS_H

#include "config.h"
#include <EEPROM.h>

// Структура для хранения в EEPROM
struct Settings {
    int8_t start_hour;
    int8_t end_hour;
    float lux_threshold;
    uint8_t log_level;      // НОВОЕ: уровень логирования
    char wifi_ssid[32];
    char wifi_password[64];
    uint32_t magic;
    uint32_t version;
} settings;

// MAGIC_NUMBER и SETTINGS_VERSION — это страховка от:
// 💩 Мусора в EEPROM при первом запуске
// ⚡ Сбоев питания во время записи
// 📦 Несовместимости при обновлении прошивки
// 🔋 Износа ячеек памяти
const uint32_t MAGIC_NUMBER = 0xDEADBEEF;
const uint32_t SETTINGS_VERSION = 2;  // Увеличили версию!

// Текущие рабочие значения
int8_t START_HOUR = DEFAULT_START_HOUR;
int8_t END_HOUR = DEFAULT_END_HOUR;
float LUX_THRESHOLD = DEFAULT_LUX_THRESHOLD;
uint8_t currentLogLevel = DEFAULT_LOG_LEVEL;  // НОВОЕ


void resetToDefaults() {
  settings.start_hour = DEFAULT_START_HOUR;
  settings.end_hour = DEFAULT_END_HOUR;
  settings.lux_threshold = DEFAULT_LUX_THRESHOLD;
  settings.log_level = DEFAULT_LOG_LEVEL;
  strcpy(settings.wifi_ssid, WIFI_SSID);
  strcpy(settings.wifi_password, WIFI_PASSWORD);
  settings.magic = MAGIC_NUMBER;
  settings.version = SETTINGS_VERSION;
  
  EEPROM.put(0, settings);
  EEPROM.commit();
  
  // Обновляем рабочие переменные
  START_HOUR = DEFAULT_START_HOUR;
  END_HOUR = DEFAULT_END_HOUR;
  LUX_THRESHOLD = DEFAULT_LUX_THRESHOLD;
  currentLogLevel = DEFAULT_LOG_LEVEL;
}

void saveSettings() {
  settings.start_hour = START_HOUR;
  settings.end_hour = END_HOUR;
  settings.lux_threshold = LUX_THRESHOLD;
  settings.log_level = currentLogLevel;  // Сохраняем уровень логирования
  settings.magic = MAGIC_NUMBER;
  settings.version = SETTINGS_VERSION;
  
  EEPROM.put(0, settings);
  EEPROM.commit();
  LOG_I("SETTINGS", "Settings saved to EEPROM");
}

void loadSettings() {
    EEPROM.begin(sizeof(Settings));
    EEPROM.get(0, settings);
    
    // Проверка MAGIC_NUMBER - защита от мусора в EEPROM [citation:5]
    if (settings.magic == MAGIC_NUMBER) {
        // Проверка версии - миграция при необходимости
        if (settings.version == SETTINGS_VERSION) {
            // Все ок - используем сохраненные значения
            START_HOUR = settings.start_hour;
            END_HOUR = settings.end_hour;
            LUX_THRESHOLD = settings.lux_threshold;
            currentLogLevel = settings.log_level;
            LOG_I("SETTINGS", "Settings loaded from EEPROM");
        } else {
            // Версия не совпадает - миграция
            LOG_I("SETTINGS", "Migrating settings from v%d to v%d", 
                  settings.version, SETTINGS_VERSION);
            
            // Сохраняем что можем
            START_HOUR = settings.start_hour;
            END_HOUR = settings.end_hour;
            LUX_THRESHOLD = settings.lux_threshold;
            currentLogLevel = DEFAULT_LOG_LEVEL;  // Новое поле - берем default
            
            saveSettings();  // Пересохраняем с новой версией
        }
    } else {
        // MAGIC_NUMBER не совпал - первый запуск или битые данные
        LOG_I("SETTINGS", "First boot or corrupted EEPROM, using defaults");
        resetToDefaults();
    }
}


void saveWiFiCredentials(const String& ssid, const String& password) {
    strlcpy(settings.wifi_ssid, ssid.c_str(), sizeof(settings.wifi_ssid));
    strlcpy(settings.wifi_password, password.c_str(), sizeof(settings.wifi_password));
    settings.magic = MAGIC_NUMBER;
    settings.version = SETTINGS_VERSION;
    
    EEPROM.put(0, settings);
    EEPROM.commit();
    LOG_I("SETTINGS", "WiFi credentials saved to EEPROM");
}

#endif