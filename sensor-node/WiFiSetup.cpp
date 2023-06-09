#include <Arduino.h>
#include "WiFiSetup.h"

void connectToWiFi(const char *ssid, const char *password) {  
  WiFi.begin(ssid, password);
}

bool isConnectedToWiFi() {
  return WiFi.status() == WL_CONNECTED;
}
