#ifndef LIGHT_CONTROL_H
#define LIGHT_CONTROL_H

#include "config.h"
#include <Wire.h>
#include <BH1750.h>
#include <NTPClient.h>

// Глобальные переменные
float currentLux = 0;
bool isBacklightOn = false;
uint32_t systemUptime = 0;

// Таймеры
uint32_t fastTimer = 0;
uint32_t slowTimer = 0;
uint32_t sendTimer = 0;

// NTP клиент
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800, 86400000); // UTC+3

// Датчик освещенности
BH1750 lightMeter(0x23);

void initHardware() {
  pinMode(PIN_BACKLIGHT, OUTPUT);
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, LOW);
  digitalWrite(PIN_LED_BUILTIN, HIGH);
  
  // I2C для BH1750
  Wire.begin(PIN_SDA, PIN_SCL);
  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    LOG_D("INIT","✅ BH1750 initialized")
  } else {
    LOG_D("INIT","❌ BH1750 initialization failed")
  }
  
  // NTP клиент
  timeClient.begin();

  LOG_D("INIT","✅ Hardware initialized")
}

void readLuxSensor() {
  if (lightMeter.measurementReady()) {
    currentLux = lightMeter.readLightLevel();
    static uint32_t lastLog = 0;
    if (millis() - lastLog > 60000) { // Лог раз в минуту
      LOG_I("READ_LUX","📊 Current lux: %.1f\n", currentLux);
      lastLog = millis();
    }
  }
}

void runLightLogic() {
  // Обновление uptime
  static uint32_t lastUptimeUpdate = 0;
  if (millis() - lastUptimeUpdate >= 1000) {
    systemUptime++;
    lastUptimeUpdate = millis();
  }
  
  // Обновление времени
  timeClient.update();
  
  // Чтение датчика (каждую секунду)
  if (millis() - fastTimer >= FAST_TIMER_DELAY) {
    readLuxSensor();
    fastTimer = millis();
  }
  
  // Отправка данных (раз в минуту)
  if (millis() - sendTimer >= SEND_LUX_DELAY) {
    #ifdef MQTT_ENABLED
      publishLightData();   // из mqtt_manager.h
    #endif
    sendTimer = millis();
  }
  
  int currentHour = timeClient.getHours();
  
  // Ночью подсветка всегда выключена
  if (currentHour < START_HOUR || currentHour >= END_HOUR) {
    if (isBacklightOn) {
      digitalWrite(PIN_BACKLIGHT, LOW);
      isBacklightOn = false;
      LOG_D("BACKLIGHT","🌙 Backlight OFF (night time)");
    }
    return;
  }
  
  // Проверка необходимости включения/выключения (раз в 10 минут)
  if (millis() - slowTimer >= SLOW_TIMER_DELAY) {
    bool shouldBeOn = (currentLux < LUX_THRESHOLD);
    
    if (shouldBeOn != isBacklightOn) {
      digitalWrite(PIN_BACKLIGHT, shouldBeOn ? HIGH : LOW);
      isBacklightOn = shouldBeOn;
      LOG_D("BACKLIGHT","💡 Backlight %s (lux: %.1f, threshold: %.0f)\n", 
                    shouldBeOn ? "ON" : "OFF", currentLux, LUX_THRESHOLD);
    }
    
    slowTimer = millis();
  }
}

#endif