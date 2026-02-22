#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>
#include "config.h"
#include "secrets.h"

void initOTA() {
    // Настройка OTA
    ArduinoOTA.setHostname("esp-light");
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    // Обработчики событий OTA
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }
        LOG_I("OTA", "🔥 Start updating %s", type.c_str());
    });
    
    ArduinoOTA.onEnd([]() {
        LOG_I("OTA", "\n✅ Update complete!");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t lastPercent = 0;
        uint8_t percent = (progress / (total / 100));
        if (percent != lastPercent && percent % 10 == 0) {
            LOG_I("OTA", "Progress: %u%%", percent);
            lastPercent = percent;
        }
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        LOG_E("OTA", "❌ Error [%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            LOG_E("OTA", "Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            LOG_E("OTA", "Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            LOG_E("OTA", "Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            LOG_E("OTA", "Receive Failed");
        } else if (error == OTA_END_ERROR) {
            LOG_E("OTA", "End Failed");
        }
    });
    
    ArduinoOTA.begin();
    LOG_I("OTA", "✅ OTA ready");
    LOG_I("OTA", "   Hostname: esp-light");
    LOG_I("OTA", "   Password: %s", OTA_PASSWORD);
}

void handleOTA() {
    ArduinoOTA.handle();
}

#endif