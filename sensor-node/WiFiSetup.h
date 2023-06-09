#ifndef WIFI_SETUP_H_INCLUDED
#define WIFI_SETUP_H_INCLUDED
  #if defined(ESP32)
    #include <WiFi.h>
  #endif

  #if defined(ESP8266)
    #include <ESP8266WiFi.h>
  #endif

  void connectToWiFi(const char *ssid, const char *password);
  bool isConnectedToWiFi();
#endif
