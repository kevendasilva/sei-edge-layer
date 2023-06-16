#include <PubSubClient.h>

#include "Barrier.h"
#include "WiFiSetup.h"

// Constants for ESP8266
#if defined(ESP8266)
  #define FAIL_LED_PIN D8
  #define SUCCESS_LED_PIN D7

  #define NODE_ID 1

  // Presence Sensor
  #define TRIG_PIN D1
  #define ECHO_PIN D0

  #define ACTUATOR_PIN D2
#endif

// Constans for ESP32
#if defined(ESP32)
  #define FAIL_LED_PIN 41
  #define SUCCESS_LED_PIN 42

  #define NODE_ID 2

  // Presence Sensor
  #define TRIG_PIN 19
  #define ECHO_PIN 20

  #define ACTUATOR_PIN 45
#endif

// WiFi
const char *ssidWiFi = "ParkingLAN";
const char *passwordWiFi = "12345678";

// MQTT Broker
const char *brokerAddress = "10.0.0.100";
int brokerPort = 1883;
const char *brokerUsername = "user";
const char *brokerPassword = "12345678";

// Clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Topics
// Defining the topic to handle the barrier state
String topicBarrierState = "barrier/" + String(NODE_ID) + "/state";

// Virtual Camera
String topicCameraImage = "camera/" + String(NODE_ID);
#define LIMIT_TIME 12000
bool receivedResponse = false;

// Barrier
Barrier barrier(FAIL_LED_PIN, SUCCESS_LED_PIN, TRIG_PIN, ECHO_PIN, ACTUATOR_PIN);

void subscribeTopics();
// Callback for new messages
void callback(char *topic, byte *payload, unsigned int length);
void connectToBroker();

void setup() {
  barrier.loadingSignal(3, 300);
  barrier.setDistanceLimit(100, 50);
  barrier.close();

  connectToWiFi(ssidWiFi, passwordWiFi);
  while (!isConnectedToWiFi()) {
    barrier.loadingSignal(3, 300);
  }

  connectToBroker();
  subscribeTopics();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi(ssidWiFi, passwordWiFi);
    while (!isConnectedToWiFi()) {
      barrier.loadingSignal(3, 300);
    }
    return;
  }

  if (!client.connected()) {
    ESP.restart();
  }

  if (barrier.vehicleIsNear()) {
    client.publish(topicCameraImage.c_str(), "image");

    waitForResponse();
    receivedResponse = false;
  }

  client.loop();
}

void waitForResponse() {
  long startTime = millis();

  while (millis() - startTime < LIMIT_TIME && !receivedResponse) {
    barrier.waitingSignal(3, 100);

    client.loop();
  }
}

void subscribeTopics() {
  // Subscribe topic 'barrier/NODE_ID/state'
  client.subscribe(topicBarrierState.c_str());
}

void barrierStateCallback(String message) {
  if (message == "open") {
    barrier.open();
    while (barrier.vehicleIsNear()) {
      delay(500);
    }
    delay(6000);
    barrier.close();
  }

  if (message == "error") {
    barrier.errorSignal(3, 100);
  }

  receivedResponse = true;
}

void callback(char *topic, byte *payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++)
    message += (char)payload[i];

  // Open barrier
  if (String(topic) == topicBarrierState)
    barrierStateCallback(message);
}

void connectToBroker() {
  client.setServer(brokerAddress, brokerPort);
  client.setCallback(callback);

  const char *clientID = NODE_ID == 1 ? "ParkingEntry":"ParkingExit";

  while (!client.connected()) {
    client.connect(clientID, brokerUsername, brokerPassword);

    barrier.loadingSignal(3, 300);
  }
}
