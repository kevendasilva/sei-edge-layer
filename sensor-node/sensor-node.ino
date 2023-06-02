#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#if defined(ESP32)
  #include <WiFi.h>
#endif

#include <PubSubClient.h>

// Constants for ESP8266
#if defined(ESP8266)
  #define BOARD_NAME "ESP8266"

  #define FAIL_LED_PIN D1
  #define SUCCESS_LED_PIN D0

  #define NODE_ID 1
#endif

// Constans for ESP32
#if defined(ESP32)
  #define BOARD_NAME "ESP32"
  
  #define FAIL_LED_PIN 42
  #define SUCCESS_LED_PIN 41

  #define NODE_ID 2
#endif

// WiFi
const char *ssidWiFi = "ParkingLAN";
const char *passwordWiFi = "12345678";

// MQTT Broker
const char *mqttBroker = "10.0.0.100";
int mqttPort = 1883;
const char *mqttUsername = "user";
const char *mqttPassword = "12345678";

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

void subscribeTopics() {
  // Defining the topic to handle the barrier state
  topicBarrierState = "barrier/" + String(NODE_ID) + "/state";

  // Subscribe topic 'barrier/NODE_ID/state'
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
    String clientId = String(BOARD_NAME) + "-#" + String(NODE_ID);

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
