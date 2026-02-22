#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Пины
#define PIN_BACKLIGHT D7
#define PIN_LED_BUILTIN LED_BUILTIN
#define PIN_SDA D2
#define PIN_SCL D1

// Временные интервалы
#define FAST_TIMER_DELAY 1000
#define SLOW_TIMER_DELAY 600000
#define SEND_LUX_DELAY 60000
#define MQTT_RECONNECT_DELAY 5000

// MQTT топики
#define MQTT_ENABLED 1
#define MQTT_TOPIC_LIGHT "home/lights"
#define MQTT_TOPIC_SET "home/lights/set"

// Значения по умолчанию
#define DEFAULT_START_HOUR 7
#define DEFAULT_END_HOUR 19
#define DEFAULT_LUX_THRESHOLD 2000.0f
#define DEFAULT_LOG_LEVEL 2  // INFO по умолчанию

// Версия прошивки
#define FIRMWARE_VERSION "2.2.0"

// Уровни логирования
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

// Макросы логирования (используют текущий уровень из EEPROM)
#define LOG_E(tag, ...) if (currentLogLevel >= LOG_LEVEL_ERROR) { Serial.printf("[ERROR] [" tag "] " __VA_ARGS__); Serial.println(); }
#define LOG_I(tag, ...) if (currentLogLevel >= LOG_LEVEL_INFO) { Serial.printf("[INFO] [" tag "] " __VA_ARGS__); Serial.println(); }
#define LOG_D(tag, ...) if (currentLogLevel >= LOG_LEVEL_DEBUG) { Serial.printf("[DEBUG] [" tag "] " __VA_ARGS__); Serial.println(); }

// Глобальные переменные
extern int8_t START_HOUR;
extern int8_t END_HOUR;
extern float LUX_THRESHOLD;
extern float currentLux;
extern bool isBacklightOn;
extern uint32_t systemUptime;
extern uint8_t currentLogLevel;  // Текущий уровень логирования

#endif