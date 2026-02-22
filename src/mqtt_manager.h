#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include "config.h"
#include "secrets.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Внешние переменные
extern float currentLux;
extern bool isBacklightOn;

void initMQTT()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

void connectMQTT()
{
  static uint32_t lastTry = 0;

  if (mqttClient.connected())
    return;
  if (millis() - lastTry < MQTT_RECONNECT_DELAY)
    return;

  lastTry = millis();

  String clientId = "ESP-Light-";
  clientId += String(random(0xffff), HEX);

  LOG_D("MQTT", "📡 Connecting to MQTT... ");

  bool connected;
  if (strlen(MQTT_USER) > 0)
  {
    connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  }
  else
  {
    connected = mqttClient.connect(clientId.c_str());
  }

  if (connected)
  {
    LOG_D("MQTT", "✅ Connected!");
    mqttClient.subscribe(MQTT_TOPIC_SET);
  }
  else
  {
    LOG_D("MQTT", "❌ Failed (state: %d)\n", mqttClient.state());
  }
}

void publishLightData()
{
  if (!mqttClient.connected())
    return;

  String payload = "{\"lux\":" + String((int)currentLux) + ",\"enabled\":" + String(isBacklightOn ? "true" : "false") + "}";

  if (mqttClient.publish(MQTT_TOPIC_LIGHT, payload.c_str()))
  {
    LOG_D("MQTT", "📤 MQTT data sent");
  }
}

void handleMQTT()
{
  if (!mqttClient.connected())
  {
    connectMQTT();
  }
  mqttClient.loop();
}

#endif