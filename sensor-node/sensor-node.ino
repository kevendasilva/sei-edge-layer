#include <ArduinoJson.h>
#include "FS.h"

// Constants for ESP8266
#if defined(ESP8266)
  #define FAIL_LED_PIN D0
  #define SUCCESS_LED_PIN D1
#endif

// WiFi
const char *ssidWifi;
const char *passwordWifi;

// MQTT Broker
const char *mqttBroker;
int mqttPort;
const char *mqttUsername;
const char *mqttPassword;

// Node ID
short nodeID;

void setup() {
  Serial.begin(115200);
  
  // LEDs for signaling
  pinMode(FAIL_LED_PIN, OUTPUT);
  pinMode(SUCCESS_LED_PIN, OUTPUT);
  
  Serial.println("");
  delay(500);
  Serial.println("Mounting FS...");

  loadingSignal();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  loadingSignal();

  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }
}

void loop() {
}

void turnOnLED(unsigned short pin) { digitalWrite(pin, HIGH); }

void turnOffLED(unsigned short pin) { digitalWrite(pin, LOW); }

void loadingSignal() {
  int blinkCount = 3;
  short delayInMilliseconds = 500;

  while (blinkCount > 0) {
    turnOnLED(SUCCESS_LED_PIN); turnOnLED(FAIL_LED_PIN);
    delay(delayInMilliseconds);
    turnOffLED(SUCCESS_LED_PIN); turnOffLED(FAIL_LED_PIN);
    delay(delayInMilliseconds);    

    blinkCount--;
  }
}

// Function to load configuration data
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  
  if (!configFile) {
    return false;
  }

  size_t size = configFile.size();

  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1024> jsonDocument;
  auto error = deserializeJson(jsonDocument, buf.get());

  if (error) {
    Serial.println("Failed to parse config file");
    Serial.println(error.c_str());
    return false;
  }

  JsonObject json = jsonDocument.as<JsonObject>();

  ssidWifi = json["wifi"]["ssid"].as<const char*>();
  passwordWifi = json["wifi"]["password"].as<const char*>();

  mqttBroker = json["broker"]["address"].as<const char*>();
  mqttPort = json["broker"]["port"].as<int>();
  mqttUsername = json["broker"]["username"].as<const char*>();
  mqttPassword = json["broker"]["password"].as<const char*>();

  nodeID = json["node_id"].as<short>();;

  return true;
}
