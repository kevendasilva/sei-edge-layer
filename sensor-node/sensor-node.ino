// Constants for ESP8266
#if defined(ESP8266)
  #define FAIL_LED_PIN D0
  #define SUCCESS_LED_PIN D1
#endif

void setup() {
  // LEDs for signaling
  pinMode(FAIL_LED_PIN, OUTPUT);
  pinMode(SUCCESS_LED_PIN, OUTPUT);
}

void loop() {

}

void turnOnLED(unsigned short pin) { digitalWrite(pin, HIGH); }

void turnOffLED(unsigned short pin) { digitalWrite(pin, LOW); }
