#include <Arduino.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <GyverTM1637.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid = "my-fi";
const char *password = "my-fipass";
const uint8_t IS_DARK = D6;
const uint8_t B_LIGHT = D7;
const int8_t START_H = 7;
const int8_t END_H = 19;

const int8_t EN_DISP = 7;   // Display enabed time
const int8_t DIS_DISP = 22; // Display disabled time

uint32_t fastTimer = 0;
uint32_t fastTimerDelay = 1000;
uint32_t slowTimer = 0;
uint32_t slowTimerDelay = 20 * 60 * 1000; // 20 minutes
uint32_t sendStatDelay = 1 * 60 * 1000;   // 1 minute
uint32 volatile elapsed = 0;              // backlight on counter in seconds
bool isBackLightON = false;

const char *mqtt_server = "192.168.10.100";
const char *TOPIC = "domoticz/in";
int stat_idx = 1;

WiFiClient espClient;
GyverTM1637 disp(D1, D2);
PubSubClient mqttClient(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60 * 60 * 3, 60 * 60 * 24);

void wifiConnect();
void initOTA();
void showTime();
void sendStat();
void runFlow();
void mqttconnect();

void setup()
{
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IS_DARK, INPUT);
  pinMode(B_LIGHT, OUTPUT);
  digitalWrite(B_LIGHT, false);

  Serial.begin(115200);
  wifiConnect();
  initOTA();

  disp.clear();
  disp.brightness(0);
  timeClient.begin();
  mqttClient.setServer(mqtt_server, 1883);
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
    showTime();
    sendStat();
  }
  else
  {
    disp.clear();
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

  if (digitalRead(IS_DARK))
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

// Show time on display
void showTime()
{
  disp.displayClock(elapsed / 3600, elapsed % 3600 / 60);
}

// Connect to MQTT Server
void mqttconnect()
{
  while (!mqttClient.connected())
  {
    if (mqttClient.connect("plants_stat"))
    {
      mqttClient.subscribe(TOPIC);
    }
  }
}

// Sending stat to Domoticz
void sendStat()
{
  static uint32_t timer;
  if (millis() - timer <= sendStatDelay)
  {
    return;
  }
  timer = millis();
  if (!isBackLightON){
    return;
  }

  String in_str = "{\"idx\":" + String(stat_idx) + ",\"svalue\":\"60\"}";
  if (!mqttClient.publish(TOPIC, in_str.c_str()))
  {
    mqttconnect();
  }
}