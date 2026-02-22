#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "config.h"
#include "secrets.h"

extern Settings settings;

void connectWiFi() {
  LOG_D("WiFi","📡 Connecting to WiFi");
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-Light");
  WiFi.begin(settings.wifi_ssid, settings.wifi_password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // 20 секунд максимум
    delay(500);
    LOG_D("WiFi",".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    LOG_D("WiFi","\n✅ WiFi connected!");
    LOG_D("WiFi","   IP address: %s\n", WiFi.localIP().toString().c_str());
    LOG_D("WiFi","   RSSI: %d dBm\n", WiFi.RSSI());
    LOG_D("WiFi","   Channel: %d\n", WiFi.channel());
    
    // Запускаем mDNS
    if (MDNS.begin("esp-light")) {
     LOG_D("WiFi","   mDNS: http://esp-light.local");
    }
  } else {
   LOG_D("WiFi","\n❌ WiFi connection failed!");
   LOG_D("WiFi","   Starting in AP mode for configuration...");
    
    // Режим точки доступа для настройки
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP-Light-Config", "12345678");
    LOG_D("WiFi","   AP IP: %s\n", WiFi.softAPIP().toString().c_str());
  }
}

void handleWiFi() {
  static uint32_t lastCheck = 0;
  
  // Проверяем соединение каждые 30 секунд
  if (millis() - lastCheck > 30000) {
    if (WiFi.status() != WL_CONNECTED) {
     LOG_D("WiFi","📡 WiFi disconnected, reconnecting...");
      WiFi.reconnect();
    }
    lastCheck = millis();
  }
}


#endif