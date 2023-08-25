#include <Arduino.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BH1750.h>
#include <Wire.h>

const char *ssid = "my-fi";
const char *password = "my-fipass";
const uint8_t B_LIGHT = D7;
const int8_t START_H = 7;
const int8_t END_H = 19;

const int8_t EN_DISP = 7;   // Display enabed time
const int8_t DIS_DISP = 22; // Display disabled time
const float LUX_THRESHOLD = 400;

uint32_t fastTimer = 0;
uint32_t fastTimerDelay = 1000;
uint32_t slowTimerDelay = 10 * 60 * 1000; // 10 minutes
uint32_t slowTimer = slowTimerDelay;
uint32_t sendStatDelay = 15 * 1000; // 15
uint32_t volatile elapsed = 0;      // backlight on counter in seconds
float lux_val = 0;
bool isBackLightON = false;

const char *mqtt_server = "192.168.10.100";
const char *TOPIC = "domoticz/in";
const uint8_t TRIES = 3;
int stat_c_idx = 1;
int stat_lux_idx = 2;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60 * 60 * 3, 60 * 60 * 24);
BH1750 lightMeter(0x23);

void wifiConnect();
void initOTA();
void readLuxSendStat();
void runFlow();
void mqttconnect();

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(B_LIGHT, OUTPUT);
  digitalWrite(B_LIGHT, false);

  Wire.begin(D2, D1);
   if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  wifiConnect();
  initOTA();

  timeClient.begin();
  mqttClient.setServer(mqtt_server, 1883);
  readLuxSendStat();
}

void loop()
{
  wifiConnect();
  ArduinoOTA.handle();
  timeClient.update();

  if (millis() - fastTimer <= fastTimerDelay)
  {
    return;
  }

  if (!mqttClient.connected())
  {
    mqttconnect();
  }
  mqttClient.loop();

  runFlow();
  fastTimer = millis();
}

// Main function
void runFlow()
{
  if (isBackLightON)
  {
    elapsed += fastTimerDelay / 1000;
  }

  int8_t cHour = timeClient.getHours();
  // Showing backlight time
  if (cHour >= EN_DISP && cHour <= DIS_DISP)
  {
    readLuxSendStat();
  }

  // Resetting counter
  if (cHour == DIS_DISP)
  {
    elapsed = 0;
  }

  // Disabling backlight at night
  if (cHour < START_H || cHour > END_H)
  {
    digitalWrite(B_LIGHT, false);
    isBackLightON = false;
    return;
  }

  // Skipping often checks for backlight
  if (millis() - slowTimer <= slowTimerDelay)
  {
    return;
  }

  if (lux_val <= LUX_THRESHOLD)
  {
    digitalWrite(B_LIGHT, true);
    isBackLightON = true;
  }
  else
  {
    digitalWrite(B_LIGHT, false);
    isBackLightON = false;
  }

  slowTimer = millis();
}

// OTA Initialization
void initOTA()
{
  ArduinoOTA.setPassword("100500");
  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();
}

// Wi-Fi
void wifiConnect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  WiFi.hostname("ESP_Plant_Backlight");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connection status: %d\n", WiFi.status());
  while (WiFi.status() != WL_CONNECTED)
  {
    uint32_t uptime = millis();
    // restart ESP if could not connect during 5 minutes
    if (WiFi.status() != WL_CONNECTED && (uptime > 1000 * 60 * 5))
    {
      ESP.restart();
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
  }
  Serial.printf("\nConnection status: %d\n", WiFi.status());
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  Serial.printf("Hostname: %s\n", WiFi.hostname().c_str());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect to MQTT Server
void mqttconnect()
{
  static uint8_t tries;
  while (!mqttClient.connected())
  {
    if (tries >= TRIES)
    {
      tries = 0;
      return;
    }

    if (mqttClient.connect("plants_stat"))
    {
      mqttClient.subscribe(TOPIC);
      tries = 0;
    }

    tries++;
  }
}

void readLuxSensor()
{
}

// Sending stat to Domoticz
void readLuxSendStat()
{
  static uint32_t timer;
  if (millis() - timer <= sendStatDelay)
  {
    return;
  }
  timer = millis();

  if (lightMeter.measurementReady())
  {
    lux_val = lightMeter.readLightLevel();
  }


  if (!isBackLightON)
  {
    return;
  }

  String in_c_str = "{\"idx\":" + String(stat_c_idx) + ",\"svalue\":\"" + sendStatDelay / 1000 + "\"}";
  mqttClient.publish(TOPIC, in_c_str.c_str());

  String in_lux_str = "{\"idx\":" + String(stat_lux_idx) + ",\"svalue\":\"" + String(lux_val, 0) + "\"}";
  mqttClient.publish(TOPIC, in_lux_str.c_str());
}