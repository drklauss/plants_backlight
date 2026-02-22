/**
 * ESP8266 Plant Backlight Controller
 * Управление подсветкой растений с веб-интерфейсом и MQTT
 */

 #include "config.h"
 #include "secrets.h"
 #include "settings.h"
 #include "ota_manager.h"
 #include "wifi_manager.h"
 #include "mqtt_manager.h"
 #include "light_control.h"
 #include "web_server.h"
 
 void setup() {
   Serial.begin(115200);
   delay(100);
   LOG_D("ESP","\n\n=== ESP Plant Backlight Starting ===");
   
   // Инициализация hardware
   initHardware();
   
   // Загрузка настроек из EEPROM
   loadSettings();
   
   // Подключение к WiFi
   connectWiFi();

  // Инициализация OTA (после WiFi)
  initOTA();
   
   // Запуск веб-сервера
   initWebServer();
   
   // Подключение к MQTT (опционально)
   initMQTT();
   
   LOG_D("ESP","Setup complete")
 }
 
 void loop() {
    // Обработка OTA (должна быть первой в loop)
    handleOTA();
    
   // Поддержка WiFi соединения
   handleWiFi();
   
   // Обработка веб-запросов
   handleWebServer();
   
   // Обработка MQTT
   handleMQTT();
   
   // Основная логика подсветки
   runLightLogic();
   
   // Небольшая задержка для стабильности
   delay(10);
 }