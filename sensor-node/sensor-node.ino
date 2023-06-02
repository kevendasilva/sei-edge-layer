#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <ArduinoJson.h>
#include "FS.h"
#include <PubSubClient.h>

// Constants for ESP8266
#if defined(ESP8266)
  #define BOARD_NAME "ESP8266"

  #define FAIL_LED_PIN D0
  #define SUCCESS_LED_PIN D1
#endif

#define CONFIG_FILE "/config.json"
#define JSON_BUFFER_SIZE 1024

// WiFi
const char *ssidWiFi;
const char *passwordWiFi;

// MQTT Broker
const char *mqttBroker;
int mqttPort;
const char *mqttUsername;
const char *mqttPassword;

// Node ID
short nodeID;

// Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Topic
String topicBarrierState;

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
    Serial.println("Failed to mount file system.");
    return;
  }

  loadingSignal();

  if (!loadConfig()) {
    Serial.println("Failed to load config.");
  } else {
    Serial.println("Config loaded.");
  }

  connectToWiFi();
  connectToBroker();

  subscribeTopics();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
    return;
  }

  if (!client.connected()) {
    ESP.restart();
  }

  turnOnLED(FAIL_LED_PIN);

  client.loop();
}

void openBarrier() {
  turnOffLED(FAIL_LED_PIN);
  turnOnLED(SUCCESS_LED_PIN);
  delay(3000);
  turnOffLED(SUCCESS_LED_PIN);
}

void turnOnLED(unsigned short pin) { digitalWrite(pin, HIGH); }

void turnOffLED(unsigned short pin) { digitalWrite(pin, LOW); }

void loadingSignal() {
  int blinkCount = 3;
  short delayInMilliseconds = 300;

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
  File configFile = SPIFFS.open(CONFIG_FILE, "r");
  
  if (!configFile) {
    return false;
  }

  size_t size = configFile.size();

  if (size > JSON_BUFFER_SIZE) {
    Serial.println("Config file size is too large.");
    return false;
  }

  StaticJsonDocument<JSON_BUFFER_SIZE> jsonDocument;
  
  DeserializationError error = deserializeJson(jsonDocument, configFile);
  
  if (error) {
    Serial.print("Failed to parse config file: ");
    Serial.println(error.c_str());
    return false;
  }

  ssidWiFi = jsonDocument["wifi"]["ssid"];
  passwordWiFi = jsonDocument["wifi"]["password"];

  mqttBroker = jsonDocument["broker"]["address"];
  mqttPort = jsonDocument["broker"]["port"];
  mqttUsername = jsonDocument["broker"]["username"];
  mqttPassword = jsonDocument["broker"]["password"];

  nodeID = jsonDocument["node_id"];

  configFile.close();

  return true;
}

void subscribeTopics() {
  // Defining the topic to handle the barrier state
  topicBarrierState = "barrier/" + String(nodeID) + "/state";

  // Subscribe topic 'barrier/nodeID/state'
  client.subscribe(topicBarrierState.c_str());
}

// Callback for new messages
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");

  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print(message);

  // Open barrier
  if (String(topic) == topicBarrierState) {
    if (message == "open") {
      openBarrier();
    }
  }

  Serial.println();
}

void connectToBroker() {  
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {    
    String clientId = String(BOARD_NAME) + "-#" + String(nodeID);

    Serial.printf("The client %s connects to Mosquitto MQTT broker.\n", clientId.c_str());

    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
      Serial.println("Connected to the Mosquitto MQTT broker.");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      Serial.println();

      loadingSignal();
    }
  }
}

void connectToWiFi() {
  WiFi.begin(ssidWiFi, passwordWiFi);

  while (WiFi.status() != WL_CONNECTED) {
    loadingSignal();
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to the WiFi network.");
}
